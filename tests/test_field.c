//BSD 3-Clause License
//
//Copyright (c) 2018, jadeblaquiere
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <assert.h>
#include <ecurve.h>
#include <field.h>
#include <gmp.h>
#include <math.h>
#include <mpzurandom.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    _mpECurve_eq_type type;
    char *name;
    char *p;
    char *a;
    char *b;
    char *n;
    char *h;
    char *Gx;
    char *Gy;
    int bits;
} _std_ws_curve_t;

static _std_ws_curve_t _std_ws_curve[] = {
    {
        EQTypeShortWeierstrass,
        "secp256k1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F",
        "0",
        "7",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141",
        "1",
        "0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798",
        "0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8",
        256
    },
    {
        EQTypeShortWeierstrass,
        "secp256r1",
        "0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
        "0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
        "0x5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B",
        "0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551",
        "1",
        "0x6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
        "0x4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5",
        256
    },
};

static char p25519[] = "57896044618658097711785492504343953926634992332820282019728792003956564819949";
static char p112r1[] = "DB7C2ABF62E35E668076BEAD208B";

char *test_prime_fields[] = { "65521", "131071", "4294967291", "8589934583", "18446744073709551557", "36893488147419103183",
    "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF",
    "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed",
    "0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef",
    "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"};

#define ARRAY_SZ    (20000)

static FILE *_f_urandom = NULL;

unsigned long int ui_urandom(unsigned long int max) {
    int sz_read;
    unsigned long int value;
    if (_f_urandom == NULL) {
        _f_urandom = fopen("/dev/urandom", "rb");
        assert(_f_urandom != NULL);
    }
    // bytes intentionally long to ensure uniformity, will truncate with modulo
    sz_read = fread(&value, sizeof(unsigned long int), 1, _f_urandom);
    assert(sz_read == 1);
    if (max > 0) {
        value = value % max;
    }
    return value;
}

START_TEST(test_mpFp_neg_basic)
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpz_init(p);
    mpz_init(d);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_neg(c, a);
    assert(mpFp_cmp_ui(c, 5) == 0);
    mpFp_neg(c, b);
    assert(mpFp_cmp_ui(c, 8) == 0);
    mpFp_set_ui(b, 0, p);
    mpFp_neg(c, b);
    assert(mpFp_cmp_ui(c, 0) == 0);
    mpFp_set_ui(b, 1, p);
    mpFp_neg(c, b);
    assert(mpFp_cmp_ui(c, 16) == 0);
    mpFp_set_ui(b, 16, p);
    mpFp_neg(c, b);
    assert(mpFp_cmp_ui(c, 1) == 0);

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpz_clear(d);
    mpz_clear(p);
END_TEST

START_TEST(test_mpFp_add_basic)
    int64_t i,j;
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpz_t aa;
    mpz_t bb;
    mpz_init(p);
    mpz_init(d);
    mpz_init(aa);
    mpz_init(bb);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_add(c, a, b);
    assert(mpFp_cmp_ui(c, 4) == 0);
    mpFp_add_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 4) == 0);
    mpFp_neg(b,a);
    assert(mpFp_cmp_ui(b, 5) == 0);
    mpFp_add(c, a, b);
    assert(mpFp_cmp_ui(c, 0) == 0);

    for (i = 0; i < 17; i++) {
        for (j = 0; j < 17; j++) {
            mpFp_set_ui(a, i, p);
            mpFp_set_ui(b, j, p);
            mpFp_set_ui(c, (i + j) % 17, p);
            mpFp_add(a, a, b);
            assert(mpFp_cmp(a, c) == 0);
        }
    }

    // 2**255-19 (a prime number)
    //mpz_set_str(p, p25519, 10);
    // field constant for secp112r1 (a prime number)
    mpz_set_str(p, p112r1, 16);
    mpz_set_ui(d, 12);

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_add(c, a, b);
    assert(mpFp_cmp_ui(c, 21) == 0);
    mpFp_add_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 21) == 0);

    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            mpz_urandom(aa, p);
            mpFp_set_mpz(a, aa, p);
            mpz_urandom(bb, p);
            mpFp_set_mpz(b, bb, p);
            mpFp_add(a, a, b);
            mpz_add(aa, aa, bb);
            if (mpz_cmp(aa, p) >= 0) {
                mpz_sub(aa, aa, p);
            }
            assert(mpFp_cmp_mpz(a, aa) == 0);
        }
    }

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpz_clear(bb);
    mpz_clear(aa);
    mpz_clear(d);
    mpz_clear(p);
