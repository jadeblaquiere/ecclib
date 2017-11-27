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

#ifdef __cplusplus
}
#endif

#endif // _EC_FIELD_H_INCLUDED_
