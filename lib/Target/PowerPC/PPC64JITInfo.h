//===- PPC64JITInfo.h - PowerPC/AIX impl. of the JIT interface -*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file contains the PowerPC/AIX implementation of the TargetJITInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef POWERPC_AIX_JITINFO_H
#define POWERPC_AIX_JITINFO_H

#include "PowerPCJITInfo.h"

namespace llvm {
  class TargetMachine;

  class PPC64JITInfo : public PowerPCJITInfo {
  public:
    PPC64JITInfo(TargetMachine &tm) : PowerPCJITInfo(tm) {}

    /// addPassesToJITCompile - Add passes to the specified pass manager to
    /// implement a fast dynamic compiler for this target.  Return true if this
    /// is not supported for this target.
    ///
    virtual void addPassesToJITCompile(FunctionPassManager &PM);
    
    /// replaceMachineCodeForFunction - Make it so that calling the function
    /// whose machine code is at OLD turns into a call to NEW, perhaps by
    /// overwriting OLD with a branch to NEW.  This is used for self-modifying
    /// code.
    ///
    virtual void replaceMachineCodeForFunction(void *Old, void *New);
    
    /// getJITStubForFunction - Create or return a stub for the specified
    /// function.  This stub acts just like the specified function, except that
    /// it allows the "address" of the function to be taken without having to
    /// generate code for it.
    virtual void *getJITStubForFunction(Function *F, MachineCodeEmitter &MCE);
  };
}

#endif
