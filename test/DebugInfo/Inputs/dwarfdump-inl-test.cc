#include "dwarfdump-inl-test.h"
static inline int inlined_f() {
  volatile int x = inlined_g();
  return x;
}

int main() {
  return inlined_f();
}

// Built with Clang 3.2
// $ mkdir -p /tmp/dbginfo
// $ cp dwarfdump-inl-test.* /tmp/dbginfo
// $ cd /tmp/dbginfo
// $ clang++ -O2 -gline-tables-only -fsanitize=address -fPIC -shared dwarfdump-inl-test.cc -o <output>
