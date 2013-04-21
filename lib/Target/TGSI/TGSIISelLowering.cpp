//===-- TGSIISelLowering.cpp - TGSI DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that TGSI uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "TGSIISelLowering.h"
#include "TGSITargetMachine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/ManagedStatic.h"
using namespace llvm;


//===----------------------------------------------------------------------===//
// Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "TGSIGenCallingConv.inc"

SDValue
TGSITargetLowering::LowerReturn(SDValue Chain,
                                CallingConv::ID CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                DebugLoc dl, SelectionDAG &DAG) const {
   SDValue Flag;

   // CCValAssign - represent the assignment of the return value to locations.
   SmallVector<CCValAssign, 16> RVLocs;

   // CCState - Info about the registers and stack slot.
   CCState ccinfo(CallConv, isVarArg, DAG.getMachineFunction(),
                  getTargetMachine(), RVLocs, *DAG.getContext());

   ccinfo.AnalyzeReturn(Outs, RetCC_TGSI);

   for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
      CCValAssign& VA = RVLocs[i];

      assert(VA.isRegLoc() && "CCValAssign must be RegLoc");

      unsigned Reg = VA.getLocReg();

      Chain = DAG.getCopyToReg(Chain, dl, Reg, OutVals[i], Flag);

      // Guarantee that all emitted copies are stuck together.
      Flag = Chain.getValue(1);
   }

   if (Flag.getNode())
      return DAG.getNode(TGSIISD::RET, dl, MVT::Other, Chain, Flag);
   else
      return DAG.getNode(TGSIISD::RET, dl, MVT::Other, Chain);
}

SDValue
TGSITargetLowering::LowerFormalArguments(SDValue Chain,
                                         CallingConv::ID CallConv, bool isVarArg,
                                         const SmallVectorImpl<ISD::InputArg> &Ins,
                                         DebugLoc dl, SelectionDAG &DAG,
                                         SmallVectorImpl<SDValue> &InVals) const {
   MachineFunction &mf = DAG.getMachineFunction();
   MachineRegisterInfo &reginfo = mf.getRegInfo();

   // Assign locations to all of the incoming arguments.
   SmallVector<CCValAssign, 16> ArgLocs;
   CCState ccinfo(CallConv, isVarArg, DAG.getMachineFunction(),
                  getTargetMachine(), ArgLocs, *DAG.getContext());
   CCAssignFn *ccfn = (isKernelFunction(mf.getFunction()) ? KernCC_TGSI : FunCC_TGSI);

   ccinfo.AnalyzeFormalArguments(Ins, ccfn);

   for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
      CCValAssign &va = ArgLocs[i];
      MVT vt = va.getValVT();
      SDValue arg;

      if (va.isRegLoc()) {
         unsigned vreg = reginfo.createVirtualRegister(getRegClassFor(vt));

         reginfo.addLiveIn(va.getLocReg(), vreg);
         arg = DAG.getCopyFromReg(Chain, dl, vreg, vt);

      } else {
         SDValue ptr = DAG.getConstant(va.getLocMemOffset(), MVT::i32);

         arg = DAG.getNode(TGSIISD::LOAD_INPUT, dl, vt, Chain, ptr);
      }

      InVals.push_back(arg);
   }

   return Chain;
}

