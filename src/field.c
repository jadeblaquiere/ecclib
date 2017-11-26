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
#include <field.h>
#include <gmp.h>

void mpFp_init(mpFp_t i) {
    mpz_init(i.i);
    mpz_init(i.p);
    return;
}

void mpFp_clear(mpFp_t i) {
    mpz_clear(i.i);
    mpz_clear(i.p);
    return;
}

void mpFp_set(mpFp_t rop, mpFp_t op) {
    mpz_set(rop.p, op.p);
    mpz_set(rop.i, op.i);
    return;
}

void mpFp_set_mpz(mpFp_t rop, mpz_t i, mpz_t p) {
    mpz_set(rop.p, p);
    mpz_set(rop.i, i);
    return;
}

void mpFp_set_ui(mpFp_t rop, unsigned long i, mpz_t p) {
    mpz_set(rop.p, p);
    mpz_set_ui(rop.i, i);
    return;
}

void mpz_set_mpFp(mpz_t rop, mpFp_t op) {
    mpz_set(rop, op.i);
    return;
}

void mpFp_swap(mpFp_t rop, mpFp_t op) {
    mpz_t t;
    mpz_init(t);

    mpz_set(t, rop.p);
    mpz_set(rop.p, op.p);
    mpz_set(op.p, t);

    mpz_set(t, rop.i);
    mpz_set(rop.i, op.i);
    mpz_set(op.i, t);

    mpz_clear(t);
    return;
}

/* constant time conditional swap algorithm */ 

void mpFp_cswap(mpFp_t rop, mpFp_t op, int swap) {
    volatile unsigned long s, ns;
    mpz_t a, b, c, d;

    mpz_init(a);
    mpz_init(b);
    mpz_init(c);
    mpz_init(d);
    
    assert(mpz_cmp(rop.p, op.p) == 0);

    if (swap != 0) {
        s = 1;
        ns = 0;
    } else {
        s = 0;
        ns = 1;
    }
    
    mpz_mul_ui(a, op.i, s);
    mpz_mul_ui(b, rop.i, ns);
    
    mpz_mul_ui(c, op.i, ns);
    mpz_mul_ui(d, rop.i, s);
    
    mpz_add(rop.i, a, b);
    mpz_add(op.i, c, d);

    mpz_clear(d);
    mpz_clear(c);
    mpz_clear(b);
    mpz_clear(a);
    return;
}

/* basic arithmetic */

void mpFp_add(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
    mpz_t t;
    assert (mpz_cmp(op1.p, op2.p) == 0);
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_add(t, op1.i, op2.i);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_add_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_add_ui(t, op1.i, op2);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_sub(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
    mpz_t t;

    assert (mpz_cmp(op1.p, op2.p) == 0);
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_sub(t, op1.i, op2.i);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_sub_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_sub_ui(t, op1.i, op2);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_mul(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
    mpz_t t;
    assert (mpz_cmp(op1.p, op2.p) == 0);
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_mul(t, op1.i, op2.i);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_mul_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_mul_ui(t, op1.i, op2);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_pow(mpFp_t rop, mpFp_t op1, mpz_t op2) {
    mpz_t t;
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_powm(t, op1.i, op2, op1.p);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_pow_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop.p, op1.p);
    mpz_init(t);

    mpz_powm_ui(t, op1.i, op2, op1.p);
    mpz_mod(rop.i, t, op1.p);

    mpz_clear(t);
    return;
}

void mpFp_neg(mpFp_t rop, mpFp_t op) {
    mpz_set(rop.p, op.p);
    mpz_sub(rop.i, op.p, op.i);
    return;
}

// modular inversion 
// translated from python public domain version:
// def _modinv(a, m):
//     lastr, r, x, lastx = a, m, 0, 1
//     while r:
//         lastr, (q, r) = r, divmod(lastr, r)
//         x, lastx = lastx - q*x, x
//     return lastx % m

void mpFp_inv(mpFp_t rop, mpFp_t op) {
    mpz_t lastr, lastx, r, x, y, q;
    mpz_init(lastr);
    mpz_init(lastx);
    mpz_init(r);
    mpz_init(x);
    mpz_init(y);
    mpz_init(q);
    
    mpz_set(lastr, op.i);
    mpz_set(r, op.p);
    mpz_set_ui(x, 0);

    while (mpz_cmp_ui(r, 0) != 0) {
        mpz_set(y, r);
        mpz_tdiv_qr(q, r, lastr, y);
        mpz_set(lastr, y);
        mpz_set(y, x);
        mpz_set(x, lastx);
        mpz_mul(q, q, y);
        mpz_sub(x, x, q);
    }
    mpz_mod(rop.i, lastx, op.p);
    mpz_set(rop.p, op.p);
    mpz_clear(q);
    mpz_clear(y);
    mpz_clear(x);
    mpz_clear(r);
    mpz_clear(lastx);
    mpz_clear(lastr);
    return;
}

/* comparison */

int mpFp_cmp(mpFp_t op1, mpFp_t op2) {
    assert (mpz_cmp(op1.p, op2.p) == 0);
    
    return mpz_cmp(op1.i, op2.i);
}

int mpFp_cmp_ui(mpFp_t op1, unsigned long op2) {
    return mpz_cmp_ui(op1.i, op2);
}
