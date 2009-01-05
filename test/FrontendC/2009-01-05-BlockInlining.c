// RUN: %llvmgcc %s -S -emit-llvm -O2 -o - | grep {call i32 .*printf.*argc} | count 3
// rdar://5865221

// All of these should be inlined equivalently into a single printf call.

static int fun(int x) {
	return x+1;
}

static int block(int x) {
	return (^(int x){return x+1;})(x);
}

static void print(int result) {
    printf("%d\n", result);
}

int main (int argc, const char * argv[]) {
    int	x = argc-1;
    print(fun(x));
    print(block(x));
    int	(^block_inline)(int) = ^(int x){return x+1;};
    print(block_inline(x));
    return 0;
}