SDValue
TGSITargetLowering::LowerCall(SDValue Chain, SDValue Callee,
                              CallingConv::ID CallConv, bool isVarArg,
                              bool &isTailCall,
                              const SmallVectorImpl<ISD::OutputArg> &Outs,
                              const SmallVectorImpl<SDValue> &OutVals,
                              const SmallVectorImpl<ISD::InputArg> &Ins,
                              DebugLoc dl, SelectionDAG &DAG,
                              SmallVectorImpl<SDValue> &InVals) const {
   SDValue InFlag;
   // Analyze operands of the call, assigning locations to each operand.
   SmallVector<CCValAssign, 16> ArgLocs;
   CCState ccinfo(CallConv, isVarArg, DAG.getMachineFunction(),
                  DAG.getTarget(), ArgLocs, *DAG.getContext());
   ccinfo.AnalyzeCallOperands(Outs, FunCC_TGSI);

   // Walk the register assignments, inserting copies.
   for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
      CCValAssign &VA = ArgLocs[i];
      SDValue Arg = OutVals[i];

      if (VA.getLocVT() == MVT::f32)
         Arg = DAG.getNode(ISD::BITCAST, dl, MVT::i32, Arg);

      Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), Arg, InFlag);
      InFlag = Chain.getValue(1);
   }

   // Returns a chain & a flag for retval copy to use
   SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
   SmallVector<SDValue, 8> Ops;
   Ops.push_back(Chain);
   Ops.push_back(Callee);
   for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
      unsigned Reg = ArgLocs[i].getLocReg();

      Ops.push_back(DAG.getRegister(Reg, ArgLocs[i].getLocVT()));
   }
   if (InFlag.getNode())
      Ops.push_back(InFlag);

   Chain = DAG.getNode(TGSIISD::CALL, dl, NodeTys, &Ops[0], Ops.size());
   InFlag = Chain.getValue(1);

   // Assign locations to each value returned by this call.
   SmallVector<CCValAssign, 16> RVLocs;
   CCState RVInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                  DAG.getTarget(), RVLocs, *DAG.getContext());

   RVInfo.AnalyzeCallResult(Ins, RetCC_TGSI);

   // Copy all of the result registers out of their specified physreg.
   for (unsigned i = 0; i != RVLocs.size(); ++i) {
      unsigned Reg = RVLocs[i].getLocReg();

      Chain = DAG.getCopyFromReg(Chain, dl, Reg,
                                 RVLocs[i].getValVT(), InFlag).getValue(1);
      InFlag = Chain.getValue(2);
      InVals.push_back(Chain.getValue(0));
   }

   isTailCall = false;
   return Chain;
}

//===----------------------------------------------------------------------===//
// TargetLowering Implementation
//===----------------------------------------------------------------------===//

TGSITargetLowering::TGSITargetLowering(TargetMachine &TM)
   : TargetLowering(TM, new TargetLoweringObjectFileELF()) {

   // Set up the register classes.
   addRegisterClass(MVT::i32, &TGSI::IRegsRegClass);
   addRegisterClass(MVT::v4i32, &TGSI::IVRegsRegClass);
   addRegisterClass(MVT::f32, &TGSI::FRegsRegClass);
   addRegisterClass(MVT::v4f32, &TGSI::FVRegsRegClass);

   setStackPointerRegisterToSaveRestore(TGSI::TEMP0);
   computeRegisterProperties();

   setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
   setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
   setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
   setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
   setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
   setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
   setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
   setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
   setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
   setOperationAction(ISD::BR_CC, MVT::i32, Expand);
   setOperationAction(ISD::BR_CC, MVT::f32, Expand);
   setOperationAction(ISD::BR_CC, MVT::f64, Expand);
   setOperationAction(ISD::BR_CC, MVT::Other, Expand);
   setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
   setOperationAction(ISD::SELECT_CC, MVT::Other, Expand);

   setOperationAction(ISD::ConstantFP, MVT::f32, Legal);
   setOperationAction(ISD::FGETSIGN, MVT::f32, Legal);
   setOperationAction(ISD::FLOG2, MVT::f32, Legal);
   setOperationAction(ISD::FEXP2, MVT::f32, Legal);

   setOperationAction(ISD::BUILD_VECTOR, MVT::v4i32, Expand);
   setOperationAction(ISD::BUILD_VECTOR, MVT::v4f32, Expand);
   setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4i32, Expand);
   setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4f32, Expand);
}

const char *TGSITargetLowering::getTargetNodeName(unsigned Opcode) const {
   switch (Opcode) {
      case TGSIISD::LOAD_INPUT:
         return "TGSIISD::LOAD_INPUT";
      case TGSIISD::CALL:
         return "TGSIISD::CALL";
      case TGSIISD::RET:
         return "TGSIISD::RET";
      default:
         llvm_unreachable("Invalid opcode");
   }
}

SDValue TGSITargetLowering::
LowerOperation(SDValue op, SelectionDAG &dag) const {
   switch (op.getOpcode()) {
      default:
         llvm_unreachable("Should not custom lower this!");
   };
}
