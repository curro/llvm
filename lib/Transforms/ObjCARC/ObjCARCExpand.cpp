//===- ObjCARCExpand.cpp - ObjC ARC Optimization --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines ObjC ARC optimizations. ARC stands for Automatic
/// Reference Counting and is a system for managing reference counts for objects
/// in Objective C.
///
/// This specific file deals with early optimizations which perform certain
/// cleanup operations.
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "objc-arc-expand"
#include "ObjCARC.h"

using namespace llvm;
using namespace llvm::objcarc;

namespace {
  /// \brief Early ARC transformations.
  class ObjCARCExpand : public FunctionPass {
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool doInitialization(Module &M);
    virtual bool runOnFunction(Function &F);

    /// A flag indicating whether this optimization pass should run.
    bool Run;

  public:
    static char ID;
    ObjCARCExpand() : FunctionPass(ID) {
      initializeObjCARCExpandPass(*PassRegistry::getPassRegistry());
    }
  };
}

char ObjCARCExpand::ID = 0;
INITIALIZE_PASS(ObjCARCExpand,
                "objc-arc-expand", "ObjC ARC expansion", false, false)

Pass *llvm::createObjCARCExpandPass() {
  return new ObjCARCExpand();
}

void ObjCARCExpand::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

bool ObjCARCExpand::doInitialization(Module &M) {
  Run = ModuleHasARC(M);
  return false;
}

bool ObjCARCExpand::runOnFunction(Function &F) {
  if (!EnableARCOpts)
    return false;

  // If nothing in the Module uses ARC, don't do anything.
  if (!Run)
    return false;

  bool Changed = false;

  DEBUG(dbgs() << "ObjCARCExpand: Visiting Function: " << F.getName() << "\n");

  for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I) {
    Instruction *Inst = &*I;

    DEBUG(dbgs() << "ObjCARCExpand: Visiting: " << *Inst << "\n");

    switch (GetBasicInstructionClass(Inst)) {
    case IC_Retain:
    case IC_RetainRV:
    case IC_Autorelease:
    case IC_AutoreleaseRV:
    case IC_FusedRetainAutorelease:
    case IC_FusedRetainAutoreleaseRV: {
      // These calls return their argument verbatim, as a low-level
      // optimization. However, this makes high-level optimizations
      // harder. Undo any uses of this optimization that the front-end
      // emitted here. We'll redo them in the contract pass.
      Changed = true;
      Value *Value = cast<CallInst>(Inst)->getArgOperand(0);
      DEBUG(dbgs() << "ObjCARCExpand: Old = " << *Inst << "\n"
                      "               New = " << *Value << "\n");
      Inst->replaceAllUsesWith(Value);
      break;
    }
    default:
      break;
    }
  }

  DEBUG(dbgs() << "ObjCARCExpand: Finished List.\n\n");

  return Changed;
}

