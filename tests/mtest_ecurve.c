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
#include <ecc/ecurve.h>
#include <ecc/safememory.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int i, status;
    mpECurve_t a;
    char **clist;
    mpECurve_init(a);

    // attach gmp realloc/free functions to clear memory before free
    _enable_gmp_safe_clean();

    clist = _mpECurve_list_standard_curves();
    i = 0;
    while(clist[i] != NULL) {
        int error;
        mpECurve_t b;
        mpECurve_init(b);
        printf("TEST: mpECurve found curve %s\n", clist[i]);
        error = mpECurve_set_named(a,clist[i]);
        printf("testing mpz_import\n");
        assert(error == 0);
        switch(a->type) {
            case EQTypeShortWeierstrass:
                    status = mpECurve_set_mpz_ws(b, a->fp->p, a->coeff.ws.a->i,
                        a->coeff.ws.b->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            case EQTypeEdwards:
                    status = mpECurve_set_mpz_ed(b, a->fp->p, a->coeff.ed.c->i,
                        a->coeff.ed.d->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            case EQTypeMontgomery:
                    status = mpECurve_set_mpz_mo(b, a->fp->p, a->coeff.mo.B->i,
                        a->coeff.mo.A->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            case EQTypeTwistedEdwards:
                    status = mpECurve_set_mpz_te(b, a->fp->p, a->coeff.te.a->i,
                        a->coeff.te.d->i, a->n, a->h, a->G[0], a->G[1],
                        a->bits);
                    assert(status == 0);
                break;
            default:
                assert(0);
        }
        assert(mpECurve_cmp(a, b) == 0);
        assert(mpECurve_point_check(b, a->G[0], a->G[1]) == 1);
        free(clist[i]);
        mpECurve_clear(b);
        i += 1;
    }
    free(clist);

    mpECurve_clear(a);
    
    return 0;
}
