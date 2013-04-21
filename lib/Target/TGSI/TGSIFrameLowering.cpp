//====- TGSIFrameLowering.cpp - TGSI Frame Information -------*- C++ -*-====//
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

#include "TGSIFrameLowering.h"
#include "TGSIInstrInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

void TGSIFrameLowering::emitPrologue(MachineFunction &mf) const {
   MachineBasicBlock &mbb = mf.front();
   MachineFrameInfo *mfi = mf.getFrameInfo();
   const TGSIInstrInfo &tii =
      *static_cast<const TGSIInstrInfo*>(mf.getTarget().getInstrInfo());
   MachineBasicBlock::iterator mbbi = mbb.begin();
   DebugLoc dl;

   // Get the number of bytes to allocate from the FrameInfo
   int frame_sz = mfi->getStackSize();

   BuildMI(mbb, mbbi, dl, tii.get(TGSI::UADDs), TGSI::TEMP0x)
      .addReg(TGSI::TEMP0x).addImm(frame_sz);
}

void TGSIFrameLowering::emitEpilogue(MachineFunction &mf,
                                     MachineBasicBlock &mbb) const {
   MachineBasicBlock::iterator mbbi = mbb.getLastNonDebugInstr();
   MachineFrameInfo *mfi = mf.getFrameInfo();
   const TGSIInstrInfo &tii =
      *static_cast<const TGSIInstrInfo*>(mf.getTarget().getInstrInfo());
   DebugLoc dl;

   // Get the number of bytes to allocate from the FrameInfo
   int frame_sz = mfi->getStackSize();

   BuildMI(mbb, mbbi, dl, tii.get(TGSI::UADDs), TGSI::TEMP0x)
      .addReg(TGSI::TEMP0x).addImm(-frame_sz);
}
