//===- LoopSimplify.cpp - Loop Canonicalization Pass ----------------------===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This pass performs several transformations to transform natural loops into a
// simpler form, which makes subsequent analyses and transformations simpler and
// more effective.
//
// Loop pre-header insertion guarantees that there is a single, non-critical
// entry edge from outside of the loop to the loop header.  This simplifies a
// number of analyses and transformations, such as LICM.
//
// Loop exit-block insertion guarantees that all exit blocks from the loop
// (blocks which are outside of the loop that have predecessors inside of the
// loop) only have predecessors from inside of the loop (and are thus dominated
// by the loop header).  This simplifies transformations such as store-sinking
// that are built into LICM.
//
// This pass also guarantees that loops will have exactly one backedge.
//
// Note that the simplifycfg pass will clean up blocks which are split out but
// end up being unnecessary, so usage of this pass should not pessimize
// generated code.
//
// This pass obviously modifies the CFG, but updates loop information and
// dominator information.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar.h"
#include "llvm/Function.h"
#include "llvm/iTerminators.h"
#include "llvm/iPHINode.h"
#include "llvm/Constant.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CFG.h"
#include "Support/SetOperations.h"
#include "Support/Statistic.h"
#include "Support/DepthFirstIterator.h"
using namespace llvm;

namespace {
  Statistic<>
  NumInserted("loopsimplify", "Number of pre-header or exit blocks inserted");

  struct LoopSimplify : public FunctionPass {
    virtual bool runOnFunction(Function &F);
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // We need loop information to identify the loops...
      AU.addRequired<LoopInfo>();
      AU.addRequired<DominatorSet>();

      AU.addPreserved<LoopInfo>();
      AU.addPreserved<DominatorSet>();
      AU.addPreserved<ImmediateDominators>();
      AU.addPreserved<DominatorTree>();
      AU.addPreserved<DominanceFrontier>();
      AU.addPreservedID(BreakCriticalEdgesID);  // No crit edges added....
    }
  private:
    bool ProcessLoop(Loop *L);
    BasicBlock *SplitBlockPredecessors(BasicBlock *BB, const char *Suffix,
                                       const std::vector<BasicBlock*> &Preds);
    void RewriteLoopExitBlock(Loop *L, BasicBlock *Exit);
    void InsertPreheaderForLoop(Loop *L);
    void InsertUniqueBackedgeBlock(Loop *L);

    void UpdateDomInfoForRevectoredPreds(BasicBlock *NewBB,
                                         std::vector<BasicBlock*> &PredBlocks);
  };

  RegisterOpt<LoopSimplify>
  X("loopsimplify", "Canonicalize natural loops", true);
}

// Publically exposed interface to pass...
const PassInfo *llvm::LoopSimplifyID = X.getPassInfo();
Pass *llvm::createLoopSimplifyPass() { return new LoopSimplify(); }

/// runOnFunction - Run down all loops in the CFG (recursively, but we could do
/// it in any convenient order) inserting preheaders...
///
bool LoopSimplify::runOnFunction(Function &F) {
  bool Changed = false;
  LoopInfo &LI = getAnalysis<LoopInfo>();

  for (LoopInfo::iterator I = LI.begin(), E = LI.end(); I != E; ++I)
    Changed |= ProcessLoop(*I);

  return Changed;
}


/// ProcessLoop - Walk the loop structure in depth first order, ensuring that
/// all loops have preheaders.
///
bool LoopSimplify::ProcessLoop(Loop *L) {
  bool Changed = false;

  // Does the loop already have a preheader?  If so, don't modify the loop...
  if (L->getLoopPreheader() == 0) {
    InsertPreheaderForLoop(L);
    NumInserted++;
    Changed = true;
  }

  // Next, check to make sure that all exit nodes of the loop only have
  // predecessors that are inside of the loop.  This check guarantees that the
  // loop preheader/header will dominate the exit blocks.  If the exit block has
  // predecessors from outside of the loop, split the edge now.
  for (unsigned i = 0, e = L->getExitBlocks().size(); i != e; ++i) {
    BasicBlock *ExitBlock = L->getExitBlocks()[i];
    for (pred_iterator PI = pred_begin(ExitBlock), PE = pred_end(ExitBlock);
         PI != PE; ++PI)
      if (!L->contains(*PI)) {
        RewriteLoopExitBlock(L, ExitBlock);
        NumInserted++;
        Changed = true;
        break;
      }
    }

  // The preheader may have more than two predecessors at this point (from the
  // preheader and from the backedges).  To simplify the loop more, insert an
  // extra back-edge block in the loop so that there is exactly one backedge.
  if (L->getNumBackEdges() != 1) {
    InsertUniqueBackedgeBlock(L);
    NumInserted++;
    Changed = true;
  }

  for (Loop::iterator I = L->begin(), E = L->end(); I != E; ++I)
    Changed |= ProcessLoop(*I);
  return Changed;
}

