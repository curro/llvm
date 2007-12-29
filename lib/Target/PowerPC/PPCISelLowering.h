//===-- PPCISelLowering.h - PPC32 DAG Lowering Interface --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that PPC uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_POWERPC_PPC32ISELLOWERING_H
#define LLVM_TARGET_POWERPC_PPC32ISELLOWERING_H

#include "llvm/Target/TargetLowering.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "PPC.h"
#include "PPCSubtarget.h"

namespace llvm {
  namespace PPCISD {
    enum NodeType {
      // Start the numbering where the builtin ops and target ops leave off.
      FIRST_NUMBER = ISD::BUILTIN_OP_END+PPC::INSTRUCTION_LIST_END,

      /// FSEL - Traditional three-operand fsel node.
      ///
      FSEL,
      
      /// FCFID - The FCFID instruction, taking an f64 operand and producing
      /// and f64 value containing the FP representation of the integer that
      /// was temporarily in the f64 operand.
      FCFID,
      
      /// FCTI[D,W]Z - The FCTIDZ and FCTIWZ instructions, taking an f32 or f64 
      /// operand, producing an f64 value containing the integer representation
      /// of that FP value.
      FCTIDZ, FCTIWZ,
      
      /// STFIWX - The STFIWX instruction.  The first operand is an input token
      /// chain, then an f64 value to store, then an address to store it to,
      /// then a SRCVALUE for the address.
      STFIWX,
      
      // VMADDFP, VNMSUBFP - The VMADDFP and VNMSUBFP instructions, taking
      // three v4f32 operands and producing a v4f32 result.
      VMADDFP, VNMSUBFP,
      
      /// VPERM - The PPC VPERM Instruction.
      ///
      VPERM,
      
      /// Hi/Lo - These represent the high and low 16-bit parts of a global
      /// address respectively.  These nodes have two operands, the first of
      /// which must be a TargetGlobalAddress, and the second of which must be a
      /// Constant.  Selected naively, these turn into 'lis G+C' and 'li G+C',
      /// though these are usually folded into other nodes.
      Hi, Lo,
      
      /// OPRC, CHAIN = DYNALLOC(CHAIN, NEGSIZE, FRAME_INDEX)
      /// This instruction is lowered in PPCRegisterInfo::eliminateFrameIndex to
      /// compute an allocation on the stack.
      DYNALLOC,
      
      /// GlobalBaseReg - On Darwin, this node represents the result of the mflr
      /// at function entry, used for PIC code.
      GlobalBaseReg,
      
      /// These nodes represent the 32-bit PPC shifts that operate on 6-bit
      /// shift amounts.  These nodes are generated by the multi-precision shift
      /// code.
      SRL, SRA, SHL,
      
      /// EXTSW_32 - This is the EXTSW instruction for use with "32-bit"
      /// registers.
      EXTSW_32,

      /// STD_32 - This is the STD instruction for use with "32-bit" registers.
      STD_32,
      
      /// CALL - A direct function call.
      CALL_Macho, CALL_ELF,
      
      /// CHAIN,FLAG = MTCTR(VAL, CHAIN[, INFLAG]) - Directly corresponds to a
      /// MTCTR instruction.
      MTCTR,
      
      /// CHAIN,FLAG = BCTRL(CHAIN, INFLAG) - Directly corresponds to a
      /// BCTRL instruction.
      BCTRL_Macho, BCTRL_ELF,
      
      /// Return with a flag operand, matched by 'blr'
      RET_FLAG,
      
      /// R32 = MFCR(CRREG, INFLAG) - Represents the MFCR/MFOCRF instructions.
      /// This copies the bits corresponding to the specified CRREG into the
      /// resultant GPR.  Bits corresponding to other CR regs are undefined.
      MFCR,

      /// RESVEC = VCMP(LHS, RHS, OPC) - Represents one of the altivec VCMP*
      /// instructions.  For lack of better number, we use the opcode number
      /// encoding for the OPC field to identify the compare.  For example, 838
      /// is VCMPGTSH.
      VCMP,
      
      /// RESVEC, OUTFLAG = VCMPo(LHS, RHS, OPC) - Represents one of the
      /// altivec VCMP*o instructions.  For lack of better number, we use the 
      /// opcode number encoding for the OPC field to identify the compare.  For
      /// example, 838 is VCMPGTSH.
      VCMPo,
      
