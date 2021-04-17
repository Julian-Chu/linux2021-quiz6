#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include "fiber.h"

static void fibonacci() {
    int fib[2] = {0, 1};
    printf("Fib(0) = 0\nFib(1) = 1\n");
    for (int i = 2; i < 15; ++i) {
        int nextFib = fib[0] + fib[1];
        printf("Fib(%d) = %d\n", i, nextFib);
        fib[0] = fib[1];
        fib[1] = nextFib;
        fiber_yield();
    }
}

static void squares() {
    for (int i = 1; i < 10; ++i) {
        printf("%d * %d = %d\n", i, i, i * i);
        fiber_yield();
    }
}

int main() {
    fiber_init();

    fiber_spawn(&fibonacci);
    fiber_spawn(&squares);

    /* Since fibers are non-preemptive, we must allow them to run */
    fiber_wait_all();

    return 0;

}
