//===-- TGSI.h - Top-level interface for TGSI representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// TGSI back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TGSI_H
#define TGSI_H

#include "MCTargetDesc/TGSIMCTargetDesc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include <cassert>

namespace llvm {
   class FunctionPass;
   class TGSITargetMachine;
   class TargetLoweringObjectFile;

   FunctionPass *createTGSIISelDag(TGSITargetMachine &tm);
   FunctionPass *createTGSIBranchConvPass(const TGSITargetMachine &tm);

   namespace tgsi {
      enum AddressSpace {
         // Enums corresponding to OpenCL address spaces
         PRIVATE = 0,
         GLOBAL = 1,
         LOCAL = 2,
         CONSTANT = 3,
         // Special address space the arguments of a __kernel function are stored in
         INPUT = 4
      };
   }

   namespace {
      inline bool isKernelFunction(const Function *f) {
         NamedMDNode *md = f->getParent()->getNamedMetadata("opencl.kernels");

         if (md) {
            for (unsigned i = 0; i < md->getNumOperands(); ++i) {
               Value *v = md->getOperand(i)->getOperand(0);

               assert(v && isa<Function>(v));

               if (f == static_cast<Function *>(v))
                  return true;
            }
         }

         return false;
      }
   }
}
#endif
