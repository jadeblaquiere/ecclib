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

#ifndef _EC_ECDSA_H_INCLUDED_
#define _EC_ECDSA_H_INCLUDED_

#include <ecc/ecpoint.h>
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*_ecdsa_dohash)(unsigned char *hash, unsigned char *msg, size_t sz);

typedef struct {
    _ecdsa_dohash   dohash;
    size_t          hsz;
} _mpECDSAHashfunc_t;

typedef _mpECDSAHashfunc_t mpECDSAHashfunc_t[1];
typedef _mpECDSAHashfunc_t *mpECDSAHashfunc_ptr;

void mpECDSAHashfunc_init(mpECDSAHashfunc_t H);
void mpECDSAHashfunc_set(mpECDSAHashfunc_t H, mpECDSAHashfunc_ptr Hp);
void mpECDSAHashfunc_clear(mpECDSAHashfunc_t H);

typedef struct {
    mpECurve_ptr cvp;
    mpECP_t cv_G;
    size_t nsz;
    mpECDSAHashfunc_t H;
} _mpECDSASignatureScheme_t;

typedef _mpECDSASignatureScheme_t mpECDSASignatureScheme_t[1];
typedef _mpECDSASignatureScheme_t *mpECDSASignatureScheme_ptr;

int mpECDSASignatureScheme_init(mpECDSASignatureScheme_t sscheme, mpECurve_t cv, mpECDSAHashfunc_t H);
void mpECDSASignatureScheme_clear(mpECDSASignatureScheme_t sscheme);

typedef struct {
    mpFp_t  r;
    mpFp_t  s;
    mpECDSASignatureScheme_ptr sscheme;
} _mpECDSASignature_t;

typedef _mpECDSASignature_t mpECDSASignature_t[1];
typedef _mpECDSASignature_t *mpECDSASignature_ptr;

int mpECDSASignature_init_Sign(mpECDSASignature_t sig, mpECDSASignatureScheme_t sscheme, mpFp_t sK, unsigned char *msg, size_t sz);
int mpECDSASignature_init_import_bytes(mpECDSASignature_t sig, mpECDSASignatureScheme_t sscheme, unsigned char *bsig, size_t sz);
int mpECDSASignature_init_import_str(mpECDSASignature_t sig, mpECDSASignatureScheme_t sscheme, char *ssig);

int mpECDSASignature_verify_cmp(mpECDSASignature_t sig, mpECP_t pK, unsigned char *msg, size_t sz);
unsigned char *mpECDSASignature_export_bytes(mpECDSASignature_t sig, size_t *sz);
char *mpECDSASignature_export_str(mpECDSASignature_t sig);

void mpECDSASignature_clear(mpECDSASignature_t sig);

#ifdef __cplusplus
}
#endif

#endif // _EC_ECDSA_H_INCLUDED_
