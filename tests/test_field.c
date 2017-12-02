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
#include <field.h>
#include <gmp.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static char p25519[] = "57896044618658097711785492504343953926634992332820282019728792003956564819949";

START_TEST(test_mpFp_add)
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
    mpFp_add(c, a, b);
    assert(mpFp_cmp_ui(c, 4) == 0);
    mpFp_add_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 4) == 0);
    mpFp_neg(b,a);
    assert(mpFp_cmp_ui(b, 5) == 0);
    mpFp_add(c, a, b);
    assert(mpFp_cmp_ui(c, 0) == 0);
    
    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);
    
    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_add(c, a, b);
    assert(mpFp_cmp_ui(c, 21) == 0);
    mpFp_add_ui(c, a, 9);
    assert(mpFp_cmp_ui(c, 21) == 0);
    
    mpz_clear(d);
    mpz_clear(p);
    mpFp_clear(c);
    mpFp_clear(b);
    mpFp_clear(a);
END_TEST

START_TEST(test_mpFp_sub)
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
