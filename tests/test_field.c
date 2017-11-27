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
    
    // 2**255-19 (a prime number)
    mpz_set_str(p, p25519, 10);
    
    mpFp_set_mpz(a, d, p);
    mpFp_set_ui(b, 9, p);
    mpFp_add(c, a, b);
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

static Suite *mpFp_test_suite(void) {
    Suite *s;
    TCase *tc;
    
    s = suite_create("Multi-Precision over Prime Fields");
    tc = tcase_create("arithmetic");

    tcase_add_test(tc, test_mpFp_add);
    tcase_add_test(tc, test_mpFp_sub);
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
