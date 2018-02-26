//BSD 3-Clause License
//
//Copyright (c) 2017, jadeblaquiere
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

START_TEST(test_mpFp_add)
    int64_t i,j;
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpz_t aa;
    mpz_t bb;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);
    mpz_init(d);
    mpz_init(aa);
    mpz_init(bb);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

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

START_TEST(test_mpFp_point_check)
    int ncurve;
    int i;
    mpz_t z_p, z_a, z_b, z_n, z_h, z_gx, z_gy, z_r, z_l;
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

    mpFp_init(f_a);
    mpFp_init(f_b);
    mpFp_init(f_gx);
    mpFp_init(f_gy);
    mpFp_init(f_r);
    mpFp_init(f_l);

    ncurve = sizeof(_std_ws_curve) / sizeof(_std_ws_curve_t);

    for (i = 0; i < ncurve; i++) {
        printf("testing for curve %s\n", _std_ws_curve[i].name);
        mpz_set_str(z_p, _std_ws_curve[i].p, 0);
        mpz_set_str(z_a, _std_ws_curve[i].a, 0);
        mpFp_set_mpz(f_a, z_a, z_p);
        assert(mpz_cmp(f_a->i, z_a) == 0);
        mpz_set_str(z_b, _std_ws_curve[i].b, 0);
        mpFp_set_mpz(f_b, z_b, z_p);
        assert(mpz_cmp(f_b->i, z_b) == 0);
        mpz_set_str(z_n, _std_ws_curve[i].n, 0);
        mpz_set_str(z_h, _std_ws_curve[i].h, 0);
        mpz_set_str(z_gx, _std_ws_curve[i].Gx, 0);
        mpFp_set_mpz(f_gx, z_gx, z_p);
        assert(mpz_cmp(f_gx->i, z_gx) == 0);
        mpz_set_str(z_gy, _std_ws_curve[i].Gy, 0);
        mpFp_set_mpz(f_gy, z_gy, z_p);
        assert(mpz_cmp(f_gy->i, z_gy) == 0);

        mpFp_pow_ui(f_r, f_gx, 3);
        mpz_pow_ui(z_r, z_gx, 3);
        mpz_mod(z_r, z_r, z_p);
        assert(mpz_cmp(f_r->i, z_r) == 0);
        mpFp_mul(f_l, f_a, f_gx);
        mpz_mul(z_l, z_a, z_gx);
        mpz_mod(z_l, z_l, z_p);
        assert(mpz_cmp(f_l->i, z_l) == 0);
        mpFp_add(f_r, f_r, f_l);
        mpz_add(z_r, z_r, z_l);
        mpz_mod(z_r, z_r, z_p);
        assert(mpz_cmp(f_r->i, z_r) == 0);
        mpFp_add(f_r, f_r, f_b);
        mpz_add(z_r, z_r, z_b);
        mpz_mod(z_r, z_r, z_p);
        assert(mpz_cmp(f_r->i, z_r) == 0);
        mpFp_pow_ui(f_l, f_gy, 2);
        mpz_pow_ui(z_l, z_gy, 2);
        mpz_mod(z_l, z_l, z_p);
        assert(mpz_cmp(f_l->i, z_l) == 0);
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
    }

    mpFp_clear(f_l);
    mpFp_clear(f_r);
    mpFp_clear(f_gy);
    mpFp_clear(f_gx);
    mpFp_clear(f_b);
    mpFp_clear(f_a);

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

START_TEST(test_mpFp_sub)
    int i,j;
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpz_t aa, bb;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);
    mpz_init(d);
    mpz_init(aa);
    mpz_init(bb);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_sub(c, a, b);
    assert(mpFp_cmp_ui(c, 3) == 0);
    mpFp_sub(c, b, a);
    assert(mpFp_cmp_ui(c, 14) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

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

START_TEST(test_mpFp_mul)
    mpFp_t a, b, c;
    mpz_t p;
    mpz_t d;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);
    mpz_init(d);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

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

