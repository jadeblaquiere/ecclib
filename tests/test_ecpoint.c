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
        mpECP_set_neutral(a, cv);
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
    char *test_curve[] = {"secp256k1", "Curve25519", "Curve41417", "Ed25519"};
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
        // printf("Exporting base point for curve %s:\n", test_curve[i]);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        mpECP_out_str(buffer, a, 0);
        // printf("Uncompressed : %s\n", buffer);
        assert(strlen(buffer) == slen);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 0);
        // printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
        //free(buffer);
        slen = mpECP_out_strlen(a, 1);
        //buffer = malloc((slen + 1)* sizeof(char));
        //assert(buffer != NULL);
        mpECP_out_str(buffer, a, 1);
        // printf("Compressed   : %s\n", buffer);
        assert(strlen(buffer) == slen);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 1);
        // printf("Export/import: %s\n", buffer);
        //free(buffer);
        slen = mpECP_out_strlen(a, 0);
        //buffer = malloc((slen + 1)* sizeof(char));
        //assert(buffer != NULL);
        mpECP_out_str(buffer, b, 0);
        // printf("Export/import (full): %s\n", buffer);
        assert(error == 0);
        //assert(mpFp_cmp(a->x, b->x) == 0);
        //assert(mpFp_cmp(a->y, b->y) == 0);
        //assert(mpFp_cmp(a->z, b->z) == 0);
        assert(a->is_neutral == b->is_neutral);
        assert(a->cv->type == b->cv->type);
        assert(mpECP_cmp(a, b) == 0);
        mpECP_set_neutral(a, cv);
        // printf("Exporting neutral for curve %s:\n", test_curve[i]);
        //free(buffer);
        slen = mpECP_out_strlen(a, 0);
        //buffer = malloc((slen + 1)* sizeof(char));
        //assert(buffer != NULL);
        mpECP_out_str(buffer, a, 0);
        // printf("Uncompressed : %s\n", buffer);
        assert(strlen(buffer) == slen);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 0);
        // printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
        //free(buffer);
        slen = mpECP_out_strlen(a, 1);
        //buffer = malloc((slen + 1)* sizeof(char));
        //assert(buffer != NULL);
        mpECP_out_str(buffer, a, 1);
        // printf("Compressed   : %s\n", buffer);
        assert(strlen(buffer) == slen);
        error = mpECP_set_str(b, buffer, cv);
        mpECP_out_str(buffer, b, 1);
        // printf("Export/import: %s\n", buffer);
        assert(error == 0);
        assert(mpECP_cmp(a, b) == 0);
    }

    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

typedef struct {
    char *curve;
    char *pvt;
    char *pub;
} _test_point_type;