/// SplitBlockPredecessors - Split the specified block into two blocks.  We want
/// to move the predecessors specified in the Preds list to point to the new
/// block, leaving the remaining predecessors pointing to BB.  This method
/// updates the SSA PHINode's, but no other analyses.
///
BasicBlock *LoopSimplify::SplitBlockPredecessors(BasicBlock *BB,
                                                 const char *Suffix,
                                       const std::vector<BasicBlock*> &Preds) {
  
  // Create new basic block, insert right before the original block...
  BasicBlock *NewBB = new BasicBlock(BB->getName()+Suffix, BB->getParent(), BB);

  // The preheader first gets an unconditional branch to the loop header...
  BranchInst *BI = new BranchInst(BB, NewBB);
  
  // For every PHI node in the block, insert a PHI node into NewBB where the
  // incoming values from the out of loop edges are moved to NewBB.  We have two
  // possible cases here.  If the loop is dead, we just insert dummy entries
  // into the PHI nodes for the new edge.  If the loop is not dead, we move the
  // incoming edges in BB into new PHI nodes in NewBB.
  //
  if (!Preds.empty()) {  // Is the loop not obviously dead?
    // Check to see if the values being merged into the new block need PHI
    // nodes.  If so, insert them.
    for (BasicBlock::iterator I = BB->begin();
         PHINode *PN = dyn_cast<PHINode>(I); ++I) {
      
      // Check to see if all of the values coming in are the same.  If so, we
      // don't need to create a new PHI node.
      Value *InVal = PN->getIncomingValueForBlock(Preds[0]);
      for (unsigned i = 1, e = Preds.size(); i != e; ++i)
        if (InVal != PN->getIncomingValueForBlock(Preds[i])) {
          InVal = 0;
          break;
        }
      
      // If the values coming into the block are not the same, we need a PHI.
      if (InVal == 0) {
        // Create the new PHI node, insert it into NewBB at the end of the block
        PHINode *NewPHI = new PHINode(PN->getType(), PN->getName()+".ph", BI);
        
        // Move all of the edges from blocks outside the loop to the new PHI
        for (unsigned i = 0, e = Preds.size(); i != e; ++i) {
          Value *V = PN->removeIncomingValue(Preds[i]);
          NewPHI->addIncoming(V, Preds[i]);
        }
        InVal = NewPHI;
      } else {
        // Remove all of the edges coming into the PHI nodes from outside of the
        // block.
        for (unsigned i = 0, e = Preds.size(); i != e; ++i)
          PN->removeIncomingValue(Preds[i], false);
      }

      // Add an incoming value to the PHI node in the loop for the preheader
      // edge.
      PN->addIncoming(InVal, NewBB);
    }
    
    // Now that the PHI nodes are updated, actually move the edges from
    // Preds to point to NewBB instead of BB.
    //
    for (unsigned i = 0, e = Preds.size(); i != e; ++i) {
      TerminatorInst *TI = Preds[i]->getTerminator();
      for (unsigned s = 0, e = TI->getNumSuccessors(); s != e; ++s)
        if (TI->getSuccessor(s) == BB)
          TI->setSuccessor(s, NewBB);
    }
    
  } else {                       // Otherwise the loop is dead...
    for (BasicBlock::iterator I = BB->begin();
         PHINode *PN = dyn_cast<PHINode>(I); ++I)
      // Insert dummy values as the incoming value...
      PN->addIncoming(Constant::getNullValue(PN->getType()), NewBB);
  }  
  return NewBB;
}

