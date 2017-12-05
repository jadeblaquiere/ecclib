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
    _test_point_type test_point[] = { 
{"0x0000000000000000000000000000000000000000000000000000000000000000", "000000000000000000000000000000000000000000000000000000000000000000"},
{"0x0000000000000000000000000000000000000000000000000000000000000001", "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"},
{"0x0000000000000000000000000000000000000000000000000000000000000002", "02c6047f9441ed7d6d3045406e95c07cd85c778e4b8cef3ca7abac09b95c709ee5"},
{"0x0000000000000000000000000000000000000000000000000000000000000003", "02f9308a019258c31049344f85f89d5229b531c845836f99b08601f113bce036f9"},
{"0x0000000000000000000000000000000000000000000000000000000000000004", "02e493dbf1c10d80f3581e4904930b1404cc6c13900ee0758474fa94abe8c4cd13"},
{"0x0000000000000000000000000000000000000000000000000000000000000005", "022f8bde4d1a07209355b4a7250a5c5128e88b84bddc619ab7cba8d569b240efe4"},
{"0x0000000000000000000000000000000000000000000000000000000000000006", "03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556"},
{"0x0000000000000000000000000000000000000000000000000000000000000007", "025cbdf0646e5db4eaa398f365f2ea7a0e3d419b7e0330e39ce92bddedcac4f9bc"},
{"0x0000000000000000000000000000000000000000000000000000000000000008", "022f01e5e15cca351daff3843fb70f3c2f0a1bdd05e5af888a67784ef3e10a2a01"},
{"0x0000000000000000000000000000000000000000000000000000000000000009", "03acd484e2f0c7f65309ad178a9f559abde09796974c57e714c35f110dfc27ccbe"},
{"0x354BB39BCD37CB4AB2C92592051466B590109E011A6820450DCCBE5A51220540", "031a4f349170d660f7b1ec95c0684adf7aa71f5d8e55e6dc1d62304eb6f5dc67be"},
{"0x2224CAEDC717B10D69230238615EFFE29F4EA50027F8FEF43471C0DE2474DBE7", "03ab84660212cf53a1914d3f4fa2d761b92e27ab105599a762a9e68a2d6f3dfe77"},
{"0xC64966BC798FAD7C37FB5735B3011DD673F35D5F0970C1DF33D9FBB2CA758584", "0279ba8451263c954232e58462b65a77186cc2bfde272a74d220183bf8333683a2"},
{"0x5FC66BE7CB820236ADFC2EAA04B6DB81F5D58ED86CE21A808B070B513EA06E04", "03d77669148e85e6516438aaa71c411b960421b08eaa559fe803611ca0ced6d9e1"},
{"0x20013A64235962BD39B05901F7F73B31984709A3D5FC274E8B64A4326A023F4F", "03602e31eb62551ae782b548731b1ddd28a563b1ffce0ece804d011dd479212361"},
{"0x4F4FA10EBCCC0318802657D4DCAF6D19F1C6482B1021B4CC75EA1943C463FB89", "03c6753c192bf5da8e49a78d258502ceb38cfa0587efe80a226efcf409f8ebd79f"},
{"0x72DBD60DCD59CEE7547110A2F938A37A240DDD08AE89CE593F121F773C9BA4DB", "02a13b4003258306d92b9cc1c1119ea5010629a5c9621ea4b6393086e5e5302182"},
{"0x7E51750F5B0F2D6EC136F20BB86572D38ABA655D3827EA0A7BFC3A2D4E136B90", "03efb47e2514afa8e9b80fa84ccbf5db08fb182dfad54b05672d88d057092582da"},
{"0x72AF62E95C9855C13E93BBC1DAD9C9215B39CBA04DC8D93257033B78B2A5A8FB", "02fa45672a4b655e6b4cfb54b4b4239ab903c134f4d8736b84c2bdb736ccfcf3aa"},
{"0x2FB09FBDDD1C4B2B41A93BFC5DA6A82912B6F0AF9C8369950882243FDB2C440C", "029f1d9d1c0a71a91072b7fa59288da58ee1618ad47884bcbda49d44d650c872e5"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", "03b42b34a9748238715c0b8b853b6939aabc3b5224bcdd4b4b65e902417e7af914"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E", "0202541d1403fc71a5d927923b20a673e769284b16e7d1f597f9413dc64e82fc48"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2D", "03a7e35f321a3dbb4271b9d194a614018806161547aa835932a21a2ac176221b27"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2C", "03a48179276e052224e95478a8237dc131743cdb6f8c855c1c7770a3f63eccd25a"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2B", "0362d7174e4198f03606ed8c887da8c85a4cfec76f2577e5435eb0af0374506f7e"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2A", "03538be1492eae677b57569c11481b8434fc1aa2b6bf2c6632bf40cfda678a7dc5"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC29", "03a74f885d770e92e5515350976fdfdec62baac9097059457b461e59b01057d346"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC28", "03f49e1c31476bcddfb751b60a7018821ddcefd9f3969d485c907964d3f9e84d66"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC27", "0330479fda5ab649d1f3b61486c5e64814c521125bf741caeccb88b2a171718a54"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC26", "02e9e5631bd7fa5a0563a2da34f168dd33df6f9970ff3e2012d25f6b618ee3700a"},
    };
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    
    error = mpECurve_set_named(cv, test_curve);
    assert(error == 0);
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 1 ; i < npoints; i++) {
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

START_TEST(test_mpECP_ed_double)
    int error, i, npoints;
    char test_curve[] = "Curve41417";
    _test_point_type test_point[] = { {"0x2B6ADADA50C86E17A35BD5850052A3A3B1232691F304808F623E8A276AADB37107A15746B1113E2B4F62F840A9C1E58AA5B7888B", 
"040711B58ACC265D718D4D21A08131493DD565E7782556B3E706AE91453355BFF159D40CD0FC48988315F143730033C522100B0EFB1E6727870D9CE17721FCA7B767EA5127E035827BA5F6A5CC798C3FE771AF041122242F55FF10A9FD8624468B33C4E10554CD74BE"}, 
{ "0x1C86243B837F9B037A6403F3ED30F99C774E991C9729F93D1EE211D7946E277BE3AD85956495879A0F48C845D3F0DBEA286077C9", 
"04098AEB45B524C887A9B0E28D33290036B33E6123F2764C365C4D15FF3A6667F0A2B2BD24C810AB7582C49834550FB6B7A3E719D93BE285FCFB27035F93F0E3023D01088EFA0C18CABEFE0A5F6CF660F8DA0D14BB7D0007D21D7BA9442BC855D73A7872974836FF51"},
{ "0x2FAC619A3E84FA0363D9B9A685EA51293E2A5336F246805F156E1461A01DA03C0922FE1DFA59E12BDD5430541C69946637090CDF", 
"040A334BCDDD89E2825C78B96F1E487709B294196EE813CE61418E35D8CE819581146CDA575E54C055C1399E30E91D710306C60D850F7F5A7E575154D73290D13B52643659BB2CFCFA2F8CEF52A7001A30FD22F4E5E35E74A9DF7E3A77068A512965D450A958B81EEC"},
{ "0x12FE51653894E46560B22DD4F5E01B2F903A7E624C5C47548E6D22C5E64B86D9547A940CDCF44A3FCDCB9AD758E7A6B8C88931CD",
"0402F83AAB7C519133EFB2578924B5105DE0C35EE69E07A3E654F6203497A35F788CDC17E510E8EC30B717F95B7CEA95C90A8C494902063AEA28974A2F052B8999E7D150E979A3B3D45D1ECCDD6CD6E2A3E33E1C385662201228F04AA7163D76DCE5643577948D537D"}};
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

START_TEST(test_mpECP_ws_scalar_mul)
    int error, i, npoints;
    char test_curve[] = "secp256k1";
    _test_point_type test_point[] = { 
{"0x0000000000000000000000000000000000000000000000000000000000000000", "000000000000000000000000000000000000000000000000000000000000000000"},
{"0x0000000000000000000000000000000000000000000000000000000000000001", "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"},
{"0x0000000000000000000000000000000000000000000000000000000000000002", "02c6047f9441ed7d6d3045406e95c07cd85c778e4b8cef3ca7abac09b95c709ee5"},
{"0x0000000000000000000000000000000000000000000000000000000000000003", "02f9308a019258c31049344f85f89d5229b531c845836f99b08601f113bce036f9"},
{"0x0000000000000000000000000000000000000000000000000000000000000004", "02e493dbf1c10d80f3581e4904930b1404cc6c13900ee0758474fa94abe8c4cd13"},
{"0x0000000000000000000000000000000000000000000000000000000000000005", "022f8bde4d1a07209355b4a7250a5c5128e88b84bddc619ab7cba8d569b240efe4"},
{"0x0000000000000000000000000000000000000000000000000000000000000006", "03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556"},
{"0x0000000000000000000000000000000000000000000000000000000000000007", "025cbdf0646e5db4eaa398f365f2ea7a0e3d419b7e0330e39ce92bddedcac4f9bc"},
{"0x0000000000000000000000000000000000000000000000000000000000000008", "022f01e5e15cca351daff3843fb70f3c2f0a1bdd05e5af888a67784ef3e10a2a01"},
{"0x0000000000000000000000000000000000000000000000000000000000000009", "03acd484e2f0c7f65309ad178a9f559abde09796974c57e714c35f110dfc27ccbe"},
{"0x354BB39BCD37CB4AB2C92592051466B590109E011A6820450DCCBE5A51220540", "031a4f349170d660f7b1ec95c0684adf7aa71f5d8e55e6dc1d62304eb6f5dc67be"},
{"0x2224CAEDC717B10D69230238615EFFE29F4EA50027F8FEF43471C0DE2474DBE7", "03ab84660212cf53a1914d3f4fa2d761b92e27ab105599a762a9e68a2d6f3dfe77"},
{"0xC64966BC798FAD7C37FB5735B3011DD673F35D5F0970C1DF33D9FBB2CA758584", "0279ba8451263c954232e58462b65a77186cc2bfde272a74d220183bf8333683a2"},
{"0x5FC66BE7CB820236ADFC2EAA04B6DB81F5D58ED86CE21A808B070B513EA06E04", "03d77669148e85e6516438aaa71c411b960421b08eaa559fe803611ca0ced6d9e1"},
{"0x20013A64235962BD39B05901F7F73B31984709A3D5FC274E8B64A4326A023F4F", "03602e31eb62551ae782b548731b1ddd28a563b1ffce0ece804d011dd479212361"},
{"0x4F4FA10EBCCC0318802657D4DCAF6D19F1C6482B1021B4CC75EA1943C463FB89", "03c6753c192bf5da8e49a78d258502ceb38cfa0587efe80a226efcf409f8ebd79f"},
{"0x72DBD60DCD59CEE7547110A2F938A37A240DDD08AE89CE593F121F773C9BA4DB", "02a13b4003258306d92b9cc1c1119ea5010629a5c9621ea4b6393086e5e5302182"},
{"0x7E51750F5B0F2D6EC136F20BB86572D38ABA655D3827EA0A7BFC3A2D4E136B90", "03efb47e2514afa8e9b80fa84ccbf5db08fb182dfad54b05672d88d057092582da"},
{"0x72AF62E95C9855C13E93BBC1DAD9C9215B39CBA04DC8D93257033B78B2A5A8FB", "02fa45672a4b655e6b4cfb54b4b4239ab903c134f4d8736b84c2bdb736ccfcf3aa"},
{"0x2FB09FBDDD1C4B2B41A93BFC5DA6A82912B6F0AF9C8369950882243FDB2C440C", "029f1d9d1c0a71a91072b7fa59288da58ee1618ad47884bcbda49d44d650c872e5"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", "03b42b34a9748238715c0b8b853b6939aabc3b5224bcdd4b4b65e902417e7af914"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E", "0202541d1403fc71a5d927923b20a673e769284b16e7d1f597f9413dc64e82fc48"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2D", "03a7e35f321a3dbb4271b9d194a614018806161547aa835932a21a2ac176221b27"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2C", "03a48179276e052224e95478a8237dc131743cdb6f8c855c1c7770a3f63eccd25a"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2B", "0362d7174e4198f03606ed8c887da8c85a4cfec76f2577e5435eb0af0374506f7e"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2A", "03538be1492eae677b57569c11481b8434fc1aa2b6bf2c6632bf40cfda678a7dc5"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC29", "03a74f885d770e92e5515350976fdfdec62baac9097059457b461e59b01057d346"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC28", "03f49e1c31476bcddfb751b60a7018821ddcefd9f3969d485c907964d3f9e84d66"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC27", "0330479fda5ab649d1f3b61486c5e64814c521125bf741caeccb88b2a171718a54"},
{"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC26", "02e9e5631bd7fa5a0563a2da34f168dd33df6f9970ff3e2012d25f6b618ee3700a"},
    };
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpz_t r;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    mpz_init(r);
    
    error = mpECurve_set_named(cv, test_curve);
    assert(error == 0);
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 1 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        error = mpz_set_str(r, test_point[i].pvt, 0);
        assert(error == 0);
        mpECP_scalar_mul_mpz(b, a, r);
        mpECP_out_str(buffer, a, 0);
        printf(" A: %s\n", buffer);
        gmp_printf("*b: %ZX\n", r);
        printf("-------\n");
        mpECP_out_str(buffer, b, 0);
        printf("=B: %s\n", buffer);
        mpECP_set_str(c, test_point[i].pub, cv);
        // A + A + (-A) = A
        mpECP_out_str(buffer, c, 0);
        printf("=C: %s\n", buffer);
        assert(mpECP_cmp(b, c) == 0);
        free(buffer);
    }

    mpz_clear(r);
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
    tcase_add_test(tc, test_mpECP_ed_double);
    tcase_add_test(tc, test_mpECP_ws_scalar_mul);
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
