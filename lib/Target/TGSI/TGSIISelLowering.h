//===-- TGSIISelLowering.h - TGSI DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that TGSI uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_ISEL_LOWERING_H
#define TGSI_ISEL_LOWERING_H

#include "llvm/Target/TargetLowering.h"
#include "TGSI.h"

namespace llvm {
   namespace TGSIISD {
      enum {
         FIRST_NUMBER = ISD::BUILTIN_OP_END,
         LOAD_INPUT,
         CALL,
         RET
      };
   }

   class TGSITargetLowering : public TargetLowering {
   public:
      TGSITargetLowering(TargetMachine &TM);
      virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

      virtual const char *getTargetNodeName(unsigned Opcode) const;

      virtual SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv,
                           bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

      virtual SDValue
      LowerCall(SDValue Chain, SDValue Callee,
                CallingConv::ID CallConv, bool isVarArg,
                bool &isTailCall,
                const SmallVectorImpl<ISD::OutputArg> &Outs,
                const SmallVectorImpl<SDValue> &OutVals,
                const SmallVectorImpl<ISD::InputArg> &Ins,
                DebugLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals) const;

      virtual SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  DebugLoc dl, SelectionDAG &DAG) const;
   };
}

#endif