// ChangeExitBlock - This recursive function is used to change any exit blocks
// that use OldExit to use NewExit instead.  This is recursive because children
// may need to be processed as well.
//
static void ChangeExitBlock(Loop *L, BasicBlock *OldExit, BasicBlock *NewExit) {
  if (L->hasExitBlock(OldExit)) {
    L->changeExitBlock(OldExit, NewExit);
    for (Loop::iterator I = L->begin(), E = L->end(); I != E; ++I)
      ChangeExitBlock(*I, OldExit, NewExit);
  }
}


/// InsertPreheaderForLoop - Once we discover that a loop doesn't have a
/// preheader, this method is called to insert one.  This method has two phases:
/// preheader insertion and analysis updating.
///
void LoopSimplify::InsertPreheaderForLoop(Loop *L) {
  BasicBlock *Header = L->getHeader();

  // Compute the set of predecessors of the loop that are not in the loop.
  std::vector<BasicBlock*> OutsideBlocks;
  for (pred_iterator PI = pred_begin(Header), PE = pred_end(Header);
       PI != PE; ++PI)
      if (!L->contains(*PI))           // Coming in from outside the loop?
        OutsideBlocks.push_back(*PI);  // Keep track of it...
  
  // Split out the loop pre-header
  BasicBlock *NewBB =
    SplitBlockPredecessors(Header, ".preheader", OutsideBlocks);
  
  //===--------------------------------------------------------------------===//
  //  Update analysis results now that we have performed the transformation
  //
  
  // We know that we have loop information to update... update it now.
  if (Loop *Parent = L->getParentLoop())
    Parent->addBasicBlockToLoop(NewBB, getAnalysis<LoopInfo>());

  // If the header for the loop used to be an exit node for another loop, then
  // we need to update this to know that the loop-preheader is now the exit
  // node.  Note that the only loop that could have our header as an exit node
  // is a sibling loop, ie, one with the same parent loop, or one if it's
  // children.
  //
  LoopInfo::iterator ParentLoops, ParentLoopsE;
  if (Loop *Parent = L->getParentLoop()) {
    ParentLoops = Parent->begin();
    ParentLoopsE = Parent->end();
  } else {      // Must check top-level loops...
    ParentLoops = getAnalysis<LoopInfo>().begin();
    ParentLoopsE = getAnalysis<LoopInfo>().end();
  }

  // Loop over all sibling loops, performing the substitution (recursively to
  // include child loops)...
  for (; ParentLoops != ParentLoopsE; ++ParentLoops)
    ChangeExitBlock(*ParentLoops, Header, NewBB);
  
  DominatorSet &DS = getAnalysis<DominatorSet>();  // Update dominator info
  {
    // The blocks that dominate NewBB are the blocks that dominate Header,
    // minus Header, plus NewBB.
    DominatorSet::DomSetType DomSet = DS.getDominators(Header);
    DomSet.insert(NewBB);  // We dominate ourself
    DomSet.erase(Header);  // Header does not dominate us...
    DS.addBasicBlock(NewBB, DomSet);

    // The newly created basic block dominates all nodes dominated by Header.
    for (Function::iterator I = Header->getParent()->begin(),
           E = Header->getParent()->end(); I != E; ++I)
      if (DS.dominates(Header, I))
        DS.addDominator(I, NewBB);
  }
  
  // Update immediate dominator information if we have it...
  if (ImmediateDominators *ID = getAnalysisToUpdate<ImmediateDominators>()) {
    // Whatever i-dominated the header node now immediately dominates NewBB
    ID->addNewBlock(NewBB, ID->get(Header));
    
    // The preheader now is the immediate dominator for the header node...
    ID->setImmediateDominator(Header, NewBB);
  }
  
  // Update DominatorTree information if it is active.
  if (DominatorTree *DT = getAnalysisToUpdate<DominatorTree>()) {
    // The immediate dominator of the preheader is the immediate dominator of
    // the old header.
    //
    DominatorTree::Node *HeaderNode = DT->getNode(Header);
    DominatorTree::Node *PHNode = DT->createNewNode(NewBB,
                                                    HeaderNode->getIDom());
    
    // Change the header node so that PNHode is the new immediate dominator
    DT->changeImmediateDominator(HeaderNode, PHNode);
  }

  // Update dominance frontier information...
  if (DominanceFrontier *DF = getAnalysisToUpdate<DominanceFrontier>()) {
    // The DF(NewBB) is just (DF(Header)-Header), because NewBB dominates
    // everything that Header does, and it strictly dominates Header in
    // addition.
    assert(DF->find(Header) != DF->end() && "Header node doesn't have DF set?");
    DominanceFrontier::DomSetType NewDFSet = DF->find(Header)->second;
    NewDFSet.erase(Header);
    DF->addBasicBlock(NewBB, NewDFSet);

    // Now we must loop over all of the dominance frontiers in the function,
    // replacing occurrences of Header with NewBB in some cases.  If a block
    // dominates a (now) predecessor of NewBB, but did not strictly dominate
    // Header, it will have Header in it's DF set, but should now have NewBB in
    // its set.
    for (unsigned i = 0, e = OutsideBlocks.size(); i != e; ++i) {
      // Get all of the dominators of the predecessor...
      const DominatorSet::DomSetType &PredDoms =
        DS.getDominators(OutsideBlocks[i]);
      for (DominatorSet::DomSetType::const_iterator PDI = PredDoms.begin(),
             PDE = PredDoms.end(); PDI != PDE; ++PDI) {
        BasicBlock *PredDom = *PDI;
        // If the loop header is in DF(PredDom), then PredDom didn't dominate
        // the header but did dominate a predecessor outside of the loop.  Now
        // we change this entry to include the preheader in the DF instead of
        // the header.
        DominanceFrontier::iterator DFI = DF->find(PredDom);
        assert(DFI != DF->end() && "No dominance frontier for node?");
        if (DFI->second.count(Header)) {
          DF->removeFromFrontier(DFI, Header);
          DF->addToFrontier(DFI, NewBB);
        }
      }
    }
  }
}

