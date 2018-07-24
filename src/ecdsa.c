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
#include <ecc/ecpoint.h>
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void mpECDSAHashfunc_init(mpECDSAHashfunc_t H) {
    return;
}

void mpECDSAHashfunc_set(mpECDSAHashfunc_t H, mpECDSAHashfunc_ptr Hp) {
    H->dohash = Hp->dohash;
    H->hsz = Hp->hsz;
    return;
}

void mpECDSAHashfunc_clear(mpECDSAHashfunc_t H) {
    H->dohash = NULL;
    H->hsz = 0;
    return;
}

int mpECDSASignatureScheme_init(mpECDSASignatureScheme_t sscheme, mpECurve_t cv, mpECDSAHashfunc_t H) {
    size_t nsz;

    // require |H| >= |n| (as ECDSA is defined for the leftmost |n| bits of H)
    nsz = (mpz_sizeinbase(cv->n, 2) + 7) >> 3;
    if (nsz > H->hsz) return -1;
    sscheme->nsz = nsz;
    sscheme->cvp = (mpECurve_ptr)cv;
    mpECDSAHashfunc_init(sscheme->H);
    mpECDSAHashfunc_set(sscheme->H, H);
    mpECP_init(sscheme->cv_G, cv);
    mpECP_set_mpz(sscheme->cv_G, cv->G[0], cv->G[1], cv);
    mpECP_scalar_base_mul_setup(sscheme->cv_G);
    return 0;
}

void mpECDSASignatureScheme_clear(mpECDSASignatureScheme_t sscheme) {
    mpECP_clear(sscheme->cv_G);
    mpECDSAHashfunc_clear(sscheme->H);
    sscheme->cvp = NULL;
    sscheme->nsz = 0;
    return;
}

int mpECDSASignature_init_Sign(mpECDSASignature_t sig, mpECDSASignatureScheme_t sscheme, mpFp_t sK, unsigned char *msg, size_t sz) {
    unsigned char *hash;
    mpFp_t k_n;
    mpFp_t r_n;
    mpFp_t s_n;
    mpFp_t e_n;
    mpz_t e;
    mpz_t r;
    mpECP_t R;
    int status;

    if (sscheme == NULL) return -1;

    if ((msg == NULL) || (sz == 0)) return -1;

    hash = (unsigned char *)malloc(sscheme->H->hsz * sizeof(char));
    assert(hash != NULL);

    mpFp_init(k_n, sscheme->cvp->n);
    mpFp_init(r_n, sscheme->cvp->n);
    mpFp_init(s_n, sscheme->cvp->n);
    mpFp_init(e_n, sscheme->cvp->n);
    mpz_init(r);
    mpz_init(e);
    mpECP_init(R, sscheme->cvp);

    sscheme->H->dohash(hash, msg, sz);
    mpz_import(e, sscheme->nsz, 1, 1, 1, 0, hash);
    mpFp_set_mpz(e_n, e, sscheme->cvp->n);
new_random:
    mpFp_urandom(k_n, sscheme->cvp->n);
    if (__GMP_UNLIKELY(mpFp_cmp_ui(k_n, 0) == 0)) goto new_random;

    mpECP_scalar_base_mul(R, sscheme->cv_G, k_n);
    mpz_set_mpECP_affine_x(r, R);
    if (__GMP_UNLIKELY(mpz_cmp_ui(r, 0) == 0)) goto new_random;

    mpFp_set_mpz(r_n, r, sscheme->cvp->n);
    mpFp_mul(s_n, r_n, sK);
    mpFp_add(s_n, s_n, e_n);
    status = mpFp_inv(e_n, k_n);
    if (status != 0) goto new_random;
    mpFp_mul(s_n, s_n, e_n);
    if (__GMP_UNLIKELY(mpFp_cmp_ui(s_n, 0) == 0)) goto new_random;

    mpFp_init(sig->r, sscheme->cvp->n);
    mpFp_init(sig->s, sscheme->cvp->n);

    mpFp_set(sig->r, r_n);
    mpFp_set(sig->s, s_n);
    sig->sscheme = sscheme;

    mpECP_clear(R);
    mpz_clear(e);
    mpz_clear(r);
    mpFp_clear(e_n);
    mpFp_clear(s_n);
    mpFp_clear(r_n);
    mpFp_clear(k_n);
    return 0;
}