static _test_point_type test_point[] = {
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000000", "000000000000000000000000000000000000000000000000000000000000000000"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000001", "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000002", "02c6047f9441ed7d6d3045406e95c07cd85c778e4b8cef3ca7abac09b95c709ee5"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000003", "02f9308a019258c31049344f85f89d5229b531c845836f99b08601f113bce036f9"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000004", "02e493dbf1c10d80f3581e4904930b1404cc6c13900ee0758474fa94abe8c4cd13"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000005", "022f8bde4d1a07209355b4a7250a5c5128e88b84bddc619ab7cba8d569b240efe4"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000006", "03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000007", "025cbdf0646e5db4eaa398f365f2ea7a0e3d419b7e0330e39ce92bddedcac4f9bc"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000008", "022f01e5e15cca351daff3843fb70f3c2f0a1bdd05e5af888a67784ef3e10a2a01"},
{"secp256k1", "0x0000000000000000000000000000000000000000000000000000000000000009", "03acd484e2f0c7f65309ad178a9f559abde09796974c57e714c35f110dfc27ccbe"},
{"secp256k1", "0xE13E7DF18AA27413036FB082D63EA49D2E9C7F2FDAB349A4490DAAE783974EDE", "0282f05ce54d04ba5fac0c458af0e9b20f3ff80d7c265cd1bd05b9a7ad29337b87"},
{"secp256k1", "0x014E205A5D594285F171511C3B1BAC73B6841D591C240BE276DA490BCB7CE1F4", "02cc744cbb8704124cf2e74fd26b901eec29efa40090ad905893e56ac8cbe2561d"},
{"secp256k1", "0x2D40B83755F8DADEAE5AE8978D29C6517156D86B8CD72FA4BBAD50660C6A73A1", "026094d2b079781eb51e3cda3719699bfd880d1aac9a38e18f028fd6b926d66ea6"},
{"secp256k1", "0x19447113DFBDDBAFF48E92D5368C7B14BC0503F54A96A2E9C00F77F1A1A70F51", "032bd0ba1097947d03c0279b1ea19ee00e7192f1c5f1642f772b6ff31fd3f575d9"},
{"secp256k1", "0x5F1DD9718AC7B6DD0243339C45F1354831B71DD6595ED071056819B367958C3C", "0341f8a75a12bbecbaf846f2f9ac3b6c8eada1353a9b3f3302152e3cd1313f9aef"},
{"secp256k1", "0xFF032D8F20C5811828853387B8FA3A5829B0EEA0D05425118D15EA9BD3ABA9A7", "02eeb62fd4a5e8a9cc050c3ab5df534d09a49be0878b217d53cbad9825fa75d915"},
{"secp256k1", "0xF1746C2A3D8B699B80F39BBFCA69C7C2BFCB551ADB28A86D2CB6DC2FC74AB159", "037c6d895b037cdf92533e1f17fb701fd3be78296b8ed3dce754e2df5730cbf9dc"},
{"secp256k1", "0xE08F85BFD28E261836492929FEA244B921FFAD67E470F51F0B0692F915B49F70", "028362ff2555b251704b9198cc66f0031084bacdfe0ccaeb47ff3c50ee9a43e1a2"},
{"secp256k1", "0x929F720BA069FDA53500D9A80F3139D75545519C97444DEF5DAAC64977772969", "039ff2ecabd4eb1535b1a07ca15cf67a1f2c56151a29a0de530d8137d715975e28"},
{"secp256k1", "0x7924173284AA8C75E1F8936E54DA76B6BEF1596D78ACFBC83C760F52FF4D8E8A", "020d276f73ee462568b96ee84c62c8062722bd8e56637d2a2fc33254780e859802"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E", "0202541d1403fc71a5d927923b20a673e769284b16e7d1f597f9413dc64e82fc48"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2D", "03a7e35f321a3dbb4271b9d194a614018806161547aa835932a21a2ac176221b27"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2C", "03a48179276e052224e95478a8237dc131743cdb6f8c855c1c7770a3f63eccd25a"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2B", "0362d7174e4198f03606ed8c887da8c85a4cfec76f2577e5435eb0af0374506f7e"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2A", "03538be1492eae677b57569c11481b8434fc1aa2b6bf2c6632bf40cfda678a7dc5"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC29", "03a74f885d770e92e5515350976fdfdec62baac9097059457b461e59b01057d346"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC28", "03f49e1c31476bcddfb751b60a7018821ddcefd9f3969d485c907964d3f9e84d66"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC27", "0330479fda5ab649d1f3b61486c5e64814c521125bf741caeccb88b2a171718a54"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC26", "02e9e5631bd7fa5a0563a2da34f168dd33df6f9970ff3e2012d25f6b618ee3700a"},
{"secp256k1", "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC25", "03675d5652ab3d289dd1815db7f81618d22f485db412751960a90d6def758ee432"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001", "021A334905141443300218C0631C326E5FCD46369F44C03EC7F57FF35498A4AB4D6D6BA111301A73FAA8537C64C4FD3812F3CBC595"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002", "0330F39EEA21BE42C1D13DD776F230AAEBAA3FE6980EC491F4143DAFCA8A1BE5CFEC9CC4F8D743C47E952689EE13CD0BB003A5982B"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000003", "033B4D7B22046023D1EDBB9C0ACBD0EE123EE04155AF3F6FCF52E27164BE7A4AEC0069B435540EFC793E289C1C8846461CC78B5A55"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004", "0328CE3371978912445AB7F806BE79339EA4C5357CE49D73C54A490AC304CE0762089D6B76A902570B68A9EB662FCBE09172D79E35"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005", "031102831F41FAB8FAECF7E7CF4395E52C6725269656282B1E7D3721A12BB29BB32AB5D86D2E24591D1C93437C11486372EFBEEA05"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000006", "020E9234799F962C1283C898AF0B74EBE7CFC28021F2803CFBC8BFB32A5519DA72A22BDD7C8C9D674E7A941A3EDD078184FBB747C2"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007", "023ABF7F8D32646D0EA90151A91060A944877FE2BD21B8C922ED61FDCEB5C1652F455C6B7101F7B958FF6539F58C9B208F0A908594"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000008", "03018CB1739CA993208E66C75ECE8E800328C703DDFD2713D2FDD7A825D66FBEBF97D188CFEBBA8A8FC74AEC83F08B4D1C26C1A8D3"},
{"Curve41417", "0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000009", "02258CDF7751BBC5A104E6995C57A926D187A81A40D9A3A5B95CC2E4EAAE24F70A164EDBC24F74D3745E3D9CE3442D2715EC6789BE"},
{"Curve41417", "0x012C2063B5D3D12298EF821CC4181DE9F5AD62DC8DAF3B6BEC3B80278E144A50E01E06E2787A1FCB9C7551AADCC87EA8A9D4E54B", "02093C4FEB747A11ABC84788202E0670579CC68F1AB8E6CC75944931DB1DA29256504FCE5F824B80072AD681A7182999BF7E8F5FED"},
{"Curve41417", "0x1EE90202E9D35F4146155B7CB31EA1893F8DEB16A7C3BDC7299D87E953B3548F48B9C48824920F6040BADE7BEAD8613B5853CB30", "033559FC1A9D5FE1A0774CC471A0CB39FE271FE69E4AAEA3AF08F84BC5AC9A2D60684A61D26EC6973389DB6F3225FB94C2E92A2CD3"},
{"Curve41417", "0x33F391973132465858BC441C3819920F100B294D8222BB7FD1B6428D2D518C1344501899C639A14DEB965F2415C4913567ABA982", "031D172623F3F5965D69C9CA4237A9462E8F3C3F3042415466B63E6F29D7470031A0DE9BE3C5CF9C68F0810E37F196FC42EF74514A"},
{"Curve41417", "0x03B5D08B67B7FF8FBB8457E078DE9EEA0A8FD15BC33FAD521C4227504C5FECD1A3177AFC50F4099B84CF400257C60CBBBCC2F5B5", "0202F8CE1D19843EE53A05E68ADF7B14637E41194DB7487149A4103246E0711C6701E84C5324C4D1281E15E25C5602530001757E35"},
{"Curve41417", "0x273ABFFF407AE8B4B0C061C51FE130F623196915DEB5A688EFE838D24E03A214A39D6AD4A6ECC1E08E0848F798FB17B7A8064584", "0327372FFB396760EE1F05282167FA21B8286F23345CC03B176EC5728D3EC4E382874DA558D3ED93E71FA2837309D9F56096BEA801"},
{"Curve41417", "0x1BAD3ED5760CF453112EB5B3916A53B1002633687066EADDDF26499124BCE6BA52C2332C7AB8779EDC59B493385213660F970987", "0328A252573580B45524358AAE124651035A640987A601EC88372A64C5E951BCFC22783085880AF648771BE9A980494BB6D05C557F"},
{"Curve41417", "0x2CE22EF7CF53DDD57428E5E94387794582E2C7D5C039D731A44CA6F9216F80267471A2CE234AED407A18F603F33CF6DD7F8C8F84", "03316121820D74C271DD30DF13D065467F63C8A965AE7A290941F41F62897C5A9084F45A5E5B16120F1E6BE1AD8234E525266AA926"},
{"Curve41417", "0x19CFE97587D7DFA78DA95EDD1032054D2675282F11A1679A54DDE04CF0F9038375FEC70AAB3585D4CFB01928DC9EA9A2EC5945FF", "02370157F3FF6AC11454F78626C2EE2F940D585F2C1ECC1C3E3CBD2C3703E67E0808F1001DDA8DEA31BD309D4FBADDF49F91E700D0"},
{"Curve41417", "0x32AA752EE8732AA1A93ECA50ADF96B230E860C0CE91E6EADB9C7515DE097B8D757D2159F8E9E13C636FB7F0867844C917314E9C4", "021DA327981399DDFB0868BFB5513BFC7E5FB4F1892CB789AF06A672041B7B41D90F4A41A25BCFAC03FD996F4C27F15450F3E7B96A"},
{"Curve41417", "0x079913B079EF8B79DA58DD82E0569F7B44D48154F3E8A4A9ED2E961445F7D3958606CEC2ECF6BB9E5C2C42E7630C7D39E84F21F8", "021418BD6D0D3A98EE3AA8850615D8A581EB1D214FF8550EA76EF34B8008B28D4FE82A466C972782141360AD3A7C974C3AB113BA7D"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEE", "022B3368E30E148B3917E5DEDA3A73845ADBC61E11DFE51E0C0CDA06DEBBAD63A3E0B871FB11025B0858583BC9C538CCFED4B6E24B"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED", "0337A885FE3FABC6FFFD42F22B841950A40BABFCD98E5D4D4096B22FE2826C6C026DAFB4CD8BC84CE037009A37E25EC6DB0DCC0A48"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC", "0324C7711B0BDAB798EBC2FB32A816681BC798CD15CED9F21DCEE498F0A9CC42D406A0C34229B2B298B52EB7DAFDC0FF70CB7C7D09"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEB", "0208F7B75F8B6CA2C4CDD4B3DF292493A122BD2F4459CD31D52AB212FAC7C43CA43FAC2B83C106266C36B7EBAC67F7428CD3795712"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEA", "0201693A5D70A1E2ABF1D366F38820BF01DC6D79B4A285244B7B1B8AFCBB6FA28C359CFF0CE360A264ECC66F566524CC65AF157990"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE9", "0223007EF71F772F0FBF8B6E685D39F9557806FE13DFEC2C91CC3A2C2294B3DF331C27AE430B68DB47B23CD6D9618EA01DCA9ECD2C"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE8", "02078DC7DE28F30EEDED62E89F74A238FFACE864469A7A505987B1270341B1C3E663168F30A41836A7DBE82D18C9133B135B8D25CB"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE7", "032BCD9A2390A35229F33AD235A19D374DD49C3FEABBEAB472511C3D757D7F4788433B9C8121FC11C7884E571ED93D3DEDC11E016B"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE6", "0339551FAC19FA6B7220A8F938B79071BC0E910D97149F5D5D9F7DA87AF7BB01E0966838284A251E05A4D316A396BFB4B42C1F83E5"},
{"Curve41417", "0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE5", "030A02106661D130C814B0ED064A16F5A8765D57D9C83811DB026930357017D5E0D737130B36E64DC73670BB05F4B2AACE4C6A7C47"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000000", "000000000000000000000000000000000000000000000000000000000000000000"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000001", "02216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000002", "0336AB384C9F5A046C3D043B7D1833E7AC080D8E4515D7A45F83C5A14E2843CE0E"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000003", "0267AE9C4A22928F491FF4AE743EDAC83A6343981981624886AC62485FD3F8E25C"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000004", "03203DA8DB56CFF1468325D4B87A3520F91A739EC193CE1547493AA657C4C9F870"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000005", "0349FDA73EADE3587BFCEF7CF7D12DA5DE5C2819F93E1BE1A591409CC0322EF233"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000006", "024C9797BA7A45601C62AEACC0DD0A29BEA1E599826C7B4427783A741A7DCBF23D"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000007", "0214568685FCF4BD4EE9E3EE194B1D810783E809F3BBF1CE955855981AF50E4107"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000008", "026742E15F97D771B642862D5CF84ECF93EB3AC67B80698B993B87FDBC08A584C8"},
{"Ed25519", "0x0000000000000000000000000000000000000000000000000000000000000009", "02357CC970C80071651BF336E06F9422B886D80E5C2E4E0294D3E023065185715C"},
{"Ed25519", "0x36C93824E3C8662DA3FB658765B3D158AE0DB925DA706736A1C407251058B6F3", "0247165846E2AA7A84B44F2BBA49491EE1ACB71BF13550B07D074EBB331F3703EA"},
{"Ed25519", "0x518F9F55C83EB8DDFDA9BF6340543A4EBB2B23221480836031951B5881EFA9CB", "03349BDDBE317C6D191911C3C46EC4AC84D98B12573328F763F29D323FE87E0790"},
{"Ed25519", "0x29F26E6FA140E617CB777BF90D5E18E43A65575933FF44FFB60F6B6FB507A0B8", "024E0C6AC71F7C0EB0C7E05AE5B1D4B0FBCAD499A257208D6F62122733A0A5F721"},
{"Ed25519", "0x3EDD7F19D69109213ACCA24379318490346E9287062CAFB437836A46DF652647", "020C140A150068448EFFEE67E5F3720C89D27A93BE78E64FD27FACD0C2C464BD7A"},
{"Ed25519", "0x2AE90A79FB6C3BCB45588567F06AE19E1058CE14B196C7AF2C5FB1FB10075C7F", "025D592A00390D8E99952EF7799738E9111FD791EEE9747A6B86802171247FF257"},
{"Ed25519", "0x059CB23E93009210AB61C29819F8C8EBD81FFC91F0C3E9810EBE6712FAE3A370", "02108930D014954A42EF69B37C1DB5A072B6BDF2A43A77547B26C769B3B72B9937"},
{"Ed25519", "0x6254D50ED6744DC37FA5045B4A5A2E93A0A182CAEDF0234A3D3D5F0F38134B5E", "035714D3A721F622D30CF459776A040991E4FEE5F93E15B26CC86C50072821DE5D"},
{"Ed25519", "0x5C3A044FCF90E12E02DAFB775975203F1B518FBF7D8E873DC2941561F583A1F1", "0270BBAA9F26722B77CCCD2FCB4B15D802570187DEA85F472F1D47B2AA3E917905"},
{"Ed25519", "0x2EA50B3EFC524B1745808AA302FEDE3218D939BCD2CFC09F7AC864C6B79FB4EA", "03020E17044E6F483FDC64B97D97179D60CFE9AC3EADF41146DE181B0DA3B3059D"},
{"Ed25519", "0x0C9E976CAAB4F45FC82FCA1B6805821EFAD6D564CA58C0812DBB9D873EC5783A", "0377B004183CCA51632D04B9AE0ADF91D49A62F508A15FDE4680189806C3C6865C"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC", "030F70DC6321F0100E50A9299A63B02D6722FA3AAD87F55B45D26E77860EA2BAD6"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEB", "026187590A933071BFAF5751DD6F1E310212EB0E6F0EDF0C7D1647F5739EF69A0F"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEA", "026264C827E7DF736BE0F2708CFDBC5E3D8A5155667FD09D626D2A2B22A2336E40"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE9", "031025A1A731C4258A3E2332B3026D11E2D37F55B622C2289904251BBB7AEBADB4"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE8", "034532B9B5AF6349FD9F71F1D2671875C78FD68A53A6C1872B3B59746A25062C06"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE7", "021C808F1336D1E77AE252C9354C7465B01F359D1D11043377DBAB97AC5ACD881A"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE6", "0358F3D421BAF9C5013D197B99A15BE542188D963745883BF7440BE7D60B3083E7"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE5", "032E1E3807E876AF2A9F97042208258EF5C9F6B7DBB49CF40B416597C3DF57F357"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE4", "020DE5DDBAB3AFB9972367D0F3203857E76ED607BC6258D592DC33FE97D930F0B1"},
{"Ed25519", "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE3", "02115A1F4AFB1992739C25ADF12FCF87A1A9A9138FF46968B65C8E8C7DA220DB86"},
};

START_TEST(test_mpECP_add)
    int error, i, j, npoints;
    mpECurve_t cv;
    mpECP_t a, b, c, d;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    mpECP_init(d);
    
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints ; i++) {
        int slen;
        char *buffer;

        error = mpECurve_set_named(cv, test_point[i].curve);
        assert(error == 0);
        error = mpECP_set_str(a, test_point[i].pub, cv);
        assert(error == 0);
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        for (j = i + 1; j < npoints; j++) {
            if (test_point[j].curve != test_point[i].curve) continue;
            error = mpECP_set_str(b, test_point[j].pub, cv);
            assert(error == 0);
            mpECP_add(c, a, b);
            mpECP_out_str(buffer, a, 0);
            printf(" A: %s\n", buffer);
            mpECP_out_str(buffer, b, 0);
            printf("+B: %s\n", buffer);
            printf("-------\n");
            mpECP_out_str(buffer, c, 0);
            printf("=C: %s\n\n", buffer);
            // commutative A + B = B + A
            mpECP_add(d, b, a);
            assert(mpECP_cmp(c, d) == 0);
            mpECP_neg(d, b);
            // A + B + (-B) = A
            mpECP_out_str(buffer, d, 0);
            // printf("+D: %s (D = -B)\n", buffer);
            mpECP_add(b, c, d);
            mpECP_out_str(buffer, b, 0);
            // printf("=A: %s\n\n", buffer);
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

START_TEST(test_mpECP_double)
    int error, i, npoints;
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        error = mpECurve_set_named(cv, test_point[i].curve);
        assert(error == 0);
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
        printf("=B: %s\n\n", buffer);
        mpECP_neg(c, a);
        // A + A + (-A) = A
        mpECP_out_str(buffer, c, 0);
        // printf("+C: %s (C = -A)\n", buffer);
        mpECP_add(b, b, c);
        mpECP_out_str(buffer, b, 0);
        // printf("=A: %s\n\n", buffer);
        assert(mpECP_cmp(a, b) == 0);
        free(buffer);
    }

    mpECP_clear(c);
    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_scalar_mul)
    int error, i, npoints;
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpz_t r;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    mpz_init(r);
    
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        error = mpECurve_set_named(cv, test_point[i].curve);
        assert(error == 0);
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
        printf("=B: %s\n\n", buffer);
        mpECP_set_str(c, test_point[i].pub, cv);
        // A + A + (-A) = A
        mpECP_out_str(buffer, c, 0);
        // printf("=C: %s\n\n", buffer);
        assert(mpECP_cmp(b, c) == 0);
        free(buffer);
    }

    mpz_clear(r);
    mpECP_clear(c);
    mpECP_clear(b);
    mpECP_clear(a);
    mpECurve_clear(cv);
END_TEST

START_TEST(test_mpECP_scalar_base_mul)
    int error, i, npoints;
    mpECurve_t cv;
    mpECP_t a, b, c;
    mpz_t r;
    mpECurve_init(cv);
    mpECP_init(a);
    mpECP_init(b);
    mpECP_init(c);
    mpz_init(r);
    
    error = mpECurve_set_named(cv, test_point[0].curve);
    assert(error == 0);
    mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);
    mpECP_scalar_base_mul_setup(a);
    npoints = sizeof(test_point) / sizeof(test_point[0]);
    for (i = 0 ; i < npoints; i++) {
        int slen;
        char *buffer;
        
        if (i > 0) {
            if (strcmp(test_point[i].curve, test_point[i-1].curve) != 0) {
                error = mpECurve_set_named(cv, test_point[i].curve);
                assert(error == 0);
                mpECP_set_mpz(a, cv->G[0], cv->G[1], cv);
                mpECP_scalar_base_mul_setup(a);
            }
        }
        slen = mpECP_out_strlen(a, 0);
        buffer = malloc((slen + 1)* sizeof(char));
        assert(buffer != NULL);
        error = mpz_set_str(r, test_point[i].pvt, 0);
        assert(error == 0);
        mpECP_scalar_base_mul_mpz(b, a, r);
        mpECP_out_str(buffer, a, 0);
        printf(" A: %s\n", buffer);
        gmp_printf("*b: %ZX\n", r);
        printf("-------\n");
        mpECP_out_str(buffer, b, 0);
        printf("=B: %s\n\n", buffer);
        mpECP_set_str(c, test_point[i].pub, cv);
        // A + A + (-A) = A
        mpECP_out_str(buffer, c, 0);
        // printf("=C: %s\n\n", buffer);
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
    tcase_add_test(tc, test_mpECP_add);
    tcase_add_test(tc, test_mpECP_double);
    tcase_add_test(tc, test_mpECP_scalar_mul);
    tcase_add_test(tc, test_mpECP_scalar_base_mul);
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
