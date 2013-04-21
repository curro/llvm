//===- TGSIFrameLowering.h - Define frame lowering for TGSI --*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the TGSI implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_FRAME_LOWERING_H
#define TGSI_FRAME_LOWERING_H

#include "TGSI.h"
#include "TGSISubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
   class TGSISubtarget;

   class TGSIFrameLowering : public TargetFrameLowering {
      const TGSISubtarget &STI;
   public:
      explicit TGSIFrameLowering(const TGSISubtarget &sti)
         : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 16, 0), STI(sti) {
      }

      void emitPrologue(MachineFunction &MF) const;
      void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

      bool hasFP(const MachineFunction &MF) const { return false; }
   };
}

#endif