int mpECDSASignature_verify_cmp(mpECDSASignature_t sig, mpECP_t pK, unsigned char *msg, size_t sz) {
    unsigned char *hash;
    mpFp_t w;
    mpFp_t u1;
    mpFp_t u2;
    mpFp_t e_n;
    mpFp_t p_n;
    mpz_t e;
    mpECP_t P;
    mpECP_t Pq;
    int status;

    if (sz == 0) return -1;

    if (sig == NULL) return -1;

    // validate signature input, presume public key was validated at import
    if ((mpz_cmp(sig->r->fp->p, sig->sscheme->cvp->n) != 0) ||
        (mpz_cmp(sig->s->fp->p, sig->sscheme->cvp->n) != 0)) {
        return -1;
    }

    if ((mpFp_cmp_ui(sig->r, 0) == 0) || (mpFp_cmp_ui(sig->s, 0) == 0)) {
        return -1;
    }
    
    // require |H| >= |n| (as ECDSA is defined for the leftmost |n| bits of H)
    assert (sig->sscheme->H->hsz >= sig->sscheme->nsz);
    hash = (unsigned char *)malloc(sig->sscheme->H->hsz * sizeof(char));

    mpz_init(e);
    mpFp_init_fp(p_n, sig->sscheme->cvp->fp);
    mpFp_init(e_n, sig->sscheme->cvp->n);
    mpFp_init(w, sig->sscheme->cvp->n);
    mpFp_init(u1, sig->sscheme->cvp->n);
    mpFp_init(u2, sig->sscheme->cvp->n);
    mpECP_init(P, sig->sscheme->cvp);
    mpECP_init(Pq, sig->sscheme->cvp);

    sig->sscheme->H->dohash(hash, msg, sz);
    mpz_import (e, sig->sscheme->nsz, 1, 1, 1, 0, hash);
    mpFp_set_mpz(e_n, e, sig->sscheme->cvp->n);
    mpFp_inv(w, sig->s);
    mpFp_mul(u1, e_n, w);
    mpFp_mul(u2, sig->r, w);
    mpECP_scalar_base_mul(P, sig->sscheme->cv_G, u1);
    mpECP_scalar_mul(Pq, pK, u2);
    mpECP_add(P, P, Pq);
    mpz_set_mpECP_affine_x(e, P);
    mpFp_set_mpz(p_n, e, sig->sscheme->cvp->n);

    status = mpFp_cmp(p_n, sig->r);

    mpECP_clear(Pq);
    mpECP_clear(P);
    mpFp_clear(u2);
    mpFp_clear(u1);
    mpFp_clear(w);
    mpFp_clear(e_n);
    mpFp_clear(p_n);
    mpz_clear(e);

    return status;
}

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

unsigned char *mpECDSASignature_export_bytes(mpECDSASignature_t sig, size_t *sz) {
    mpz_t r;
    mpz_t s;
    unsigned char *buffer;
    unsigned char *br;
    unsigned char *bs;
    size_t bsz;

    mpz_init(r);
    mpz_init(s);
    
    buffer = (unsigned char *)malloc((sig->sscheme->nsz << 1) * sizeof(char));
    br = buffer;
    bs = buffer + sig->sscheme->nsz;
    assert(buffer != NULL);

    mpz_set_mpFp(r, sig->r);
    mpz_set_mpFp(s, sig->s);

    mpz_export(br, &bsz, 1, sizeof(char), 0, 0, r);
    assert(bsz <= sig->sscheme->nsz);
    if (bsz < sig->sscheme->nsz) {
        _shift_right_and_zero_pad(br, sig->sscheme->nsz, sig->sscheme->nsz - bsz);
    }

    mpz_export(bs, &bsz, 1, sizeof(char), 0, 0, s);
    assert(bsz <= sig->sscheme->nsz);
    if (bsz < sig->sscheme->nsz) {
        _shift_right_and_zero_pad(bs, sig->sscheme->nsz, sig->sscheme->nsz - bsz);
    }

    mpz_clear(s);
    mpz_clear(r);

    *sz = (sig->sscheme->nsz << 1);
    return buffer;
}

int mpECDSASignature_init_import_bytes(mpECDSASignature_t sig, mpECDSASignatureScheme_t sscheme, unsigned char *bsig, size_t sz) {
    mpz_t r;
    mpz_t s;
    unsigned char *br;
    unsigned char *bs;
    
    if (sig == NULL) return -1;

    if (sscheme == NULL) return -1;

    if (sz != (sscheme->nsz << 1)) {
        return -1;
    }

    br = bsig;
    bs = bsig + sscheme->nsz;

    mpz_init(r);
    mpz_init(s);

    mpz_import (r, sscheme->nsz, 1, sizeof(char), 0, 0, br);
    mpz_import (s, sscheme->nsz, 1, sizeof(char), 0, 0, bs);

    if ((mpz_cmp_ui(r, 0) <= 0) || (mpz_cmp_ui(s, 0) <= 0) ||
        (mpz_cmp(r, sscheme->cvp->n) >= 0) || (mpz_cmp(s, sscheme->cvp->n) >= 0)) {
        mpz_clear(s);
        mpz_clear(r);
        return -1;
    }

    mpFp_init(sig->r, sscheme->cvp->n);
    mpFp_set_mpz(sig->r, r, sscheme->cvp->n);
    mpFp_init(sig->s, sscheme->cvp->n);
    mpFp_set_mpz(sig->s, s, sscheme->cvp->n);
    sig->sscheme = sscheme;
    mpz_clear(s);
    mpz_clear(r);
    return 0;
}

void mpECDSASignature_clear(mpECDSASignature_t sig) {
    mpFp_clear(sig->r);
    mpFp_clear(sig->s);
    sig->sscheme = NULL;
    return;
}

