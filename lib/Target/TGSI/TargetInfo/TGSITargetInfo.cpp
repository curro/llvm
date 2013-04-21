//===-- TGSITargetInfo.cpp - TGSI Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TGSI.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheTGSITarget;

extern "C" void LLVMInitializeTGSITargetInfo() {
  RegisterTarget<Triple::tgsi> X(TheTGSITarget, "tgsi", "TGSI");
}
