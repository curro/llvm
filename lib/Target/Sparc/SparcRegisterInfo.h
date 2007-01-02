//===- SparcRegisterInfo.h - Sparc Register Information Impl ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sparc implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef SPARCREGISTERINFO_H
#define SPARCREGISTERINFO_H

#include "llvm/Target/MRegisterInfo.h"
#include "SparcGenRegisterInfo.h.inc"

namespace llvm {

class SparcSubtarget;
class TargetInstrInfo;
class Type;

struct SparcRegisterInfo : public SparcGenRegisterInfo {
  SparcSubtarget &Subtarget;
  const TargetInstrInfo &TII;
  
  SparcRegisterInfo(SparcSubtarget &st, const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI,
                           unsigned SrcReg, int FrameIndex,
                           const TargetRegisterClass *RC) const;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            unsigned DestReg, int FrameIndex,
                            const TargetRegisterClass *RC) const;

  void copyRegToReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                    unsigned DestReg, unsigned SrcReg,
                    const TargetRegisterClass *RC) const;
  
  virtual MachineInstr* foldMemoryOperand(MachineInstr* MI,
                                          unsigned OpNum,
                                          int FrameIndex) const;

  const unsigned *getCalleeSavedRegs() const;

  const TargetRegisterClass* const* getCalleeSavedRegClasses() const;

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

  void eliminateFrameIndex(MachineBasicBlock::iterator II) const;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;
  
  // Debug information queries.
  unsigned getRARegister() const;
  unsigned getFrameRegister(MachineFunction &MF) const;
};

} // end namespace llvm

#endif
