; RUN: llvm-as < %s > Output/%s.out1.bc
; RUN: echo "%S = external global { int, opaque* } declare void %F(opaque*)" | llvm-as > Output/%s.out2.bc
; RUN: llvm-link Output/%s.out[12].bc | llvm-dis | not grep opaque

; After linking this testcase, there should be no opaque types left.  The two
; S's should cause the opaque type to be resolved to 'int'.
%S = global { int, int* } { int 5, int* null }

declare void %F(int*)
