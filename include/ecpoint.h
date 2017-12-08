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

#ifndef _EC_POINT_H_INCLUDED_
#define _EC_POINT_H_INCLUDED_

#include <gmp.h>
#include <field.h>
#include <ecurve.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Implementation of Elliptic Curve math loosely following GNU GMP sytle */ 
/* internal representation supports projective (e.g. Jacobian) coords */

typedef struct _p_mpECP_t {
    mpFp_t x;
    mpFp_t y;
    mpFp_t z;
    int is_neutral;
    mpECurve_t cv;
    int base_bits;
    struct _p_mpECP_t *base_pt;
} _mpECP_t;

typedef _mpECP_t mpECP_t[1];

void mpECP_init(mpECP_t pt);
void mpECP_clear(mpECP_t pt);

void mpECP_set(mpECP_t rpt, mpECP_t op);
void mpECP_set_mpz(mpECP_t rpt, mpz_t x, mpz_t y, mpECurve_t cv);
void mpECP_set_mpFp(mpECP_t rpt, mpFp_t x, mpFp_t y, mpECurve_t cv);
void mpECP_set_neutral(mpECP_t rpt, mpECurve_t cv);

void mpFp_set_mpECP_affine_x(mpFp_t x, mpECP_t pt);
void mpFp_set_mpECP_affine_y(mpFp_t y, mpECP_t pt);
void mpz_set_mpECP_affine_x(mpz_t x, mpECP_t pt);
void mpz_set_mpECP_affine_y(mpz_t y, mpECP_t pt);

int  mpECP_set_str(mpECP_t rpt, char *s, mpECurve_t cv);
int  mpECP_out_strlen(mpECP_t pt, int compress);
void mpECP_out_str(char *s, mpECP_t pt, int compress);

void mpECP_swap(mpECP_t rop, mpECP_t op);
void mpECP_cswap(mpECP_t rop, mpECP_t op, int swap);

void mpECP_add(mpECP_t rpt, mpECP_t pt1, mpECP_t pt2);
void mpECP_double(mpECP_t rpt, mpECP_t pt);
void mpECP_sub(mpECP_t rpt, mpECP_t pt1, mpECP_t pt2);

void mpECP_scalar_mul(mpECP_t rpt, mpECP_t pt, mpFp_t sc);
void mpECP_scalar_mul_mpz(mpECP_t rpt, mpECP_t pt, mpz_t sc);

void mpECP_neg(mpECP_t rpt, mpECP_t pt);
int  mpECP_cmp(mpECP_t pt1, mpECP_t pt2);

void mpECP_scalar_base_mul_setup(mpECP_t pt);
void mpECP_scalar_base_mul(mpECP_t rpt, mpECP_t pt, mpFp_t sc);
void mpECP_scalar_base_mul_mpz(mpECP_t rpt, mpECP_t pt, mpz_t sc);

void mpECP_urandom(mpECP_t rpt, mpECurve_t cv);

#ifdef __cplusplus
}
#endif

#endif // _EC_POINT_H_INCLUDED_
