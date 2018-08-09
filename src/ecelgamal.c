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
#include <ecc/ecelgamal.h>
#include <ecc/ecpoint.h>
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mpECElgamal_init_encrypt(mpECElgamalCiphertext_t ctxt, mpECP_t pK, mpECP_t ptxt) {
    mpFp_t k;
    if (mpECurve_cmp(pK->cvp, ptxt->cvp) != 0) {
        return -1;
    }
    mpECP_init(ctxt->C, pK->cvp);
    mpECP_init(ctxt->D, pK->cvp);
    mpFp_init(k, pK->cvp->n);

    do {
        mpFp_urandom(k, pK->cvp->n);
    } while (mpFp_cmp_ui(k, 0) == 0);

    mpECP_set_mpz(ctxt->C, pK->cvp->G[0], pK->cvp->G[1], pK->cvp);
    mpECP_scalar_mul(ctxt->C, ctxt->C, k);

    mpECP_scalar_mul(ctxt->D, pK, k);
    mpECP_add(ctxt->D, ctxt->D, ptxt);

    mpFp_clear(k);
    return 0;
}

int mpECElgamal_init_decrypt(mpECP_t ptxt, mpFp_t sK, mpECElgamalCiphertext_t ctxt) {
    assert(ctxt->C->cvp == ctxt->D->cvp);
    if (mpz_cmp(sK->fp->p, ctxt->C->cvp->n) != 0) {
        return -1;
    }
    mpECP_init(ptxt, ctxt->C->cvp);
    mpECP_scalar_mul(ptxt, ctxt->C, sK);
    mpECP_sub(ptxt, ctxt->D, ptxt);
    return 0;
}

void mpECElgamal_clear(mpECElgamalCiphertext_t ctxt) {
    mpECP_clear(ctxt->D);
    mpECP_clear(ctxt->C);
    return;
}
