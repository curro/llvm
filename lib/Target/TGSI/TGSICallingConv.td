//===- TGSICallingConv.td - Calling Conventions TGSI -----*- tablegen -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the TGSI architectures.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

// TGSI 32-bit C return-value convention.
def RetCC_TGSI : CallingConv<[
  CCIfType<[i32,f32], CCAssignToReg<[TEMP1x]>>,
  CCIfType<[v4i32,v4f32], CCAssignToReg<[TEMP1]>>
]>;

// TGSI 32-bit C Calling convention.
def FunCC_TGSI : CallingConv<[
  // i32 f32 arguments get passed in integer registers if there is space.
  CCIfType<[i32, f32], CCAssignToReg<[TEMP1x, TEMP1y, TEMP1z, TEMP1w,
         TEMP2x, TEMP2y, TEMP2z, TEMP2w, TEMP3x, TEMP3y, TEMP3z, TEMP3w,
         TEMP4x, TEMP4y, TEMP4z, TEMP4w, TEMP5x, TEMP5y, TEMP5z, TEMP5w,
         TEMP6x, TEMP6y, TEMP6z, TEMP6w, TEMP7x, TEMP7y, TEMP7z, TEMP7w,
         TEMP8x, TEMP8y, TEMP8z, TEMP8w, TEMP9x, TEMP9y, TEMP9z, TEMP9w,
         TEMP10x, TEMP10y, TEMP10z, TEMP10w, TEMP11x, TEMP11y, TEMP11z, TEMP11w,
         TEMP12x, TEMP12y, TEMP12z, TEMP12w, TEMP13x, TEMP13y, TEMP13z, TEMP13w,
         TEMP14x, TEMP14y, TEMP14z, TEMP14w, TEMP15x, TEMP15y, TEMP15z, TEMP15w,
         TEMP16x, TEMP16y, TEMP16z, TEMP16w, TEMP17x, TEMP17y, TEMP17z, TEMP17w,
         TEMP18x, TEMP18y, TEMP18z, TEMP18w, TEMP19x, TEMP19y, TEMP19z, TEMP19w,
         TEMP20x, TEMP20y, TEMP20z, TEMP20w, TEMP21x, TEMP21y, TEMP21z, TEMP21w,
         TEMP22x, TEMP22y, TEMP22z, TEMP22w, TEMP23x, TEMP23y, TEMP23z, TEMP23w,
         TEMP24x, TEMP24y, TEMP24z, TEMP24w, TEMP25x, TEMP25y, TEMP25z, TEMP25w,
         TEMP26x, TEMP26y, TEMP26z, TEMP26w, TEMP27x, TEMP27y, TEMP27z, TEMP27w,
         TEMP28x, TEMP28y, TEMP28z, TEMP28w, TEMP29x, TEMP29y, TEMP29z, TEMP29w,
         TEMP30x, TEMP30y, TEMP30z, TEMP30w, TEMP31x, TEMP31y, TEMP31z, TEMP31w]>>,

  CCIfType<[v4i32, v4f32], CCAssignToReg<[TEMP1, TEMP2, TEMP3, TEMP4, TEMP5,
         TEMP6, TEMP7, TEMP8, TEMP9, TEMP10, TEMP11, TEMP12, TEMP13, TEMP14,
         TEMP15, TEMP16, TEMP17, TEMP18, TEMP19, TEMP20, TEMP21, TEMP22, TEMP23,
         TEMP24, TEMP25, TEMP26, TEMP27, TEMP28, TEMP29, TEMP30, TEMP31]>>
]>;

def KernCC_TGSI : CallingConv<[
  // i32 f32 arguments get passed through the input space.
  CCIfType<[i32, f32], CCAssignToStack<4, 4>>,
  CCIfType<[v4i32, v4f32], CCAssignToStack<16, 16>>
]>;
