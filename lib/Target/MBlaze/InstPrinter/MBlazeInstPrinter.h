//===-- MBLazeInstPrinter.h - Convert MBlaze MCInst to assembly syntax ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints a MBlaze MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef MBLAZEINSTPRINTER_H
#define MBLAZEINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {
  class MCOperand;

  class MBlazeInstPrinter : public MCInstPrinter {
  public:
    MBlazeInstPrinter(const MCAsmInfo &MAI) : MCInstPrinter(MAI) {
    }

    virtual void printInst(const MCInst *MI, raw_ostream &O);

    // Autogenerated by tblgen.
    void printInstruction(const MCInst *MI, raw_ostream &O);
    static const char *getRegisterName(unsigned RegNo);
    static const char *getInstructionName(unsigned Opcode);

    void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                      const char *Modifier = 0);
    void printPCRelImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
    void printSrcMemOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                            const char *Modifier = 0);
    void printFSLImm(const MCInst *MI, int OpNo, raw_ostream &O);
    void printUnsignedImm(const MCInst *MI, int OpNo, raw_ostream &O);
    void printMemOperand(const MCInst *MI, int OpNo,raw_ostream &O,
                         const char *Modifier = 0);
  };
}

#endif
