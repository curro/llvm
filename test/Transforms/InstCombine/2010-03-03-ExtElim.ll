; RUN: opt -instcombine -S < %s | FileCheck %s
; PR6486

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-unknown-linux-gnu"

@g_92 = common global [2 x i32*] zeroinitializer, align 4 ; <[2 x i32*]*> [#uses=1]
@g_177 = constant i32** bitcast (i8* getelementptr (i8* bitcast ([2 x i32*]* @g_92 to i8*), i64 4) to i32**), align 4 ; <i32***> [#uses=1]

define i1 @test() nounwind {
; CHECK: @test
  %tmp = load i32*** @g_177                       ; <i32**> [#uses=1]
  %cmp = icmp ne i32** null, %tmp                 ; <i1> [#uses=1]
  %conv = zext i1 %cmp to i32                     ; <i32> [#uses=1]
  %cmp1 = icmp sle i32 0, %conv                   ; <i1> [#uses=1]
  ret i1 %cmp1
; CHECK: ret i1 true
}
