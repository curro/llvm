; This should turn into a single extsh
; RUN: llvm-upgrade < %s | llvm-as | llc -march=ppc32 | grep extsh | count 1
int %test(int %X) {
        %tmp.81 = shl int %X, ubyte 16             ; <int> [#uses=1]
        %tmp.82 = shr int %tmp.81, ubyte 16             ; <int> [#uses=1]
        ret int %tmp.82
}
