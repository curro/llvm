//===-- llvm/Target/Mangler.h - Self-contained name mangler ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Unified name mangler for various backends.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MANGLER_H
#define LLVM_SUPPORT_MANGLER_H

#include "llvm/ADT/DenseMap.h"
#include <string>

namespace llvm {
class StringRef;
class Twine;
class Value;
class GlobalValue;
template <typename T> class SmallVectorImpl; 
class MCAsmInfo;

class Mangler {
public:
  enum ManglerPrefixTy {
    Default,               ///< Emit default string before each symbol.
    Private,               ///< Emit "private" prefix before each symbol.
    LinkerPrivate          ///< Emit "linker private" prefix before each symbol.
  };

private:
  const MCAsmInfo &MAI;

  /// AnonGlobalIDs - We need to give global values the same name every time
  /// they are mangled.  This keeps track of the number we give to anonymous
  /// ones.
  ///
  DenseMap<const GlobalValue*, unsigned> AnonGlobalIDs;

  /// NextAnonGlobalID - This simple counter is used to unique value names.
  ///
  unsigned NextAnonGlobalID;

public:
  // Mangler ctor - if a prefix is specified, it will be prepended onto all
  // symbols.
  Mangler(const MCAsmInfo &mai) : MAI(mai), NextAnonGlobalID(1) {}

  /// getUniqueID() - Allocate and return a unique ID.
  /// FIXME: Remove this.
  unsigned getUniqueID() {
    return NextAnonGlobalID++;
  }
  
  /// getNameWithPrefix - Fill OutName with the name of the appropriate prefix
  /// and the specified global variable's name.  If the global variable doesn't
  /// have a name, this fills in a unique name for the global.
  void getNameWithPrefix(SmallVectorImpl<char> &OutName, const GlobalValue *GV,
                         bool isImplicitlyPrivate);
  
  /// getNameWithPrefix - Fill OutName with the name of the appropriate prefix
  /// and the specified name as the global variable name.  GVName must not be
  /// empty.
  void getNameWithPrefix(SmallVectorImpl<char> &OutName, const Twine &GVName,
                         ManglerPrefixTy PrefixTy = Mangler::Default);

  /// getNameWithPrefix - Return the name of the appropriate prefix
  /// and the specified global variable's name.  If the global variable doesn't
  /// have a name, this fills in a unique name for the global.
  std::string getNameWithPrefix(const GlobalValue *GV,
                                bool isImplicitlyPrivate = false);
};

} // End llvm namespace

#endif // LLVM_SUPPORT_MANGLER_H
