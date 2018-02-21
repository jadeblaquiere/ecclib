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
#include <mpzurandom.h>

void mpFp_init(mpFp_t i) {
    mpz_init(i->i);
    mpz_init(i->p);
    return;
}

void mpFp_clear(mpFp_t i) {
    mpz_clear(i->i);
    mpz_clear(i->p);
    return;
}

void mpFp_set(mpFp_t rop, mpFp_t op) {
    mpz_set(rop->p, op->p);
    mpz_set(rop->i, op->i);
    return;
}

void mpFp_set_mpz(mpFp_t rop, mpz_t i, mpz_t p) {
    mpz_set(rop->p, p);
    mpz_mod(rop->i, i, p);
    return;
}

void mpFp_set_ui(mpFp_t rop, unsigned long i, mpz_t p) {
    mpz_t t;
    mpz_init(t);
    
    mpz_set(rop->p, p);
    mpz_set_ui(t, i);
    mpz_mod(rop->i, t, p);
    //mpz_set_ui(rop->i, i);
    
    mpz_clear(t);
    return;
}

void mpz_set_mpFp(mpz_t rop, mpFp_t op) {
    mpz_set(rop, op->i);
    return;
}

void mpFp_swap(mpFp_t rop, mpFp_t op) {
    mpz_t t;
    mpz_init(t);

    mpz_set(t, rop->p);
    mpz_set(rop->p, op->p);
    mpz_set(op->p, t);

    mpz_set(t, rop->i);
    mpz_set(rop->i, op->i);
    mpz_set(op->i, t);

    mpz_clear(t);
    return;
}

/* constant time conditional swap algorithm */ 

void mpFp_cswap(mpFp_t rop, mpFp_t op, int swap) {
    mpz_t a, b;

    mpz_init(a);
    mpz_init(b);
    
    assert(mpz_cmp(rop->p, op->p) == 0);

    mpz_set(a, op->i);
    mpz_set(b, rop->i);
    if (swap != 0) {
        mpz_set(op->i, b);
        mpz_set(rop->i, a);
    } else {
        mpz_set(op->i, a);
        mpz_set(rop->i, b);
    }

    mpz_clear(b);
    mpz_clear(a);
    return;
}

/* basic arithmetic */

