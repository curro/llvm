// RUN: llvm-mc -filetype=obj -triple x86_64-pc-linux-gnu %s -o - | llvm-readobj -s -sd | FileCheck %s

// Test that .debug_line is populated.

// CHECK:        Section {
// CHECK:          Name: .debug_line
// CHECK-NEXT:     Type: SHT_PROGBITS
// CHECK-NEXT:     Flags [
// CHECK-NEXT:     ]
// CHECK-NEXT:     Address: 0x0
// CHECK-NEXT:     Offset: 0x44
// CHECK-NEXT:     Size: 55
// CHECK-NEXT:     Link: 0
// CHECK-NEXT:     Info: 0
// CHECK-NEXT:     AddressAlignment: 1
// CHECK-NEXT:     EntrySize: 0
// CHECK-NEXT:     SectionData (
// CHECK-NEXT:       0000: 33000000 02001C00 00000101 FB0E0D00
// CHECK-NEXT:       0010: 01010101 00000001 00000100 666F6F2E
// CHECK-NEXT:       0020: 63000000 00000009 02000000 00000000
// CHECK-NEXT:       0030: 00150204 000101
// CHECK-NEXT:     )
// CHECK-NEXT:   }

	.section	.debug_line,"",@progbits
	.text

	.file 1 "foo.c"
	.loc 1 4 0
	subq	$8, %rsp
