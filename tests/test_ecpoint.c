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
#include <check.h>
#include <ecurve.h>
#include <gmp.h>
#include <ecpoint.h>
#include <stdio.h>
#include <stdlib.h>

START_TEST(test_mpECP_create)
    int error;
    mpECurve_t cv;
    mpECP_t a;
    mpECurve_init(cv);
    mpECP_init(a);
    
    error = mpECurve_set_named(cv, "secp256k1");
    assert(error == 0);
    mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);

    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_str_out)
    int error, i, ncurves;
    char *test_curve[] = {"secp256k1", "Curve25519", "Curve41417"};
    mpECurve_t cv;
    mpECP_t a;
    mpECurve_init(cv);
    mpECP_init(a);
    
    ncurves = sizeof(test_curve) / sizeof(test_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        int slen;
        char *buffer;
        error = mpECurve_set_named(cv, test_curve[i]);
        assert(error == 0);
        mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);
        printf("Exporting base point for curve %s:\n", test_curve[i]);
        slen = mpECP_out_strlen(a, 1);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 1);
        assert(strlen(buffer) == slen);
        printf("Compressed   : %s\n", buffer);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 0);
        assert(strlen(buffer) == slen);
        printf("Uncompressed : %s\n", buffer);
    }

    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_affine)
    int error, i, ncurves;
    char *test_curve[] = {"secp256k1", "Curve25519", "Curve41417"};
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    
    ncurves = sizeof(test_curve) / sizeof(test_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        mpz_t x, y;
        mpz_init(x);
        mpz_init(y);
        error = mpECurve_set_named(cv, test_curve[i]);
        assert(error == 0);
        mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);
        printf("Exporting affine base point for curve %s:\n", test_curve[i]);
        mpz_set_mpECP_affine_x(x, a);
        mpz_set_mpECP_affine_y(y, a);
        gmp_printf("(0x%ZX, 0x%ZX)\n", x, y);
        //gmp_printf("(0x%ZX, y)\n", x);
        mpECP_set_mpz(b, x, y, cv);
        assert(mpECP_cmp(a,b) == 0);
        mpz_clear(y);
        mpz_clear(x);
    }

    mpECP_clear(c);
    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_infinity)
    int error, i, ncurves;
    char *test_curve[] = {"secp256k1", "Curve25519", "Curve41417"};
    mpECurve_t cv;
    mpECP_t a;
    mpECurve_init(cv);
    mpECP_init(a);
    
    ncurves = sizeof(test_curve) / sizeof(test_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        int slen;
        char *buffer;
        error = mpECurve_set_named(cv, test_curve[i]);
        assert(error == 0);
        mpECP_set_infinite(a, cv);
        printf("Exporting neutral point for curve %s:\n", test_curve[i]);
        slen = mpECP_out_strlen(a, 1);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 1);
        assert(strlen(buffer) == slen);
        printf("Compressed   : %s\n", buffer);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 0);
        assert(strlen(buffer) == slen);
        printf("Uncompressed : %s\n", buffer);
    }

    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_export_import)
    int error, i, ncurves;
    char *test_curve[] = {"secp256k1", "Curve25519", "Curve41417"};
    mpECurve_t cv;
    mpECP_t a, b;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    
    ncurves = sizeof(test_curve) / sizeof(test_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        int slen;
        char *buffer;
        error = mpECurve_set_named(cv, test_curve[i]);
        assert(error == 0);
        mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);
        //printf("Exporting base point for curve %s:\n", test_curve[i]);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 0);
        assert(strlen(buffer) == slen);
        //printf("Uncompressed : %s\n", buffer);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 0);
        //printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
        slen = mpECP_out_strlen(a, 1);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 1);
        assert(strlen(buffer) == slen);
        //printf("Compressed   : %s\n", buffer);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 1);
        //printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
        mpECP_set_infinite(a, cv);
        //printf("Exporting neutral for curve %s:\n", test_curve[i]);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 0);
        assert(strlen(buffer) == slen);
        //printf("Uncompressed : %s\n", buffer);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 0);
        //printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
        slen = mpECP_out_strlen(a, 1);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 1);
        assert(strlen(buffer) == slen);
        //printf("Compressed   : %s\n", buffer);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 1);
        //printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
    }

    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

static Suite *mpECP_test_suite(void) {
    Suite *s;
    TCase *tc;
    
    s = suite_create("Elliptic Curve Point");
    tc = tcase_create("basic");

    tcase_add_test(tc, test_mpECP_create);
    tcase_add_test(tc, test_mpECP_str_out);
    tcase_add_test(tc, test_mpECP_affine);
    tcase_add_test(tc, test_mpECP_infinity);
    tcase_add_test(tc, test_mpECP_export_import);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = mpECP_test_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
