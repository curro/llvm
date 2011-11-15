@ RUN: llvm-mc -mcpu=cortex-a8 -triple arm-unknown-unknown -show-encoding < %s | FileCheck %s

@ CHECK: vext.8	d16, d17, d16, #3       @ encoding: [0xa0,0x03,0xf1,0xf2]
	vext.8	d16, d17, d16, #3
@ CHECK: vext.8	d16, d17, d16, #5       @ encoding: [0xa0,0x05,0xf1,0xf2]
	vext.8	d16, d17, d16, #5
@ CHECK: vext.8	q8, q9, q8, #3          @ encoding: [0xe0,0x03,0xf2,0xf2]
	vext.8	q8, q9, q8, #3
@ CHECK: vext.8	q8, q9, q8, #7          @ encoding: [0xe0,0x07,0xf2,0xf2]
	vext.8	q8, q9, q8, #7
@ CHECK: vext.16	d16, d17, d16, #3       @ encoding: [0xa0,0x06,0xf1,0xf2]
	vext.16	d16, d17, d16, #3
@ CHECK: vext.32	q8, q9, q8, #3          @ encoding: [0xe0,0x0c,0xf2,0xf2]
	vext.32	q8, q9, q8, #3
@ CHECK: vtrn.8	d17, d16                @ encoding: [0xa0,0x10,0xf2,0xf3]
	vtrn.8	d17, d16
@ CHECK: vtrn.16	d17, d16                @ encoding: [0xa0,0x10,0xf6,0xf3]
	vtrn.16	d17, d16
@ CHECK: vtrn.32	d17, d16                @ encoding: [0xa0,0x10,0xfa,0xf3]
	vtrn.32	d17, d16
@ CHECK: vtrn.8	q9, q8                  @ encoding: [0xe0,0x20,0xf2,0xf3]
	vtrn.8	q9, q8
@ CHECK: vtrn.16	q9, q8                  @ encoding: [0xe0,0x20,0xf6,0xf3]
	vtrn.16	q9, q8
@ CHECK: vtrn.32	q9, q8                  @ encoding: [0xe0,0x20,0xfa,0xf3]
	vtrn.32	q9, q8
@ CHECK: vuzp.8	d17, d16                @ encoding: [0x20,0x11,0xf2,0xf3]
	vuzp.8	d17, d16
@ CHECK: vuzp.16	d17, d16                @ encoding: [0x20,0x11,0xf6,0xf3]
	vuzp.16	d17, d16
@ CHECK: vuzp.8	q9, q8                  @ encoding: [0x60,0x21,0xf2,0xf3]
	vuzp.8	q9, q8
@ CHECK: vuzp.16	q9, q8                  @ encoding: [0x60,0x21,0xf6,0xf3]
	vuzp.16	q9, q8
@ CHECK: vuzp.32	q9, q8                  @ encoding: [0x60,0x21,0xfa,0xf3]
	vuzp.32	q9, q8
@ CHECK: vzip.8	d17, d16                @ encoding: [0xa0,0x11,0xf2,0xf3]
	vzip.8	d17, d16
@ CHECK: vzip.16	d17, d16                @ encoding: [0xa0,0x11,0xf6,0xf3]
	vzip.16	d17, d16
@ CHECK: vzip.8	q9, q8                  @ encoding: [0xe0,0x21,0xf2,0xf3]
	vzip.8	q9, q8
@ CHECK: vzip.16	q9, q8                  @ encoding: [0xe0,0x21,0xf6,0xf3]
	vzip.16	q9, q8
@ CHECK: vzip.32	q9, q8                  @ encoding: [0xe0,0x21,0xfa,0xf3]
	vzip.32	q9, q8


