//===- TGSIInstrInfo.h - TGSI Instruction Information ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the TGSI implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_INSTRUCTION_INFO_H
#define TGSI_INSTRUCTION_INFO_H

#include "TGSI.h"
#include "TGSIRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "TGSIGenInstrInfo.inc"

namespace llvm {
   namespace TGSIIF {
      enum {
         OPCODE = 0xff,
         NEGATE_OP0 = 0x100,
         NEGATE_OP1 = 0x200,
         DST_RESOURCE = 0x400,
         SRC_RESOURCE = 0x800,
         ADDR_SPACE = 0x7000,
         ADDR_SPACE_PRIVATE = tgsi::PRIVATE << 12,
         ADDR_SPACE_GLOBAL = tgsi::GLOBAL << 12,
         ADDR_SPACE_LOCAL = tgsi::LOCAL << 12,
         ADDR_SPACE_CONSTANT = tgsi::CONSTANT << 12,
         ADDR_SPACE_INPUT = tgsi::INPUT << 12
      };
   }

   class TGSIInstrInfo : public TGSIGenInstrInfo {
      const TGSIRegisterInfo RI;
      const TGSISubtarget& Subtarget;
   public:
      explicit TGSIInstrInfo(TGSISubtarget &ST);

      virtual const TGSIRegisterInfo &getRegisterInfo() const {
         return RI;
      }

      virtual unsigned isLoadFromStackSlot(const MachineInstr *MI,
                                           int &FrameIndex) const;

      virtual unsigned isStoreToStackSlot(const MachineInstr *MI,
                                          int &FrameIndex) const;


      virtual bool AnalyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify = false) const ;

      virtual unsigned RemoveBranch(MachineBasicBlock &MBB) const;

      virtual unsigned InsertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    const SmallVectorImpl<MachineOperand> &Cond,
                                    DebugLoc DL) const;

      virtual void copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I, DebugLoc DL,
                               unsigned DestReg, unsigned SrcReg,
                               bool KillSrc) const;

      virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MBBI,
                                       unsigned SrcReg, bool isKill,
                                       int FrameIndex,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const;

      virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MBBI,
                                        unsigned DestReg, int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const;
   };
}

#endif
