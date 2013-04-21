//===-- TGSITargetMachine.cpp - Define TargetMachine for TGSI -----------===//
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

#include "TGSI.h"
#include "TGSITargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace {
   class TGSIPassConfig : public TargetPassConfig {
   public:
      TGSIPassConfig(TGSITargetMachine *TM, PassManagerBase &PM)
         : TargetPassConfig(TM, PM) {}

      TGSITargetMachine &getTGSITargetMachine() const {
         return getTM<TGSITargetMachine>();
      }

      virtual bool addInstSelector();
   };
}

extern "C" void LLVMInitializeTGSITarget() {
   RegisterTargetMachine<TGSITargetMachine> X(TheTGSITarget);
}

TGSITargetMachine::TGSITargetMachine(const Target &T, StringRef TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Reloc::Model RM, CodeModel::Model CM,
                                     CodeGenOpt::Level OL)
   : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
     Subtarget(TT, CPU, FS),
     dl(Subtarget.getDataLayout()),
     InstrInfo(Subtarget),
     TLInfo(*this), TSInfo(*this),
     FrameLowering(Subtarget) {
     }

TargetPassConfig *TGSITargetMachine::createPassConfig(PassManagerBase &PM) {
   return new TGSIPassConfig(this, PM);
}

bool TGSIPassConfig::addInstSelector() {
   addPass(createTGSIISelDag(getTGSITargetMachine()));
   return false;
}
