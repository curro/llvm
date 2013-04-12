// RUN: llvm-mc -filetype=obj -triple x86_64-pc-linux-gnu %s -o - | llvm-readobj -s -r -t | FileCheck  %s

// Test that relocations with local symbols are represented as relocations
// with the section. They should be equivalent, but gas behaves like this.

	movl	foo, %r14d
foo:

// CHECKT:     Relocations [
// CHECK:        Section (1) .text {
// CHECK-NEXT:     0x{{[^ ]+}} R_X86_64_32S .text 0x{{[^ ]+}}
// CHECK-NEXT:   }
// CHECK-NEXT: ]
