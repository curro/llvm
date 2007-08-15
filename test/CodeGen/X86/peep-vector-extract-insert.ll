; RUN: llvm-as < %s | llc -march=x86-64 | grep {pxor	%xmm0, %xmm0} | count 2

define float @foo(<4 x float> %a) {
  %b = insertelement <4 x float> %a, float 0.0, i32 3
  %c = extractelement <4 x float> %b, i32 3
  ret float %c
}
define float @bar(float %a) {
  %b = insertelement <4 x float> <float 3.4, float 4.5, float 0.0, float 9.2>, float %a, i32 3
  %c = extractelement <4 x float> %b, i32 2
  ret float %c
}
