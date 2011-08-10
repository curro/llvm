//===-- llvm/Transforms/Utils/SimplifyIndVar.h - Indvar Utils ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines in interface for induction variable simplification. It does
// not define any actual pass or policy, but provides a single function to
// simplify a loop's induction variables based on ScalarEvolution.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_SIMPLIFYINDVAR_H
#define LLVM_TRANSFORMS_UTILS_SIMPLIFYINDVAR_H

#include "llvm/Support/CommandLine.h"

namespace llvm {

extern cl::opt<bool> DisableIVRewrite;

class Loop;
class LoopInfo;
class DominatorTree;
class ScalarEvolution;
class LPPassManager;
class IVUsers;

/// Interface for visiting interesting IV users that are recognized but not
/// simplified by this utility.
class IVVisitor {
public:
  virtual ~IVVisitor() {}
  virtual void visitCast(CastInst *Cast) = 0;
};

/// simplifyUsersOfIV - Simplify instructions that use this induction variable
/// by using ScalarEvolution to analyze the IV's recurrence.
bool simplifyUsersOfIV(PHINode *CurrIV, LPPassManager *LPM,
                       SmallVectorImpl<WeakVH> &Dead, IVVisitor *V = NULL);

/// SimplifyLoopIVs - Simplify users of induction variables within this
/// loop. This does not actually change or add IVs.
bool simplifyLoopIVs(Loop *L, LPPassManager *LPM,
                     SmallVectorImpl<WeakVH> &Dead);

/// simplifyIVUsers - Simplify instructions recorded by the IVUsers pass.
/// This is a legacy implementation to reproduce the behavior of the
/// IndVarSimplify pass prior to DisableIVRewrite.
bool simplifyIVUsers(IVUsers *IU, LPPassManager *LPM,
                     SmallVectorImpl<WeakVH> &Dead);

} // namespace llvm

#endif
