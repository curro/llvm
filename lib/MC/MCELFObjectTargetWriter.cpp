//===-- MCELFObjectTargetWriter.cpp - ELF Target Writer Subclass ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCELFObjectWriter.h"

using namespace llvm;

MCELFObjectTargetWriter::MCELFObjectTargetWriter(bool Is64Bit_,
                                                 uint8_t OSABI_,
                                                 uint16_t EMachine_,
                                                 bool HasRelocationAddend_)
  : OSABI(OSABI_), EMachine(EMachine_),
    HasRelocationAddend(HasRelocationAddend_), Is64Bit(Is64Bit_) {
}

MCELFObjectTargetWriter::~MCELFObjectTargetWriter() {
}
