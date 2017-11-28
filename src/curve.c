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

#include <assert.h>
#include <curve.h>
#include <field.h>
#include <gmp.h>

void mpECurve_init(mpECurve_t c) {
    mpz_init(c->p);
    mpz_init(c->a);
    mpz_init(c->b);
    mpz_init(c->n);
    mpz_init(c->h);
    mpz_init(c->G[0]);
    mpz_init(c->G[1]);
    c->bits = 0;
    return;
}

void mpECurve_clear(mpECurve_t c) {
    mpz_clear(c->p);
    mpz_clear(c->a);
    mpz_clear(c->b);
    mpz_clear(c->n);
    mpz_clear(c->h);
    mpz_clear(c->G[0]);
    mpz_clear(c->G[1]);
    return;
}

void mpECurve_set(mpECurve_t rop, mpECurve_t op){
    mpz_set(rop->p, op->p);
    mpz_set(rop->a, op->a);
    mpz_set(rop->b, op->b);
    mpz_set(rop->n, op->n);
    mpz_set(rop->h, op->h);
    mpz_set(rop->G[0], op->G[0]);
    mpz_set(rop->G[1], op->G[1]);
    rop->bits = op->bits;
    return;
}

void mpECurve_set_str(mpECurve_t c, char *p, char *a, char *b, char *n,
char *h, char *Gx, char *Gy, unsigned int bits){
    mpz_set_str(c->p, p, 0);
    mpz_set_str(c->a, a, 0);
    mpz_set_str(c->b, b, 0);
    mpz_set_str(c->n, n, 0);
    mpz_set_str(c->h, h, 0);
    mpz_set_str(c->G[0], Gx, 0);
    mpz_set_str(c->G[1], Gy, 0);
    c->bits = bits;
    assert(mpECurve_point_check(c, c->G[0], c->G[1]));
    return;
}

int mpECurve_point_check(mpECurve_t c, mpz_t Px, mpz_t Py) {
    mpFp_t x, y, r, l, a, b;
    int on_curve;
    mpFp_init(x);
    mpFp_init(y);
    mpFp_init(r);
    mpFp_init(l);
    mpFp_init(a);
    mpFp_init(b);

    mpFp_set_mpz(x, Px, c->p);
    mpFp_set_mpz(y, Py, c->p);
    mpFp_set_mpz(a, c->a, c->p);
    mpFp_set_mpz(b, c->b, c->p);
    mpFp_pow_ui(r, x, 3);
    mpFp_mul(l, a, x);
    mpFp_add(r, r, l);
    mpFp_add(r, r, b);
    mpFp_pow_ui(l, y, 2);
    
    on_curve = (mpFp_cmp(l, r) == 0);
    mpFp_clear(b);
    mpFp_clear(a);
    mpFp_clear(l);
    mpFp_clear(r);
    mpFp_clear(y);
    mpFp_clear(x);
    return on_curve;
}

int mpECurve_cmp(mpECurve_t op1, mpECurve_t op2) {
    int r;
    r = mpz_cmp(op1->p, op2->p) ; if (r != 0) return r;
    r = mpz_cmp(op1->a, op2->a) ; if (r != 0) return r;
    r = mpz_cmp(op1->b, op2->b) ; if (r != 0) return r;
    r = mpz_cmp(op1->n, op2->n) ; if (r != 0) return r;
    r = mpz_cmp(op1->h, op2->h) ; if (r != 0) return r;
    r = mpz_cmp(op1->G[0], op2->G[0]) ; if (r != 0) return r;
    r = mpz_cmp(op1->G[1], op2->G[1]) ; if (r != 0) return r;
    return 0;
}