void LoopSimplify::RewriteLoopExitBlock(Loop *L, BasicBlock *Exit) {
  DominatorSet &DS = getAnalysis<DominatorSet>();
  assert(std::find(L->getExitBlocks().begin(), L->getExitBlocks().end(), Exit)
         != L->getExitBlocks().end() && "Not a current exit block!");
  
  std::vector<BasicBlock*> LoopBlocks;
  for (pred_iterator I = pred_begin(Exit), E = pred_end(Exit); I != E; ++I)
    if (L->contains(*I))
      LoopBlocks.push_back(*I);

  assert(!LoopBlocks.empty() && "No edges coming in from outside the loop?");
  BasicBlock *NewBB = SplitBlockPredecessors(Exit, ".loopexit", LoopBlocks);

  // Update Loop Information - we know that the new block will be in the parent
  // loop of L.
  if (Loop *Parent = L->getParentLoop())
    Parent->addBasicBlockToLoop(NewBB, getAnalysis<LoopInfo>());

  // Replace any instances of Exit with NewBB in this and any nested loops...
  for (df_iterator<Loop*> I = df_begin(L), E = df_end(L); I != E; ++I)
    if (I->hasExitBlock(Exit))
      I->changeExitBlock(Exit, NewBB);   // Update exit block information

  // Update dominator information (set, immdom, domtree, and domfrontier)
  UpdateDomInfoForRevectoredPreds(NewBB, LoopBlocks);
}

