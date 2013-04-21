//===-- TGSIMCTargetDesc.h - TGSI Target Descriptions ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides TGSI specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_MC_TARGET_DESC_H
#define TGSI_MC_TARGET_DESC_H

namespace llvm {
   class Target;
   class MCStreamer;
   class StringRef;
   class MCContext;
   class MCAsmBackend;
   class raw_ostream;
   class MCCodeEmitter;
   class MCInstPrinter;
   class MCAsmInfo;
   class MCInstrInfo;
   class MCRegisterInfo;
   class MCSubtargetInfo;

   extern Target TheTGSITarget;

   MCStreamer *createTGSIObjStreamer(const Target &T, StringRef TT,
                                     MCContext &Ctx, MCAsmBackend &MAB,
                                     raw_ostream &OS,
                                     MCCodeEmitter *Emitter,
                                     bool RelaxAll,
                                     bool NoExecStack);

   MCInstPrinter *createTGSIMCInstPrinter(const Target &t,
                                          unsigned SyntaxVariant,
                                          const MCAsmInfo &mai,
                                          const MCInstrInfo &mii,
                                          const MCRegisterInfo &mri,
                                          const MCSubtargetInfo &sti);
}

// Defines symbolic names for TGSI registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "TGSIGenRegisterInfo.inc"

// Defines symbolic names for the TGSI instructions.
//
#define GET_INSTRINFO_ENUM
#include "TGSIGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "TGSIGenSubtargetInfo.inc"

#endif
