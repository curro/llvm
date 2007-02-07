//===- lli.cpp - LLVM Interpreter / Dynamic compiler ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This utility provides a simple wrapper around the LLVM Execution Engines,
// which allow the direct execution of LLVM programs through a Just-In-Time
// compiler, or through an intepreter if no JIT is available for this platform.
//
//===----------------------------------------------------------------------===//

#include "llvm/Module.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Type.h"
#include "llvm/Bytecode/Reader.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compressor.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/System/Process.h"
#include "llvm/System/Signals.h"
#include <iostream>

using namespace llvm;

namespace {
  cl::opt<std::string>
  InputFile(cl::desc("<input bytecode>"), cl::Positional, cl::init("-"));

  cl::list<std::string>
  InputArgv(cl::ConsumeAfter, cl::desc("<program arguments>..."));

  cl::opt<bool> ForceInterpreter("force-interpreter",
                                 cl::desc("Force interpretation: disable JIT"),
                                 cl::init(false));
  cl::opt<std::string>
  TargetTriple("mtriple", cl::desc("Override target triple for module"));
  
  cl::opt<std::string>
  FakeArgv0("fake-argv0",
            cl::desc("Override the 'argv[0]' value passed into the executing"
                     " program"), cl::value_desc("executable"));
  
  cl::opt<bool>
  DisableCoreFiles("disable-core-files", cl::Hidden,
                   cl::desc("Disable emission of core files if possible"));
}

//===----------------------------------------------------------------------===//
// main Driver function
//
int main(int argc, char **argv, char * const *envp) {
  atexit(llvm_shutdown);  // Call llvm_shutdown() on exit.
  try {
    cl::ParseCommandLineOptions(argc, argv,
                                " llvm interpreter & dynamic compiler\n");
    sys::PrintStackTraceOnErrorSignal();

    // If the user doesn't want core files, disable them.
    if (DisableCoreFiles)
      sys::Process::PreventCoreFiles();
    
    // Load the bytecode...
    std::string ErrorMsg;
    ModuleProvider *MP = 0;
    MP = getBytecodeModuleProvider(InputFile, 
                                   Compressor::decompressToNewBuffer,
                                   &ErrorMsg);
    if (!MP) {
      std::cerr << "Error loading program '" << InputFile << "': "
                << ErrorMsg << "\n";
      exit(1);
    }

    // If we are supposed to override the target triple, do so now.
    if (!TargetTriple.empty())
      MP->getModule()->setTargetTriple(TargetTriple);
    
    ExecutionEngine *EE = ExecutionEngine::create(MP, ForceInterpreter);
    assert(EE &&"Couldn't create an ExecutionEngine, not even an interpreter?");

    // If the user specifically requested an argv[0] to pass into the program,
    // do it now.
    if (!FakeArgv0.empty()) {
      InputFile = FakeArgv0;
    } else {
      // Otherwise, if there is a .bc suffix on the executable strip it off, it
      // might confuse the program.
      if (InputFile.rfind(".bc") == InputFile.length() - 3)
        InputFile.erase(InputFile.length() - 3);
    }

    // Add the module's name to the start of the vector of arguments to main().
    InputArgv.insert(InputArgv.begin(), InputFile);

    // Call the main function from M as if its signature were:
    //   int main (int argc, char **argv, const char **envp)
    // using the contents of Args to determine argc & argv, and the contents of
    // EnvVars to determine envp.
    //
    Function *Fn = MP->getModule()->getFunction("main");
    if (!Fn) {
      std::cerr << "'main' function not found in module.\n";
      return -1;
    }

    // Run static constructors.
    EE->runStaticConstructorsDestructors(false);
    
    // Run main.
    int Result = EE->runFunctionAsMain(Fn, InputArgv, envp);

    // Run static destructors.
    EE->runStaticConstructorsDestructors(true);
    
    // If the program didn't explicitly call exit, call exit now, for the
    // program. This ensures that any atexit handlers get called correctly.
    Constant *Exit = MP->getModule()->getOrInsertFunction("exit", Type::VoidTy,
                                                          Type::Int32Ty, NULL);
    if (Function *ExitF = dyn_cast<Function>(Exit)) {
      std::vector<GenericValue> Args;
      GenericValue ResultGV;
      ResultGV.Int32Val = Result;
      Args.push_back(ResultGV);
      EE->runFunction(ExitF, Args);
      std::cerr << "ERROR: exit(" << Result << ") returned!\n";
      abort();
    } else {
      std::cerr << "ERROR: exit defined with wrong prototype!\n";
      abort();
    }
  } catch (const std::string& msg) {
    std::cerr << argv[0] << ": " << msg << "\n";
  } catch (...) {
    std::cerr << argv[0] << ": Unexpected unknown exception occurred.\n";
  }
  abort();
}
