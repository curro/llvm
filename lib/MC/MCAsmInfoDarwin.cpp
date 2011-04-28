//===-- MCAsmInfoDarwin.cpp - Darwin asm properties -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines target asm properties related what form asm statements
// should take in general on Darwin-based targets
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAsmInfoDarwin.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
using namespace llvm;

MCAsmInfoDarwin::MCAsmInfoDarwin() {
  // Common settings for all Darwin targets.
  // Syntax:
  GlobalPrefix = "_";
  PrivateGlobalPrefix = "L";
  LinkerPrivateGlobalPrefix = "l";
  AllowQuotesInName = true;
  HasSingleParameterDotFile = false;
  HasSubsectionsViaSymbols = true;

  AlignmentIsInBytes = false;
  COMMDirectiveAlignmentIsInBytes = false;
  InlineAsmStart = " InlineAsm Start";
  InlineAsmEnd = " InlineAsm End";

  // Directives:
  WeakDefDirective = "\t.weak_definition ";
  WeakRefDirective = "\t.weak_reference ";
  ZeroDirective = "\t.space\t";  // ".space N" emits N zeros.
  HasMachoZeroFillDirective = true;  // Uses .zerofill
  HasMachoTBSSDirective = true; // Uses .tbss
  HasStaticCtorDtorReferenceInStaticMode = true;

  // FIXME: Darwin 10 and newer don't need this.
  LinkerRequiresNonEmptyDwarfLines = true;

  // FIXME: Change this once MC is the system assembler.
  HasAggressiveSymbolFolding = false;

  HiddenVisibilityAttr = MCSA_PrivateExtern;
  HiddenDeclarationVisibilityAttr = MCSA_Invalid;
  // Doesn't support protected visibility.
  ProtectedVisibilityAttr = MCSA_Global;
  
  HasDotTypeDotSizeDirective = false;
  HasNoDeadStrip = true;
  HasSymbolResolver = true;

  DwarfUsesAbsoluteLabelForStmtList = false;
  DwarfUsesLabelOffsetForRanges = false;
}

const MCExpr *
MCAsmInfoDarwin::getExprForPersonalitySymbol(const MCSymbol *Sym,
                                             MCStreamer &Streamer) const {
  MCContext &Context = Streamer.getContext();
  const MCExpr *Res = MCSymbolRefExpr::Create(Sym, Context);
  MCSymbol *PCSym = Context.CreateTempSymbol();
  Streamer.EmitLabel(PCSym);
  const MCExpr *PC = MCSymbolRefExpr::Create(PCSym, Context);
  return MCBinaryExpr::CreateSub(Res, PC, Context);
}
