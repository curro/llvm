//===-- TGSITargetMachine.h - Define TargetMachine for TGSI ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TGSI specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_TARGET_MACHINE_H
#define TGSI_TARGET_MACHINE_H

#include "TGSIInstrInfo.h"
#include "TGSIISelLowering.h"
#include "TGSIFrameLowering.h"
#include "TGSISelectionDAGInfo.h"
#include "TGSISubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
   class TGSITargetMachine : public LLVMTargetMachine {
      TGSISubtarget Subtarget;
      const DataLayout dl;       // Calculates type size & alignment
      TGSIInstrInfo InstrInfo;
      TGSITargetLowering TLInfo;
      TGSISelectionDAGInfo TSInfo;
      TGSIFrameLowering FrameLowering;
   public:
      TGSITargetMachine(const Target &T, StringRef TT,
                        StringRef CPU, StringRef FS,
                        const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM,
                        CodeGenOpt::Level OL);

      virtual const TGSIInstrInfo *getInstrInfo() const { return &InstrInfo; }
      virtual const TargetFrameLowering  *getFrameLowering() const {
         return &FrameLowering;
      }
      virtual const TGSISubtarget   *getSubtargetImpl() const{ return &Subtarget; }
      virtual const TargetRegisterInfo *getRegisterInfo() const {
         return &InstrInfo.getRegisterInfo();
      }
      virtual const TGSITargetLowering* getTargetLowering() const {
         return &TLInfo;
      }
      virtual const TGSISelectionDAGInfo* getSelectionDAGInfo() const {
         return &TSInfo;
      }
      virtual const DataLayout *getDataLayout() const { return &dl; }

      virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
   };

}

#endif
