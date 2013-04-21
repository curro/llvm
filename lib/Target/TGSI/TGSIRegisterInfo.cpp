//===- TGSIRegisterInfo.cpp - TGSI Register Information -------*- C++ -*-===//
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

#include "TGSI.h"
#include "TGSIRegisterInfo.h"
#include "TGSISubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"

#define GET_REGINFO_TARGET_DESC
#include "TGSIGenRegisterInfo.inc"

using namespace llvm;

TGSIRegisterInfo::TGSIRegisterInfo(TGSISubtarget &st,
				   const TargetInstrInfo &tii)
  : TGSIGenRegisterInfo(0), Subtarget(st), TII(tii) {
}

const MCPhysReg *TGSIRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF)
                                                                         const {
  static const MCPhysReg regs[] = {
    TGSI::TEMP2, TGSI::TEMP3, TGSI::TEMP4, TGSI::TEMP5, TGSI::TEMP6,
    TGSI::TEMP7, TGSI::TEMP8, TGSI::TEMP9, TGSI::TEMP10, TGSI::TEMP11,
    TGSI::TEMP12, TGSI::TEMP13, TGSI::TEMP14, TGSI::TEMP15, TGSI::TEMP16,
    TGSI::TEMP17, TGSI::TEMP18, TGSI::TEMP19, TGSI::TEMP20, TGSI::TEMP21,
    TGSI::TEMP22, TGSI::TEMP23, TGSI::TEMP24, TGSI::TEMP25, TGSI::TEMP26,
    TGSI::TEMP27, TGSI::TEMP28, TGSI::TEMP29, TGSI::TEMP30, TGSI::TEMP31
  };

  return regs;
}

BitVector TGSIRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector rsv(getNumRegs());

  rsv.set(TGSI::TEMP0x);
  rsv.set(TGSI::TEMP0y);
  rsv.set(TGSI::TEMP0z);
  rsv.set(TGSI::TEMP0w);
  rsv.set(TGSI::TEMP0);

  rsv.set(TGSI::ADDR0x);
  rsv.set(TGSI::ADDR0y);
  rsv.set(TGSI::ADDR0z);
  rsv.set(TGSI::ADDR0w);
  rsv.set(TGSI::ADDR0);
  return rsv;
}

void TGSIRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator ii,
                                           int spadj, unsigned int fiop,
                                           RegScavenger *rs) const {
  unsigned i = 0;
  MachineInstr &mi = *ii;
  DebugLoc dl = mi.getDebugLoc();
  MachineFunction &mf = *mi.getParent()->getParent();

  assert(spadj == 0);

  while (!mi.getOperand(i).isFI()) {
    ++i;
    assert(i < mi.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  int frameidx = mi.getOperand(i).getIndex();
  int offset = mf.getFrameInfo()->getObjectOffset(frameidx);

  // Replace frame index with a frame pointer reference.
  BuildMI(*mi.getParent(), ii, dl, TII.get(TGSI::UADDs), TGSI::TEMP0y)
    .addReg(TGSI::TEMP0x).addImm(offset);
  mi.getOperand(i).ChangeToRegister(TGSI::TEMP0y, false);
}

unsigned TGSIRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  llvm_unreachable("What is the frame register");
  return 0;
}

unsigned TGSIRegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned TGSIRegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}
