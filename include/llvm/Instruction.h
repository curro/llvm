//===-- llvm/Instruction.h - Instruction class definition -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Instruction class, which is the
// base class for all of the LLVM instructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_INSTRUCTION_H
#define LLVM_INSTRUCTION_H

#include "llvm/User.h"

namespace llvm {

struct AssemblyAnnotationWriter;
class BinaryOperator;

template<typename SC> struct ilist_traits;
template<typename ValueSubClass, typename ItemParentClass, typename SymTabClass,
         typename SubClass> class SymbolTableListTraits;

class Instruction : public User {
  void operator=(const Instruction &);     // Do not implement
  Instruction(const Instruction &);        // Do not implement

  BasicBlock *Parent;
  Instruction *Prev, *Next; // Next and Prev links for our intrusive linked list

  void setNext(Instruction *N) { Next = N; }
  void setPrev(Instruction *N) { Prev = N; }

  friend class SymbolTableListTraits<Instruction, BasicBlock, Function,
                                     ilist_traits<Instruction> >;
  void setParent(BasicBlock *P);

private:
  // FIXME: This is a dirty hack.  Setcc instructions shouldn't encode the CC
  // into the opcode field.  When they don't, this will be unneeded.
  void setOpcode(unsigned NewOpcode);
  friend class BinaryOperator;
protected:
  Instruction(const Type *Ty, unsigned iType, Use *Ops, unsigned NumOps,
              const std::string &Name = "",
              Instruction *InsertBefore = 0);
  Instruction(const Type *Ty, unsigned iType, Use *Ops, unsigned NumOps,
              const std::string &Name, BasicBlock *InsertAtEnd);
public:
  // Out of line virtual method, so the vtable, etc has a home.
  ~Instruction();
  
  /// mayWriteToMemory - Return true if this instruction may modify memory.
  ///
  virtual bool mayWriteToMemory() const { return false; }

  /// clone() - Create a copy of 'this' instruction that is identical in all
  /// ways except the following:
  ///   * The instruction has no parent
  ///   * The instruction has no name
  ///
  virtual Instruction *clone() const = 0;

  /// isIdenticalTo - Return true if the specified instruction is exactly
  /// identical to the current one.  This means that all operands match and any
  /// extra information (e.g. load is volatile) agree.
  bool isIdenticalTo(Instruction *I) const;

  /// use_back - Specialize the methods defined in Value, as we know that an
  /// instruction can only be used by other instructions.
  Instruction       *use_back()       { return cast<Instruction>(*use_begin());}
  const Instruction *use_back() const { return cast<Instruction>(*use_begin());}
  
  // Accessor methods...
  //
  inline const BasicBlock *getParent() const { return Parent; }
  inline       BasicBlock *getParent()       { return Parent; }

  // getNext/Prev - Return the next or previous instruction in the list.  The
  // last node in the list is a terminator instruction.
        Instruction *getNext()       { return Next; }
  const Instruction *getNext() const { return Next; }
        Instruction *getPrev()       { return Prev; }
  const Instruction *getPrev() const { return Prev; }

  /// removeFromParent - This method unlinks 'this' from the containing basic
  /// block, but does not delete it.
  ///
  void removeFromParent();

  /// eraseFromParent - This method unlinks 'this' from the containing basic
  /// block and deletes it.
  ///
  void eraseFromParent();

  /// moveBefore - Unlink this instruction from its current basic block and
  /// insert it into the basic block that MovePos lives in, right before
  /// MovePos.
  void moveBefore(Instruction *MovePos);

  // ---------------------------------------------------------------------------
  /// Subclass classification... getOpcode() returns a member of
  /// one of the enums that is coming soon (down below)...
  ///
  unsigned getOpcode() const { return getValueType() - InstructionVal; }
  const char *getOpcodeName() const {
    return getOpcodeName(getOpcode());
  }
  static const char* getOpcodeName(unsigned OpCode);

