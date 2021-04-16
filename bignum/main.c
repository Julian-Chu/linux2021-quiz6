#include <stdio.h>
#include "bignum.h"
/*
 * factorial(N) := N * (N-1) * (N-2) * ... * 1
 */ 
static void factorial(struct bn *n, struct bn *res) {
    struct bn tmp; 
    bn_assign(&tmp, n);
    bn_dec(n); 
        
    while (!bn_is_zero(n)) {
        bn_mul(&tmp, n, res); /* res = tmp * n */
        bn_dec(n);            /* n -= 1 */
        bn_assign(&tmp, res); /* tmp = res */
    }
    bn_assign(res, &tmp);
}   
    
int main() {   
    struct bn num, result;
    char buf[8192];                        
    bn_from_int(&num, 100);
    factorial(&num, &result);
    bn_to_str(&result, buf, sizeof(buf));
    printf("factorial(100) = %s\n", buf);
    return 0;
}
