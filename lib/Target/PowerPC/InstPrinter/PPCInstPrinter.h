//===- PPCInstPrinter.h - Convert PPC MCInst to assembly syntax -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an PPC MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef PPCINSTPRINTER_H
#define PPCINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class MCOperand;

class PPCInstPrinter : public MCInstPrinter {
  // 0 -> AIX, 1 -> Darwin.
  unsigned SyntaxVariant;
public:
  PPCInstPrinter(const MCAsmInfo &MAI, const MCRegisterInfo &MRI,
                 unsigned syntaxVariant)
    : MCInstPrinter(MAI, MRI), SyntaxVariant(syntaxVariant) {}
  
  bool isDarwinSyntax() const {
    return SyntaxVariant == 1;
  }
  
  virtual void printRegName(raw_ostream &OS, unsigned RegNo) const;
  virtual void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot);
  virtual StringRef getOpcodeName(unsigned Opcode) const;
  
  static const char *getInstructionName(unsigned Opcode);
  
  // Autogenerated by tblgen.
  void printInstruction(const MCInst *MI, raw_ostream &O);
  static const char *getRegisterName(unsigned RegNo);
  

  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printPredicateOperand(const MCInst *MI, unsigned OpNo,
                             raw_ostream &O, const char *Modifier);


  void printS5ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printU5ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printU6ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printS16ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printU16ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printS16X4ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printBranchOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printAbsAddrOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  void printcrbitm(const MCInst *MI, unsigned OpNo, raw_ostream &O);

  void printMemRegImm(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printMemRegImmShifted(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printMemRegReg(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  
  // FIXME: Remove
  void printSymbolLo(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printSymbolHi(const MCInst *MI, unsigned OpNo, raw_ostream &O);
};
} // end namespace llvm

#endif
