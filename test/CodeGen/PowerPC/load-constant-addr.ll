; Should fold the ori into the lfs.
; RUN: llvm-upgrade < %s | llvm-as | llc -march=ppc32 | grep lfs
; RUN: llvm-upgrade < %s | llvm-as | llc -march=ppc32 | not grep ori

float %test() {
  %tmp.i = load float* cast (uint 186018016 to float*)
  ret float %tmp.i
}