  static inline bool isTerminator(unsigned OpCode) {
    return OpCode >= TermOpsBegin && OpCode < TermOpsEnd;
  }

  inline bool isTerminator() const {   // Instance of TerminatorInst?
    return isTerminator(getOpcode());
  }

  inline bool isBinaryOp() const {
    return getOpcode() >= BinaryOpsBegin && getOpcode() < BinaryOpsEnd;
  }

  /// @brief Determine if the OpCode is one of the CastInst instructions.
  static inline bool isCast(unsigned OpCode) {
    return OpCode >= CastOpsBegin && OpCode < CastOpsEnd;
  }

  /// @brief Determine if this is one of the CastInst instructions.
  inline bool isCast() const {
    return isCast(getOpcode());
  }

  /// isAssociative - Return true if the instruction is associative:
  ///
  ///   Associative operators satisfy:  x op (y op z) === (x op y) op z
  ///
  /// In LLVM, the Add, Mul, And, Or, and Xor operators are associative, when
  /// not applied to floating point types.
  ///
  bool isAssociative() const { return isAssociative(getOpcode(), getType()); }
  static bool isAssociative(unsigned op, const Type *Ty);

  /// isCommutative - Return true if the instruction is commutative:
  ///
  ///   Commutative operators satisfy: (x op y) === (y op x)
  ///
  /// In LLVM, these are the associative operators, plus SetEQ and SetNE, when
  /// applied to any type.
  ///
  bool isCommutative() const { return isCommutative(getOpcode()); }
  static bool isCommutative(unsigned op);

  /// isComparison - Return true if the instruction is a Set* instruction:
  ///
  bool isComparison() const { return isComparison(getOpcode()); }
  static bool isComparison(unsigned op);


  /// isTrappingInstruction - Return true if the instruction may trap.
  ///
  bool isTrapping() const {
    return isTrapping(getOpcode());
  }
  static bool isTrapping(unsigned op);

  virtual void print(std::ostream &OS) const { print(OS, 0); }
  void print(std::ostream &OS, AssemblyAnnotationWriter *AAW) const;

  /// Methods for support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const Instruction *) { return true; }
  static inline bool classof(const Value *V) {
    return V->getValueType() >= Value::InstructionVal;
  }

  //----------------------------------------------------------------------
  // Exported enumerations...
  //
  enum TermOps {       // These terminate basic blocks
#define  FIRST_TERM_INST(N)             TermOpsBegin = N,
#define HANDLE_TERM_INST(N, OPC, CLASS) OPC = N,
#define   LAST_TERM_INST(N)             TermOpsEnd = N+1
#include "llvm/Instruction.def"
  };

  enum BinaryOps {
#define  FIRST_BINARY_INST(N)             BinaryOpsBegin = N,
#define HANDLE_BINARY_INST(N, OPC, CLASS) OPC = N,
#define   LAST_BINARY_INST(N)             BinaryOpsEnd = N+1
#include "llvm/Instruction.def"
  };

  enum MemoryOps {
#define  FIRST_MEMORY_INST(N)             MemoryOpsBegin = N,
#define HANDLE_MEMORY_INST(N, OPC, CLASS) OPC = N,
#define   LAST_MEMORY_INST(N)             MemoryOpsEnd = N+1
#include "llvm/Instruction.def"
  };

  enum CastOps {
#define  FIRST_CAST_INST(N)             CastOpsBegin = N,
#define HANDLE_CAST_INST(N, OPC, CLASS) OPC = N,
#define   LAST_CAST_INST(N)             CastOpsEnd = N+1
#include "llvm/Instruction.def"
  };

  enum OtherOps {
#define  FIRST_OTHER_INST(N)             OtherOpsBegin = N,
#define HANDLE_OTHER_INST(N, OPC, CLASS) OPC = N,
#define   LAST_OTHER_INST(N)             OtherOpsEnd = N+1
#include "llvm/Instruction.def"
  };
};

} // End llvm namespace

#endif
