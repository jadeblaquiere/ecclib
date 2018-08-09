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
#include <ecc/ecelgamal.h>
#include <ecc/ecurve.h>
#include <ecc/ecpoint.h>
#include <ecc/field.h>
#include <ecc/mpzurandom.h>
#include <ecc/safememory.h>
#include <gmp.h>
#include <math.h>
#include <check.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

START_TEST(test_mpECElgamal_init)
    int i;
    char **curves;

    i = 0;
    curves = _mpECurve_list_standard_curves();
    while (curves[i] != NULL) {
        mpECurve_t cv;
        mpECP_t cv_G;
        mpFp_t sK;
        mpECP_t pK;
        mpECP_t ptxt0;
        mpECP_t ptxt1;
        mpECElgamalCiphertext_t ctxt;
        int status;
        int j;

        printf("Testing Elgamal for curve %s\n", curves[i]);
        mpECurve_init(cv);
        status = mpECurve_set_named(cv, curves[i]);
        assert(status == 0);

        mpECP_init(cv_G, cv);
        mpECP_set_mpz(cv_G, cv->G[0], cv->G[1], cv);

        mpFp_init(sK, cv->n);
        do {
            mpFp_urandom(sK, cv->n);
        } while (mpFp_cmp_ui(sK, 0) == 0);

        mpECP_init(pK, cv);
        mpECP_scalar_mul(pK, cv_G, sK);

        mpECP_init(ptxt0, cv);
        mpECP_urandom(ptxt0, cv);

        status = mpECElgamal_init_encrypt(ctxt, pK, ptxt0);
        assert(status == 0);
        status = mpECElgamal_init_decrypt(ptxt1, sK, ctxt);
        assert(status == 0);

        assert(mpECP_cmp(ptxt1, ptxt0) == 0);

        mpECP_clear(ptxt1);

        for (j = 0; j < 100; j++) {
            mpFp_t rsK;
            
            mpFp_init(rsK, cv->n);
            do {
                mpFp_urandom(rsK, cv->n);
            } while ((mpFp_cmp_ui(rsK, 0) == 0) || (mpFp_cmp(sK, rsK) == 0));

            status = mpECElgamal_init_decrypt(ptxt1, rsK, ctxt);
            assert(status == 0);
            assert(mpECP_cmp(ptxt1, ptxt0) != 0);
            mpECP_clear(ptxt1);
            mpFp_clear(rsK);
        }

        mpECElgamal_clear(ctxt);
        mpECP_clear(ptxt0);
        mpECP_clear(pK);
        mpFp_clear(sK);
        mpECP_clear(cv_G);
        mpECurve_clear(cv);
        free(curves[i]);
        i++;
    }
    free(curves);
END_TEST

static Suite *mpECDSA_test_suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("Multi-Precision over Prime Fields");
    tc = tcase_create("EC Elgamal encryption algorithm");

    tcase_add_test(tc, test_mpECElgamal_init);

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
    
#ifdef SAFE_CLEAN
    _enable_gmp_safe_clean();
#endif

    s = mpECDSA_test_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