void mpFp_add(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
    mpz_t t;
    assert (mpz_cmp(op1->p, op2->p) == 0);
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_add(t, op1->i, op2->i);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_add_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_add_ui(t, op1->i, op2);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_sub(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
    mpz_t t;

    assert (mpz_cmp(op1->p, op2->p) == 0);
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_sub(t, op1->i, op2->i);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_sub_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_sub_ui(t, op1->i, op2);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_mul(mpFp_t rop, mpFp_t op1, mpFp_t op2) {
    mpz_t t;
    assert (mpz_cmp(op1->p, op2->p) == 0);
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_mul(t, op1->i, op2->i);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_mul_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_mul_ui(t, op1->i, op2);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_pow(mpFp_t rop, mpFp_t op1, mpz_t op2) {
    mpz_t t;
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_powm(t, op1->i, op2, op1->p);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_pow_ui(mpFp_t rop, mpFp_t op1, unsigned long op2) {
    mpz_t t;
    mpz_set(rop->p, op1->p);
    mpz_init(t);

    mpz_powm_ui(t, op1->i, op2, op1->p);
    mpz_mod(rop->i, t, op1->p);

    mpz_clear(t);
    return;
}

void mpFp_neg(mpFp_t rop, mpFp_t op) {
    mpz_set(rop->p, op->p);
    mpz_sub(rop->i, op->p, op->i);
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
    
    mpz_set(lastr, op->i);
    mpz_set(r, op->p);
    mpz_set_ui(x, 0);
    mpz_set_ui(lastx, 1);

    while (mpz_cmp_ui(r, 0) != 0) {
        mpz_set(y, r);
        mpz_tdiv_qr(q, r, lastr, y);
        mpz_set(lastr, y);
        mpz_set(y, x);
        mpz_mul(q, q, x);
        mpz_sub(x, lastx, q);
        mpz_set(lastx, y);
    }
    mpz_mod(rop->i, lastx, op->p);
    mpz_set(rop->p, op->p);
    mpz_clear(q);
    mpz_clear(y);
    mpz_clear(x);
    mpz_clear(r);
    mpz_clear(lastx);
    mpz_clear(lastr);
    return;
}

// python from RosettaCode
//def tonelli(n, p):
//    assert legendre(n, p) == 1, "not a square (mod p)"
//    q = p - 1
//    s = 0
//    while q % 2 == 0:
//        q //= 2
//        s += 1
//    if s == 1:
//        return pow(n, (p + 1) // 4, p)
//    for z in range(2, p):
//        if p - 1 == legendre(z, p):
//            break
//    c = pow(z, q, p)
//    r = pow(n, (q + 1) // 2, p)
//    t = pow(n, q, p)
//    m = s
//    t2 = 0
//    while (t - 1) % p != 0:
//        t2 = (t * t) % p
//        for i in range(1, m):
//            if (t2 - 1) % p == 0:
//                break
//            t2 = (t2 * t2) % p
//        b = pow(c, 1 << (m - i - 1), p)
//        r = (r * b) % p
//        c = (b * b) % p
//        t = (t * c) % p
//        m = i
//    return r

/* modular square root - return nonzero if not quadratic residue */ 

int mpFp_sqrt(mpFp_t rop, mpFp_t op) {
    int s;
    mpz_t t, q;
    // determine whether i is a quadratic residue (mod p)
    if (mpz_legendre(op->i, op->p) != 1) return -1;
    mpz_init(t);
    mpz_init(q);
    // tonelli shanks algorithm
    mpz_sub_ui(q, op->p, 1);
    s = 0;
    while (mpz_tstbit(q, 0) == 0) {
        mpz_tdiv_q_ui(q, q, 2);
        s += 1;
    }
    if (s == 1) {
        // p = 3 mod 4 case, sqrt by exponentiation
        mpz_add_ui(t, op->p, 1);
        mpz_tdiv_q_ui(t, t, 4);
        mpz_powm(rop->i, op->i, t, op->p);
        mpz_set(rop->p, op->p);
    } else {
        int m, i;
        mpz_t z, c, r, b;
        mpz_init(z);
        mpz_init(c);
        mpz_init(r);
        mpz_init(b);
        mpz_set_ui(z, 2);
        while(mpz_legendre(z, op->p) != -1) {
            mpz_add_ui(z, z, 1);
            assert (mpz_cmp(z, op->p) < 0);
        }
        mpz_powm(c, z, q, op->p);
        mpz_add_ui(t, q, 1);
        mpz_tdiv_q_ui(t, t, 2);
        mpz_powm(r, op->i, t, op->p);
        mpz_powm(t, op->i, q, op->p);
        m = s;
        while (1) {
            if (mpz_cmp_ui(t, 1) == 0) {
                break;
            }
            mpz_set(z, t);
            for (i = 0; i < (m-1); i++) {
                if (mpz_cmp_ui(z, 1) == 0) {
                    break;
                }
                mpz_powm_ui(z, z, 2, op->p);
                //mpz_add_ui(i, i, 1); 
            }
            mpz_powm_ui(b, c, 1 << (m - i - 1), op->p);
            mpz_mul(r, r, b);
            mpz_mod(r, r, op->p);
            mpz_powm_ui(c, b, 2, op->p);
            mpz_mul(t, t, c);
            mpz_mod(t, t, op->p);
            //mpz_set(m, i);
            m = i;
        }
        mpz_set(rop->i, r);
        mpz_set(rop->p, op->p);
        mpz_clear(b);
        mpz_clear(r);
        mpz_clear(c);
        mpz_clear(z);
    }
    mpz_clear(q);
    mpz_clear(t);
    return 0;
}

/* bit operations */ 

int  mpFp_tstbit(mpFp_t op, int bit) {
    return mpz_tstbit(op->i, bit);
}

/* comparison */

int mpFp_cmp(mpFp_t op1, mpFp_t op2) {
    assert (mpz_cmp(op1->p, op2->p) == 0);
    
    return mpz_cmp(op1->i, op2->i);
}

int mpFp_cmp_ui(mpFp_t op1, unsigned long op2) {
    return mpz_cmp_ui(op1->i, op2);
}

void mpFp_urandom(mpFp_t rop, mpz_t p) {
    mpz_set(rop->p, p);
    mpz_urandom(rop->i, p);
    return;
}
