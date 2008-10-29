//===-- ARM/ARMCodeEmitter.cpp - Convert ARM code to machine code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the pass that transforms the ARM machine instructions into
// relocatable machine code.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "jit"
#include "ARM.h"
#include "ARMAddressingModes.h"
#include "ARMConstantPoolValue.h"
#include "ARMInstrInfo.h"
#include "ARMRelocations.h"
#include "ARMSubtarget.h"
#include "ARMTargetMachine.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/MachineCodeEmitter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

STATISTIC(NumEmitted, "Number of machine instructions emitted");

namespace {
  class VISIBILITY_HIDDEN ARMCodeEmitter : public MachineFunctionPass {
    ARMJITInfo                *JTI;
    const ARMInstrInfo        *II;
    const TargetData          *TD;
    TargetMachine             &TM;
    MachineCodeEmitter        &MCE;
    const MachineConstantPool *MCP;
  public:
    static char ID;
    explicit ARMCodeEmitter(TargetMachine &tm, MachineCodeEmitter &mce)
      : MachineFunctionPass(&ID), JTI(0), II(0), TD(0), TM(tm),
      MCE(mce), MCP(0) {}
    ARMCodeEmitter(TargetMachine &tm, MachineCodeEmitter &mce,
            const ARMInstrInfo &ii, const TargetData &td)
      : MachineFunctionPass(&ID), JTI(0), II(&ii), TD(&td), TM(tm),
      MCE(mce), MCP(0) {}

    bool runOnMachineFunction(MachineFunction &MF);

    virtual const char *getPassName() const {
      return "ARM Machine Code Emitter";
    }

    void emitInstruction(const MachineInstr &MI);

  private:

    void emitConstPoolInstruction(const MachineInstr &MI);

    void emitPseudoInstruction(const MachineInstr &MI);

    unsigned getAddrModeNoneInstrBinary(const MachineInstr &MI,
                                        const TargetInstrDesc &TID,
                                        unsigned Binary);

    unsigned getMachineSoRegOpValue(const MachineInstr &MI,
                                    const TargetInstrDesc &TID,
                                    unsigned OpIdx);

    unsigned getAddrMode1SBit(const MachineInstr &MI,
                              const TargetInstrDesc &TID) const;

    unsigned getAddrMode1InstrBinary(const MachineInstr &MI,
                                     const TargetInstrDesc &TID,
                                     unsigned Binary);
    unsigned getAddrMode2InstrBinary(const MachineInstr &MI,
                                     const TargetInstrDesc &TID,
                                     unsigned Binary);
    unsigned getAddrMode3InstrBinary(const MachineInstr &MI,
                                     const TargetInstrDesc &TID,
                                     unsigned Binary);
    unsigned getAddrMode4InstrBinary(const MachineInstr &MI,
                                     const TargetInstrDesc &TID,
                                     unsigned Binary);

    /// getInstrBinary - Return binary encoding for the specified
    /// machine instruction.
    unsigned getInstrBinary(const MachineInstr &MI);

    /// getBinaryCodeForInstr - This function, generated by the
    /// CodeEmitterGenerator using TableGen, produces the binary encoding for
    /// machine instructions.
    ///
    unsigned getBinaryCodeForInstr(const MachineInstr &MI);

    /// getMachineOpValue - Return binary encoding of operand. If the machine
    /// operand requires relocation, record the relocation and return zero.
    unsigned getMachineOpValue(const MachineInstr &MI, unsigned OpIdx) {
      return getMachineOpValue(MI, MI.getOperand(OpIdx));
    }
    unsigned getMachineOpValue(const MachineInstr &MI,
                               const MachineOperand &MO);

    /// getBaseOpcodeFor - Return the opcode value.
    ///
    unsigned getBaseOpcodeFor(const TargetInstrDesc &TID) const {
      return (TID.TSFlags & ARMII::OpcodeMask) >> ARMII::OpcodeShift;
    }

    /// getShiftOp - Return the shift opcode (bit[6:5]) of the machine operand.
    ///
    unsigned getShiftOp(const MachineOperand &MO) const ;

