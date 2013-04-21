set(LLVM_TARGET_DEFINITIONS TGSI.td)

tablegen(LLVM TGSIGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TGSIGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TGSIGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TGSIGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM TGSIGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM TGSIGenCallingConv.inc -gen-callingconv)
add_public_tablegen_target(TGSICommonTableGen)

add_llvm_target(TGSICodeGen
  TGSIAsmPrinter.cpp
  TGSIInstrInfo.cpp
  TGSIISelDAGToDAG.cpp
  TGSIISelLowering.cpp
  TGSIFrameLowering.cpp
  TGSISubtarget.cpp
  TGSITargetMachine.cpp
  TGSISelectionDAGInfo.cpp
  TGSIRegisterInfo.cpp
  )

add_dependencies(LLVMTGSICodeGen intrinsics_gen)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
