; This fails because the linker renames the non-opaque type not the opaque 
; one...

; RUN: echo "%X = linkonce global int 8" | llvm-as > %t.2.bc
; RUN: llvm-as < %s > %t.1.bc
; RUN: llvm-link %t.[12].bc | llvm-dis

%X = linkonce global int 7
