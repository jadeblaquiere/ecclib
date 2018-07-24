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

int main(void) {
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
            unsigned char *sigbytes;
            size_t sigbytessz;
            
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
            free(sigbytes);
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
    return 0;
}
