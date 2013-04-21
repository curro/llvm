//===- TGSIRegisterInfo.h - TGSI Register Information Impl ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the TGSI implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_REGISTER_INFO_H
#define TGSI_REGISTER_INFO_H

#define GET_REGINFO_HEADER
#include "TGSIGenRegisterInfo.inc"

namespace llvm {
   class TGSISubtarget;
   class TargetInstrInfo;
   class Type;

   namespace TGSIRF {
      enum {
         INDEX = 0xfff,
         COMPONENT = 0xf000,
         COMPONENT_X = 0x1000,
         COMPONENT_Y = 0x2000,
         COMPONENT_Z = 0x4000,
         COMPONENT_W = 0x8000,
         COMPONENT_XYZW = 0xf000
      };
   }

   struct TGSIRegisterInfo : public TGSIGenRegisterInfo {
      TGSISubtarget &Subtarget;
      const TargetInstrInfo &TII;

      TGSIRegisterInfo(TGSISubtarget &st, const TargetInstrInfo &tii);

      const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF = 0) const;

      BitVector getReservedRegs(const MachineFunction &MF) const;

      void eliminateFrameIndex(MachineBasicBlock::iterator II,
                               int SPAdj, unsigned FIOperandNum,
                               RegScavenger *RS = NULL) const;

      unsigned getFrameRegister(const MachineFunction &MF) const;

      unsigned getEHExceptionRegister() const;
      unsigned getEHHandlerRegister() const;
   };

}

#endif