@ VTRN alternate size suffices

        vtrn.8 d3, d9
        vtrn.i8 d3, d9
        vtrn.u8 d3, d9
        vtrn.p8 d3, d9
        vtrn.16 d3, d9
        vtrn.i16 d3, d9
        vtrn.u16 d3, d9
        vtrn.p16 d3, d9
        vtrn.32 d3, d9
        vtrn.i32 d3, d9
        vtrn.u32 d3, d9
        vtrn.f32 d3, d9
        vtrn.f d3, d9

        vtrn.8 q14, q6
        vtrn.i8 q14, q6
        vtrn.u8 q14, q6
        vtrn.p8 q14, q6
        vtrn.16 q14, q6
        vtrn.i16 q14, q6
        vtrn.u16 q14, q6
        vtrn.p16 q14, q6
        vtrn.32 q14, q6
        vtrn.i32 q14, q6
        vtrn.u32 q14, q6
        vtrn.f32 q14, q6
        vtrn.f q14, q6

@ CHECK: vtrn.8	d3, d9                  @ encoding: [0x89,0x30,0xb2,0xf3]
@ CHECK: vtrn.8	d3, d9                  @ encoding: [0x89,0x30,0xb2,0xf3]
@ CHECK: vtrn.8	d3, d9                  @ encoding: [0x89,0x30,0xb2,0xf3]
@ CHECK: vtrn.8	d3, d9                  @ encoding: [0x89,0x30,0xb2,0xf3]
@ CHECK: vtrn.16	d3, d9          @ encoding: [0x89,0x30,0xb6,0xf3]
@ CHECK: vtrn.16	d3, d9          @ encoding: [0x89,0x30,0xb6,0xf3]
@ CHECK: vtrn.16	d3, d9          @ encoding: [0x89,0x30,0xb6,0xf3]
@ CHECK: vtrn.16	d3, d9          @ encoding: [0x89,0x30,0xb6,0xf3]
@ CHECK: vtrn.32	d3, d9          @ encoding: [0x89,0x30,0xba,0xf3]
@ CHECK: vtrn.32	d3, d9          @ encoding: [0x89,0x30,0xba,0xf3]
@ CHECK: vtrn.32	d3, d9          @ encoding: [0x89,0x30,0xba,0xf3]
@ CHECK: vtrn.32	d3, d9          @ encoding: [0x89,0x30,0xba,0xf3]
@ CHECK: vtrn.32	d3, d9          @ encoding: [0x89,0x30,0xba,0xf3]

@ CHECK: vtrn.8	q14, q6                 @ encoding: [0xcc,0xc0,0xf2,0xf3]
@ CHECK: vtrn.8	q14, q6                 @ encoding: [0xcc,0xc0,0xf2,0xf3]
@ CHECK: vtrn.8	q14, q6                 @ encoding: [0xcc,0xc0,0xf2,0xf3]
@ CHECK: vtrn.8	q14, q6                 @ encoding: [0xcc,0xc0,0xf2,0xf3]
@ CHECK: vtrn.16	q14, q6         @ encoding: [0xcc,0xc0,0xf6,0xf3]
@ CHECK: vtrn.16	q14, q6         @ encoding: [0xcc,0xc0,0xf6,0xf3]
@ CHECK: vtrn.16	q14, q6         @ encoding: [0xcc,0xc0,0xf6,0xf3]
@ CHECK: vtrn.16	q14, q6         @ encoding: [0xcc,0xc0,0xf6,0xf3]
@ CHECK: vtrn.32	q14, q6         @ encoding: [0xcc,0xc0,0xfa,0xf3]
@ CHECK: vtrn.32	q14, q6         @ encoding: [0xcc,0xc0,0xfa,0xf3]
@ CHECK: vtrn.32	q14, q6         @ encoding: [0xcc,0xc0,0xfa,0xf3]
@ CHECK: vtrn.32	q14, q6         @ encoding: [0xcc,0xc0,0xfa,0xf3]
@ CHECK: vtrn.32	q14, q6         @ encoding: [0xcc,0xc0,0xfa,0xf3]

