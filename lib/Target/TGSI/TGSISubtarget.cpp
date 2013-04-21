//===- TGSISubtarget.cpp - TGSI Subtarget Information -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the TGSI specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "TGSISubtarget.h"
#include "TGSI.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TGSIGenSubtargetInfo.inc"

using namespace llvm;

TGSISubtarget::TGSISubtarget(const std::string &TT, const std::string &CPU,
			     const std::string &FS) :
   TGSIGenSubtargetInfo(TT, CPU, FS) {

   // Determine default and user specified characteristics
   std::string CPUName = CPU.empty() ? "TGSI" : CPU;

   // Parse features string.
   ParseSubtargetFeatures(CPUName, FS);
}
