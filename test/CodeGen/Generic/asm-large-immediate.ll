; RUN: llc < %s | FileCheck %s

; FIXME: Seek around r158932 to r158946.
; XFAIL: powerpc

define void @test() {
entry:
; CHECK: /* result: 68719476738 */
        tail call void asm sideeffect "/* result: ${0:c} */", "i,~{dirflag},~{fpsr},~{flags}"( i64 68719476738 )
; CHECK: /* result: -68719476738 */
        tail call void asm sideeffect "/* result: ${0:n} */", "i,~{dirflag},~{fpsr},~{flags}"( i64 68719476738 )
        ret void
}
