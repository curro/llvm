//===- TGSIRegisterInfo.td - TGSI Register defs ----------*- tablegen -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the TGSI register file 
//===----------------------------------------------------------------------===//

let Namespace = "TGSI" in {
    def sub_x  : SubRegIndex;
    def sub_y  : SubRegIndex;
    def sub_z  : SubRegIndex;
    def sub_w  : SubRegIndex;

    class TGSIReg<string n, int id, int c> : Register<n> {
        let HWEncoding{11-0} = id;
        let HWEncoding{15-12} = c;
    }

    class TGSIVReg<string n, int id, list<Register> subregs> :
          RegisterWithSubRegs<n, subregs> {
        let HWEncoding{11-0} = id;
        let HWEncoding{15-12} = 0xf;
        let SubRegIndices = [sub_x, sub_y, sub_z, sub_w];
    }
}

// Temporary registers
def TEMP0x : TGSIReg<"TEMP0x", 0, 1>;
def TEMP0y : TGSIReg<"TEMP0y", 0, 2>;
def TEMP0z : TGSIReg<"TEMP0z", 0, 4>;
def TEMP0w : TGSIReg<"TEMP0w", 0, 8>;
def TEMP0 : TGSIVReg<"TEMP0", 0, [TEMP0x, TEMP0y, TEMP0z, TEMP0w]>;
def TEMP1x : TGSIReg<"TEMP1x", 1, 1>;
def TEMP1y : TGSIReg<"TEMP1y", 1, 2>;
def TEMP1z : TGSIReg<"TEMP1z", 1, 4>;
def TEMP1w : TGSIReg<"TEMP1w", 1, 8>;
def TEMP1 : TGSIVReg<"TEMP1", 1, [TEMP1x, TEMP1y, TEMP1z, TEMP1w]>;
def TEMP2x : TGSIReg<"TEMP2x", 2, 1>;
def TEMP2y : TGSIReg<"TEMP2y", 2, 2>;
def TEMP2z : TGSIReg<"TEMP2z", 2, 4>;
def TEMP2w : TGSIReg<"TEMP2w", 2, 8>;
def TEMP2 : TGSIVReg<"TEMP2", 2, [TEMP2x, TEMP2y, TEMP2z, TEMP2w]>;
def TEMP3x : TGSIReg<"TEMP3x", 3, 1>;
def TEMP3y : TGSIReg<"TEMP3y", 3, 2>;
def TEMP3z : TGSIReg<"TEMP3z", 3, 4>;
def TEMP3w : TGSIReg<"TEMP3w", 3, 8>;
def TEMP3 : TGSIVReg<"TEMP3", 3, [TEMP3x, TEMP3y, TEMP3z, TEMP3w]>;
def TEMP4x : TGSIReg<"TEMP4x", 4, 1>;
def TEMP4y : TGSIReg<"TEMP4y", 4, 2>;
def TEMP4z : TGSIReg<"TEMP4z", 4, 4>;
def TEMP4w : TGSIReg<"TEMP4w", 4, 8>;
def TEMP4 : TGSIVReg<"TEMP4", 4, [TEMP4x, TEMP4y, TEMP4z, TEMP4w]>;
def TEMP5x : TGSIReg<"TEMP5x", 5, 1>;
def TEMP5y : TGSIReg<"TEMP5y", 5, 2>;
def TEMP5z : TGSIReg<"TEMP5z", 5, 4>;
def TEMP5w : TGSIReg<"TEMP5w", 5, 8>;
def TEMP5 : TGSIVReg<"TEMP5", 5, [TEMP5x, TEMP5y, TEMP5z, TEMP5w]>;
def TEMP6x : TGSIReg<"TEMP6x", 6, 1>;
def TEMP6y : TGSIReg<"TEMP6y", 6, 2>;
def TEMP6z : TGSIReg<"TEMP6z", 6, 4>;
def TEMP6w : TGSIReg<"TEMP6w", 6, 8>;
def TEMP6 : TGSIVReg<"TEMP6", 6, [TEMP6x, TEMP6y, TEMP6z, TEMP6w]>;
def TEMP7x : TGSIReg<"TEMP7x", 7, 1>;
def TEMP7y : TGSIReg<"TEMP7y", 7, 2>;
def TEMP7z : TGSIReg<"TEMP7z", 7, 4>;
def TEMP7w : TGSIReg<"TEMP7w", 7, 8>;
def TEMP7 : TGSIVReg<"TEMP7", 7, [TEMP7x, TEMP7y, TEMP7z, TEMP7w]>;
def TEMP8x : TGSIReg<"TEMP8x", 8, 1>;
def TEMP8y : TGSIReg<"TEMP8y", 8, 2>;
def TEMP8z : TGSIReg<"TEMP8z", 8, 4>;
def TEMP8w : TGSIReg<"TEMP8w", 8, 8>;
def TEMP8 : TGSIVReg<"TEMP8", 8, [TEMP8x, TEMP8y, TEMP8z, TEMP8w]>;
def TEMP9x : TGSIReg<"TEMP9x", 9, 1>;
def TEMP9y : TGSIReg<"TEMP9y", 9, 2>;
def TEMP9z : TGSIReg<"TEMP9z", 9, 4>;
def TEMP9w : TGSIReg<"TEMP9w", 9, 8>;
def TEMP9 : TGSIVReg<"TEMP9", 9, [TEMP9x, TEMP9y, TEMP9z, TEMP9w]>;
def TEMP10x : TGSIReg<"TEMP10x", 10, 1>;
def TEMP10y : TGSIReg<"TEMP10y", 10, 2>;
def TEMP10z : TGSIReg<"TEMP10z", 10, 4>;
def TEMP10w : TGSIReg<"TEMP10w", 10, 8>;
def TEMP10 : TGSIVReg<"TEMP10", 10, [TEMP10x, TEMP10y, TEMP10z, TEMP10w]>;
def TEMP11x : TGSIReg<"TEMP11x", 11, 1>;
def TEMP11y : TGSIReg<"TEMP11y", 11, 2>;
def TEMP11z : TGSIReg<"TEMP11z", 11, 4>;
def TEMP11w : TGSIReg<"TEMP11w", 11, 8>;
def TEMP11 : TGSIVReg<"TEMP11", 11, [TEMP11x, TEMP11y, TEMP11z, TEMP11w]>;
def TEMP12x : TGSIReg<"TEMP12x", 12, 1>;
def TEMP12y : TGSIReg<"TEMP12y", 12, 2>;
def TEMP12z : TGSIReg<"TEMP12z", 12, 4>;
def TEMP12w : TGSIReg<"TEMP12w", 12, 8>;
def TEMP12 : TGSIVReg<"TEMP12", 12, [TEMP12x, TEMP12y, TEMP12z, TEMP12w]>;
def TEMP13x : TGSIReg<"TEMP13x", 13, 1>;
def TEMP13y : TGSIReg<"TEMP13y", 13, 2>;
def TEMP13z : TGSIReg<"TEMP13z", 13, 4>;
def TEMP13w : TGSIReg<"TEMP13w", 13, 8>;
def TEMP13 : TGSIVReg<"TEMP13", 13, [TEMP13x, TEMP13y, TEMP13z, TEMP13w]>;
def TEMP14x : TGSIReg<"TEMP14x", 14, 1>;
def TEMP14y : TGSIReg<"TEMP14y", 14, 2>;
def TEMP14z : TGSIReg<"TEMP14z", 14, 4>;
def TEMP14w : TGSIReg<"TEMP14w", 14, 8>;
def TEMP14 : TGSIVReg<"TEMP14", 14, [TEMP14x, TEMP14y, TEMP14z, TEMP14w]>;
def TEMP15x : TGSIReg<"TEMP15x", 15, 1>;
def TEMP15y : TGSIReg<"TEMP15y", 15, 2>;
def TEMP15z : TGSIReg<"TEMP15z", 15, 4>;
def TEMP15w : TGSIReg<"TEMP15w", 15, 8>;
def TEMP15 : TGSIVReg<"TEMP15", 15, [TEMP15x, TEMP15y, TEMP15z, TEMP15w]>;
def TEMP16x : TGSIReg<"TEMP16x", 16, 1>;
def TEMP16y : TGSIReg<"TEMP16y", 16, 2>;
def TEMP16z : TGSIReg<"TEMP16z", 16, 4>;
def TEMP16w : TGSIReg<"TEMP16w", 16, 8>;
def TEMP16 : TGSIVReg<"TEMP16", 16, [TEMP16x, TEMP16y, TEMP16z, TEMP16w]>;
def TEMP17x : TGSIReg<"TEMP17x", 17, 1>;
def TEMP17y : TGSIReg<"TEMP17y", 17, 2>;
def TEMP17z : TGSIReg<"TEMP17z", 17, 4>;
def TEMP17w : TGSIReg<"TEMP17w", 17, 8>;
def TEMP17 : TGSIVReg<"TEMP17", 17, [TEMP17x, TEMP17y, TEMP17z, TEMP17w]>;
def TEMP18x : TGSIReg<"TEMP18x", 18, 1>;
def TEMP18y : TGSIReg<"TEMP18y", 18, 2>;
def TEMP18z : TGSIReg<"TEMP18z", 18, 4>;
def TEMP18w : TGSIReg<"TEMP18w", 18, 8>;
def TEMP18 : TGSIVReg<"TEMP18", 18, [TEMP18x, TEMP18y, TEMP18z, TEMP18w]>;
def TEMP19x : TGSIReg<"TEMP19x", 19, 1>;
def TEMP19y : TGSIReg<"TEMP19y", 19, 2>;
def TEMP19z : TGSIReg<"TEMP19z", 19, 4>;
def TEMP19w : TGSIReg<"TEMP19w", 19, 8>;
def TEMP19 : TGSIVReg<"TEMP19", 19, [TEMP19x, TEMP19y, TEMP19z, TEMP19w]>;
def TEMP20x : TGSIReg<"TEMP20x", 20, 1>;
def TEMP20y : TGSIReg<"TEMP20y", 20, 2>;
def TEMP20z : TGSIReg<"TEMP20z", 20, 4>;
def TEMP20w : TGSIReg<"TEMP20w", 20, 8>;
def TEMP20 : TGSIVReg<"TEMP20", 20, [TEMP20x, TEMP20y, TEMP20z, TEMP20w]>;
def TEMP21x : TGSIReg<"TEMP21x", 21, 1>;
def TEMP21y : TGSIReg<"TEMP21y", 21, 2>;
def TEMP21z : TGSIReg<"TEMP21z", 21, 4>;
def TEMP21w : TGSIReg<"TEMP21w", 21, 8>;
def TEMP21 : TGSIVReg<"TEMP21", 21, [TEMP21x, TEMP21y, TEMP21z, TEMP21w]>;
def TEMP22x : TGSIReg<"TEMP22x", 22, 1>;
def TEMP22y : TGSIReg<"TEMP22y", 22, 2>;
def TEMP22z : TGSIReg<"TEMP22z", 22, 4>;
def TEMP22w : TGSIReg<"TEMP22w", 22, 8>;
def TEMP22 : TGSIVReg<"TEMP22", 22, [TEMP22x, TEMP22y, TEMP22z, TEMP22w]>;
def TEMP23x : TGSIReg<"TEMP23x", 23, 1>;
def TEMP23y : TGSIReg<"TEMP23y", 23, 2>;
def TEMP23z : TGSIReg<"TEMP23z", 23, 4>;
def TEMP23w : TGSIReg<"TEMP23w", 23, 8>;
def TEMP23 : TGSIVReg<"TEMP23", 23, [TEMP23x, TEMP23y, TEMP23z, TEMP23w]>;
def TEMP24x : TGSIReg<"TEMP24x", 24, 1>;
def TEMP24y : TGSIReg<"TEMP24y", 24, 2>;
def TEMP24z : TGSIReg<"TEMP24z", 24, 4>;
def TEMP24w : TGSIReg<"TEMP24w", 24, 8>;
def TEMP24 : TGSIVReg<"TEMP24", 24, [TEMP24x, TEMP24y, TEMP24z, TEMP24w]>;
def TEMP25x : TGSIReg<"TEMP25x", 25, 1>;
def TEMP25y : TGSIReg<"TEMP25y", 25, 2>;
def TEMP25z : TGSIReg<"TEMP25z", 25, 4>;
def TEMP25w : TGSIReg<"TEMP25w", 25, 8>;
def TEMP25 : TGSIVReg<"TEMP25", 25, [TEMP25x, TEMP25y, TEMP25z, TEMP25w]>;
def TEMP26x : TGSIReg<"TEMP26x", 26, 1>;
def TEMP26y : TGSIReg<"TEMP26y", 26, 2>;
def TEMP26z : TGSIReg<"TEMP26z", 26, 4>;
def TEMP26w : TGSIReg<"TEMP26w", 26, 8>;
def TEMP26 : TGSIVReg<"TEMP26", 26, [TEMP26x, TEMP26y, TEMP26z, TEMP26w]>;
def TEMP27x : TGSIReg<"TEMP27x", 27, 1>;
def TEMP27y : TGSIReg<"TEMP27y", 27, 2>;
def TEMP27z : TGSIReg<"TEMP27z", 27, 4>;
def TEMP27w : TGSIReg<"TEMP27w", 27, 8>;
def TEMP27 : TGSIVReg<"TEMP27", 27, [TEMP27x, TEMP27y, TEMP27z, TEMP27w]>;
def TEMP28x : TGSIReg<"TEMP28x", 28, 1>;
def TEMP28y : TGSIReg<"TEMP28y", 28, 2>;
def TEMP28z : TGSIReg<"TEMP28z", 28, 4>;
def TEMP28w : TGSIReg<"TEMP28w", 28, 8>;
def TEMP28 : TGSIVReg<"TEMP28", 28, [TEMP28x, TEMP28y, TEMP28z, TEMP28w]>;
def TEMP29x : TGSIReg<"TEMP29x", 29, 1>;
def TEMP29y : TGSIReg<"TEMP29y", 29, 2>;
def TEMP29z : TGSIReg<"TEMP29z", 29, 4>;
def TEMP29w : TGSIReg<"TEMP29w", 29, 8>;
def TEMP29 : TGSIVReg<"TEMP29", 29, [TEMP29x, TEMP29y, TEMP29z, TEMP29w]>;
def TEMP30x : TGSIReg<"TEMP30x", 30, 1>;
def TEMP30y : TGSIReg<"TEMP30y", 30, 2>;
def TEMP30z : TGSIReg<"TEMP30z", 30, 4>;
def TEMP30w : TGSIReg<"TEMP30w", 30, 8>;
def TEMP30 : TGSIVReg<"TEMP30", 30, [TEMP30x, TEMP30y, TEMP30z, TEMP30w]>;
def TEMP31x : TGSIReg<"TEMP31x", 31, 1>;
def TEMP31y : TGSIReg<"TEMP31y", 31, 2>;
def TEMP31z : TGSIReg<"TEMP31z", 31, 4>;
def TEMP31w : TGSIReg<"TEMP31w", 31, 8>;
def TEMP31 : TGSIVReg<"TEMP31", 31, [TEMP31x, TEMP31y, TEMP31z, TEMP31w]>;