    /// Routines that handle operands which add machine relocations which are
    /// fixed up by the JIT fixup stage.
    void emitGlobalAddress(GlobalValue *GV, unsigned Reloc,
                           bool NeedStub);
    void emitExternalSymbolAddress(const char *ES, unsigned Reloc);
    void emitConstPoolAddress(unsigned CPI, unsigned Reloc,
                              int Disp = 0, unsigned PCAdj = 0 );
    void emitJumpTableAddress(unsigned JTIndex, unsigned Reloc,
                              unsigned PCAdj = 0);
    void emitGlobalConstant(const Constant *CV);
    void emitMachineBasicBlock(MachineBasicBlock *BB);
  };
  char ARMCodeEmitter::ID = 0;
}

/// createARMCodeEmitterPass - Return a pass that emits the collected ARM code
/// to the specified MCE object.
FunctionPass *llvm::createARMCodeEmitterPass(ARMTargetMachine &TM,
                                             MachineCodeEmitter &MCE) {
  return new ARMCodeEmitter(TM, MCE);
}

bool ARMCodeEmitter::runOnMachineFunction(MachineFunction &MF) {
  assert((MF.getTarget().getRelocationModel() != Reloc::Default ||
          MF.getTarget().getRelocationModel() != Reloc::Static) &&
         "JIT relocation model must be set to static or default!");
  II = ((ARMTargetMachine&)MF.getTarget()).getInstrInfo();
  TD = ((ARMTargetMachine&)MF.getTarget()).getTargetData();
  JTI = ((ARMTargetMachine&)MF.getTarget()).getJITInfo();
  MCP = MF.getConstantPool();

  do {
    DOUT << "JITTing function '" << MF.getFunction()->getName() << "'\n";
    MCE.startFunction(MF);
    for (MachineFunction::iterator MBB = MF.begin(), E = MF.end(); 
         MBB != E; ++MBB) {
      MCE.StartMachineBasicBlock(MBB);
      for (MachineBasicBlock::const_iterator I = MBB->begin(), E = MBB->end();
           I != E; ++I)
        emitInstruction(*I);
    }
  } while (MCE.finishFunction(MF));

  return false;
}

/// getShiftOp - Return the shift opcode (bit[6:5]) of the machine operand.
///
unsigned ARMCodeEmitter::getShiftOp(const MachineOperand &MO) const {
  switch (ARM_AM::getAM2ShiftOpc(MO.getImm())) {
  default: assert(0 && "Unknown shift opc!");
  case ARM_AM::asr: return 2;
  case ARM_AM::lsl: return 0;
  case ARM_AM::lsr: return 1;
  case ARM_AM::ror:
  case ARM_AM::rrx: return 3;
  }
  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned ARMCodeEmitter::getMachineOpValue(const MachineInstr &MI,
                                           const MachineOperand &MO) {
  if (MO.isReg())
    return ARMRegisterInfo::getRegisterNumbering(MO.getReg());
  else if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());
  else if (MO.isGlobal())
    emitGlobalAddress(MO.getGlobal(), ARM::reloc_arm_branch, true);
  else if (MO.isSymbol())
    emitExternalSymbolAddress(MO.getSymbolName(), ARM::reloc_arm_relative);
  else if (MO.isCPI())
    emitConstPoolAddress(MO.getIndex(), ARM::reloc_arm_cp_entry);
  else if (MO.isJTI())
    emitJumpTableAddress(MO.getIndex(), ARM::reloc_arm_relative);
  else if (MO.isMBB())
    emitMachineBasicBlock(MO.getMBB());
  else {
    cerr << "ERROR: Unknown type of MachineOperand: " << MO << "\n";
    abort();
  }
  return 0;
}

/// emitGlobalAddress - Emit the specified address to the code stream.
///
void ARMCodeEmitter::emitGlobalAddress(GlobalValue *GV,
                                       unsigned Reloc, bool NeedStub) {
  MCE.addRelocation(MachineRelocation::getGV(MCE.getCurrentPCOffset(),
                                             Reloc, GV, 0, NeedStub));
}

/// emitExternalSymbolAddress - Arrange for the address of an external symbol to
/// be emitted to the current location in the function, and allow it to be PC
/// relative.
void ARMCodeEmitter::emitExternalSymbolAddress(const char *ES, unsigned Reloc) {
  MCE.addRelocation(MachineRelocation::getExtSym(MCE.getCurrentPCOffset(),
                                                 Reloc, ES));
}