END_TEST

START_TEST(test_mpFp_add_extended)
    int i, j;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing ADD for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            bui[i] = ui_urandom(0);
        }

        // ADDITION
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_add(c, a[i], b[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_add(d, aa, bb);
            mpz_mod(d, d, p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_add_ui(c, a[i], bui[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_add_ui(d, aa, bui[i]);
            mpz_mod(d, d, p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        //gmp_printf("c[n] = 0x%ZX\n", aa);
        //gmp_printf("d[n] = 0x%ZX\n", d);

        mpz_add(d, c->fp->p, c->fp->pc);

        // corner cases for add ... carry conditions and add to -1, 0, 1 (mod p)

        // a+b = carry + 1, result = pc + 1
        mpz_set_mpFp(aa, a[0]);
        mpz_sub(bb, d, aa);
        mpz_add_ui(bb, bb, 1);
        mpFp_set_mpz(b[0], bb, p);

        // a+b = carry + 0, result = pc
        mpz_set_mpFp(aa, a[1]);
        mpz_sub(bb, d, aa);
        mpFp_set_mpz(b[1], bb, p);

        // a + b = carry - 1, result does not carry, but still > p
        mpz_set_mpFp(aa, a[2]);
        mpz_sub(bb, d, aa);
        mpz_sub_ui(bb, bb, 1);
        mpFp_set_mpz(b[2], bb, p);

        // a + b = p + 1, result = 1
        mpz_set_mpFp(aa, a[3]);
        mpz_sub(bb, p, aa);
        mpz_add_ui(bb, bb, 1);
        mpFp_set_mpz(b[3], bb, p);

        // a + b = p, result = 0
        mpz_set_mpFp(aa, a[4]);
        mpz_sub(bb, p, aa);
        mpFp_set_mpz(b[4], bb, p);

        // a + p = p - 1, result = p - 1
        mpz_set_mpFp(aa, a[5]);
        mpz_sub(bb, p, aa);
        mpz_sub_ui(bb, bb, 1);
        mpFp_set_mpz(b[5], bb, p);

        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        for (i = 0; i < 6; i++) {
            mpFp_add(c, a[i], b[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_add(d, aa, bb);
            mpz_mod(d, d, p);

            //printf("a[%d]: limbs = %d, limballoc = %d, limbsize = %lu\n", i, a[i]->i->_mp_size, a[i]->i->_mp_alloc, sizeof(a[i]->i->_mp_d[0]));
            //printf("b[%d]: limbs = %d, limballoc = %d, limbsize = %lu\n", i, b[i]->i->_mp_size, b[i]->i->_mp_alloc, sizeof(b[i]->i->_mp_d[0]));
            //printf("c: limbs = %d, limballoc = %d, limbsize = %lu\n", c->i->_mp_size, c->i->_mp_alloc, sizeof(c->i->_mp_d[0]));
            //printf("d: limbs = %d, limballoc = %d, limbsize = %lu\n", d->_mp_size, d->_mp_alloc, sizeof(d->_mp_d[0]));

            mpz_set_mpFp(aa, c);

            //gmp_printf("a[%d] = 0x%ZX\n", i, aa);
            //gmp_printf("b[%d] = 0x%ZX\n", i, bb);
            //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
            //gmp_printf("d[%d] = 0x%ZX\n", i, d);

            assert(mpz_cmp(d, aa) == 0);
        }
    
        mpFp_set(c, a[0]);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_add(c, c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_add(d, d, aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        printf("mpFp ADD rate = %g adds/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  ADD rate  = %g adds/sec\n", mpz_rate);
        assert(mpz_cmp(d, aa) == 0);
    
        mpFp_set_ui(c, bui[0], p);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_add_ui(c, c, bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_add_ui(d, d, bui[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        printf("mpFp ADD_UI rate = %g adds/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  ADD_UI rate  = %g adds/sec\n", mpz_rate);
        assert(mpz_cmp(d, aa) == 0);
    
        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_sub_basic)
    int i,j;
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpz_t aa, bb;
    mpz_init(p);
    mpz_init(d);
    mpz_init(aa);
    mpz_init(bb);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_sub(c, a, b);
    assert(mpFp_cmp_ui(c, 3) == 0);
    mpFp_sub(c, b, a);
    assert(mpFp_cmp_ui(c, 14) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_sub(c, a, b);
    assert(mpFp_cmp_ui(c, 3) == 0);
    mpFp_sub(c, b, a);
    mpFp_set_ui(b, 0, p);
    mpFp_sub_ui(b, b, 3);
    assert(mpFp_cmp(c, b) == 0);

    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            mpz_urandom(aa, p);
            mpFp_set_mpz(a, aa, p);
            mpz_urandom(bb, p);
            mpFp_set_mpz(b, bb, p);
            mpFp_sub(a, a, b);
            mpz_sub(aa, aa, bb);
            if (mpz_cmp_ui(aa, 0) < 0) {
                mpz_add(aa, aa, p);
            }
            assert(mpz_cmp(a->i, aa) == 0);
        }
    }

    mpz_clear(bb);
    mpz_clear(aa);
    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_sub_extended)
    int i, j;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing SUB for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            bui[i] = ui_urandom(0);
        }

        // SUBTRACTION
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sub(c, a[i], b[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_sub(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }
    
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sub_ui(c, a[i], bui[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_sub_ui(d, aa, bui[i]);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }
    
        // corner cases for sub ... borrow conditions -> -1, 0, 1 (mod p)
    
        // a-b = 1, result = 1
        mpz_set_mpFp(aa, a[0]);
        mpz_set(bb, aa);
        mpz_sub_ui(bb, bb, 1);
        mpFp_set_mpz(b[0], bb, p);
    
        // a-b = 0, result = 0
        mpz_set_mpFp(aa, a[1]);
        mpz_set(bb, aa);
        mpFp_set_mpz(b[1], bb, p);
    
        // a - b = - 1, result = p - (b - a) = p - 1
        mpz_set_mpFp(aa, a[2]);
        mpz_set(bb, aa);
        mpz_add_ui(bb, bb, 1);
        mpFp_set_mpz(b[2], bb, p);
    
        for (i = 0; i < 3; i++) {
            mpFp_sub(c, a[i], b[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_sub(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }
    
        mpFp_set(c, a[0]);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_sub(c, c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_sub(d, d, aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp SUB rate = %g subs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  SUB rate  = %g subs/sec\n", mpz_rate);

        mpFp_set_ui(c, bui[0], p);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_sub_ui(c, c, bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_sub_ui(d, d, bui[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp SUB_UI rate = %g subs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  SUB_UI rate  = %g subs/sec\n", mpz_rate);

        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_mul_basic)
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpz_init(p);
    mpz_init(d);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 6) == 0);
    mpFp_swap(b, a);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 6) == 0);
    mpFp_mul_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 13) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 108) == 0);
    mpFp_mul_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 108) == 0);

    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_mul_extended)
    int i, j;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing MUL for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            bui[i] = ui_urandom(0);
        }

        // Multiplication
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_mul(c, a[i], b[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_mul(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_mul_ui(c, a[i], bui[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_mul_ui(d, aa, bui[i]);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        mpFp_set(c, a[0]);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_mul(c, c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mul(d, d, aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp MUL rate = %g muls/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  MUL rate  = %g muls/sec\n", mpz_rate);
        
        mpFp_set_ui(c, bui[0], p);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_mul_ui(c, c, bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mul_ui(d, d, bui[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp MUL_UI rate = %g muls/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  MUL_UI rate  = %g muls/sec\n", mpz_rate);
        
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mul(ccc[i], aaa[i], aaa[i-1]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mod(ccc[i], ccc[i], p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        printf("mpz  MUL(noMOD) rate = %g ops/sec\n", fp_rate);
        printf("mpz  MOD(noMUL) rate  = %g ops/sec\n", mpz_rate);

        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_pow_basic)
    mpFp_t a, b, c;
    mpz_t p, d, e;
    mpz_init(p);
    mpz_init(d);
    mpz_init(e);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpz_set_mpFp(e, b);
    //mpFp_pow(c, a, e);
    //assert(mpFp_cmp_ui(c, 5) == 0);
    mpFp_swap(b, a);
    mpz_set_mpFp(e, b);
    //mpFp_pow(c, a, e);
    //assert(mpFp_cmp_ui(c, 16) == 0);
    mpFp_pow_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 9) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpz_set_mpFp(e, b);
    //mpFp_pow(c, a, e);
    //assert(mpFp_cmp_ui(c, 5159780352) == 0);
    mpFp_pow_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 5159780352) == 0);

    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_pow_extended)
    int i, j;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t bbb[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing POW for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(bbb[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            mpz_set_mpFp(bbb[i], b[i]);
            bui[i] = ui_urandom(0);
        }

        // Exponentiation
        for (i = 0; i < (ARRAY_SZ >> 4); i++) {
            mpFp_pow_ui(c, a[i], bui[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_powm_ui(d, aa, bui[i], p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);

            mpFp_pow_mpz(c, a[i], bbb[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_powm(d, aa, bbb[i], p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        start_time = clock();
        for (i = 0; i < (ARRAY_SZ >> 4); i++) {
            mpFp_pow_ui(c, a[i], bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)(ARRAY_SZ >> 4) * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 0; i < (ARRAY_SZ >> 4); i++) {
            mpz_powm_ui(d, aaa[i], bui[i], p);
        }
        stop_time = clock();
        mpz_rate = ((double)(ARRAY_SZ >> 4) * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp POWM rate = %g exps/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  POWM rate = %g exps/sec\n", mpz_rate);
        
        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_sqr_extended)
    int i, j;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    //unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing SQR for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            //bui[i] = ui_urandom(0);
        }

        // Squaring
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sqr(c, a[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, a[i]);
            mpz_mul(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);

            mpz_powm_ui(d, bb, 2, p);
            assert(mpz_cmp(d, aa) == 0);
        }

        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sqr(c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpz_mul(d, aaa[i], aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp SQR rate = %g sqrs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  SQR rate = %g sqrs/sec\n", mpz_rate);
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpz_powm_ui(d, aaa[i], 2, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        printf("mpz  SQR rate = %g sqrs/sec (using powm_ui)\n", mpz_rate);
        
        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_inv_basic)
    mpFp_t a, b, c;
    mpz_t p, d, e;
    mpz_init(p);
    mpz_init(d);
    mpz_init(e);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    mpFp_set_mpz(a, d, p);
    mpz_sub_ui(e, p, 2);
    //mpFp_pow(c, a, e);
    mpFp_inv(b, a);
    //assert(mpFp_cmp_ui(c, 10) == 0);
    assert(mpFp_cmp_ui(b, 10) == 0);
    //assert(mpFp_cmp(c, b) == 0);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 1) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);

    // compare extended euclidean algorithm result to inverse by pow
    // from Fermat's little thereom
    mpFp_set_mpz(a, d, p);
    mpz_sub_ui(e, p, 2);
    //mpFp_pow(c, a, e);
    mpFp_inv(b, a);
    //assert(mpFp_cmp(c, b) == 0);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 1) == 0);

    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_inv_extended)
    int i, j;
    int nfields;
    int status, rstatus;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    //unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing INV for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            //bui[i] = ui_urandom(0);
        }

        // Inverse
        for (i = 0; i < ARRAY_SZ; i++) {
            status = mpFp_inv(c, a[i]);
    
            mpz_set_mpFp(aa, a[i]);
            rstatus = mpz_invert(d, aa, p);

            assert(rstatus != status);
            if (status == 0) {
                mpz_set_mpFp(aa, c);
                assert(mpz_cmp(d, aa) == 0);
            }
        }

        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_inv(c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpz_invert(d, aaa[i], p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp INV rate = %g sqrs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  INV rate = %g sqrs/sec\n", mpz_rate);

        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_swap_cswap)
    mpFp_t a, b, c, d;
    mpz_t p;
    mpz_t e;
    mpz_init(p);
    mpz_init(e);

    mpz_set_ui(p, 17);
    mpz_set_ui(e, 12);

    mpFp_init(a, p);
    mpFp_init(b, p);
    mpFp_init(c, p);
    mpFp_init(d, p);

    mpFp_set_mpz(a, e, p);
    mpFp_set_ui(b, 9, p);
    mpFp_set(c, a);
    mpFp_set(d, b);
    mpFp_swap(a, b);
    assert(mpFp_cmp(c, b) == 0);
    assert(mpFp_cmp(c, a) != 0);
    assert(mpFp_cmp(d, b) != 0);
    assert(mpFp_cmp(d, a) == 0);

    mpFp_set_mpz(a, e, p);
    mpFp_set_ui(b, 9, p);
    mpFp_set(c, a);
    mpFp_set(d, b);
    mpFp_cswap(a, b, 0);
    assert(mpFp_cmp(c, b) != 0);
    assert(mpFp_cmp(c, a) == 0);
    assert(mpFp_cmp(d, b) == 0);
    assert(mpFp_cmp(d, a) != 0);

    mpFp_set_mpz(a, e, p);
    mpFp_set_ui(b, 9, p);
    mpFp_set(c, a);
    mpFp_set(d, b);
    mpFp_cswap(a, b, 1);
    assert(mpFp_cmp(c, b) == 0);
    assert(mpFp_cmp(c, a) != 0);
    assert(mpFp_cmp(d, b) != 0);
    assert(mpFp_cmp(d, a) == 0);

    mpz_clear(e);
    mpz_clear(p);
    mpFp_clear(d);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_cswap_extended)
    int i, j, ii;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    //unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing CSWAP for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            //bui[i] = ui_urandom(0);
        }

        // Conditional Swap
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpFp_set(c, a[i]);
            mpFp_cswap(a[i], b[i], ii);
            if (ii == 0) {
                assert(mpFp_cmp(c, a[i]) == 0);
            } else {
                assert(mpFp_cmp(c, b[i]) == 0);
            }
            mpFp_cswap(a[i], b[i], ii);
        }

        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpFp_cswap(a[i], b[i], ii);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpFp_cswap(a[i], b[i], ii);
        }
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpz_set(d, aaa[i]);
            mpz_set(e, ccc[i]);
            if (ii == 0) {
                mpz_set(aaa[i], d);
                mpz_set(ccc[i], e);
            } else {
                mpz_set(aaa[i], e);
                mpz_set(ccc[i], d);
            }
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpz_set(d, aaa[i]);
            mpz_set(e, ccc[i]);
            if (ii == 0) {
                mpz_set(aaa[i], d);
                mpz_set(ccc[i], e);
            } else {
                mpz_set(aaa[i], e);
                mpz_set(ccc[i], d);
            }
        }
        printf("mpFp CSWAP rate = %g swaps/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  CSWAP rate = %g swaps/sec\n", mpz_rate);
        
        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_sqrt_basic)
    int i, j, error, bb, cc;
    int primes[] = {11, 13, 17, 19, 23, 29, 31, 1021};
    mpFp_t a, b, c;
    mpz_t p;
    mpz_init(p);

    for (j = 0 ; j < (sizeof(primes)/sizeof(primes[0])); j++) {
        mpz_set_ui(p, primes[j]);
        mpFp_init(a, p);
        mpFp_init(b, p);
        mpFp_init(c, p);
        for (i = 0; i < primes[j]; i++) {
            mpFp_set_ui(a, i, p);
            error = mpFp_sqrt(b, a);
            if (error != 0) {
                printf("%d is not a quadratic residue\n", i);
                continue;
            }
            mpFp_neg(c, b);
            bb = mpz_get_ui(b->i);
            cc = mpz_get_ui(c->i);
            mpFp_pow_ui(b, b, 2);
            assert(mpFp_cmp(a, b) == 0);
            mpFp_pow_ui(c, c, 2);
            assert(mpFp_cmp(a, c) == 0);
            printf("Square roots of %d (mod %d) are %d, %d\n", i, primes[j], bb, cc);
        }
        mpFp_clear(c);
        mpFp_clear(b);
        mpFp_clear(a);
    }

    mpz_clear(p);
END_TEST

START_TEST(test_mpFp_sqrt_extended)
    int i, j;
    int nfields;
    int status;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    //unsigned long int bui[ARRAY_SZ];
    mpFp_t c, cc;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        gmp_printf("Testing SQRT for field 0x%ZX\n", p);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);
        mpFp_init(cc, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            //bui[i] = ui_urandom(0);
        }

        // Squaring
        for (i = 0; i < ARRAY_SZ; i++) {
            status = mpFp_sqrt(c, a[i]);
            
            if (status == 0) {
                mpFp_sqr(cc, c);
                assert(mpFp_cmp(cc, a[i]) == 0);
                mpFp_neg(cc, c);
                mpFp_sqr(cc, cc);
                assert(mpFp_cmp(cc, a[i]) == 0);
            }
        }

        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sqrt(c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        printf("mpFp SQRT rate = %g sqrts/sec\n", fp_rate);

        mpFp_clear(cc);
        mpFp_clear(c);

        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
END_TEST

START_TEST(test_mpFp_tstbit)
    int i, j, k, bit;
    int primes[] = {11, 13, 17, 19, 23, 29, 31};
    mpFp_t a, b, c;
    mpz_t p;
    mpz_init(p);

    for (j = 0 ; j < (sizeof(primes)/sizeof(primes[0])); j++) {
        mpz_set_ui(p, primes[j]);
        mpFp_init(a, p);
        mpFp_init(b, p);
        mpFp_init(c, p);
        for (i = 0; i < primes[j]; i++) {
            mpFp_set_ui(a, i, p);
            mpFp_set_ui(b, 0, p);
            for (k = 0; pow(2,k) <= primes[j]; k++) {
                mpFp_set_ui(c, 2, p);
                mpFp_pow_ui(c, c, k);
                bit = mpFp_tstbit(a, k);
                mpFp_mul_ui(c, c, bit);
                mpFp_add(b, b, c);
            }
            assert(mpFp_cmp(a, b) == 0);
        }
        mpFp_clear(c);
        mpFp_clear(a);
    }

    mpz_clear(p);
END_TEST

START_TEST(test_mpFp_urandom)
    int i;
    mpz_t a;
    mpFp_t b;
    mpz_init(a);

    mpz_set_ui(a, 251);
    mpFp_init(b, a);

    for (i = 0; i < 1000; i++) {
        mpFp_urandom(b, a);
        //gmp_printf("A: %ZX\n", a);
        //gmp_printf("B: %ZX\n\n", b->i);
        assert (mpz_cmp(b->fp->p, a) == 0);
        assert (mpz_cmp_ui(b->i, 0) >= 0);
        assert (mpz_cmp(a,b->i) >= 0);
        mpz_add(a, a, b->i);
    }

    mpFp_clear(b);
    mpz_clear(a);
END_TEST

START_TEST(test_mpFp_point_check)
    int ncurve;
    int i;
    mpz_t z_p, z_a, z_b, z_n, z_h, z_gx, z_gy, z_r, z_l, t;
    mpFp_t f_a, f_b, f_gx, f_gy, f_r, f_l;
    mpECurve_t cv;

    mpECurve_init(cv);
    mpz_init(z_p);
    mpz_init(z_a);
    mpz_init(z_b);
    mpz_init(z_n);
    mpz_init(z_h);
    mpz_init(z_gx);
    mpz_init(z_gy);
    mpz_init(z_r);
    mpz_init(z_l);
    mpz_init(t);

    ncurve = sizeof(_std_ws_curve) / sizeof(_std_ws_curve_t);

    for (i = 0; i < ncurve; i++) {
        printf("testing for curve %s\n", _std_ws_curve[i].name);
        mpz_set_str(z_p, _std_ws_curve[i].p, 0);

        mpFp_init(f_a, z_p);
        mpFp_init(f_b, z_p);
        mpFp_init(f_gx, z_p);
        mpFp_init(f_gy, z_p);
        mpFp_init(f_r, z_p);
        mpFp_init(f_l, z_p);

        mpz_set_str(z_a, _std_ws_curve[i].a, 0);
        mpFp_set_mpz(f_a, z_a, z_p);
        mpz_set_mpFp(t, f_a);
        assert(mpz_cmp(t, z_a) == 0);
        mpz_set_str(z_b, _std_ws_curve[i].b, 0);
        mpFp_set_mpz(f_b, z_b, z_p);
        mpz_set_mpFp(t, f_b);
        assert(mpz_cmp(t, z_b) == 0);
        mpz_set_str(z_n, _std_ws_curve[i].n, 0);
        mpz_set_str(z_h, _std_ws_curve[i].h, 0);
        mpz_set_str(z_gx, _std_ws_curve[i].Gx, 0);
        mpFp_set_mpz(f_gx, z_gx, z_p);
        mpz_set_mpFp(t, f_gx);
        assert(mpz_cmp(t, z_gx) == 0);
        mpz_set_str(z_gy, _std_ws_curve[i].Gy, 0);
        mpFp_set_mpz(f_gy, z_gy, z_p);
        mpz_set_mpFp(t, f_gy);
        assert(mpz_cmp(t, z_gy) == 0);

        mpFp_pow_ui(f_r, f_gx, 3);
        mpz_pow_ui(z_r, z_gx, 3);
        mpz_mod(z_r, z_r, z_p);
        mpz_set_mpFp(t, f_r);
        assert(mpz_cmp(t, z_r) == 0);
        mpFp_mul(f_l, f_a, f_gx);
        mpz_mul(z_l, z_a, z_gx);
        mpz_mod(z_l, z_l, z_p);
        mpz_set_mpFp(t, f_l);
        assert(mpz_cmp(t, z_l) == 0);
        mpFp_add(f_r, f_r, f_l);
        mpz_add(z_r, z_r, z_l);
        mpz_mod(z_r, z_r, z_p);
        mpz_set_mpFp(t, f_r);
        assert(mpz_cmp(t, z_r) == 0);
        mpFp_add(f_r, f_r, f_b);
        mpz_add(z_r, z_r, z_b);
        mpz_mod(z_r, z_r, z_p);
        mpz_set_mpFp(t, f_r);
        assert(mpz_cmp(t, z_r) == 0);
        mpFp_pow_ui(f_l, f_gy, 2);
        mpz_pow_ui(z_l, z_gy, 2);
        mpz_mod(z_l, z_l, z_p);
        mpz_set_mpFp(t, f_l);
        assert(mpz_cmp(t, z_l) == 0);
        assert(mpFp_cmp(f_l, f_r) == 0);
        assert(mpz_cmp(z_l, z_r) == 0);

    // basic approach: calculate left and right sides and then compare l=r?

    //switch (cv->type) {
    //    case EQTypeShortWeierstrass: {
    //        // y**2 = x**3 + ax + b
    //        mpFp_pow_ui(r, x, 3);
    //        mpFp_mul(l, cv->coeff.ws.a, x);
    //        mpFp_add(r, r, l);
    //        mpFp_add(r, r, cv->coeff.ws.b);
    //        mpFp_pow_ui(l, y, 2);
    //        break;

        mpECurve_set_str_ws(cv, _std_ws_curve[i].p, _std_ws_curve[i].a,
            _std_ws_curve[i].b, _std_ws_curve[i].n, _std_ws_curve[i].h,
            _std_ws_curve[i].Gx, _std_ws_curve[i].Gy, _std_ws_curve[i].bits);

        mpFp_clear(f_l);
        mpFp_clear(f_r);
        mpFp_clear(f_gy);
        mpFp_clear(f_gx);
        mpFp_clear(f_b);
        mpFp_clear(f_a);
    }

    mpz_clear(t);
    mpz_clear(z_l);
    mpz_clear(z_r);
    mpz_clear(z_gy);
    mpz_clear(z_gx);
    mpz_clear(z_h);
    mpz_clear(z_n);
    mpz_clear(z_b);
    mpz_clear(z_a);
    mpz_clear(z_p);

    mpECurve_clear(cv);
END_TEST

static Suite *mpFp_test_suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("Multi-Precision over Prime Fields");
    tc = tcase_create("arithmetic");

    tcase_add_test(tc, test_mpFp_neg_basic);
    tcase_add_test(tc, test_mpFp_add_basic);
    tcase_add_test(tc, test_mpFp_add_extended);
    tcase_add_test(tc, test_mpFp_sub_basic);
    tcase_add_test(tc, test_mpFp_sub_extended);
    tcase_add_test(tc, test_mpFp_swap_cswap);
    tcase_add_test(tc, test_mpFp_cswap_extended);
    tcase_add_test(tc, test_mpFp_mul_basic);
    tcase_add_test(tc, test_mpFp_mul_extended);
    tcase_add_test(tc, test_mpFp_pow_basic);
    tcase_add_test(tc, test_mpFp_pow_extended);
    tcase_add_test(tc, test_mpFp_sqr_extended);
    tcase_add_test(tc, test_mpFp_inv_basic);
    tcase_add_test(tc, test_mpFp_inv_extended);
    tcase_add_test(tc, test_mpFp_sqrt_basic);
    tcase_add_test(tc, test_mpFp_sqrt_extended);
    tcase_add_test(tc, test_mpFp_tstbit);
    tcase_add_test(tc, test_mpFp_urandom);
    tcase_add_test(tc, test_mpFp_point_check);

     // set no timeout instead of default 4
    tcase_set_timeout(tc, 0.0);

    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = mpFp_test_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