      /// CHAIN = COND_BRANCH CHAIN, CRRC, OPC, DESTBB [, INFLAG] - This
      /// corresponds to the COND_BRANCH pseudo instruction.  CRRC is the
      /// condition register to branch on, OPC is the branch opcode to use (e.g.
      /// PPC::BLE), DESTBB is the destination block to branch to, and INFLAG is
      /// an optional input flag argument.
      COND_BRANCH,
      
      /// CHAIN = STBRX CHAIN, GPRC, Ptr, SRCVALUE, Type - This is a 
      /// byte-swapping store instruction.  It byte-swaps the low "Type" bits of
      /// the GPRC input, then stores it through Ptr.  Type can be either i16 or
      /// i32.
      STBRX, 
      
      /// GPRC, CHAIN = LBRX CHAIN, Ptr, SRCVALUE, Type - This is a 
      /// byte-swapping load instruction.  It loads "Type" bits, byte swaps it,
      /// then puts it in the bottom bits of the GPRC.  TYPE can be either i16
      /// or i32.
      LBRX,

      // The following 5 instructions are used only as part of the
      // long double-to-int conversion sequence.

      /// OUTFLAG = MFFS F8RC - This moves the FPSCR (not modelled) into the
      /// register.
      MFFS,

      /// OUTFLAG = MTFSB0 INFLAG - This clears a bit in the FPSCR.
      MTFSB0,

      /// OUTFLAG = MTFSB1 INFLAG - This sets a bit in the FPSCR.
      MTFSB1,

      /// F8RC, OUTFLAG = FADDRTZ F8RC, F8RC, INFLAG - This is an FADD done with
      /// rounding towards zero.  It has flags added so it won't move past the 
      /// FPSCR-setting instructions.
      FADDRTZ,

      /// MTFSF = F8RC, INFLAG - This moves the register into the FPSCR.
      MTFSF
    };
  }

  /// Define some predicates that are used for node matching.
  namespace PPC {
    /// isVPKUHUMShuffleMask - Return true if this is the shuffle mask for a
    /// VPKUHUM instruction.
    bool isVPKUHUMShuffleMask(SDNode *N, bool isUnary);
    
    /// isVPKUWUMShuffleMask - Return true if this is the shuffle mask for a
    /// VPKUWUM instruction.
    bool isVPKUWUMShuffleMask(SDNode *N, bool isUnary);

    /// isVMRGLShuffleMask - Return true if this is a shuffle mask suitable for
    /// a VRGL* instruction with the specified unit size (1,2 or 4 bytes).
    bool isVMRGLShuffleMask(SDNode *N, unsigned UnitSize, bool isUnary);

    /// isVMRGHShuffleMask - Return true if this is a shuffle mask suitable for
    /// a VRGH* instruction with the specified unit size (1,2 or 4 bytes).
    bool isVMRGHShuffleMask(SDNode *N, unsigned UnitSize, bool isUnary);
    
    /// isVSLDOIShuffleMask - If this is a vsldoi shuffle mask, return the shift
    /// amount, otherwise return -1.
    int isVSLDOIShuffleMask(SDNode *N, bool isUnary);
    
    /// isSplatShuffleMask - Return true if the specified VECTOR_SHUFFLE operand
    /// specifies a splat of a single element that is suitable for input to
    /// VSPLTB/VSPLTH/VSPLTW.
    bool isSplatShuffleMask(SDNode *N, unsigned EltSize);
    
    /// isAllNegativeZeroVector - Returns true if all elements of build_vector
    /// are -0.0.
    bool isAllNegativeZeroVector(SDNode *N);

    /// getVSPLTImmediate - Return the appropriate VSPLT* immediate to splat the
    /// specified isSplatShuffleMask VECTOR_SHUFFLE mask.
    unsigned getVSPLTImmediate(SDNode *N, unsigned EltSize);
    
    /// get_VSPLTI_elt - If this is a build_vector of constants which can be
    /// formed by using a vspltis[bhw] instruction of the specified element
    /// size, return the constant being splatted.  The ByteSize field indicates
    /// the number of bytes of each element [124] -> [bhw].
    SDOperand get_VSPLTI_elt(SDNode *N, unsigned ByteSize, SelectionDAG &DAG);
  }
  
