;
; Regression Test: EnvironmentTest.ll
;
; Description:
;	This is a regression test that verifies that the JIT passes the
;	environment to the main() function.
;

target endian = little
target pointersize = 32

implementation

declare uint %strlen(sbyte*)

int %main(int %argc.1, sbyte** %argv.1, sbyte** %envp.1) {
	%tmp.2 = load sbyte** %envp.1		; <sbyte*> [#uses=2]
	%tmp.3 = call uint %strlen( sbyte* %tmp.2 )		; <uint> [#uses=1]
	%T = seteq uint %tmp.3, 0
	%R = cast bool %T to int	
	ret int %R
}

