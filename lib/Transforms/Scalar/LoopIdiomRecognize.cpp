//===-- LoopIdiomRecognize.cpp - Loop idiom recognition -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass implements an idiom recognizer that transforms simple loops into a
// non-loop form.  In cases that this kicks in, it can be a significant
// performance win.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "loop-idiom"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

// TODO: Recognize "N" size array multiplies: replace with call to blas or
// something.

namespace {
  class LoopIdiomRecognize : public LoopPass {
    Loop *CurLoop;
    const TargetData *TD;
    ScalarEvolution *SE;
  public:
    static char ID;
    explicit LoopIdiomRecognize() : LoopPass(ID) {
      initializeLoopIdiomRecognizePass(*PassRegistry::getPassRegistry());
    }

    bool runOnLoop(Loop *L, LPPassManager &LPM);

    bool processLoopStore(StoreInst *SI, const SCEV *BECount);
    
    bool processLoopStoreOfSplatValue(StoreInst *SI, unsigned StoreSize,
                                      Value *SplatValue,
                                      const SCEVAddRecExpr *Ev,
                                      const SCEV *BECount);
    
    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG.
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addPreserved<LoopInfo>();
      AU.addRequiredID(LoopSimplifyID);
      AU.addPreservedID(LoopSimplifyID);
      AU.addRequiredID(LCSSAID);
      AU.addPreservedID(LCSSAID);
      AU.addRequired<AliasAnalysis>();
      AU.addPreserved<AliasAnalysis>();
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();
      AU.addPreserved<DominatorTree>();
    }
  };
}

char LoopIdiomRecognize::ID = 0;
INITIALIZE_PASS_BEGIN(LoopIdiomRecognize, "loop-idiom", "Recognize loop idioms",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfo)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(LCSSA)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_END(LoopIdiomRecognize, "loop-idiom", "Recognize loop idioms",
                    false, false)

Pass *llvm::createLoopIdiomPass() { return new LoopIdiomRecognize(); }

/// DeleteDeadInstruction - Delete this instruction.  Before we do, go through
/// and zero out all the operands of this instruction.  If any of them become
/// dead, delete them and the computation tree that feeds them.
///
static void DeleteDeadInstruction(Instruction *I, ScalarEvolution &SE) {
  SmallVector<Instruction*, 32> NowDeadInsts;
  
  NowDeadInsts.push_back(I);
  
  // Before we touch this instruction, remove it from SE!
  do {
    Instruction *DeadInst = NowDeadInsts.pop_back_val();
    
    // This instruction is dead, zap it, in stages.  Start by removing it from
    // SCEV.
    SE.forgetValue(DeadInst);
    
    for (unsigned op = 0, e = DeadInst->getNumOperands(); op != e; ++op) {
      Value *Op = DeadInst->getOperand(op);
      DeadInst->setOperand(op, 0);
      
      // If this operand just became dead, add it to the NowDeadInsts list.
      if (!Op->use_empty()) continue;
      
      if (Instruction *OpI = dyn_cast<Instruction>(Op))
        if (isInstructionTriviallyDead(OpI))
          NowDeadInsts.push_back(OpI);
    }
    
    DeadInst->eraseFromParent();
    
  } while (!NowDeadInsts.empty());
}

bool LoopIdiomRecognize::runOnLoop(Loop *L, LPPassManager &LPM) {
  CurLoop = L;
  
  // We only look at trivial single basic block loops.
  // TODO: eventually support more complex loops, scanning the header.
  if (L->getBlocks().size() != 1)
    return false;
  
  // The trip count of the loop must be analyzable.
  SE = &getAnalysis<ScalarEvolution>();
  if (!SE->hasLoopInvariantBackedgeTakenCount(L))
    return false;
  const SCEV *BECount = SE->getBackedgeTakenCount(L);
  if (isa<SCEVCouldNotCompute>(BECount)) return false;
  
  // We require target data for now.
  TD = getAnalysisIfAvailable<TargetData>();
  if (TD == 0) return false;
  
  BasicBlock *BB = L->getHeader();
  DEBUG(dbgs() << "loop-idiom Scanning: F[" << BB->getParent()->getName()
               << "] Loop %" << BB->getName() << "\n");

  bool MadeChange = false;
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ) {
    // Look for store instructions, which may be memsets.
    StoreInst *SI = dyn_cast<StoreInst>(I++);
    if (SI == 0 || SI->isVolatile()) continue;
    
    WeakVH InstPtr(SI);
    if (!processLoopStore(SI, BECount)) continue;
    
    MadeChange = true;
    
    // If processing the store invalidated our iterator, start over from the
    // head of the loop.
    if (InstPtr == 0)
      I = BB->begin();
  }
  
  return MadeChange;
}

/// scanBlock - Look over a block to see if we can promote anything out of it.
bool LoopIdiomRecognize::processLoopStore(StoreInst *SI, const SCEV *BECount) {
  Value *StoredVal = SI->getValueOperand();
  Value *StorePtr = SI->getPointerOperand();
  
  // Reject stores that are so large that they overflow an unsigned.
  uint64_t SizeInBits = TD->getTypeSizeInBits(StoredVal->getType());
  if ((SizeInBits & 7) || (SizeInBits >> 32) != 0)
    return false;
  
  // See if the pointer expression is an AddRec like {base,+,1} on the current
  // loop, which indicates a strided store.  If we have something else, it's a
  // random store we can't handle.
  const SCEVAddRecExpr *Ev = dyn_cast<SCEVAddRecExpr>(SE->getSCEV(StorePtr));
  if (Ev == 0 || Ev->getLoop() != CurLoop || !Ev->isAffine())
    return false;

  // Check to see if the stride matches the size of the store.  If so, then we
  // know that every byte is touched in the loop.
  unsigned StoreSize = (unsigned)SizeInBits >> 3; 
  const SCEVConstant *Stride = dyn_cast<SCEVConstant>(Ev->getOperand(1));
  
  // TODO: Could also handle negative stride here someday, that will require the
  // validity check in mayLoopModRefLocation to be updated though.
  if (Stride == 0 || StoreSize != Stride->getValue()->getValue())
    return false;
  
  // If the stored value is a byte-wise value (like i32 -1), then it may be
  // turned into a memset of i8 -1, assuming that all the consequtive bytes
  // are stored.  A store of i32 0x01020304 can never be turned into a memset.
  if (Value *SplatValue = isBytewiseValue(StoredVal))
    return processLoopStoreOfSplatValue(SI, StoreSize, SplatValue, Ev, BECount);

  // Handle the memcpy case here.
 // errs() << "Found strided store: " << *Ev << "\n";

  return false;
}