/// emitConstPoolAddress - Arrange for the address of an constant pool
/// to be emitted to the current location in the function, and allow it to be PC
/// relative.
void ARMCodeEmitter::emitConstPoolAddress(unsigned CPI, unsigned Reloc,
                                          int Disp /* = 0 */,
                                          unsigned PCAdj /* = 0 */) {
  // Tell JIT emitter we'll resolve the address.
  MCE.addRelocation(MachineRelocation::getConstPool(MCE.getCurrentPCOffset(),
                                                    Reloc, CPI, PCAdj, true));
}

/// emitJumpTableAddress - Arrange for the address of a jump table to
/// be emitted to the current location in the function, and allow it to be PC
/// relative.
void ARMCodeEmitter::emitJumpTableAddress(unsigned JTIndex, unsigned Reloc,
                                          unsigned PCAdj /* = 0 */) {
  MCE.addRelocation(MachineRelocation::getJumpTable(MCE.getCurrentPCOffset(),
                                                    Reloc, JTIndex, PCAdj));
}

/// emitMachineBasicBlock - Emit the specified address basic block.
void ARMCodeEmitter::emitMachineBasicBlock(MachineBasicBlock *BB) {
  MCE.addRelocation(MachineRelocation::getBB(MCE.getCurrentPCOffset(),
                                             ARM::reloc_arm_branch, BB));
}

void ARMCodeEmitter::emitInstruction(const MachineInstr &MI) {
  DOUT << "JIT: " << "0x" << MCE.getCurrentPCValue() << ":\t" << MI;

  NumEmitted++;  // Keep track of the # of mi's emitted
  if ((MI.getDesc().TSFlags & ARMII::FormMask) == ARMII::Pseudo)
    emitPseudoInstruction(MI);
  else
    MCE.emitWordLE(getInstrBinary(MI));
}

unsigned ARMCodeEmitter::getAddrModeNoneInstrBinary(const MachineInstr &MI,
                                                    const TargetInstrDesc &TID,
                                                    unsigned Binary) {
  // Set the conditional execution predicate
  Binary |= II->getPredicate(&MI) << 28;

  switch (TID.TSFlags & ARMII::FormMask) {
  default:
    assert(0 && "Unknown instruction subtype!");
    break;
  case ARMII::Branch: {
    // Set signed_immed_24 field
    Binary |= getMachineOpValue(MI, 0);

    // if it is a conditional branch, set cond field
    if (TID.Opcode == ARM::Bcc) {
      Binary &= 0x0FFFFFFF;                      // clear conditional field
      Binary |= getMachineOpValue(MI, 1) << 28;  // set conditional field
    }
    break;
  }
  case ARMII::BranchMisc: {
    if (TID.Opcode == ARM::BX)
      abort(); // FIXME
    if (TID.Opcode == ARM::BX_RET)
      Binary |= 0xe; // the return register is LR
    else 
      // otherwise, set the return register
      Binary |= getMachineOpValue(MI, 0);
    break;
  }
  }

  return Binary;
}

