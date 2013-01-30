//===-- MipsELFStreamer.cpp - MipsELFStreamer ---------------------------===//
//
//                       The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===-------------------------------------------------------------------===//
#include "MCTargetDesc/MipsELFStreamer.h"
#include "MipsSubtarget.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

  MCELFStreamer* createMipsELFStreamer(MCContext &Context, MCAsmBackend &TAB,
                                       raw_ostream &OS, MCCodeEmitter *Emitter,
                                       bool RelaxAll, bool NoExecStack) {
    MipsELFStreamer *S = new MipsELFStreamer(Context, TAB, OS, Emitter,
                                             RelaxAll, NoExecStack);
    return S;
  }

  // For llc. Set a group of ELF header flags
  void
  MipsELFStreamer::emitELFHeaderFlagsCG(const MipsSubtarget &Subtarget) {

    if (hasRawTextSupport())
      return;

    // Update e_header flags
    MCAssembler& MCA = getAssembler();
    unsigned EFlags = MCA.getELFHeaderEFlags();

    EFlags |= ELF::EF_MIPS_NOREORDER;

    // Architecture
    if (Subtarget.hasMips64r2())
      EFlags |= ELF::EF_MIPS_ARCH_64R2;
    else if (Subtarget.hasMips64())
      EFlags |= ELF::EF_MIPS_ARCH_64;
    else if (Subtarget.hasMips32r2())
      EFlags |= ELF::EF_MIPS_ARCH_32R2;
    else
      EFlags |= ELF::EF_MIPS_ARCH_32;

    // Relocation Model
    Reloc::Model RM = Subtarget.getRelocationModel();
    if (RM == Reloc::PIC_ || RM == Reloc::Default)
      EFlags |= ELF::EF_MIPS_PIC;
    else if (RM == Reloc::Static)
      ; // Do nothing for Reloc::Static
    else
      llvm_unreachable("Unsupported relocation model for e_flags");

    MCA.setELFHeaderEFlags(EFlags);


  }
}
