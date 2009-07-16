//===- Optimize.cpp - Optimize a complete program -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements all optimization of the linked module for llvm-ld.
//
//===----------------------------------------------------------------------===//

#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/StandardPasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/PassNameParser.h"
#include "llvm/Support/PluginLoader.h"
using namespace llvm;

// Pass Name Options as generated by the PassNameParser
static cl::list<const PassInfo*, bool, PassNameParser>
  OptimizationList(cl::desc("Optimizations available:"));

//Don't verify at the end
static cl::opt<bool> DontVerify("disable-verify", cl::ReallyHidden);

static cl::opt<bool> DisableInline("disable-inlining",
  cl::desc("Do not run the inliner pass"));

static cl::opt<bool>
DisableOptimizations("disable-opt",
  cl::desc("Do not run any optimization passes"));

static cl::opt<bool> DisableInternalize("disable-internalize",
  cl::desc("Do not mark all symbols as internal"));

static cl::opt<bool> VerifyEach("verify-each",
 cl::desc("Verify intermediate results of all passes"));

static cl::alias ExportDynamic("export-dynamic",
  cl::aliasopt(DisableInternalize),
  cl::desc("Alias for -disable-internalize"));

static cl::opt<bool> Strip("strip-all", 
  cl::desc("Strip all symbol info from executable"));

static cl::alias A0("s", cl::desc("Alias for --strip-all"), 
  cl::aliasopt(Strip));

static cl::opt<bool> StripDebug("strip-debug",
  cl::desc("Strip debugger symbol info from executable"));

static cl::alias A1("S", cl::desc("Alias for --strip-debug"),
  cl::aliasopt(StripDebug));

// A utility function that adds a pass to the pass manager but will also add
// a verifier pass after if we're supposed to verify.
static inline void addPass(PassManager &PM, Pass *P) {
  // Add the pass to the pass manager...
  PM.add(P);

  // If we are verifying all of the intermediate steps, add the verifier...
  if (VerifyEach)
    PM.add(createVerifierPass());
}

namespace llvm {

/// Optimize - Perform link time optimizations. This will run the scalar
/// optimizations, any loaded plugin-optimization modules, and then the
/// inter-procedural optimizations if applicable.
void Optimize(Module* M) {

  // Instantiate the pass manager to organize the passes.
  PassManager Passes;

  // If we're verifying, start off with a verification pass.
  if (VerifyEach)
    Passes.add(createVerifierPass());

  // Add an appropriate TargetData instance for this module...
  addPass(Passes, new TargetData(M));

  if (!DisableOptimizations)
    createStandardLTOPasses(&Passes, !DisableInternalize, !DisableInline,
                            VerifyEach);

  // If the -s or -S command line options were specified, strip the symbols out
  // of the resulting program to make it smaller.  -s and -S are GNU ld options
  // that we are supporting; they alias -strip-all and -strip-debug.
  if (Strip || StripDebug)
    addPass(Passes, createStripSymbolsPass(StripDebug && !Strip));

  // Create a new optimization pass for each one specified on the command line
  std::auto_ptr<TargetMachine> target;
  for (unsigned i = 0; i < OptimizationList.size(); ++i) {
    const PassInfo *Opt = OptimizationList[i];
    if (Opt->getNormalCtor())
      addPass(Passes, Opt->getNormalCtor()());
    else
      errs() << "llvm-ld: cannot create pass: " << Opt->getPassName() 
             << "\n";
  }

  // The user's passes may leave cruft around. Clean up after them them but
  // only if we haven't got DisableOptimizations set
  if (!DisableOptimizations) {
    addPass(Passes, createInstructionCombiningPass());
    addPass(Passes, createCFGSimplificationPass());
    addPass(Passes, createAggressiveDCEPass());
    addPass(Passes, createGlobalDCEPass());
  }

  // Make sure everything is still good.
  if (!DontVerify)
    Passes.add(createVerifierPass());

  // Run our queue of passes all at once now, efficiently.
  Passes.run(*M);
}

}
