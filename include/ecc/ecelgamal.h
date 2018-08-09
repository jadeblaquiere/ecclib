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

#ifndef _EC_ECELGAMAL_H_INCLUDED_
#define _EC_ECELGAMAL_H_INCLUDED_

#include <ecc/ecpoint.h>
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

// algorithm based on N. Koblitz "Elliptic Curve Cryptosystems", Sec 4,(2)
// http://www.ams.org/journals/mcom/1987-48-177/S0025-5718-1987-0866109-5/S0025-5718-1987-0866109-5.pdf

// EC Elgamal ciphertext is a tuple (C,D) of EC points
// Enc : C = kG, D = ptxt + k * pK; where pK = sK * G
// Dec : ptxt = D - sK * C

typedef struct {
    mpECP_t C;
    mpECP_t D;
} _mpECElgamalCiphertext_t;

typedef _mpECElgamalCiphertext_t mpECElgamalCiphertext_t[1];
typedef _mpECElgamalCiphertext_t *mpECElgamalCiphertext_ptr;

int mpECElgamal_init_encrypt(mpECElgamalCiphertext_t ctxt, mpECP_t pK, mpECP_t ptxt);
int mpECElgamal_init_decrypt(mpECP_t ptxt, mpFp_t sK, mpECElgamalCiphertext_t ctxt);
void mpECElgamal_clear(mpECElgamalCiphertext_t ctxt);

#ifdef __cplusplus
}
#endif

#endif // _EC_ECELGAMAL_H_INCLUDED_
