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
#include <ecc/ecdsa.h>
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

void wrap_libsodium_sha512(unsigned char *hash, unsigned char *msg, size_t sz) {
    int status;
    status = crypto_hash_sha512(hash, msg, (unsigned long long)sz);
    assert(status == 0);
    return;
}

void wrap_libsodium_sha256(unsigned char *hash, unsigned char *msg, size_t sz) {
    int status;
    status = crypto_hash_sha256(hash, msg, (unsigned long long)sz);
    assert(status == 0);
    return;
}

#define _TEST_MESSAGE_MAX   (100)

START_TEST(test_mpECDSA_sscheme_init)
    char **std_curves;
    mpECDSAHashfunc_t H;
    int i;

    mpECDSAHashfunc_init(H);
    H->dohash = wrap_libsodium_sha512;
    H->hsz = crypto_hash_sha512_BYTES;

    std_curves = _mpECurve_list_standard_curves();
    i = 0;
    while (std_curves[i] != NULL) {
        mpECurve_t cv;
        mpECDSASignatureScheme_t sscheme;
        mpFp_t sK;
        mpECP_t pK;
        int status;
        int j;

        mpECurve_init(cv);
        status = mpECurve_set_named(cv, std_curves[i]);
        assert(status == 0);

        printf("validating ECDSA for SHA512 and curve %s\n", std_curves[i]);

        mpFp_init(sK, cv->n);
        do {
            mpFp_urandom(sK, cv->n);
        } while (mpFp_cmp_ui(sK, 0) == 0);

        mpECDSASignatureScheme_init(sscheme, cv, H);

        mpECP_init(pK, cv);
        mpECP_scalar_base_mul(pK, sscheme->cv_G, sK);

        for (j = 1; j < _TEST_MESSAGE_MAX; j++) {
            unsigned char msg[_TEST_MESSAGE_MAX];
            mpECDSASignature_t sig;
            mpECDSASignature_t sigcp;
            unsigned char *sigbytes;
            size_t sigbytessz;
            char *sigstr;
            
            randombytes_buf(msg, j);
            status = mpECDSASignature_init_Sign(sig, sscheme, sK, msg, j);
            assert(status == 0);
            status = mpECDSASignature_verify_cmp(sig, pK, msg, j);
            assert(status == 0);

            sigbytes = mpECDSASignature_export_bytes(sig, &sigbytessz);
            assert(sigbytes != NULL);
            assert(sigbytessz > 0);

            //printf("Signature of ");
            //{
            //    int k;
            //    for (k = 0 ; k < j; k++) {
            //        printf("%02X", msg[k]);
            //    }
            //}
            //printf(" is ");
            //{
            //    int k;
            //    for (k = 0 ; k < sigbytessz; k++) {
            //        printf("%02X", sigbytes[k]);
            //    }
            //}
            //printf("\n");
            
            status = mpECDSASignature_init_import_bytes(sigcp, sscheme, sigbytes, sigbytessz);
            assert(status == 0);
            assert(mpFp_cmp(sig->r, sigcp->r) == 0);
            assert(mpFp_cmp(sig->s, sigcp->s) == 0);
            mpECDSASignature_clear(sigcp);
            free(sigbytes);

            sigstr = mpECDSASignature_export_str(sig);
            assert(sigstr != NULL);
            assert(strlen(sigstr) > 0);
            assert((strlen(sigstr) & 0x01) == 0);

            //printf("Signature of ");
            //{
            //    int k;
            //    for (k = 0 ; k < j; k++) {
            //        printf("%02X", msg[k]);
            //    }
            //}
            //printf(" is ");
            //{
            //    int k;
            //    for (k = 0 ; k < sigbytessz; k++) {
            //        printf("%02X", sigbytes[k]);
            //    }
            //}
            //printf("\n");
            
            status = mpECDSASignature_init_import_str(sigcp, sscheme, sigstr);
            assert(status == 0);
            assert(mpFp_cmp(sig->r, sigcp->r) == 0);
            assert(mpFp_cmp(sig->s, sigcp->s) == 0);
            mpECDSASignature_clear(sigcp);
            free(sigstr);

            mpECDSASignature_clear(sig);
        }

        mpECP_clear(pK);
        mpECDSASignatureScheme_clear(sscheme);
        mpFp_clear(sK);

        mpECurve_clear(cv);

        free(std_curves[i]);
        i++;
    }
    free(std_curves);
