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
#include <ecc/ecurve.h>
#include <ecc/safememory.h>
#include <gmp.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

START_TEST(test_mpECurve_create)
    int status;
    mpECurve_t a;
    mpECurve_init(a);

// name: 'secp112r1',
//    p: 0xDB7C_2ABF62E3_5E668076_BEAD208B,
//    a: 0xDB7C_2ABF62E3_5E668076_BEAD2088,
//    b: 0x659E_F8BA0439_16EEDE89_11702B22,
//    g: [0x0948_7239995A_5EE76B55_F9C2F098,
//        0xA89C_E5AF8724_C0A23E0E_0FF77500],
//    n: 0xDB7C_2ABF62E3_5E7628DF_AC6561C5,
//    h: 1

    status = mpECurve_set_str_ws(a, 
        "0xDB7C2ABF62E35E668076BEAD208B",
        "0xDB7C2ABF62E35E668076BEAD2088",
        "0x659EF8BA043916EEDE8911702B22",
        "0xDB7C2ABF62E35E7628DFAC6561C5",
        "1",
        "0x09487239995A5EE76B55F9C2F098",
        "0xA89CE5AF8724C0A23E0E0FF77500",
        112);
    assert(status == 0);

    mpECurve_clear(a);
END_TEST

START_TEST(test_mpECurve_cmp)
    int status;
    mpECurve_t a, b, c, d;
    mpECurve_init(a);
    mpECurve_init(b);
    mpECurve_init(c);
    mpECurve_init(d);

//    name: 'secp112r1',
//    p: 0xDB7C_2ABF62E3_5E668076_BEAD208B,
//    a: 0xDB7C_2ABF62E3_5E668076_BEAD2088,
//    b: 0x659E_F8BA0439_16EEDE89_11702B22,
//    g: [0x0948_7239995A_5EE76B55_F9C2F098,
//        0xA89C_E5AF8724_C0A23E0E_0FF77500],
//    n: 0xDB7C_2ABF62E3_5E7628DF_AC6561C5,
//    h: 1

    status = mpECurve_set_str_ws(a, 
        "0xDB7C2ABF62E35E668076BEAD208B",
        "0xDB7C2ABF62E35E668076BEAD2088",
        "0x659EF8BA043916EEDE8911702B22",
        "0xDB7C2ABF62E35E7628DFAC6561C5",
        "1",
        "0x09487239995A5EE76B55F9C2F098",
        "0xA89CE5AF8724C0A23E0E0FF77500",
        112);
    assert(status == 0);

//    name: 'secp112r2',
//    p: 0xDB7C_2ABF62E3_5E668076_BEAD208B,
//    a: 0x6127_C24C05F3_8A0AAAF6_5C0EF02C,
//    b: 0x51DE_F1815DB5_ED74FCC3_4C85D709,
//    g: [0x4BA3_0AB5E892_B4E1649D_D0928643,
//        0xADCD_46F5882E_3747DEF3_6E956E97],
//    n: 0x36DF_0AAFD8B8_D7597CA1_0520D04B,
//    h: 4

    status = mpECurve_set_str_ws(b, 
        "0xDB7C2ABF62E35E668076BEAD208B",
        "0x6127C24C05F38A0AAAF65C0EF02C",
        "0x51DEF1815DB5ED74FCC34C85D709",
        "0x36DF0AAFD8B8D7597CA10520D04B",
        "4",
        "0x4BA30AB5E892B4E1649DD0928643",
        "0xADCD46F5882E3747DEF36E956E97",
        112);
    assert(status == 0);

    status = mpECurve_set_str_ws(c, 
        "0xDB7C2ABF62E35E668076BEAD208B",
        "0xDB7C2ABF62E35E668076BEAD2088",
        "0x659EF8BA043916EEDE8911702B22",
        "0xDB7C2ABF62E35E7628DFAC6561C5",
        "1",
        "0x09487239995A5EE76B55F9C2F098",
        "0xA89CE5AF8724C0A23E0E0FF77500",
        112);
    assert(status == 0);

    mpECurve_set(d, b);

    assert(mpECurve_cmp(a,b) != 0);
    assert(mpECurve_cmp(b,c) != 0);
    assert(mpECurve_cmp(a,c) == 0);
    assert(mpECurve_cmp(d,b) == 0);
    assert(mpECurve_cmp(d,c) != 0);

    mpECurve_clear(d);
    mpECurve_clear(c);
    mpECurve_clear(b);
    mpECurve_clear(a);
END_TEST

START_TEST(test_mpECurve_named)
    int error;
    mpECurve_t a;
    mpECurve_init(a);

    error = mpECurve_set_named(a,"secp128r2");
    assert(error == 0);
    error = mpECurve_set_named(a,"thisisnotthenameofacurve");
    assert(error != 0);

    mpECurve_clear(a);
END_TEST

START_TEST(test_mpECurve_all_named)
    int i, error, status;
    mpECurve_t a;
    char **clist;
    mpECurve_init(a);

    clist = _mpECurve_list_standard_curves();
    i = 0;
    while(clist[i] != NULL) {
        mpECurve_t b;
        mpECurve_init(b);
        printf("TEST: mpECurve found curve %s\n", clist[i]);
        error = mpECurve_set_named(a,clist[i]);
        printf("testing mpz_import\n");
        assert(error == 0);
        switch(a->type) {
            case EQTypeShortWeierstrass:
                    status = mpECurve_set_mpz_ws(b, a->fp->p, a->coeff.ws.a->i,
                        a->coeff.ws.b->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            case EQTypeEdwards:
                    status = mpECurve_set_mpz_ed(b, a->fp->p, a->coeff.ed.c->i,
                        a->coeff.ed.d->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            case EQTypeMontgomery:
                    status = mpECurve_set_mpz_mo(b, a->fp->p, a->coeff.mo.B->i,
                        a->coeff.mo.A->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            case EQTypeTwistedEdwards:
                    status = mpECurve_set_mpz_te(b, a->fp->p, a->coeff.te.a->i,
                        a->coeff.te.d->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            default:
                assert(0);
        }
        assert(mpECurve_cmp(a, b) == 0);
        free(clist[i]);
        mpECurve_clear(b);
        i += 1;
    }
    free(clist);

    mpECurve_clear(a);
END_TEST

static Suite *mpECurve_test_suite(void) {
    Suite *s;
    TCase *tc;
    
    s = suite_create("Elliptic Curve Parameters");
    tc = tcase_create("basic");

    tcase_add_test(tc, test_mpECurve_create);
    tcase_add_test(tc, test_mpECurve_cmp);
    tcase_add_test(tc, test_mpECurve_named);
    tcase_add_test(tc, test_mpECurve_all_named);
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

    s = mpECurve_test_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
