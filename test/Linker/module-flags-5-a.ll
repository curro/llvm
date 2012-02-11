; RUN: not llvm-link %s %p/module-flags-5-b.ll -S -o - |& FileCheck %s

; Test the 'override' error.

; CHECK: Linking module flags 'foo': IDs have conflicting override values

!0 = metadata !{ i32 4, metadata !"foo", i32 927 }

!llvm.module.flags = !{ !0 }