// Address register
def ADDR0x : TGSIReg<"ADDR0x", 0, 1>;
def ADDR0y : TGSIReg<"ADDR0y", 0, 2>;
def ADDR0z : TGSIReg<"ADDR0z", 0, 4>;
def ADDR0w : TGSIReg<"ADDR0w", 0, 8>;
def ADDR0 : TGSIVReg<"ADDR0", 0, [ADDR0x, ADDR0y, ADDR0z, ADDR0w]>;

// Register classes.
//
def IRegs : RegisterClass<"TGSI", [i32], 32,
            (add (sequence "TEMP%ux", 0, 31), (sequence "TEMP%uy", 0, 31),
                 (sequence "TEMP%uz", 0, 31), (sequence "TEMP%uw", 0, 31))>;
def IVRegs : RegisterClass<"TGSI", [v4i32], 128, (sequence "TEMP%u", 0, 31)>;
def FRegs : RegisterClass<"TGSI", [f32], 32,
            (add (sequence "TEMP%ux", 0, 31), (sequence "TEMP%uy", 0, 31),
                 (sequence "TEMP%uz", 0, 31), (sequence "TEMP%uw", 0, 31))>;
def FVRegs : RegisterClass<"TGSI", [v4f32], 128, (sequence "TEMP%u", 0, 31)>;
def ARegs : RegisterClass<"TGSI", [i32], 32,
            (add (sequence "ADDR%ux", 0, 0), (sequence "ADDR%uy", 0, 0),
                 (sequence "ADDR%uz", 0, 0), (sequence "ADDR%uw", 0, 0))>;
def AVRegs : RegisterClass<"TGSI", [i32], 128, (sequence "ADDR%u", 0, 0)>;