  class PPCTargetLowering : public TargetLowering {
    int VarArgsFrameIndex;            // FrameIndex for start of varargs area.
    int VarArgsStackOffset;           // StackOffset for start of stack
                                      // arguments.
    unsigned VarArgsNumGPR;           // Index of the first unused integer
                                      // register for parameter passing.
    unsigned VarArgsNumFPR;           // Index of the first unused double
                                      // register for parameter passing.
    int ReturnAddrIndex;              // FrameIndex for return slot.
    const PPCSubtarget &PPCSubTarget;
  public:
    explicit PPCTargetLowering(PPCTargetMachine &TM);
    
    /// getTargetNodeName() - This method returns the name of a target specific
    /// DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

    /// getPreIndexedAddressParts - returns true by value, base pointer and
    /// offset pointer and addressing mode by reference if the node's address
    /// can be legally represented as pre-indexed load / store address.
    virtual bool getPreIndexedAddressParts(SDNode *N, SDOperand &Base,
                                           SDOperand &Offset,
                                           ISD::MemIndexedMode &AM,
                                           SelectionDAG &DAG);
    
    /// SelectAddressRegReg - Given the specified addressed, check to see if it
    /// can be represented as an indexed [r+r] operation.  Returns false if it
    /// can be more efficiently represented with [r+imm].
    bool SelectAddressRegReg(SDOperand N, SDOperand &Base, SDOperand &Index,
                             SelectionDAG &DAG);
    
    /// SelectAddressRegImm - Returns true if the address N can be represented
    /// by a base register plus a signed 16-bit displacement [r+imm], and if it
    /// is not better represented as reg+reg.
    bool SelectAddressRegImm(SDOperand N, SDOperand &Disp, SDOperand &Base,
                             SelectionDAG &DAG);
    
    /// SelectAddressRegRegOnly - Given the specified addressed, force it to be
    /// represented as an indexed [r+r] operation.
    bool SelectAddressRegRegOnly(SDOperand N, SDOperand &Base, SDOperand &Index,
                                 SelectionDAG &DAG);

    /// SelectAddressRegImmShift - Returns true if the address N can be
    /// represented by a base register plus a signed 14-bit displacement
    /// [r+imm*4].  Suitable for use by STD and friends.
    bool SelectAddressRegImmShift(SDOperand N, SDOperand &Disp, SDOperand &Base,
                                  SelectionDAG &DAG);

    
    /// LowerOperation - Provide custom lowering hooks for some operations.
    ///
    virtual SDOperand LowerOperation(SDOperand Op, SelectionDAG &DAG);

    virtual SDNode *ExpandOperationResult(SDNode *N, SelectionDAG &DAG);
    
    virtual SDOperand PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const;
    
    virtual void computeMaskedBitsForTargetNode(const SDOperand Op,
                                                uint64_t Mask,
                                                uint64_t &KnownZero, 
                                                uint64_t &KnownOne,
                                                const SelectionDAG &DAG,
                                                unsigned Depth = 0) const;

    virtual MachineBasicBlock *InsertAtEndOfBasicBlock(MachineInstr *MI,
                                                       MachineBasicBlock *MBB);
    
    ConstraintType getConstraintType(const std::string &Constraint) const;
    std::pair<unsigned, const TargetRegisterClass*> 
      getRegForInlineAsmConstraint(const std::string &Constraint,
                                   MVT::ValueType VT) const;

    /// LowerAsmOperandForConstraint - Lower the specified operand into the Ops
    /// vector.  If it is invalid, don't add anything to Ops.
    virtual void LowerAsmOperandForConstraint(SDOperand Op,
                                              char ConstraintLetter,
                                              std::vector<SDOperand> &Ops,
                                              SelectionDAG &DAG);
    
    /// isLegalAddressingMode - Return true if the addressing mode represented
    /// by AM is legal for this target, for a load/store of the specified type.
    virtual bool isLegalAddressingMode(const AddrMode &AM, const Type *Ty)const;
    
    /// isLegalAddressImmediate - Return true if the integer value can be used
    /// as the offset of the target addressing mode for load / store of the
    /// given type.
    virtual bool isLegalAddressImmediate(int64_t V, const Type *Ty) const;

    /// isLegalAddressImmediate - Return true if the GlobalValue can be used as
    /// the offset of the target addressing mode.
    virtual bool isLegalAddressImmediate(GlobalValue *GV) const;

    SDOperand LowerRETURNADDR(SDOperand Op, SelectionDAG &DAG);
    SDOperand LowerFRAMEADDR(SDOperand Op, SelectionDAG &DAG);
  };
}

#endif   // LLVM_TARGET_POWERPC_PPC32ISELLOWERING_H