END_TEST

typedef struct {
    char *curve_name;
    _ecdsa_dohash hash_func;
    size_t hash_size;
    char *x;
    char *Ux;
    char *Uy;
    char *message;
    char *k;
    char *r;
    char *s;
} _ecdsa_test_vector_t;

// ECDSA test vectors from https://tools.ietf.org/html/rfc6979

static _ecdsa_test_vector_t rfc6979_ecdsa_test_vectors[] = {
    {
        "P192",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "6FAB034934E4C0FC9AE67F5B5659A9D7D1FEFD187EE09FD4",
        "AC2C77F529F91689FEA0EA5EFEC7F210D8EEA0B9E047ED56",
        "3BC723E57670BD4887EBC732C523063D0A7C957BC97C1C43",
        "sample",
        "32B1B6D7D42A05CB449065727A84804FB1A3E34D8F261496",
        "4B0B8CE98A92866A2820E20AA6B75B56382E0F9BFD5ECB55",
        "CCDB006926EA9565CBADC840829D8C384E06DE1F1E381B85"
    },
    {
        "P192",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "6FAB034934E4C0FC9AE67F5B5659A9D7D1FEFD187EE09FD4",
        "AC2C77F529F91689FEA0EA5EFEC7F210D8EEA0B9E047ED56",
        "3BC723E57670BD4887EBC732C523063D0A7C957BC97C1C43",
        "sample",
        "A2AC7AB055E4F20692D49209544C203A7D1F2C0BFBC75DB1",
        "4D60C5AB1996BD848343B31C00850205E2EA6922DAC2E4B8",
        "3F6E837448F027A1BF4B34E796E32A811CBB4050908D8F67"
    },
    {
        "P192",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "6FAB034934E4C0FC9AE67F5B5659A9D7D1FEFD187EE09FD4",
        "AC2C77F529F91689FEA0EA5EFEC7F210D8EEA0B9E047ED56",
        "3BC723E57670BD4887EBC732C523063D0A7C957BC97C1C43",
        "test",
        "5C4CE89CF56D9E7C77C8585339B006B97B5F0680B4306C6C",
        "3A718BD8B4926C3B52EE6BBE67EF79B18CB6EB62B1AD97AE",
        "5662E6848A4A19B1F1AE2F72ACD4B8BBE50F1EAC65D9124F"
    },
    {
        "P192",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "6FAB034934E4C0FC9AE67F5B5659A9D7D1FEFD187EE09FD4",
        "AC2C77F529F91689FEA0EA5EFEC7F210D8EEA0B9E047ED56",
        "3BC723E57670BD4887EBC732C523063D0A7C957BC97C1C43",
        "test",
        "0758753A5254759C7CFBAD2E2D9B0792EEE44136C9480527",
        "FE4F4AE86A58B6507946715934FE2D8FF9D95B6B098FE739",
        "74CF5605C98FBA0E1EF34D4B5A1577A7DCF59457CAE52290"
    },
    {
        "P224",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "F220266E1105BFE3083E03EC7A3A654651F45E37167E88600BF257C1",
        "00CF08DA5AD719E42707FA431292DEA11244D64FC51610D94B130D6C",
        "EEAB6F3DEBE455E3DBF85416F7030CBD94F34F2D6F232C69F3C1385A",
        "sample",
        "AD3029E0278F80643DE33917CE6908C70A8FF50A411F06E41DEDFCDC",
        "61AA3DA010E8E8406C656BC477A7A7189895E7E840CDFE8FF42307BA",
        "BC814050DAB5D23770879494F9E0A680DC1AF7161991BDE692B10101"
    },
    {
        "P224",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "F220266E1105BFE3083E03EC7A3A654651F45E37167E88600BF257C1",
        "00CF08DA5AD719E42707FA431292DEA11244D64FC51610D94B130D6C",
        "EEAB6F3DEBE455E3DBF85416F7030CBD94F34F2D6F232C69F3C1385A",
        "sample",
        "9DB103FFEDEDF9CFDBA05184F925400C1653B8501BAB89CEA0FBEC14",
        "074BD1D979D5F32BF958DDC61E4FB4872ADCAFEB2256497CDAC30397",
        "A4CECA196C3D5A1FF31027B33185DC8EE43F288B21AB342E5D8EB084"
    },
    {
        "P224",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "F220266E1105BFE3083E03EC7A3A654651F45E37167E88600BF257C1",
        "00CF08DA5AD719E42707FA431292DEA11244D64FC51610D94B130D6C",
        "EEAB6F3DEBE455E3DBF85416F7030CBD94F34F2D6F232C69F3C1385A",
        "test",
        "FF86F57924DA248D6E44E8154EB69F0AE2AEBAEE9931D0B5A969F904",
        "AD04DDE87B84747A243A631EA47A1BA6D1FAA059149AD2440DE6FBA6",
        "178D49B1AE90E3D8B629BE3DB5683915F4E8C99FDF6E666CF37ADCFD"
    },
    {
        "P224",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "F220266E1105BFE3083E03EC7A3A654651F45E37167E88600BF257C1",
        "00CF08DA5AD719E42707FA431292DEA11244D64FC51610D94B130D6C",
        "EEAB6F3DEBE455E3DBF85416F7030CBD94F34F2D6F232C69F3C1385A",
        "test",
        "E39C2AA4EA6BE2306C72126D40ED77BF9739BB4D6EF2BBB1DCB6169D",
        "049F050477C5ADD858CAC56208394B5A55BAEBBE887FDF765047C17C",
        "077EB13E7005929CEFA3CD0403C7CDCC077ADF4E44F3C41B2F60ECFF"
    },
    {
        "P256",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721",
        "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6",
        "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299",
        "sample",
        "A6E3C57DD01ABE90086538398355DD4C3B17AA873382B0F24D6129493D8AAD60",
        "EFD48B2AACB6A8FD1140DD9CD45E81D69D2C877B56AAF991C34D0EA84EAF3716",
        "F7CB1C942D657C41D436C7A1B6E29F65F3E900DBB9AFF4064DC4AB2F843ACDA8"
    },
    {
        "P256",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721",
        "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6",
        "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299",
        "sample",
        "5FA81C63109BADB88C1F367B47DA606DA28CAD69AA22C4FE6AD7DF73A7173AA5",
        "8496A60B5E9B47C825488827E0495B0E3FA109EC4568FD3F8D1097678EB97F00",
        "2362AB1ADBE2B8ADF9CB9EDAB740EA6049C028114F2460F96554F61FAE3302FE"
    },
    {
        "P256",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721",
        "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6",
        "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299",
        "test",
        "D16B6AE827F17175E040871A1C7EC3500192C4C92677336EC2537ACAEE0008E0",
        "F1ABB023518351CD71D881567B1EA663ED3EFCF6C5132B354F28D3B0B7D38367",
        "019F4113742A2B14BD25926B49C649155F267E60D3814B4C0CC84250E46F0083"
    },
    {
        "P256",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721",
        "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6",
        "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299",
        "test",
        "6915D11632ACA3C40D5D51C08DAF9C555933819548784480E93499000D9F0B7F",
        "461D93F31B6540894788FD206C07CFA0CC35F46FA3C91816FFF1040AD1581A04",
        "39AF9F15DE0DB8D97E72719C74820D304CE5226E32DEDAE67519E840D1194E55"
    },
    {
        "P384",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "6B9D3DAD2E1B8C1C05B19875B6659F4DE23C3B667BF297BA9AA47740787137D896D5724E4C70A825F872C9EA60D2EDF5",
        "EC3A4E415B4E19A4568618029F427FA5DA9A8BC4AE92E02E06AAE5286B300C64DEF8F0EA9055866064A254515480BC13",
        "8015D9B72D7D57244EA8EF9AC0C621896708A59367F9DFB9F54CA84B3F1C9DB1288B231C3AE0D4FE7344FD2533264720",
        "sample",
        "180AE9F9AEC5438A44BC159A1FCB277C7BE54FA20E7CF404B490650A8ACC414E375572342863C899F9F2EDF9747A9B60",
        "21B13D1E013C7FA1392D03C5F99AF8B30C570C6F98D4EA8E354B63A21D3DAA33BDE1E888E63355D92FA2B3C36D8FB2CD",
        "F3AA443FB107745BF4BD77CB3891674632068A10CA67E3D45DB2266FA7D1FEEBEFDC63ECCD1AC42EC0CB8668A4FA0AB0"
    },
    {
        "P384",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "6B9D3DAD2E1B8C1C05B19875B6659F4DE23C3B667BF297BA9AA47740787137D896D5724E4C70A825F872C9EA60D2EDF5",
        "EC3A4E415B4E19A4568618029F427FA5DA9A8BC4AE92E02E06AAE5286B300C64DEF8F0EA9055866064A254515480BC13",
        "8015D9B72D7D57244EA8EF9AC0C621896708A59367F9DFB9F54CA84B3F1C9DB1288B231C3AE0D4FE7344FD2533264720",
        "sample",
        "92FC3C7183A883E24216D1141F1A8976C5B0DD797DFA597E3D7B32198BD35331A4E966532593A52980D0E3AAA5E10EC3",
        "ED0959D5880AB2D869AE7F6C2915C6D60F96507F9CB3E047C0046861DA4A799CFE30F35CC900056D7C99CD7882433709",
        "512C8CCEEE3890A84058CE1E22DBC2198F42323CE8ACA9135329F03C068E5112DC7CC3EF3446DEFCEB01A45C2667FDD5"
    },
    {
        "P384",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "6B9D3DAD2E1B8C1C05B19875B6659F4DE23C3B667BF297BA9AA47740787137D896D5724E4C70A825F872C9EA60D2EDF5",
        "EC3A4E415B4E19A4568618029F427FA5DA9A8BC4AE92E02E06AAE5286B300C64DEF8F0EA9055866064A254515480BC13",
        "8015D9B72D7D57244EA8EF9AC0C621896708A59367F9DFB9F54CA84B3F1C9DB1288B231C3AE0D4FE7344FD2533264720",
        "test",
        "0CFAC37587532347DC3389FDC98286BBA8C73807285B184C83E62E26C401C0FAA48DD070BA79921A3457ABFF2D630AD7",
        "6D6DEFAC9AB64DABAFE36C6BF510352A4CC27001263638E5B16D9BB51D451559F918EEDAF2293BE5B475CC8F0188636B",
        "2D46F3BECBCC523D5F1A1256BF0C9B024D879BA9E838144C8BA6BAEB4B53B47D51AB373F9845C0514EEFB14024787265"
    },
    {
        "P384",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "6B9D3DAD2E1B8C1C05B19875B6659F4DE23C3B667BF297BA9AA47740787137D896D5724E4C70A825F872C9EA60D2EDF5",
        "EC3A4E415B4E19A4568618029F427FA5DA9A8BC4AE92E02E06AAE5286B300C64DEF8F0EA9055866064A254515480BC13",
        "8015D9B72D7D57244EA8EF9AC0C621896708A59367F9DFB9F54CA84B3F1C9DB1288B231C3AE0D4FE7344FD2533264720",
        "test",
        "3780C4F67CB15518B6ACAE34C9F83568D2E12E47DEAB6C50A4E4EE5319D1E8CE0E2CC8A136036DC4B9C00E6888F66B6C",
        "A0D5D090C9980FAF3C2CE57B7AE951D31977DD11C775D314AF55F76C676447D06FB6495CD21B4B6E340FC236584FB277",
        "976984E59B4C77B0E8E4460DCA3D9F20E07B9BB1F63BEEFAF576F6B2E8B224634A2092CD3792E0159AD9CEE37659C736"
    },
    {
        "P521",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "0FAD06DAA62BA3B25D2FB40133DA757205DE67F5BB0018FEE8C86E1B68C7E75CAA896EB32F1F47C70855836A6D16FCC1466F6D8FBEC67DB89EC0C08B0E996B83538",
        "1894550D0785932E00EAA23B694F213F8C3121F86DC97A04E5A7167DB4E5BCD371123D46E45DB6B5D5370A7F20FB633155D38FFA16D2BD761DCAC474B9A2F5023A4",
        "0493101C962CD4D2FDDF782285E64584139C2F91B47F87FF82354D6630F746A28A0DB25741B5B34A828008B22ACC23F924FAAFBD4D33F81EA66956DFEAA2BFDFCF5",
        "sample",
        "0EDF38AFCAAECAB4383358B34D67C9F2216C8382AAEA44A3DAD5FDC9C32575761793FEF24EB0FC276DFC4F6E3EC476752F043CF01415387470BCBD8678ED2C7E1A0",
        "1511BB4D675114FE266FC4372B87682BAECC01D3CC62CF2303C92B3526012659D16876E25C7C1E57648F23B73564D67F61C6F14D527D54972810421E7D87589E1A7",
        "04A171143A83163D6DF460AAF61522695F207A58B95C0644D87E52AA1A347916E4F7A72930B1BC06DBE22CE3F58264AFD23704CBB63B29B931F7DE6C9D949A7ECFC"
    },
    {
        "P521",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "0FAD06DAA62BA3B25D2FB40133DA757205DE67F5BB0018FEE8C86E1B68C7E75CAA896EB32F1F47C70855836A6D16FCC1466F6D8FBEC67DB89EC0C08B0E996B83538",
        "1894550D0785932E00EAA23B694F213F8C3121F86DC97A04E5A7167DB4E5BCD371123D46E45DB6B5D5370A7F20FB633155D38FFA16D2BD761DCAC474B9A2F5023A4",
        "0493101C962CD4D2FDDF782285E64584139C2F91B47F87FF82354D6630F746A28A0DB25741B5B34A828008B22ACC23F924FAAFBD4D33F81EA66956DFEAA2BFDFCF5",
        "sample",
        "1DAE2EA071F8110DC26882D4D5EAE0621A3256FC8847FB9022E2B7D28E6F10198B1574FDD03A9053C08A1854A168AA5A57470EC97DD5CE090124EF52A2F7ECBFFD3",
        "0C328FAFCBD79DD77850370C46325D987CB525569FB63C5D3BC53950E6D4C5F174E25A1EE9017B5D450606ADD152B534931D7D4E8455CC91F9B15BF05EC36E377FA",
        "0617CCE7CF5064806C467F678D3B4080D6F1CC50AF26CA209417308281B68AF282623EAA63E5B5C0723D8B8C37FF0777B1A20F8CCB1DCCC43997F1EE0E44DA4A67A"
    },
    {
        "P521",
        wrap_libsodium_sha256,
        crypto_hash_sha256_BYTES,
        "0FAD06DAA62BA3B25D2FB40133DA757205DE67F5BB0018FEE8C86E1B68C7E75CAA896EB32F1F47C70855836A6D16FCC1466F6D8FBEC67DB89EC0C08B0E996B83538",
        "1894550D0785932E00EAA23B694F213F8C3121F86DC97A04E5A7167DB4E5BCD371123D46E45DB6B5D5370A7F20FB633155D38FFA16D2BD761DCAC474B9A2F5023A4",
        "0493101C962CD4D2FDDF782285E64584139C2F91B47F87FF82354D6630F746A28A0DB25741B5B34A828008B22ACC23F924FAAFBD4D33F81EA66956DFEAA2BFDFCF5",
        "test",
        "01DE74955EFAABC4C4F17F8E84D881D1310B5392D7700275F82F145C61E843841AF09035BF7A6210F5A431A6A9E81C9323354A9E69135D44EBD2FCAA7731B909258",
        "00E871C4A14F993C6C7369501900C4BC1E9C7B0B4BA44E04868B30B41D8071042EB28C4C250411D0CE08CD197E4188EA4876F279F90B3D8D74A3C76E6F1E4656AA8",
        "0CD52DBAA33B063C3A6CD8058A1FB0A46A4754B034FCC644766CA14DA8CA5CA9FDE00E88C1AD60CCBA759025299079D7A427EC3CC5B619BFBC828E7769BCD694E86"
    },
    {
        "P521",
        wrap_libsodium_sha512,
        crypto_hash_sha512_BYTES,
        "0FAD06DAA62BA3B25D2FB40133DA757205DE67F5BB0018FEE8C86E1B68C7E75CAA896EB32F1F47C70855836A6D16FCC1466F6D8FBEC67DB89EC0C08B0E996B83538",
        "1894550D0785932E00EAA23B694F213F8C3121F86DC97A04E5A7167DB4E5BCD371123D46E45DB6B5D5370A7F20FB633155D38FFA16D2BD761DCAC474B9A2F5023A4",
        "0493101C962CD4D2FDDF782285E64584139C2F91B47F87FF82354D6630F746A28A0DB25741B5B34A828008B22ACC23F924FAAFBD4D33F81EA66956DFEAA2BFDFCF5",
        "test",
        "16200813020EC986863BEDFC1B121F605C1215645018AEA1A7B215A564DE9EB1B38A67AA1128B80CE391C4FB71187654AAA3431027BFC7F395766CA988C964DC56D",
        "13E99020ABF5CEE7525D16B69B229652AB6BDF2AFFCAEF38773B4B7D08725F10CDB93482FDCC54EDCEE91ECA4166B2A7C6265EF0CE2BD7051B7CEF945BABD47EE6D",
        "1FBD0013C674AA79CB39849527916CE301C66EA7CE8B80682786AD60F98F7E78A19CA69EFF5C57400E3B3A0AD66CE0978214D13BAF4E9AC60752F7B155E2DE4DCE3"
    }
};

