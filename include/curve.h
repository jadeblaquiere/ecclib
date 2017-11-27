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

#ifndef _EC_CURVE_H_INCLUDED_
#define _EC_CURVE_H_INCLUDED_

#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

// Implementation of Elliptic Curve math following GNU GMP sytle
// internal representation supports projective (e.g. Jacobian) coords
// Curve is defined as y**2 = x**3 + ax + b

typedef struct {
    mpz_t p; // prime field of curve
    mpz_t a; // coefficient of equation
    mpz_t b; // coefficient of equation
    mpz_t n; // order of Generator point of curve
    mpz_t h; // cofactor of curve
    mpz_t G[2]; // x,y coordinates of Generator of EC Group
    unsigned int bits; // bit size of curve, i.e. ceil(log2(p))
} _mpECurve_t;

typedef _mpECurve_t mpECurve_t[1];

void mpECurve_init(mpECurve_t c);
void mpECurve_clear(mpECurve_t c);

void mpECurve_set_str(mpECurve_t c, char *p, char *a, char *b, char *n,
                      char *h, char *Gx, char *Gy, unsigned int bits);

int mpECurve_point_check(mpECurve_t c, mpz_t Px, mpz_t Py);

int mpECurve_cmp(mpECurve_t op1, mpECurve_t op2);

// standard curves

#ifdef __cplusplus
}
#endif

#endif // _EC_CURVE_H_INCLUDED_
