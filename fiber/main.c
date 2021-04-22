#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <linux/futex.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <errno.h>
#include <stdatomic.h>
#include "fiber.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
               } while (0)



static uint32_t *futex1;
       static int
       futex(uint32_t *uaddr, int futex_op, uint32_t val,
             const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3)
       {
           return syscall(SYS_futex, uaddr, futex_op, val,
                          timeout, uaddr2, val3);
       }

   static void
   fwait(uint32_t *futexp)
   {
       long s;

       while (1) {
           /* Is the futex available? */
           const uint32_t one = 1;
           if (atomic_compare_exchange_strong(futexp, &one, 0))
               break;      /* Yes */

           /* Futex is not available; wait. */

           s = futex(futexp, FUTEX_WAIT, 0, NULL, NULL, 0);
           if (s == -1 && errno != EAGAIN)
               errExit("futex-FUTEX_WAIT");
       }
   }


   static void
   fpost(uint32_t *futexp)
   {
       long s;

       const uint32_t zero = 0;
       if (atomic_compare_exchange_strong(futexp, &zero, 1)) {
           s = futex(futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
           if (s  == -1)
               errExit("futex-FUTEX_WAKE");
       }
   }


static void fibonacci() {
    int fib[2] = {0, 1};
    printf("Fib(0) = 0\nFib(1) = 1\n");
    for (int i = 2; i < 15; ++i) {
        int nextFib = fib[0] + fib[1];
        fwait(futex1);
        printf("Fib(%d) = %d\n", i, nextFib);
        fpost(futex1);
        fib[0] = fib[1];
        fib[1] = nextFib;
        fiber_yield();
    }
}

static void squares() {
    for (int i = 1; i < 10; ++i) {
        fwait(futex1);
        printf("%d * %d = %d\n", i, i, i * i);
        fpost(futex1);
        fiber_yield();
    }
}

int main() {
    futex1 = mmap(NULL, sizeof(*futex1) , PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (futex1 == MAP_FAILED)
        errExit("mmap");

    *futex1 = 1;
    fiber_init();

    fiber_spawn(&fibonacci);
    fiber_spawn(&squares);

    /* Since fibers are non-preemptive, we must allow them to run */
    fiber_wait_all();

    return 0;

}
