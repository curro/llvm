# Check the basic discovery process, including a sub-suite.
#
# RUN: %{lit} %{inputs}/discovery \
# RUN:   -j 1 --debug --no-execute --show-suites -v > %t.out 2> %t.err
# RUN: FileCheck --check-prefix=CHECK-BASIC-OUT < %t.out %s
# RUN: FileCheck --check-prefix=CHECK-BASIC-ERR < %t.err %s
#
# CHECK-BASIC-ERR: loading suite config '{{.*}}/discovery/lit.cfg'
# CHECK-BASIC-ERR: loading local config '{{.*}}/discovery/subdir/lit.local.cfg'
# CHECK-BASIC-ERR: loading suite config '{{.*}}/discovery/subsuite/lit.cfg'
#
# CHECK-BASIC-OUT: -- Test Suites --
# CHECK-BASIC-OUT:   sub-suite - 2 tests
# CHECK-BASIC-OUT:     Source Root: {{.*/discovery/subsuite$}}
# CHECK-BASIC-OUT:     Exec Root  : {{.*/discovery/subsuite$}}
# CHECK-BASIC-OUT:   top-level-suite - 3 tests
# CHECK-BASIC-OUT:     Source Root: {{.*/discovery$}}
# CHECK-BASIC-OUT:     Exec Root  : {{.*/discovery$}}
#
# CHECK-BASIC-OUT: -- Testing: 5 tests, 1 threads --
# CHECK-BASIC-OUT: PASS: sub-suite :: test-one
# CHECK-BASIC-OUT: PASS: sub-suite :: test-two
# CHECK-BASIC-OUT: PASS: top-level-suite :: subdir/test-three
# CHECK-BASIC-OUT: PASS: top-level-suite :: test-one
# CHECK-BASIC-OUT: PASS: top-level-suite :: test-two


# Check discovery when exact test names are given.
#
# RUN: %{lit} \
# RUN:     %{inputs}/discovery/subdir/test-three.py \
# RUN:     %{inputs}/discovery/subsuite/test-one.txt \
# RUN:   -j 1 --no-execute --show-suites -v > %t.out
# RUN: FileCheck --check-prefix=CHECK-EXACT-TEST < %t.out %s
#
# CHECK-EXACT-TEST: -- Testing: 2 tests, 1 threads --
# CHECK-EXACT-TEST: PASS: sub-suite :: test-one
# CHECK-EXACT-TEST: PASS: top-level-suite :: subdir/test-three


# Check discovery when using an exec path.
#
# RUN: %{lit} %{inputs}/exec-discovery \
# RUN:   -j 1 --debug --no-execute --show-suites -v > %t.out 2> %t.err
# RUN: FileCheck --check-prefix=CHECK-ASEXEC-OUT < %t.out %s
# RUN: FileCheck --check-prefix=CHECK-ASEXEC-ERR < %t.err %s
#
# CHECK-ASEXEC-ERR: loading suite config '{{.*}}/exec-discovery/lit.site.cfg'
# CHECK-ASEXEC-ERR: load_config from '{{.*}}/discovery/lit.cfg'
# CHECK-ASEXEC-ERR: loaded config '{{.*}}/discovery/lit.cfg'
# CHECK-ASEXEC-ERR: loaded config '{{.*}}/exec-discovery/lit.site.cfg'
# CHECK-ASEXEC-ERR: loading local config '{{.*}}/discovery/subdir/lit.local.cfg'
# CHECK-ASEXEC-ERR: loading suite config '{{.*}}/discovery/subsuite/lit.cfg'
#
# CHECK-ASEXEC-OUT: -- Test Suites --
# CHECK-ASEXEC-OUT:   sub-suite - 2 tests
# CHECK-ASEXEC-OUT:     Source Root: {{.*/discovery/subsuite$}}
# CHECK-ASEXEC-OUT:     Exec Root  : {{.*/discovery/subsuite$}}
# CHECK-ASEXEC-OUT:   top-level-suite - 3 tests
# CHECK-ASEXEC-OUT:     Source Root: {{.*/discovery$}}
# CHECK-ASEXEC-OUT:     Exec Root  : {{.*/exec-discovery$}}
#
# CHECK-ASEXEC-OUT: -- Testing: 5 tests, 1 threads --
# CHECK-ASEXEC-OUT: PASS: sub-suite :: test-one
# CHECK-ASEXEC-OUT: PASS: sub-suite :: test-two
# CHECK-ASEXEC-OUT: PASS: top-level-suite :: subdir/test-three
# CHECK-ASEXEC-OUT: PASS: top-level-suite :: test-one
# CHECK-ASEXEC-OUT: PASS: top-level-suite :: test-two


# Check discovery when exact test names are given.
#
# FIXME: Note that using a path into a subsuite doesn't work correctly here.
#
# RUN: %{lit} \
# RUN:     %{inputs}/exec-discovery/subdir/test-three.py \
# RUN:   -j 1 --no-execute --show-suites -v > %t.out
# RUN: FileCheck --check-prefix=CHECK-ASEXEC-EXACT-TEST < %t.out %s
#
# CHECK-ASEXEC-EXACT-TEST: -- Testing: 1 tests, 1 threads --
# CHECK-ASEXEC-EXACT-TEST: PASS: top-level-suite :: subdir/test-three


# Check that we don't recurse infinitely when loading an site specific test
# suite located inside the test source root.
#
# RUN: %{lit} \
# RUN:     %{inputs}/exec-discovery-in-tree/obj/ \
# RUN:   -j 1 --no-execute --show-suites -v > %t.out
# RUN: FileCheck --check-prefix=CHECK-ASEXEC-INTREE < %t.out %s
#
#      CHECK-ASEXEC-INTREE:   exec-discovery-in-tree-suite - 1 tests
# CHECK-ASEXEC-INTREE-NEXT:     Source Root: {{.*/exec-discovery-in-tree$}}
# CHECK-ASEXEC-INTREE-NEXT:     Exec Root  : {{.*/exec-discovery-in-tree/obj$}}
# CHECK-ASEXEC-INTREE-NEXT: -- Testing: 1 tests, 1 threads --
# CHECK-ASEXEC-INTREE-NEXT: PASS: exec-discovery-in-tree-suite :: test-one
