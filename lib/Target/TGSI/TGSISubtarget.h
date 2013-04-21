//=====-- TGSISubtarget.h - Define Subtarget for the TGSI ----*- C++ -*-====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TGSI specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_SUBTARGET_H
#define TGSI_SUBTARGET_H

#include "llvm/Target/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "TGSIGenSubtargetInfo.inc"

namespace llvm {
   class StringRef;

   class TGSISubtarget : public TGSIGenSubtargetInfo {
   public:
      TGSISubtarget(const std::string &TT, const std::string &CPU,
                    const std::string &FS);

      void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

      std::string getDataLayout() const {
         return std::string("E-p:32:32-i64:64:64-f64:64:64-f128:128:128-n32");
      }
   };

}

#endif