static void _shift_right_and_zero_pad(unsigned char *buffer, size_t len, size_t shift) {
    int i;

    for (i = len-1; i >= shift; i--) {
        buffer[i] = buffer[i-shift];
    }
    for (i = shift - 1; i >= 0; i--) {
        buffer[i] = 0;
    }
    return;
}

START_TEST(test_mpECDSA_reference_test_vectors)
    int nvecs;
    int i;

    nvecs = sizeof(rfc6979_ecdsa_test_vectors) / sizeof(rfc6979_ecdsa_test_vectors[0]);
    for (i = 0; i < nvecs; i++) {
        mpECDSAHashfunc_t H;
        mpECurve_t cv;
        mpECDSASignatureScheme_t sscheme;
        mpECDSASignature_t sig;
        mpFp_t sK;
        mpECP_t pK;
        int status;

        mpECDSAHashfunc_init(H);
        H->dohash = rfc6979_ecdsa_test_vectors[i].hash_func;
        H->hsz = rfc6979_ecdsa_test_vectors[i].hash_size;

        mpECurve_init(cv);
        status = mpECurve_set_named(cv, rfc6979_ecdsa_test_vectors[i].curve_name);
        assert(status == 0);

        printf("testing hash for curve %s (%zu bit hash) for message \"%s\"\n",
            rfc6979_ecdsa_test_vectors[i].curve_name,
            rfc6979_ecdsa_test_vectors[i].hash_size,
            rfc6979_ecdsa_test_vectors[i].message);

        mpECDSASignatureScheme_init(sscheme, cv, H);

        mpFp_init(sK, cv->n);

        {
            mpz_t sec_z;
            mpz_t pubx_z;
            mpz_t puby_z;

            mpz_init(sec_z);
            mpz_init(pubx_z);
            mpz_init(puby_z);

            status = mpz_set_str(sec_z, rfc6979_ecdsa_test_vectors[i].x, 16);
            status = mpz_set_str(pubx_z, rfc6979_ecdsa_test_vectors[i].Ux, 16);
            status = mpz_set_str(puby_z, rfc6979_ecdsa_test_vectors[i].Uy, 16);

            mpFp_set_mpz(sK, sec_z, cv->n);
            mpECP_init(pK, cv);
            mpECP_scalar_base_mul(pK, sscheme->cv_G, sK);
            mpz_set_mpECP_affine_x(sec_z, pK);
            assert(mpz_cmp(pubx_z, sec_z) == 0);
            mpz_set_mpECP_affine_y(sec_z, pK);
            assert(mpz_cmp(puby_z, sec_z) == 0);

            mpz_clear(puby_z);
            mpz_clear(pubx_z);
            mpz_clear(sec_z);
        }

        {
            unsigned char *buffer;
            size_t bsz;
            size_t esz;
            //int j;
            mpz_t r;
            mpz_t s;

            mpz_init(r);
            mpz_init(s);

            bsz = (strlen(rfc6979_ecdsa_test_vectors[i].r) + 1) >> 1;
            buffer = (unsigned char *)malloc((bsz << 1) * sizeof(char));

            mpz_set_str(r, rfc6979_ecdsa_test_vectors[i].r, 16);
            mpz_set_str(s, rfc6979_ecdsa_test_vectors[i].s, 16);
            esz = bsz;
            mpz_export (buffer, &esz, 1, 1, 1, 0, r);
            assert(esz <= bsz);
            if (esz < bsz) _shift_right_and_zero_pad(buffer, bsz, bsz - esz);
            mpz_export (buffer + bsz, &esz, 1, 1, 1, 0, s);
            assert(esz <= bsz);
            if (esz < bsz) _shift_right_and_zero_pad(buffer + bsz, bsz, bsz - esz);
            //printf("signature = ");
            //for (j = 0; j < (bsz << 1); j++) {
            //    printf("%02X", buffer[j]);
            //}
            //printf("\n");
            status = mpECDSASignature_init_import_bytes(sig, sscheme, buffer, bsz << 1);
            assert(status == 0);
            free(buffer);
        }

        {
            unsigned char *msg;

            msg = (unsigned char *)rfc6979_ecdsa_test_vectors[i].message;
            status = mpECDSASignature_verify_cmp(sig, pK, msg, strlen((char *)msg));
            assert(status == 0);
        }

        mpECDSASignature_clear(sig);
        mpECP_clear(pK);
        mpFp_clear(sK);

        mpECurve_clear(cv);
        mpECDSAHashfunc_clear(H);
    }
END_TEST

static Suite *mpECDSA_test_suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("Multi-Precision over Prime Fields");
    tc = tcase_create("ECDSA algorithm");

    tcase_add_test(tc, test_mpECDSA_sscheme_init);
    tcase_add_test(tc, test_mpECDSA_reference_test_vectors);

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
