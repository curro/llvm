; RUN: llc < %s -march=x86 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128"

define void @func(i32 %argc, i8** %argv) nounwind {
entry:
	%argc.addr = alloca i32		; <i32*> [#uses=1]
	%argv.addr = alloca i8**		; <i8***> [#uses=1]
	%val1 = alloca i32		; <i32*> [#uses=2]
	%val2 = alloca i32		; <i32*> [#uses=15]
	%andt = alloca i32		; <i32*> [#uses=2]
	%ort = alloca i32		; <i32*> [#uses=2]
	%xort = alloca i32		; <i32*> [#uses=2]
	%old = alloca i32		; <i32*> [#uses=18]
	%temp = alloca i32		; <i32*> [#uses=2]
	store i32 %argc, i32* %argc.addr
	store i8** %argv, i8*** %argv.addr
	store i32 0, i32* %val1
	store i32 31, i32* %val2
	store i32 3855, i32* %andt
	store i32 3855, i32* %ort
	store i32 3855, i32* %xort
	store i32 4, i32* %temp
	%tmp = load i32* %temp
        ; CHECK: lock
        ; CHECK: xaddl
  %0 = atomicrmw add i32* %val1, i32 %tmp monotonic
	store i32 %0, i32* %old
        ; CHECK: lock
        ; CHECK: xaddl
  %1 = atomicrmw sub i32* %val2, i32 30 monotonic
	store i32 %1, i32* %old
        ; CHECK: lock
        ; CHECK: xaddl
  %2 = atomicrmw add i32* %val2, i32 1 monotonic
	store i32 %2, i32* %old
        ; CHECK: lock
        ; CHECK: xaddl
  %3 = atomicrmw sub i32* %val2, i32 1 monotonic
	store i32 %3, i32* %old
        ; CHECK: andl
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %4 = atomicrmw and i32* %andt, i32 4080 monotonic
	store i32 %4, i32* %old
        ; CHECK: orl
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %5 = atomicrmw or i32* %ort, i32 4080 monotonic
	store i32 %5, i32* %old
        ; CHECK: xorl
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %6 = atomicrmw xor i32* %xort, i32 4080 monotonic
	store i32 %6, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %7 = atomicrmw min i32* %val2, i32 16 monotonic
	store i32 %7, i32* %old
	%neg = sub i32 0, 1		; <i32> [#uses=1]
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %8 = atomicrmw min i32* %val2, i32 %neg monotonic
	store i32 %8, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %9 = atomicrmw max i32* %val2, i32 1 monotonic
	store i32 %9, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %10 = atomicrmw max i32* %val2, i32 0 monotonic
	store i32 %10, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %11 = atomicrmw umax i32* %val2, i32 65535 monotonic
	store i32 %11, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %12 = atomicrmw umax i32* %val2, i32 10 monotonic
	store i32 %12, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %13 = atomicrmw umin i32* %val2, i32 1 monotonic
	store i32 %13, i32* %old
        ; CHECK: cmov
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %14 = atomicrmw umin i32* %val2, i32 10 monotonic
	store i32 %14, i32* %old
        ; CHECK: xchgl   %{{.*}}, {{.*}}(%esp)
  %15 = atomicrmw xchg i32* %val2, i32 1976 monotonic
	store i32 %15, i32* %old
	%neg1 = sub i32 0, 10		; <i32> [#uses=1]
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %16 = cmpxchg i32* %val2, i32 %neg1, i32 1 monotonic
	store i32 %16, i32* %old
        ; CHECK: lock
        ; CHECK: cmpxchgl
  %17 = cmpxchg i32* %val2, i32 1976, i32 1 monotonic
	store i32 %17, i32* %old
	ret void
}

define void @test2(i32 addrspace(256)* nocapture %P) nounwind {
entry:
; CHECK: lock
; CHECK:	cmpxchgl	%{{.*}}, %gs:(%{{.*}})

  %0 = cmpxchg i32 addrspace(256)* %P, i32 0, i32 1 monotonic
  ret void
}
