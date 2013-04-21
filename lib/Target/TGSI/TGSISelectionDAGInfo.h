//===-- TGSISelectionDAGInfo.h - TGSI SelectionDAG Info -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the TGSI subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_SELECTION_DAG_INFO_H
#define TGSI_SELECTION_DAG_INFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {
   class TGSITargetMachine;

   class TGSISelectionDAGInfo : public TargetSelectionDAGInfo {
   public:
      explicit TGSISelectionDAGInfo(const TGSITargetMachine &TM);
      ~TGSISelectionDAGInfo();
   };

}

#endif
