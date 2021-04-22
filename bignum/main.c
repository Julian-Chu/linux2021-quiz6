#include <stdio.h>
#include "bignum.h"
/*
 * factorial(N) := N * (N-1) * (N-2) * ... * 1
 */ 
static int factorial(struct bn *n, struct bn *res) {
    int err = 0;
    struct bn tmp; 
    bn_assign(&tmp, n);
    err = bn_dec(n); 
    if(err) return err;
        
    while (!bn_is_zero(n)) {
        err = bn_mul(&tmp, n, res); /* res = tmp * n */
        if(err) return err;
        err = bn_dec(n);            /* n -= 1 */
        if(err) return err;
        bn_assign(&tmp, res); /* tmp = res */
    }
    bn_assign(res, &tmp);
    return 0;
}   
    
/* int main() { */   
/*     struct bn num, result; */
/*     char buf[8192]; */                        
/*     bn_from_int(&num, 100); */
/*     int err = factorial(&num, &result); */
/*     if(err) printf("big num overflow \n"); */
/*     bn_to_str(&result, buf, sizeof(buf)); */
/*     printf("factorial(100) = %s\n", buf); */
/*     return 0; */
/* } */


int main() {   
    struct bn num, num2, result;
    bn_from_int(&num, 0);
    int err = bn_dec(&num);
    if(err) {
        printf("big num overflow \n");
    }else{
        printf("ok\n");
    }

    bn_from_int(&num, UINTMAX_MAX);
    bn_from_int(&num2, 2);
    err = bn_add(&num, &num2, &result);
    if(err) {
        printf("bn_add: big num overflow \n");
    }else{
        printf("bn_add: ok\n");
    }

    bn_from_int(&num, UINTMAX_MAX);
    char buf[8192];                        
    bn_to_str(&num, buf, sizeof(buf));
    printf("result = %s\n", buf);

    printf("UINTMAX_MAX: %lu\n", UINTMAX_MAX);
    printf("UINT32: %u\n", 0xFFFFFFFF);
    bn_from_int(&num2, 2);
    err = bn_mul(&num, &num2, &result);
    if(err) {
        printf("bn_mul: big num overflow \n");
    }else{
        printf("bn_mul: ok\n");
    }

    return 0;
}