START_TEST(test_mpFp_pow)
    mpFp_t a, b, c;
    mpz_t p, d, e;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);
    mpz_init(d);
    mpz_init(e);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpz_set_mpFp(e, b);
    mpFp_pow(c, a, e);
    assert(mpFp_cmp_ui(c, 5) == 0);
    mpFp_swap(b, a);
    mpz_set_mpFp(e, b);
    mpFp_pow(c, a, e);
    assert(mpFp_cmp_ui(c, 16) == 0);
    mpFp_pow_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 9) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpz_set_mpFp(e, b);
    mpFp_pow(c, a, e);
    assert(mpFp_cmp_ui(c, 5159780352) == 0);
    mpFp_pow_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 5159780352) == 0);

    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_inv)
    mpFp_t a, b, c;
    mpz_t p, d, e;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);
    mpz_init(d);
    mpz_init(e);

    mpz_set_ui(p, 17);
    mpz_set_ui(d, 12);

    mpFp_set_mpz(a, d, p);
    mpz_sub_ui(e, p, 2);
    mpFp_pow(c, a, e);
    mpFp_inv(b, a);
    assert(mpFp_cmp_ui(c, 10) == 0);
    assert(mpFp_cmp_ui(b, 10) == 0);
    assert(mpFp_cmp(c, b) == 0);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 1) == 0);

    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);

    // compare extended euclidean algorithm result to inverse by pow
    // from Fermat's little thereom
    mpFp_set_mpz(a, d, p);
    mpz_sub_ui(e, p, 2);
    mpFp_pow(c, a, e);
    mpFp_inv(b, a);
    assert(mpFp_cmp(c, b) == 0);
    mpFp_mul(c, a, b);
    assert(mpFp_cmp_ui(c, 1) == 0);

    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_swap_cswap)
    mpFp_t a, b, c, d;
    mpz_t p;
    mpz_t e;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpFp_init(d);
    mpz_init(p);
    mpz_init(e);

    mpz_set_ui(p, 17);
    mpz_set_ui(e, 12);

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

START_TEST(test_mpFp_sqrt)
    int i, j, error, bb, cc;
    int primes[] = {11, 13, 17, 19, 23, 29, 31};
    mpFp_t a, b, c;
    mpz_t p;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);

    for (j = 0 ; j < (sizeof(primes)/sizeof(primes[0])); j++) {
        mpz_set_ui(p, primes[j]);
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
    }

    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_tstbit)
    int i, j, k, bit;
    int primes[] = {11, 13, 17, 19, 23, 29, 31};
    mpFp_t a, b, c;
    mpz_t p;
    mpFp_init(a);
    mpFp_init(b);
    mpFp_init(c);
    mpz_init(p);

    for (j = 0 ; j < (sizeof(primes)/sizeof(primes[0])); j++) {
        mpz_set_ui(p, primes[j]);
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
    }

    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_urandom)
    int i;
    mpz_t a;
    mpFp_t b;
    mpz_init(a);
    mpFp_init(b);

    mpz_set_ui(a, 251);

    for (i = 0; i < 1000; i++) {
        mpFp_urandom(b, a);
        //gmp_printf("A: %ZX\n", a);
        //gmp_printf("B: %ZX\n\n", b->i);
        assert (mpz_cmp(b->p, a) == 0);
        assert (mpz_cmp_ui(b->i, 0) >= 0);
        assert (mpz_cmp(a,b->i) >= 0);
        mpz_add(a, a, b->i);
    }

    mpFp_clear(b);
    mpz_clear(a);
END_TEST

static Suite *mpFp_test_suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("Multi-Precision over Prime Fields");
    tc = tcase_create("arithmetic");

    tcase_add_test(tc, test_mpFp_add);
    tcase_add_test(tc, test_mpFp_sub);
    tcase_add_test(tc, test_mpFp_mul);
    tcase_add_test(tc, test_mpFp_pow);
    tcase_add_test(tc, test_mpFp_inv);
    tcase_add_test(tc, test_mpFp_sqrt);
    tcase_add_test(tc, test_mpFp_swap_cswap);
    tcase_add_test(tc, test_mpFp_tstbit);
    tcase_add_test(tc, test_mpFp_point_check);
    tcase_add_test(tc, test_mpFp_urandom);

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