unsigned ARMCodeEmitter::getMachineSoRegOpValue(const MachineInstr &MI,
                                                const TargetInstrDesc &TID,
                                                unsigned OpIdx) {
  // Set last operand (register Rm)
  unsigned Binary = getMachineOpValue(MI, OpIdx);

  const MachineOperand &MO1 = MI.getOperand(OpIdx + 1);
  const MachineOperand &MO2 = MI.getOperand(OpIdx + 2);
  ARM_AM::ShiftOpc SOpc = ARM_AM::getSORegShOp(MO2.getImm());

  // Encode the shift opcode.
  unsigned SBits = 0;
  unsigned Rs = MO1.getReg();
  if (Rs) {
    // Set shift operand (bit[7:4]).
    // LSL - 0001
    // LSR - 0011
    // ASR - 0101
    // ROR - 0111
    // RRX - 0110 and bit[11:8] clear.
    switch (SOpc) {
    default: assert(0 && "Unknown shift opc!");
    case ARM_AM::lsl: SBits = 0x1; break;
    case ARM_AM::lsr: SBits = 0x3; break;
    case ARM_AM::asr: SBits = 0x5; break;
    case ARM_AM::ror: SBits = 0x7; break;
    case ARM_AM::rrx: SBits = 0x6; break;
    }
  } else {
    // Set shift operand (bit[6:4]).
    // LSL - 000
    // LSR - 010
    // ASR - 100
    // ROR - 110
    switch (SOpc) {
    default: assert(0 && "Unknown shift opc!");
    case ARM_AM::lsl: SBits = 0x0; break;
    case ARM_AM::lsr: SBits = 0x2; break;
    case ARM_AM::asr: SBits = 0x4; break;
    case ARM_AM::ror: SBits = 0x6; break;
    }
  }
  Binary |= SBits << 4;
  if (SOpc == ARM_AM::rrx)
    return Binary;

  // Encode the shift operation Rs or shift_imm (except rrx).
  if (Rs) {
    // Encode Rs bit[11:8].
    assert(ARM_AM::getSORegOffset(MO2.getImm()) == 0);
    return Binary |
      (ARMRegisterInfo::getRegisterNumbering(Rs) << ARMII::RegRsShift);
  }

  // Encode shift_imm bit[11:7].
  return Binary | ARM_AM::getSORegOffset(MO2.getImm()) << 7;
}

unsigned ARMCodeEmitter::getAddrMode1SBit(const MachineInstr &MI,
                                          const TargetInstrDesc &TID) const {
  for (unsigned i = MI.getNumOperands(), e = TID.getNumOperands(); i != e; --i){
    const MachineOperand &MO = MI.getOperand(i-1);
    if (MO.isReg() && MO.isDef() && MO.getReg() == ARM::CPSR)
      return 1 << ARMII::S_BitShift;
  }
  return 0;
}

void ARMCodeEmitter::emitConstPoolInstruction(const MachineInstr &MI) {
  unsigned CPI = MI.getOperand(0).getImm();
  unsigned CPIndex = MI.getOperand(1).getIndex();
  const MachineConstantPoolEntry &MCPE = MCP->getConstants()[CPIndex];
  
  // Remember the CONSTPOOL_ENTRY address for later relocation.
  JTI->addConstantPoolEntryAddr(CPI, MCE.getCurrentPCValue());

  // Emit constpool island entry. In most cases, the actual values will be
  // resolved and relocated after code emission.
  if (MCPE.isMachineConstantPoolEntry()) {
    ARMConstantPoolValue *ACPV =
      static_cast<ARMConstantPoolValue*>(MCPE.Val.MachineCPVal);

    DOUT << "\t** ARM constant pool #" << CPI << ", ' @ "
         << (void*)MCE.getCurrentPCValue() << *ACPV << '\n';

    GlobalValue *GV = ACPV->getGV();
    if (GV) {
      assert(!ACPV->isStub() && "Don't know how to deal this yet!");
      emitGlobalAddress(GV, ARM::reloc_arm_absolute, false);
    } else  {
      assert(!ACPV->isNonLazyPointer() && "Don't know how to deal this yet!");
      emitExternalSymbolAddress(ACPV->getSymbol(), ARM::reloc_arm_absolute);
    }
    MCE.emitWordLE(0);
  } else {
    Constant *CV = MCPE.Val.ConstVal;

    DOUT << "\t** Constant pool #" << CPI << ", ' @ "
         << (void*)MCE.getCurrentPCValue() << *CV << '\n';

    if (GlobalValue *GV = dyn_cast<GlobalValue>(CV)) {
      emitGlobalAddress(GV, ARM::reloc_arm_absolute, false);
      MCE.emitWordLE(0);
    } else {
      abort(); // FIXME: Is this right?
      const ConstantInt *CI = dyn_cast<ConstantInt>(CV);
      uint32_t Val = *(uint32_t*)CI->getValue().getRawData();
      MCE.emitWordLE(Val);
    }
  }
}

void ARMCodeEmitter::emitPseudoInstruction(const MachineInstr &MI) {
  unsigned Opcode = MI.getDesc().Opcode;
  switch (Opcode) {
  default:
    abort(); // FIXME:
  case ARM::CONSTPOOL_ENTRY: {
    emitConstPoolInstruction(MI);
    break;
  }
  }
}

