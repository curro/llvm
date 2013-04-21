//===- TGSI.td - Describe the TGSI Target Machine --------*- tablegen -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "TGSIRegisterInfo.td"
include "TGSICallingConv.td"
include "TGSIInstrInfo.td"

def TGSIInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// TGSI processors supported.
//===----------------------------------------------------------------------===//

def : Processor<"generic", NoItineraries, []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def TGSIAsmWriter : AsmWriter {
  string AsmWriterClassName  = "MCInstPrinter";
  bit isMCAsmWriter = 1;
}

def TGSI : Target {
  let InstructionSet = TGSIInstrInfo;
  let AssemblyWriters = [TGSIAsmWriter];
}
