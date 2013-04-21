//===- TGSIInstrInfo.cpp - TGSI Instruction Information -------*- C++ -*-===//
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

#include "TGSIInstrInfo.h"
#include "TGSI.h"
#include "TGSISubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"

#define GET_INSTRINFO_CTOR
#include "TGSIGenInstrInfo.inc"

using namespace llvm;

TGSIInstrInfo::TGSIInstrInfo(TGSISubtarget &ST)
   : RI(ST, *this), Subtarget(ST) {
}

unsigned TGSIInstrInfo::isLoadFromStackSlot(const MachineInstr *mi,
                                            int &fi) const {
   if ((mi->getOpcode() == TGSI::LDpis || mi->getOpcode() == TGSI::LDpfs ||
        mi->getOpcode() == TGSI::LDpiv || mi->getOpcode() == TGSI::LDpfv) &&
       mi->getOperand(1).isFI()) {
      fi = mi->getOperand(1).getIndex();
      return mi->getOperand(0).getReg();
   }

   return 0;
}

unsigned TGSIInstrInfo::isStoreToStackSlot(const MachineInstr *mi,
                                           int &fi) const {
   if ((mi->getOpcode() == TGSI::STpis || mi->getOpcode() == TGSI::STpfs ||
        mi->getOpcode() == TGSI::STpiv || mi->getOpcode() == TGSI::STpfv) &&
       mi->getOperand(0).isFI()) {
      assert(0);
      fi = mi->getOperand(0).getIndex();
      return mi->getOperand(1).getReg();
   }

   return 0;
}

bool TGSIInstrInfo::AnalyzeBranch(MachineBasicBlock &bb,
                                  MachineBasicBlock *&tbb,
                                  MachineBasicBlock *&fbb,
                                  SmallVectorImpl<MachineOperand> &cond,
                                  bool AllowModify) const {
   for (MachineBasicBlock::iterator i = bb.getFirstTerminator();
        i != bb.end(); ++i) {
      if (i->getOpcode() == TGSI::P_BRCOND) {
         tbb = i->getOperand(1).getMBB();
         cond.clear();
         cond.push_back(i->getOperand(0));

      } else if (i->getOpcode() == TGSI::BRA) {
         MachineBasicBlock **pbb = tbb ? &fbb : &tbb;

         *pbb = i->getOperand(0).getMBB();
         return false;

      } else {
         return true;
      }
   }

   return false;
}

unsigned
TGSIInstrInfo::InsertBranch(MachineBasicBlock &bb, MachineBasicBlock *tbb,
                            MachineBasicBlock *fbb,
                            const SmallVectorImpl<MachineOperand> &cond,
                            DebugLoc dl) const {
   int n = 0;

   if (cond.empty()) {
      BuildMI(&bb, dl, get(TGSI::BRA)).addMBB(tbb);
      n++;

   } else {
      BuildMI(&bb, dl, get(TGSI::P_BRCOND)).addOperand(cond[0]).addMBB(tbb);
      n++;

      if (fbb) {
         BuildMI(&bb, dl, get(TGSI::BRA)).addMBB(fbb);
         n++;
      }
   }

   return n;
}

unsigned TGSIInstrInfo::RemoveBranch(MachineBasicBlock &bb) const {
   int n = 0;

   while (!bb.empty()) {
      MachineInstr& i = bb.back();

      if (i.getOpcode() == TGSI::BRA ||
          i.getOpcode() == TGSI::P_BRCOND) {
         i.eraseFromParent();
         n++;

      } else {
         break;
      }
   }

   return n;
}

void TGSIInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I, DebugLoc DL,
                                unsigned DestReg, unsigned SrcReg,
                                bool KillSrc) const {
   if (TGSI::IRegsRegClass.contains(DestReg, SrcReg))
      BuildMI(MBB, I, DL, get(TGSI::MOVis), DestReg)
         .addReg(SrcReg, getKillRegState(KillSrc));
   else if (TGSI::IVRegsRegClass.contains(DestReg, SrcReg))
      BuildMI(MBB, I, DL, get(TGSI::MOViv), DestReg)
         .addReg(SrcReg, getKillRegState(KillSrc));
   else
      assert(0);
}

void TGSIInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int fi,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
   DebugLoc dl = I->getDebugLoc();

   if (RC == &TGSI::IRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::STpis)).addFrameIndex(fi).addReg(SrcReg);
   else if (RC == &TGSI::IVRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::STpiv)).addFrameIndex(fi).addReg(SrcReg);
   else if (RC == &TGSI::FRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::STpfs)).addFrameIndex(fi).addReg(SrcReg);
   else if (RC == &TGSI::FVRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::STpfv)).addFrameIndex(fi).addReg(SrcReg);
   else
      llvm_unreachable("Can't store this register to stack slot");
}

void TGSIInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int fi,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
   DebugLoc dl = I->getDebugLoc();

   if (RC == &TGSI::IRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::LDpis), DestReg).addFrameIndex(fi);
   else if (RC == &TGSI::IVRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::LDpiv), DestReg).addFrameIndex(fi);
   else if (RC == &TGSI::FRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::LDpfs), DestReg).addFrameIndex(fi);
   else if (RC == &TGSI::FVRegsRegClass)
      BuildMI(MBB, I, dl, get(TGSI::LDpfv), DestReg).addFrameIndex(fi);
   else
      llvm_unreachable("Can't load this register from stack slot");
}