/// mayLoopModRefLocation - Return true if the specified loop might do a load or
/// store to the same location that the specified store could store to, which is
/// a loop-strided access. 
static bool mayLoopModRefLocation(StoreInst *SI, Loop *L, const SCEV *BECount,
                                  unsigned StoreSize, AliasAnalysis &AA) {
  // Get the location that may be stored across the loop.  Since the access is
  // strided positively through memory, we say that the modified location starts
  // at the pointer and has infinite size.
  uint64_t AccessSize = AliasAnalysis::UnknownSize;

  // If the loop iterates a fixed number of times, we can refine the access size
  // to be exactly the size of the memset, which is (BECount+1)*StoreSize
  if (const SCEVConstant *BECst = dyn_cast<SCEVConstant>(BECount))
    AccessSize = (BECst->getValue()->getZExtValue()+1)*StoreSize;
  
  // TODO: For this to be really effective, we have to dive into the pointer
  // operand in the store.  Store to &A[i] of 100 will always return may alias
  // with store of &A[100], we need to StoreLoc to be "A" with size of 100,
  // which will then no-alias a store to &A[100].
  AliasAnalysis::Location StoreLoc(SI->getPointerOperand(), AccessSize);

  for (Loop::block_iterator BI = L->block_begin(), E = L->block_end(); BI != E;
       ++BI)
    for (BasicBlock::iterator I = (*BI)->begin(), E = (*BI)->end(); I != E; ++I)
      if (AA.getModRefInfo(I, StoreLoc) != AliasAnalysis::NoModRef)
        return true;

  return false;
}

/// processLoopStoreOfSplatValue - We see a strided store of a memsetable value.
/// If we can transform this into a memset in the loop preheader, do so.
bool LoopIdiomRecognize::
processLoopStoreOfSplatValue(StoreInst *SI, unsigned StoreSize,
                             Value *SplatValue,
                             const SCEVAddRecExpr *Ev, const SCEV *BECount) {
  // Temporarily remove the store from the loop, to avoid the mod/ref query from
  // seeing it.
  Instruction *InstAfterStore = ++BasicBlock::iterator(SI);
  SI->removeFromParent();
  
  // Okay, we have a strided store "p[i]" of a splattable value.  We can turn
  // this into a memset in the loop preheader now if we want.  However, this
  // would be unsafe to do if there is anything else in the loop that may read
  // or write to the aliased location.  Check for an alias.
  bool Unsafe = mayLoopModRefLocation(SI, CurLoop, BECount, StoreSize,
                                      getAnalysis<AliasAnalysis>());

  SI->insertBefore(InstAfterStore);
  
  if (Unsafe) return false;
  
  // Okay, everything looks good, insert the memset.
  BasicBlock *Preheader = CurLoop->getLoopPreheader();
  
  IRBuilder<> Builder(Preheader->getTerminator());
  
  // The trip count of the loop and the base pointer of the addrec SCEV is
  // guaranteed to be loop invariant, which means that it should dominate the
  // header.  Just insert code for it in the preheader.
  SCEVExpander Expander(*SE);
  
  unsigned AddrSpace = SI->getPointerAddressSpace();
  Value *BasePtr = 
    Expander.expandCodeFor(Ev->getStart(), Builder.getInt8PtrTy(AddrSpace),
                           Preheader->getTerminator());
  
  // The # stored bytes is (BECount+1)*Size.  Expand the trip count out to
  // pointer size if it isn't already.
  const Type *IntPtr = TD->getIntPtrType(SI->getContext());
  unsigned BESize = SE->getTypeSizeInBits(BECount->getType());
  if (BESize < TD->getPointerSizeInBits())
    BECount = SE->getZeroExtendExpr(BECount, IntPtr);
  else if (BESize > TD->getPointerSizeInBits())
    BECount = SE->getTruncateExpr(BECount, IntPtr);
  
  const SCEV *NumBytesS = SE->getAddExpr(BECount, SE->getConstant(IntPtr, 1),
                                         true, true /*nooverflow*/);
  if (StoreSize != 1)
    NumBytesS = SE->getMulExpr(NumBytesS, SE->getConstant(IntPtr, StoreSize),
                               true, true /*nooverflow*/);
  
  Value *NumBytes = 
    Expander.expandCodeFor(NumBytesS, IntPtr, Preheader->getTerminator());
  
  Value *NewCall =
    Builder.CreateMemSet(BasePtr, SplatValue, NumBytes, SI->getAlignment());
  
  DEBUG(dbgs() << "  Formed memset: " << *NewCall << "\n"
               << "    from store to: " << *Ev << " at: " << *SI << "\n");
  (void)NewCall;
  
  // Okay, the memset has been formed.  Zap the original store and anything that
  // feeds into it.
  DeleteDeadInstruction(SI, *SE);
  return true;
}

