##===- lib/Target/TGSI/Makefile ---------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMTGSICodeGen
TARGET = TGSI

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = TGSIGenRegisterInfo.inc TGSIGenInstrInfo.inc \
		TGSIGenAsmWriter.inc TGSIGenDAGISel.inc \
		TGSIGenSubtargetInfo.inc TGSIGenCallingConv.inc

DIRS = TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

