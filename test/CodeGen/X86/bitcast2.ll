; RUN: llvm-as < %s | llc -march=x86-64 | grep movd | wc -l | grep 2
; RUN: llvm-as < %s | llc -march=x86-64 | not grep rsp

define i64 @test1(double %A) {
   %B = bitcast double %A to i64
   ret i64 %B
}

define double @test2(i64 %A) {
   %B = bitcast i64 %A to double
   ret double %B
}

