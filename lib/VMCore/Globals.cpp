//===-- Globals.cpp - Implement the Global object classes -----------------===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements the GlobalValue & GlobalVariable classes for the VMCore
// library.
//
//===----------------------------------------------------------------------===//

#include "llvm/DerivedTypes.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Module.h"
#include "llvm/SymbolTable.h"
#include "Support/LeakDetector.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
//                            GlobalValue Class
//===----------------------------------------------------------------------===//

/// This could be named "SafeToDestroyGlobalValue". It just makes sure that 
/// there are no non-constant uses of this GlobalValue. If there aren't then
/// this and the transitive closure of the constants can be deleted. See the
/// destructor for details.
namespace {
bool removeDeadConstantUsers(Constant* C) {
  while (!C->use_empty()) {
    if (Constant *User = dyn_cast<Constant>(C->use_back())) {
      if (!removeDeadConstantUsers(User)) 
        return false; // Constant wasn't dead
    } else {
      return false; // Non-constant usage;
    }
  }
  if (!isa<GlobalValue>(C))
    C->destroyConstant();
  return true;
}
}

/// removeDeadConstantUsers - If there are any dead constant users dangling
/// off of this global value, remove them.  This method is useful for clients
/// that want to check to see if a global is unused, but don't want to deal
/// with potentially dead constants hanging off of the globals.
///
/// This function returns true if the global value is now dead.  If all 
/// users of this global are not dead, this method may return false and
/// leave some of them around.
bool GlobalValue::removeDeadConstantUsers() {
  while(!use_empty()) {
    if (Constant* User = dyn_cast<Constant>(use_back())) {
      if (!::removeDeadConstantUsers(User))
        return false; // Constant wasn't dead
    } else {
      return false; // Non-constant usage;
    }
  }
  return true;
}

/// This virtual destructor is responsible for deleting any transitively dead
/// Constants that are using the GlobalValue.
GlobalValue::~GlobalValue() {
  // Its an error to attempt destruction with non-constant uses remaining.
  bool okay_to_destruct = removeDeadConstantUsers();
  assert(okay_to_destruct && 
         "Can't destroy GlobalValue with non-constant uses.");
}

/// Override destroyConstant to make sure it doesn't get called on 
/// GlobalValue's because they shouldn't be treated like other constants.
void GlobalValue::destroyConstant() {
  assert(0 && "You can't GV->destroyConstant()!");
  abort();
}
//===----------------------------------------------------------------------===//
// GlobalVariable Implementation
//===----------------------------------------------------------------------===//

GlobalVariable::GlobalVariable(const Type *Ty, bool constant, LinkageTypes Link,
                               Constant *Initializer,
                               const std::string &Name, Module *ParentModule)
  : GlobalValue(PointerType::get(Ty), Value::GlobalVariableVal, Link, Name),
    isConstantGlobal(constant) {
  if (Initializer) Operands.push_back(Use((Value*)Initializer, this));

  LeakDetector::addGarbageObject(this);

  if (ParentModule)
    ParentModule->getGlobalList().push_back(this);
}

void GlobalVariable::setParent(Module *parent) {
  if (getParent())
    LeakDetector::addGarbageObject(this);
  Parent = parent;
  if (getParent())
    LeakDetector::removeGarbageObject(this);
}

// Specialize setName to take care of symbol table majik
void GlobalVariable::setName(const std::string &name, SymbolTable *ST) {
  Module *P;
  assert((ST == 0 || (!getParent() || ST == &getParent()->getSymbolTable())) &&
         "Invalid symtab argument!");
  if ((P = getParent()) && hasName()) P->getSymbolTable().remove(this);
  Value::setName(name);
  if (P && hasName()) P->getSymbolTable().insert(this);
}

void GlobalVariable::replaceUsesOfWithOnConstant(Value *From, Value *To,
                                                 bool DisableChecking )
{
  // If you call this, then you better know this GVar has a constant
  // initializer worth replacing. Enforce that here.
  assert(getNumOperands() == 1 && 
         "Attempt to replace uses of Constants on a GVar with no initializer");

  // And, since you know it has an initializer, the From value better be
  // the initializer :)
  assert(getOperand(0) == From &&
         "Attempt to replace wrong constant initializer in GVar");

  // And, you better have a constant for the replacement value
  assert(isa<Constant>(To) &&
         "Attempt to replace GVar initializer with non-constant");
  
  // Okay, preconditions out of the way, replace the constant initializer.
  this->setOperand(0,To);
}

// vim: sw=2 ai

