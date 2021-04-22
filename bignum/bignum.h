#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* how large the underlying array size should be */
#define UNIT_SIZE 4

/* These are dedicated to UNIT_SIZE == 4 */
#define UTYPE uint32_t
#define UTYPE_TMP uint64_t
#define FORMAT_STRING "%.08x"
#define MAX_VAL ((UTYPE_TMP) 0xFFFFFFFF)

#define BN_ARRAY_SIZE (8 / UNIT_SIZE) /* size of big-numbers in bytes */

/* bn consists of array of TYPE */
struct bn { UTYPE array[BN_ARRAY_SIZE]; };

static inline void bn_init(struct bn *n) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        n->array[i] = 0;
}

static inline void bn_from_int(struct bn *n, UTYPE_TMP i) {
    bn_init(n);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        n->array[0] = i;
        UTYPE_TMP tmp = i >> 32;
        n->array[1] = tmp;
#else
        n->array[1] = i;
        UTYPE_TMP tmp = i >> 32;
        n->array[0] = tmp;
#endif
}

static void bn_to_str(struct bn *n, char *str, int nbytes) {
    /* index into array - reading "MSB" first -> big-endian */
    int j = BN_ARRAY_SIZE - 1;
    int i = 0; /* index into string representation */

    /* reading last array-element "MSB" first -> big endian */
    while ((j >= 0) && (nbytes > (i + 1))) {
        sprintf(&str[i], FORMAT_STRING, n->array[j]);
        i += (2 *
              UNIT_SIZE); /* step UNIT_SIZE hex-byte(s) forward in the string */
        j -= 1;           /* step one element back in the array */
    }

    /* Count leading zeros: */
    for (j = 0; str[j] == '0'; j++)
        ;

    /* Move string j places ahead, effectively skipping leading zeros */
    for (i = 0; i < (nbytes - j); ++i)
        str[i] = str[i + j];

    str[i] = 0;
}

#define ErrOverflow 1

/* Decrement: subtract 1 from n */
static int bn_dec(struct bn *n) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
        UTYPE tmp = n->array[i];
        UTYPE res = tmp - 1;
        n->array[i] = res;

        if(!(res>tmp)) return 0;
    }
    return ErrOverflow;
}

static int bn_add(struct bn *a, struct bn *b, struct bn *c) {
    int carry = 0;
    for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
        UTYPE_TMP tmp = (UTYPE_TMP) a->array[i] + b->array[i] + carry;
        carry = (tmp > MAX_VAL);
        if(carry && i == (BN_ARRAY_SIZE - 1)){
            return ErrOverflow;
        }
        c->array[i] = (tmp & MAX_VAL);
    }
    return 0;
}

static inline void lshift_unit(struct bn *a, int n_units) {
    int i;
    /* Shift whole units */
    for (i = (BN_ARRAY_SIZE - 1); i >= n_units; --i)
        a->array[i] = a->array[i - n_units];
    /* Zero pad shifted units */
    for (; i >= 0; --i)
        a->array[i] = 0;
}

static int bn_mul(struct bn *a, struct bn *b, struct bn *c) {
    struct bn row, tmp;
    bn_init(c);
    for (int i = 0; i < BN_ARRAY_SIZE; ++i) {
        bn_init(&row);

        for (int j = 0; j < BN_ARRAY_SIZE; ++j) {
            if (i + j < BN_ARRAY_SIZE) {
                bn_init(&tmp);
                UTYPE_TMP intermediate = a->array[i]*(UTYPE_TMP) b->array[j];
                printf("a[%d]:%u, b[%d]:%u\n", i, a->array[i], j, b->array[j]);
                printf("i:%d, j:%d, intermediate: %0.8lx\n", i, j, intermediate);
                bn_from_int(&tmp, intermediate);
                lshift_unit(&tmp, i + j);
                int err = bn_add(&tmp, &row, &row);
                if(err) return ErrOverflow;
            }
        }
        int err = bn_add(c, &row, c);
        char c_buf[8192];                        
        bn_to_str(c, c_buf, sizeof(c_buf));
        printf("c = %s\n", c_buf);
        if(err) return ErrOverflow;
    }
    return 0;
}

static bool bn_is_zero(struct bn *n) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        if (n->array[i])
            return false;
    return true;
}

/* Copy src into dst. i.e. dst := src */
static void bn_assign(struct bn *dst, struct bn *src) {
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        dst->array[i] = src->array[i];
}
