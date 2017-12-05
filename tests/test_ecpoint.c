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

typedef struct {
    char *pvt;
    char *pub;
} _test_point_type;

START_TEST(test_mpECP_ws_add)
    int error, i, j, npoints;
    char test_curve[] = "secp256k1";
    _test_point_type test_point[] = { {"0x4D711643AA3CCCFC52B4038BAACD62BF8D7D1112E2A72B9CA7AE4655FE46DBB1", "03852e6d096124039e609ba54c8c35b5fb55382c579c4250e9aec8bb1aa34d1f25"},
        {"0x8F99603C5D55629E4F8EFF8A4E8596DB9922EB1D2EE08D6E39010C2ED1D64C19", "0254f0e6ebdc0e29a93f05fdf3c8aac01d37eb268a1f356eae69aca056213e5cea"},
        {"0x3F2EA446470100B0201256C70AA6355A1276BA9317C6100793152F31AD81F0DB", "02e6baf7fd9ced919716dba98a786a97965bc90a49b8eea8bf9b39b542dc14bf3d"},
        {"0xE0519B06851A8D90E924C2756DA170582C6EB527B948759413DD07664B8F48CA", "036eb2fe692c8d79c0be3eee32040b1da49333f6d70039d7fed1f0f795e8af909e"},
        {"0x6F46CDFC748E43EC68455EE0DD565EC5E9BF296CF6391437B22F87F4E623EB15", "02709e6fa2900f24d4812d50f4cebade4f0464993d5c1a49549b9d8fc5d8a93c86"},
        {"0x7EAB87475322E2D22FFFED16B60FA38102F7E2B1215ACE186831AB7C957C926C", "03a996a4455756983fdb19c664b68c7f797c72f6f952344bd106ddcc636142223f"},
        {"0xA0B737A5947AC0408203764145C8841E893CDAAE1743AB7B993CF60103F264BD", "03ef6c40c092088ca9cf6c45cb5264d2f137c761f6d662786dfea1671bc3202c20"},
        {"0xA652CD96A51656375A0DAE3E641120D1C10BD3889CC4BE4F5962F4F788F13FF0", "03706bb96c832888e4f5868bbe076f75e1c27a0295ed9dccd804a79521daf56d60"} };
    mpECurve_t cv;
    mpECP_t a, b, c, d;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    mpECP_init(d);
    
    error = mpECurve_set_named(cv, test_curve);
    assert(error == 0);
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        error = mpECP_set_str(a, test_point[i].pub, cv);
        assert(error == 0);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        for (j = i + 1; j < npoints; j++) {
            error = mpECP_set_str(b, test_point[j].pub, cv);
            assert(error == 0);
            mpECP_add(c, a, b);
            mpECP_out_str(buffer, a, 0);
            printf(" A: %s\n", buffer);
            mpECP_out_str(buffer, b, 0);
            printf("+B: %s\n", buffer);
            printf("-------\n");
            mpECP_out_str(buffer, c, 0);
            printf("=C: %s\n", buffer);
            // commutative A + B = B + A
            mpECP_add(d, b, a);
            assert(mpECP_cmp(c, d) == 0);
            mpECP_neg(d, b);
            // A + B + (-B) = A
            mpECP_out_str(buffer, d, 0);
            printf("+D: %s (D = -B)\n", buffer);
            mpECP_add(b, c, d);
            mpECP_out_str(buffer, b, 0);
            printf("=A: %s\n", buffer);
            assert(mpECP_cmp(a, b) == 0);
        }
        free(buffer);
    }

    mpECP_clear(d);
    mpECP_clear(c);
    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_ws_double)
    int error, i, npoints;
    char test_curve[] = "secp256k1";
    _test_point_type test_point[] = { {"0x4D711643AA3CCCFC52B4038BAACD62BF8D7D1112E2A72B9CA7AE4655FE46DBB1", "03852e6d096124039e609ba54c8c35b5fb55382c579c4250e9aec8bb1aa34d1f25"},
        {"0x8F99603C5D55629E4F8EFF8A4E8596DB9922EB1D2EE08D6E39010C2ED1D64C19", "0254f0e6ebdc0e29a93f05fdf3c8aac01d37eb268a1f356eae69aca056213e5cea"},
        {"0x3F2EA446470100B0201256C70AA6355A1276BA9317C6100793152F31AD81F0DB", "02e6baf7fd9ced919716dba98a786a97965bc90a49b8eea8bf9b39b542dc14bf3d"},
        {"0xE0519B06851A8D90E924C2756DA170582C6EB527B948759413DD07664B8F48CA", "036eb2fe692c8d79c0be3eee32040b1da49333f6d70039d7fed1f0f795e8af909e"},
        {"0x6F46CDFC748E43EC68455EE0DD565EC5E9BF296CF6391437B22F87F4E623EB15", "02709e6fa2900f24d4812d50f4cebade4f0464993d5c1a49549b9d8fc5d8a93c86"},
        {"0x7EAB87475322E2D22FFFED16B60FA38102F7E2B1215ACE186831AB7C957C926C", "03a996a4455756983fdb19c664b68c7f797c72f6f952344bd106ddcc636142223f"},
        {"0xA0B737A5947AC0408203764145C8841E893CDAAE1743AB7B993CF60103F264BD", "03ef6c40c092088ca9cf6c45cb5264d2f137c761f6d662786dfea1671bc3202c20"},
        {"0xA652CD96A51656375A0DAE3E641120D1C10BD3889CC4BE4F5962F4F788F13FF0", "03706bb96c832888e4f5868bbe076f75e1c27a0295ed9dccd804a79521daf56d60"} };
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    
    error = mpECurve_set_named(cv, test_curve);
    assert(error == 0);
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        error = mpECP_set_str(a, test_point[i].pub, cv);
        assert(error == 0);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        assert(error == 0);
        mpECP_double(b, a);
        mpECP_out_str(buffer, a, 0);
        printf(" A: %s\n", buffer);
        printf("+A: %s\n", buffer);
        printf("-------\n");
        mpECP_out_str(buffer, b, 0);
        printf("=B: %s\n", buffer);
        mpECP_neg(c, a);
        // A + A + (-A) = A
        mpECP_out_str(buffer, c, 0);
        printf("+C: %s (C = -A)\n", buffer);
        mpECP_add(b, b, c);
        mpECP_out_str(buffer, b, 0);
        printf("=A: %s\n", buffer);
        assert(mpECP_cmp(a, b) == 0);
        free(buffer);
    }

    mpECP_clear(c);
    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_ed_add)
    int error, i, j, npoints;
    char test_curve[] = "Curve41417";
    // curve41417 test vectors from Apache Milagro Cryptography Library
    // https://github.com/milagro-crypto/milagro-crypto-c/blob/develop/testVectors/ecp/test_vector_C41417.txt
    _test_point_type test_point[] = { {"0x2B6ADADA50C86E17A35BD5850052A3A3B1232691F304808F623E8A276AADB37107A15746B1113E2B4F62F840A9C1E58AA5B7888B", 
"040711B58ACC265D718D4D21A08131493DD565E7782556B3E706AE91453355BFF159D40CD0FC48988315F143730033C522100B0EFB1E6727870D9CE17721FCA7B767EA5127E035827BA5F6A5CC798C3FE771AF041122242F55FF10A9FD8624468B33C4E10554CD74BE"}, 
{ "0x1C86243B837F9B037A6403F3ED30F99C774E991C9729F93D1EE211D7946E277BE3AD85956495879A0F48C845D3F0DBEA286077C9", 
"04098AEB45B524C887A9B0E28D33290036B33E6123F2764C365C4D15FF3A6667F0A2B2BD24C810AB7582C49834550FB6B7A3E719D93BE285FCFB27035F93F0E3023D01088EFA0C18CABEFE0A5F6CF660F8DA0D14BB7D0007D21D7BA9442BC855D73A7872974836FF51"},
{ "0x2FAC619A3E84FA0363D9B9A685EA51293E2A5336F246805F156E1461A01DA03C0922FE1DFA59E12BDD5430541C69946637090CDF", 
"040A334BCDDD89E2825C78B96F1E487709B294196EE813CE61418E35D8CE819581146CDA575E54C055C1399E30E91D710306C60D850F7F5A7E575154D73290D13B52643659BB2CFCFA2F8CEF52A7001A30FD22F4E5E35E74A9DF7E3A77068A512965D450A958B81EEC"},
{ "0x12FE51653894E46560B22DD4F5E01B2F903A7E624C5C47548E6D22C5E64B86D9547A940CDCF44A3FCDCB9AD758E7A6B8C88931CD",
"0402F83AAB7C519133EFB2578924B5105DE0C35EE69E07A3E654F6203497A35F788CDC17E510E8EC30B717F95B7CEA95C90A8C494902063AEA28974A2F052B8999E7D150E979A3B3D45D1ECCDD6CD6E2A3E33E1C385662201228F04AA7163D76DCE5643577948D537D"}};

    mpECurve_t cv;
    mpECP_t a, b, c, d;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    mpECP_init(d);
    
    error = mpECurve_set_named(cv, test_curve);
    assert(error == 0);
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        error = mpECP_set_str(a, test_point[i].pub, cv);
        assert(error == 0);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        for (j = i + 1; j < npoints; j++) {
            error = mpECP_set_str(b, test_point[j].pub, cv);
            assert(error == 0);
            mpECP_add(c, a, b);
            mpECP_out_str(buffer, a, 0);
            printf(" A: %s\n", buffer);
            mpECP_out_str(buffer, b, 0);
            printf("+B: %s\n", buffer);
            printf("-------\n");
            mpECP_out_str(buffer, c, 0);
            printf("=C: %s\n", buffer);
            // commutative A + B = B + A
            mpECP_add(d, b, a);
            assert(mpECP_cmp(c, d) == 0);
            mpECP_neg(d, b);
            // A + B + (-B) = A
            mpECP_out_str(buffer, d, 0);
            printf("+D: %s (D = -B)\n", buffer);
            mpECP_add(b, c, d);
            mpECP_out_str(buffer, b, 0);
            printf("=A: %s\n", buffer);
            assert(mpECP_cmp(a, b) == 0);
        }
        free(buffer);
    }

    mpECP_clear(d);
    mpECP_clear(c);
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
    tcase_add_test(tc, test_mpECP_ws_add);
    tcase_add_test(tc, test_mpECP_ws_double);
    tcase_add_test(tc, test_mpECP_ed_add);
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
