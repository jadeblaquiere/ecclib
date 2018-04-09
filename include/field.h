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

#ifndef _EC_FIELD_H_INCLUDED_
#define _EC_FIELD_H_INCLUDED_

#define _EC_FIELD_ASSUME_FIELD_EQUAL
//#define _EC_FIELD_H_INLINE_MATH

#include <gmp.h>
#include <assert.h>
#include <mpzmod.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Implementation of finite (prime) field math following GNU GMP sytle */ 

typedef struct {
    mpz_t       p;      // p defines field (mod p), assumed prime!
    mpz_t       pc;     // pc is complement of p in F(2**(limbsize*limbs))
    mp_size_t   psize;
    mp_size_t   p2size;
} _mpFp_field_struct;

typedef _mpFp_field_struct mpFp_field[1];
typedef _mpFp_field_struct *mpFp_field_ptr;

mpFp_field_ptr _mpFp_field_lookup(mpz_t p);

typedef struct {
    mpz_t           i;
    mpFp_field_ptr  fp;
} _mpFp_struct;

typedef _mpFp_struct mpFp_t[1];
typedef _mpFp_struct *mpFp_ptr;

/* swap (and conditional swap) */

void mpFp_swap(mpFp_t rop, mpFp_t op);
void mpFp_cswap(mpFp_t rop, mpFp_t op, int swap);

/* basic arithmetic */

void mpFp_init(mpFp_t i, mpz_t p);
void mpFp_init_fp(mpFp_t i, mpFp_field_ptr fp);
void mpFp_clear(mpFp_t i);

/* assignment */

void mpFp_set(mpFp_t rop, mpFp_t op);
void mpFp_set_mpz(mpFp_t rop, mpz_t i, mpz_t p);
void mpFp_set_mpz_fp(mpFp_t rop, mpz_t i, mpFp_field_ptr fp);
void mpFp_set_ui(mpFp_t rop, unsigned long i, mpz_t p);
void mpFp_set_ui_fp(mpFp_t rop, unsigned long i, mpFp_field_ptr fp);

void mpz_set_mpFp(mpz_t rop, mpFp_t op);

/* basic arith */

void mpFp_add(mpFp_t rop, mpFp_t op1, mpFp_t op2);
void mpFp_add_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_sub(mpFp_t rop, mpFp_t op1, mpFp_t op2);
void mpFp_sub_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_mul(mpFp_t rop, mpFp_t op1, mpFp_t op2);
void mpFp_mul_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_sqr(mpFp_t rop, mpFp_t op1);

//void mpFp_pow(mpFp_t rop, mpFp_t op1, mpz_t op2);
void mpFp_pow_ui(mpFp_t rop, mpFp_t op1, unsigned long op2);

void mpFp_neg(mpFp_t rop, mpFp_t op);

/* modular inversion */ 

// return nonzero on error (NOTE: return behavior opposite of mpz_invert)
int mpFp_inv(mpFp_t rop, mpFp_t op);

/* comparison */

int mpFp_cmp(mpFp_t op1, mpFp_t op2);
int mpFp_cmp_ui(mpFp_t op1, unsigned long op2);
int mpFp_cmp_mpz(mpFp_t op1, mpz_t op2);

/* modular square root - return nonzero if not quadratic residue */ 

int  mpFp_sqrt(mpFp_t rop, mpFp_t op);

/* bit operations */ 

int  mpFp_tstbit(mpFp_t op, int bit);

/* random */

void mpFp_urandom(mpFp_t rop, mpz_t p);

#ifdef __cplusplus
}
#endif

#endif // _EC_FIELD_H_INCLUDED_