/// InsertUniqueBackedgeBlock - This method is called when the specified loop
/// has more than one backedge in it.  If this occurs, revector all of these
/// backedges to target a new basic block and have that block branch to the loop
/// header.  This ensures that loops have exactly one backedge.
///
void LoopSimplify::InsertUniqueBackedgeBlock(Loop *L) {
  assert(L->getNumBackEdges() > 1 && "Must have > 1 backedge!");

  // Get information about the loop
  BasicBlock *Preheader = L->getLoopPreheader();
  BasicBlock *Header = L->getHeader();
  Function *F = Header->getParent();

  // Figure out which basic blocks contain back-edges to the loop header.
  std::vector<BasicBlock*> BackedgeBlocks;
  for (pred_iterator I = pred_begin(Header), E = pred_end(Header); I != E; ++I)
    if (*I != Preheader) BackedgeBlocks.push_back(*I);

  // Create and insert the new backedge block...
  BasicBlock *BEBlock = new BasicBlock(Header->getName()+".backedge", F);
  BranchInst *BETerminator = new BranchInst(Header, BEBlock);

  // Move the new backedge block to right after the last backedge block.
  Function::iterator InsertPos = BackedgeBlocks.back(); ++InsertPos;
  F->getBasicBlockList().splice(InsertPos, F->getBasicBlockList(), BEBlock);
  
  // Now that the block has been inserted into the function, create PHI nodes in
  // the backedge block which correspond to any PHI nodes in the header block.
  for (BasicBlock::iterator I = Header->begin();
       PHINode *PN = dyn_cast<PHINode>(I); ++I) {
    PHINode *NewPN = new PHINode(PN->getType(), PN->getName()+".be",
                                 BETerminator);
    NewPN->op_reserve(2*BackedgeBlocks.size());

    // Loop over the PHI node, moving all entries except the one for the
    // preheader over to the new PHI node.
    unsigned PreheaderIdx = ~0U;
    bool HasUniqueIncomingValue = true;
    Value *UniqueValue = 0;
    for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
      BasicBlock *IBB = PN->getIncomingBlock(i);
      Value *IV = PN->getIncomingValue(i);
      if (IBB == Preheader) {
        PreheaderIdx = i;
      } else {
        NewPN->addIncoming(IV, IBB);
        if (HasUniqueIncomingValue) {
          if (UniqueValue == 0)
            UniqueValue = IV;
          else if (UniqueValue != IV)
            HasUniqueIncomingValue = false;
        }
      }
    }
      
    // Delete all of the incoming values from the old PN except the preheader's
    assert(PreheaderIdx != ~0U && "PHI has no preheader entry??");
    if (PreheaderIdx != 0) {
      PN->setIncomingValue(0, PN->getIncomingValue(PreheaderIdx));
      PN->setIncomingBlock(0, PN->getIncomingBlock(PreheaderIdx));
    }
    PN->op_erase(PN->op_begin()+2, PN->op_end());

    // Finally, add the newly constructed PHI node as the entry for the BEBlock.
    PN->addIncoming(NewPN, BEBlock);

    // As an optimization, if all incoming values in the new PhiNode (which is a
    // subset of the incoming values of the old PHI node) have the same value,
    // eliminate the PHI Node.
    if (HasUniqueIncomingValue) {
      NewPN->replaceAllUsesWith(UniqueValue);
      BEBlock->getInstList().erase(NewPN);
    }
  }

  // Now that all of the PHI nodes have been inserted and adjusted, modify the
  // backedge blocks to just to the BEBlock instead of the header.
  for (unsigned i = 0, e = BackedgeBlocks.size(); i != e; ++i) {
    TerminatorInst *TI = BackedgeBlocks[i]->getTerminator();
    for (unsigned Op = 0, e = TI->getNumSuccessors(); Op != e; ++Op)
      if (TI->getSuccessor(Op) == Header)
        TI->setSuccessor(Op, BEBlock);
  }

  //===--- Update all analyses which we must preserve now -----------------===//

  // Update Loop Information - we know that this block is now in the current
  // loop and all parent loops.
  L->addBasicBlockToLoop(BEBlock, getAnalysis<LoopInfo>());

  // Replace any instances of Exit with NewBB in this and any nested loops...
  for (df_iterator<Loop*> I = df_begin(L), E = df_end(L); I != E; ++I)
    if (I->hasExitBlock(Header))
      I->changeExitBlock(Header, BEBlock);   // Update exit block information

  // Update dominator information (set, immdom, domtree, and domfrontier)
  UpdateDomInfoForRevectoredPreds(BEBlock, BackedgeBlocks);
}

