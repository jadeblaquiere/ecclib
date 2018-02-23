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

#ifndef _EC_FIELD_H_INCLUDED_
#define _EC_FIELD_H_INCLUDED_

#define _EC_FIELD_ASSUME_FIELD_EQUAL
#define _EC_FIELD_H_INLINE_MATH

#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Implementation of finite (prime) field math following GNU GMP sytle */ 

typedef struct {
    mpz_t i;
    mpz_t p;
} _mpFp_t;

typedef _mpFp_t mpFp_t[1];

void mpFp_init(mpFp_t i);
void mpFp_clear(mpFp_t i);

/* assignment */

void mpFp_set(mpFp_t rop, mpFp_t op);
void mpFp_set_mpz(mpFp_t rop, mpz_t i, mpz_t p);
void mpFp_set_ui(mpFp_t rop, unsigned long i, mpz_t p);

void mpz_set_mpFp(mpz_t rop, mpFp_t op);

/* swap (and conditional swap) */

void mpFp_swap(mpFp_t rop, mpFp_t op);
void mpFp_cswap(mpFp_t rop, mpFp_t op, int swap);

/* basic arithmetic */

#ifndef _EC_FIELD_H_INLINE_MATH

void mpFp_add(mpFp_t rop, mpFp_t op1, mpFp_t op2);
void mpFp_add_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_sub(mpFp_t rop, mpFp_t op1, mpFp_t op2);
void mpFp_sub_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_mul(mpFp_t rop, mpFp_t op1, mpFp_t op2);
void mpFp_mul_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_pow(mpFp_t rop, mpFp_t op1, mpz_t op2);
void mpFp_pow_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_neg(mpFp_t rop, mpFp_t op);

/* modular inversion */ 

void mpFp_inv(mpFp_t rop, mpFp_t op);

/* comparison */

int mpFp_cmp(mpFp_t op1, mpFp_t op2);
int mpFp_cmp_ui(mpFp_t op1, unsigned long op2);

#endif

/* modular square root - return nonzero if not quadratic residue */ 

int  mpFp_sqrt(mpFp_t rop, mpFp_t op);

/* bit operations */ 

int  mpFp_tstbit(mpFp_t op, int bit);

/* random */

void mpFp_urandom(mpFp_t rop, mpz_t p);

#ifdef _EC_FIELD_H_INLINE_MATH

static inline void mpFp_add(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
#ifndef _EC_FIELD_ASSUME_FIELD_EQUAL
    assert (mpz_cmp(op1->p, op2->p) == 0);
#endif
    mpz_set(rop->p, op1->p);

    mpz_add(rop->i, op1->i, op2->i);
    if (mpz_cmp(rop->i, rop->p) >= 0) {
        mpz_sub(rop->i, rop->i, rop->p);
    }
    return;
}

static inline void mpFp_add_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_set(rop->p, op1->p);

    mpz_add_ui(rop->i, op1->i, op2);
    if (mpz_cmp(rop->i, rop->p) >= 0) {
        mpz_sub(rop->i, rop->i, rop->p);
    }
    return;
}

static inline void mpFp_sub(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
#ifndef _EC_FIELD_ASSUME_FIELD_EQUAL
    assert (mpz_cmp(op1->p, op2->p) == 0);
#endif
    mpz_set(rop->p, op1->p);

    mpz_sub(rop->i, op1->i, op2->i);
    if (mpz_cmp_ui(rop->i, 0) < 0) {
        mpz_add(rop->i, rop->i, rop->p);
    }
    return;
}

static inline void mpFp_sub_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_set(rop->p, op1->p);

    mpz_sub_ui(rop->i, op1->i, op2);
    if (mpz_cmp_ui(rop->i, 0) < 0) {
        mpz_add(rop->i, rop->i, rop->p);
    }
    return;
}

static inline void mpFp_mul(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
#ifndef _EC_FIELD_ASSUME_FIELD_EQUAL
    assert (mpz_cmp(op1->p, op2->p) == 0);
#endif
    mpz_set(rop->p, op1->p);
    mpz_mul(rop->i, op1->i, op2->i);
    mpz_mod(rop->i, rop->i, op1->p);
    return;
}

static inline void mpFp_mul_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_set(rop->p, op1->p);
    mpz_mul_ui(rop->i, op1->i, op2);
    mpz_mod(rop->i, rop->i, op1->p);
    return;
}

static inline void mpFp_pow(mpFp_t rop, mpFp_t op1, mpz_t op2) {
    mpz_set(rop->p, op1->p);
    mpz_powm(rop->i, op1->i, op2, op1->p);
    return;
}

static inline void mpFp_pow_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_set(rop->p, op1->p);
    mpz_powm_ui(rop->i, op1->i, op2, op1->p);
    return;
}

static inline void mpFp_neg(mpFp_t rop, mpFp_t op) {
    mpz_set(rop->p, op->p);
    mpz_sub(rop->i, op->p, op->i);
    return;
}

static inline void mpFp_inv(mpFp_t rop, mpFp_t op) {
    mpz_set(rop->p, op->p);
    mpz_invert(rop->i, op->i, op->p);
}

static inline int mpFp_cmp(mpFp_t op1, mpFp_t op2) {
#ifndef _EC_FIELD_ASSUME_FIELD_EQUAL
    assert (mpz_cmp(op1->p, op2->p) == 0);
#endif

    return mpz_cmp(op1->i, op2->i);
}

static inline int mpFp_cmp_ui(mpFp_t op1, unsigned long op2) {
    return mpz_cmp_ui(op1->i, op2);
}

#endif // _EC_FIELD_H_INLINE_MATH

#ifdef __cplusplus
}
#endif

#endif // _EC_FIELD_H_INCLUDED_
