//===-- TGSISelectionDAGInfo.cpp - TGSI SelectionDAG Info ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the TGSISelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "tgsi-selectiondag-info"
#include "TGSITargetMachine.h"
using namespace llvm;

TGSISelectionDAGInfo::TGSISelectionDAGInfo(const TGSITargetMachine &TM)
   : TargetSelectionDAGInfo(TM) {
}

TGSISelectionDAGInfo::~TGSISelectionDAGInfo() {
}