unsigned ARMCodeEmitter::getAddrMode1InstrBinary(const MachineInstr &MI,
                                                 const TargetInstrDesc &TID,
                                                 unsigned Binary) {
  // Set the conditional execution predicate
  Binary |= II->getPredicate(&MI) << 28;

  // Encode S bit if MI modifies CPSR.
  Binary |= getAddrMode1SBit(MI, TID);

  // Encode register def if there is one.
  unsigned NumDefs = TID.getNumDefs();
  unsigned OpIdx = 0;
  if (NumDefs) {
    Binary |= getMachineOpValue(MI, OpIdx) << ARMII::RegRdShift;
    ++OpIdx;
  }

  // Encode first non-shifter register operand if there is one.
  unsigned Format = TID.TSFlags & ARMII::FormMask;
  bool hasRnOperand= !(Format == ARMII::DPRdMisc  ||
                       Format == ARMII::DPRdIm    ||
                       Format == ARMII::DPRdReg   ||
                       Format == ARMII::DPRdSoReg);
  if (hasRnOperand) {
    Binary |= getMachineOpValue(MI, OpIdx) << ARMII::RegRnShift;
    ++OpIdx;
  }

  // Encode shifter operand.
  bool HasSoReg = (Format == ARMII::DPRdSoReg ||
                   Format == ARMII::DPRnSoReg ||
                   Format == ARMII::DPRSoReg  ||
                   Format == ARMII::DPRSoRegS);
  if (HasSoReg)
    // Encode SoReg.
    return Binary | getMachineSoRegOpValue(MI, TID, OpIdx);

  const MachineOperand &MO = MI.getOperand(OpIdx);
  if (MO.isReg())
    // Encode register Rm.
    return Binary | getMachineOpValue(MI, NumDefs);

  // Encode so_imm.
  // Set bit I(25) to identify this is the immediate form of <shifter_op>
  Binary |= 1 << ARMII::I_BitShift;
  unsigned SoImm = MO.getImm();
  // Encode rotate_imm.
  Binary |= ARM_AM::getSOImmValRot(SoImm) << ARMII::RotImmShift;
  // Encode immed_8.
  Binary |= ARM_AM::getSOImmVal(SoImm);
  return Binary;
}

unsigned ARMCodeEmitter::getAddrMode2InstrBinary(const MachineInstr &MI,
                                                 const TargetInstrDesc &TID,
                                                 unsigned Binary) {
  // Set the conditional execution predicate
  Binary |= II->getPredicate(&MI) << 28;

  // Set first operand
  Binary |= getMachineOpValue(MI, 0) << ARMII::RegRdShift;

  // Set second operand
  Binary |= getMachineOpValue(MI, 1) << ARMII::RegRnShift;

  const MachineOperand &MO2 = MI.getOperand(2);
  const MachineOperand &MO3 = MI.getOperand(3);

  // Set bit U(23) according to sign of immed value (positive or negative).
  Binary |= ((ARM_AM::getAM2Op(MO3.getImm()) == ARM_AM::add ? 1 : 0) <<
             ARMII::U_BitShift);
  if (!MO2.getReg()) { // is immediate
    if (ARM_AM::getAM2Offset(MO3.getImm()))
      // Set the value of offset_12 field
      Binary |= ARM_AM::getAM2Offset(MO3.getImm());
    return Binary;
  }

  // Set bit I(25), because this is not in immediate enconding.
  Binary |= 1 << ARMII::I_BitShift;
  assert(TargetRegisterInfo::isPhysicalRegister(MO2.getReg()));
  // Set bit[3:0] to the corresponding Rm register
  Binary |= ARMRegisterInfo::getRegisterNumbering(MO2.getReg());

  // if this instr is in scaled register offset/index instruction, set
  // shift_immed(bit[11:7]) and shift(bit[6:5]) fields.
  if (unsigned ShImm = ARM_AM::getAM2Offset(MO3.getImm())) {
    Binary |= getShiftOp(MO3) << 5;  // shift
    Binary |= ShImm           << 7;  // shift_immed
  }

  return Binary;
}

