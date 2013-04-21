add_llvm_library(LLVMTGSIDesc
  TGSIMCTargetDesc.cpp
  TGSIMCInstPrinter.cpp
  )

add_dependencies(LLVMTGSIDesc LLVMTGSIInfo)

add_dependencies(LLVMTGSIDesc TGSICommonTableGen)