/// UpdateDomInfoForRevectoredPreds - This method is used to update the four
/// different kinds of dominator information (dominator sets, immediate
/// dominators, dominator trees, and dominance frontiers) after a new block has
/// been added to the CFG.
///
/// This only supports the case when an existing block (known as "NewBBSucc"),
/// had some of its predecessors factored into a new basic block.  This
/// transformation inserts a new basic block ("NewBB"), with a single
/// unconditional branch to NewBBSucc, and moves some predecessors of
/// "NewBBSucc" to now branch to NewBB.  These predecessors are listed in
/// PredBlocks, even though they are the same as
/// pred_begin(NewBB)/pred_end(NewBB).
///
void LoopSimplify::UpdateDomInfoForRevectoredPreds(BasicBlock *NewBB,
                                         std::vector<BasicBlock*> &PredBlocks) {
  assert(!PredBlocks.empty() && "No predblocks??");
  assert(succ_begin(NewBB) != succ_end(NewBB) &&
         ++succ_begin(NewBB) == succ_end(NewBB) &&
         "NewBB should have a single successor!");
  BasicBlock *NewBBSucc = *succ_begin(NewBB);
  DominatorSet &DS = getAnalysis<DominatorSet>();

  // The newly inserted basic block will dominate existing basic blocks iff the
  // PredBlocks dominate all of the non-pred blocks.  If all predblocks dominate
  // the non-pred blocks, then they all must be the same block!
  bool NewBBDominatesNewBBSucc = true;
  {
    BasicBlock *OnePred = PredBlocks[0];
    for (unsigned i = 1, e = PredBlocks.size(); i != e; ++i)
      if (PredBlocks[i] != OnePred) {
        NewBBDominatesNewBBSucc = false;
        break;
      }

    if (NewBBDominatesNewBBSucc)
      for (pred_iterator PI = pred_begin(NewBBSucc), E = pred_end(NewBBSucc);
           PI != E; ++PI)
        if (*PI != NewBB && !DS.dominates(OnePred, *PI)) {
          NewBBDominatesNewBBSucc = false;
          break;
        }
  }

  // Update dominator information...  The blocks that dominate NewBB are the
  // intersection of the dominators of predecessors, plus the block itself.
  // The newly created basic block does not dominate anything except itself.
  //
  DominatorSet::DomSetType NewBBDomSet = DS.getDominators(PredBlocks[0]);
  for (unsigned i = 1, e = PredBlocks.size(); i != e; ++i)
    set_intersect(NewBBDomSet, DS.getDominators(PredBlocks[i]));
  NewBBDomSet.insert(NewBB);  // All blocks dominate themselves...
  DS.addBasicBlock(NewBB, NewBBDomSet);

  // If NewBB dominates some blocks, then it will dominate all blocks that
  // PredBlocks[0] used to except for PredBlocks[0] itself.
  if (NewBBDominatesNewBBSucc) {
    BasicBlock *PredBlock = PredBlocks[0];
    Function *F = NewBB->getParent();
    for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I)
      if (DS.properlyDominates(PredBlock, I))
        DS.addDominator(I, NewBB);
  }

  // Update immediate dominator information if we have it...
  BasicBlock *NewBBIDom = 0;
  if (ImmediateDominators *ID = getAnalysisToUpdate<ImmediateDominators>()) {
    // To find the immediate dominator of the new exit node, we trace up the
    // immediate dominators of a predecessor until we find a basic block that
    // dominates the exit block.
    //
    BasicBlock *Dom = PredBlocks[0];  // Some random predecessor...
    while (!NewBBDomSet.count(Dom)) {  // Loop until we find a dominator...
      assert(Dom != 0 && "No shared dominator found???");
      Dom = ID->get(Dom);
    }

    // Set the immediate dominator now...
    ID->addNewBlock(NewBB, Dom);
    NewBBIDom = Dom;   // Reuse this if calculating DominatorTree info...

    // If NewBB strictly dominates other blocks, we need to update their idom's
    // now.  The only block that need adjustment is the NewBBSucc block, whose
    // idom should currently be set to PredBlocks[0].
    if (NewBBDominatesNewBBSucc) {
      assert(ID->get(NewBBSucc) == PredBlocks[0] &&
             "Immediate dominator update code broken!");
      ID->setImmediateDominator(NewBBSucc, NewBB);
    }
  }

  // Update DominatorTree information if it is active.
  if (DominatorTree *DT = getAnalysisToUpdate<DominatorTree>()) {
    // If we don't have ImmediateDominator info around, calculate the idom as
    // above.
    DominatorTree::Node *NewBBIDomNode;
    if (NewBBIDom) {
      NewBBIDomNode = DT->getNode(NewBBIDom);
    } else {
      NewBBIDomNode = DT->getNode(PredBlocks[0]); // Random pred
      while (!NewBBDomSet.count(NewBBIDomNode->getBlock())) {
        NewBBIDomNode = NewBBIDomNode->getIDom();
        assert(NewBBIDomNode && "No shared dominator found??");
      }
    }

    // Create the new dominator tree node... and set the idom of NewBB.
    DominatorTree::Node *NewBBNode = DT->createNewNode(NewBB, NewBBIDomNode);

    // If NewBB strictly dominates other blocks, then it is now the immediate
    // dominator of NewBBSucc.  Update the dominator tree as appropriate.
    if (NewBBDominatesNewBBSucc) {
      DominatorTree::Node *NewBBSuccNode = DT->getNode(NewBBSucc);
      assert(NewBBSuccNode->getIDom()->getBlock() == PredBlocks[0] &&
             "Immediate tree update code broken!");
      DT->changeImmediateDominator(NewBBSuccNode, NewBBNode);
    }
  }

  // Update dominance frontier information...
  if (DominanceFrontier *DF = getAnalysisToUpdate<DominanceFrontier>()) {
    // If NewBB dominates NewBBSucc, then the global dominance frontiers are not
    // changed.  DF(NewBB) is now going to be the DF(PredBlocks[0]) without the
    // stuff that the new block does not dominate a predecessor of.
    if (NewBBDominatesNewBBSucc) {
      DominanceFrontier::iterator DFI = DF->find(PredBlocks[0]);
      if (DFI != DF->end()) {
        DominanceFrontier::DomSetType Set = DFI->second;
        // Filter out stuff in Set that we do not dominate a predecessor of.
        for (DominanceFrontier::DomSetType::iterator SetI = Set.begin(),
               E = Set.end(); SetI != E;) {
          bool DominatesPred = false;
          for (pred_iterator PI = pred_begin(*SetI), E = pred_end(*SetI);
               PI != E; ++PI)
            if (DS.dominates(NewBB, *PI))
              DominatesPred = true;
          if (!DominatesPred)
            Set.erase(SetI++);
          else
            ++SetI;
        }

        DF->addBasicBlock(NewBB, Set);
      }

    } else {
      // DF(NewBB) is {NewBBSucc} because NewBB does not strictly dominate
      // NewBBSucc, but it does dominate itself (and there is an edge (NewBB ->
      // NewBBSucc)).  NewBBSucc is the single successor of NewBB.
      DominanceFrontier::DomSetType NewDFSet;
      NewDFSet.insert(NewBBSucc);
      DF->addBasicBlock(NewBB, NewDFSet);
      
      // Now we must loop over all of the dominance frontiers in the function,
      // replacing occurrences of NewBBSucc with NewBB in some cases.  All
      // blocks that dominate a block in PredBlocks and contained NewBBSucc in
      // their dominance frontier must be updated to contain NewBB instead.
      //
      for (unsigned i = 0, e = PredBlocks.size(); i != e; ++i) {
        BasicBlock *Pred = PredBlocks[i];
        // Get all of the dominators of the predecessor...
        const DominatorSet::DomSetType &PredDoms = DS.getDominators(Pred);
        for (DominatorSet::DomSetType::const_iterator PDI = PredDoms.begin(),
               PDE = PredDoms.end(); PDI != PDE; ++PDI) {
          BasicBlock *PredDom = *PDI;

          // If the NewBBSucc node is in DF(PredDom), then PredDom didn't
          // dominate NewBBSucc but did dominate a predecessor of it.  Now we
          // change this entry to include NewBB in the DF instead of NewBBSucc.
          DominanceFrontier::iterator DFI = DF->find(PredDom);
          assert(DFI != DF->end() && "No dominance frontier for node?");
          if (DFI->second.count(NewBBSucc)) {
            DF->removeFromFrontier(DFI, NewBBSucc);
            DF->addToFrontier(DFI, NewBB);
          }
        }
      }
    }
  }
}