unsigned ARMCodeEmitter::getAddrMode3InstrBinary(const MachineInstr &MI,
                                                 const TargetInstrDesc &TID,
                                                 unsigned Binary) {
  // Set the conditional execution predicate
  Binary |= II->getPredicate(&MI) << 28;

  // Set first operand
  Binary |= getMachineOpValue(MI, 0) << ARMII::RegRdShift;

  // Set second operand
  Binary |= getMachineOpValue(MI, 1) << ARMII::RegRnShift;

  const MachineOperand &MO2 = MI.getOperand(2);
  const MachineOperand &MO3 = MI.getOperand(3);

  // Set bit U(23) according to sign of immed value (positive or negative)
  Binary |= ((ARM_AM::getAM2Op(MO3.getImm()) == ARM_AM::add ? 1 : 0) <<
             ARMII::U_BitShift);

  // If this instr is in register offset/index encoding, set bit[3:0]
  // to the corresponding Rm register.
  if (MO2.getReg()) {
    Binary |= ARMRegisterInfo::getRegisterNumbering(MO2.getReg());
    return Binary;
  }

  // if this instr is in immediate offset/index encoding, set bit 22 to 1
  if (unsigned ImmOffs = ARM_AM::getAM3Offset(MO3.getImm())) {
    Binary |= 1 << 22;
    // Set operands
    Binary |= (ImmOffs >> 4) << 8;  // immedH
    Binary |= (ImmOffs & ~0xF);     // immedL
  }

  return Binary;
}

unsigned ARMCodeEmitter::getAddrMode4InstrBinary(const MachineInstr &MI,
                                                 const TargetInstrDesc &TID,
                                                 unsigned Binary) {
  // Set the conditional execution predicate
  Binary |= II->getPredicate(&MI) << 28;

  // Set first operand
  Binary |= getMachineOpValue(MI, 0) << ARMII::RegRnShift;

  // Set addressing mode by modifying bits U(23) and P(24)
  // IA - Increment after  - bit U = 1 and bit P = 0
  // IB - Increment before - bit U = 1 and bit P = 1
  // DA - Decrement after  - bit U = 0 and bit P = 0
  // DB - Decrement before - bit U = 0 and bit P = 1
  const MachineOperand &MO = MI.getOperand(1);
  ARM_AM::AMSubMode Mode = ARM_AM::getAM4SubMode(MO.getImm());
  switch (Mode) {
  default: assert(0 && "Unknown addressing sub-mode!");
  case ARM_AM::da:                      break;
  case ARM_AM::db: Binary |= 0x1 << 24; break;
  case ARM_AM::ia: Binary |= 0x1 << 23; break;
  case ARM_AM::ib: Binary |= 0x3 << 23; break;
  }

  // Set bit W(21)
  if (ARM_AM::getAM4WBFlag(MO.getImm()))
    Binary |= 0x1 << 21;

  // Set registers
  for (unsigned i = 4, e = MI.getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI.getOperand(i);
    if (MO.isReg() && MO.isImplicit())
      continue;
    unsigned RegNum = ARMRegisterInfo::getRegisterNumbering(MO.getReg());
    assert(TargetRegisterInfo::isPhysicalRegister(MO.getReg()) &&
           RegNum < 16);
    Binary |= 0x1 << RegNum;
  }

  return Binary;
}

/// getInstrBinary - Return binary encoding for the specified
/// machine instruction.
unsigned ARMCodeEmitter::getInstrBinary(const MachineInstr &MI) {
  // Part of binary is determined by TableGn.
  unsigned Binary = getBinaryCodeForInstr(MI);

  const TargetInstrDesc &TID = MI.getDesc();
  switch (TID.TSFlags & ARMII::AddrModeMask) {
  case ARMII::AddrModeNone:
    return getAddrModeNoneInstrBinary(MI, TID, Binary);
  case ARMII::AddrMode1:
    return getAddrMode1InstrBinary(MI, TID, Binary);
  case ARMII::AddrMode2:
    return getAddrMode2InstrBinary(MI, TID, Binary);
  case ARMII::AddrMode3:
    return getAddrMode3InstrBinary(MI, TID, Binary);
  case ARMII::AddrMode4:
    return getAddrMode4InstrBinary(MI, TID, Binary);
  }

  abort();
  return 0;
}

#include "ARMGenCodeEmitter.inc"
