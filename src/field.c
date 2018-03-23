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
#include <field.h>
#include <gmp.h>
#include <mpzurandom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SZ    (200000)
// _MPFP_MAX_LIMBS would be a 2048-bit integer in most cases (32*64)
#define _MPFP_MAX_LIMBS   (32)

#if 1
#define PARANOID_ASSERT(X)  assert((X))
#else
#define PARANOID_ASSERT(X)
#endif

void mpFp_field_init(mpFp_field field) {
    mpz_init(field->p);
    mpz_init(field->pc);
    return;
}

void mpFp_field_clear(mpFp_field field) {
    mpz_clear(field->p);
    mpz_clear(field->pc);
    return;
}

void mpFp_field_set_mpz(mpFp_field field, mpz_t p) {
    int i;
    field->psize = p->_mp_size;
    field->p2size = field->psize * 2 ;
    mpz_set(field->p, p);
    mpz_realloc(field->p, field->p2size);
    mpz_realloc(field->pc, field->p2size);
    assert(field->p->_mp_size == field->psize);

    field->pc->_mp_size = field->psize + 1;
    for (i = 0; i < field->psize; i++) {
        field->pc->_mp_d[i] = 0;
    }
    field->pc->_mp_d[field->psize] = 1;
    mpz_sub(field->pc, field->pc, field->p);

    //zero pad pc to left (out to psize)
    for (i = field->pc->_mp_size; i < field->psize; i++) {
        field->pc->_mp_d[i] = 0;
    }
    return;
}

typedef struct __mpFp_field_list_t {
    mpFp_field_ptr fp;
    struct __mpFp_field_list_t *next;
} _mpFp_field_list_t;

static _mpFp_field_list_t *_static_field_list = NULL;

mpFp_field_ptr _mpFp_field_lookup(mpz_t p) {
    _mpFp_field_list_t **l;
    _mpFp_field_list_t *l_this;

    l = &_static_field_list;
    while (*l != NULL) {
        l_this = *l;
        if (mpz_cmp(l_this->fp->p, p) == 0) {
            return l_this->fp;
        }
        l = &(l_this->next);
    }
    l_this = (_mpFp_field_list_t *)malloc(sizeof(_mpFp_field_list_t));
    assert(l_this != NULL);
    l_this->fp = (mpFp_field_ptr)malloc(sizeof(_mpFp_field_struct));
    assert(l_this->fp != NULL);
    *l = l_this;
    mpFp_field_init(l_this->fp);
    mpFp_field_set_mpz(l_this->fp, p);
    l_this->next = NULL;
    //gmp_printf("created field Fp: p = 0x%ZX\n", p);
    return l_this->fp;
}

static inline void mpFp_realloc(mpFp_t c) {
    if (__GMP_UNLIKELY(c->i->_mp_alloc < c->fp->p2size)) {
        mpz_realloc(c->i, c->fp->p2size);
    }
}

void mpFp_init(mpFp_t c, mpz_t p) {
    mpFp_field_ptr fp;
    fp = _mpFp_field_lookup(p);
    c->fp = fp;
    assert(fp != NULL);
    mpz_init2(c->i,fp->p2size*sizeof(c->i->_mp_d[0]));
    ///mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);
    return;
}

void mpFp_init_fp(mpFp_t c, mpFp_field_ptr fp) {
    c->fp = fp;
    assert(fp != NULL);
    mpz_init2(c->i,fp->p2size*sizeof(c->i->_mp_d[0]));
    ///mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);
    return;
}

void mpFp_clear(mpFp_t a) {
    mpz_clear(a->i);
    a->fp = NULL;
}

void mpFp_set(mpFp_t c, mpFp_t a) {
    int i;
    mpFp_field_ptr fp;
    fp = a->fp;
    c->fp = fp;
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);
    assert(a->i->_mp_size == fp->psize);
    c->i->_mp_size = fp->psize;
    for (i = 0; i < fp->psize; i++) {
        c->i->_mp_d[i] = a->i->_mp_d[i];
    }
}

void mpFp_set_mpz_fp(mpFp_t c, mpz_t a, mpFp_field_ptr fp) {
    mp_size_t i;
    c->fp = fp;
    assert(fp != NULL);

    mpz_set(c->i, a);
    mpz_mod(c->i, c->i, fp->p);
    assert (c->i->_mp_size <= fp->psize);
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);
    for (i = c->i->_mp_size; i < fp->psize; i++) {
        c->i->_mp_d[i] = 0;
    }
    c->i->_mp_size = fp->psize;
    return;
}

void mpFp_set_mpz(mpFp_t c, mpz_t a, mpz_t p) {
    mpFp_field_ptr fp;
    fp = _mpFp_field_lookup(p);
    return mpFp_set_mpz_fp(c, a, fp);
}

void mpFp_set_ui_fp(mpFp_t c, unsigned long int a, mpFp_field_ptr fp) {
    mp_size_t i;
    c->fp = fp;
    assert(fp != NULL);

    mpz_set_ui(c->i, a);
    if (__GMP_UNLIKELY(c->i->_mp_size >= fp->psize)) {
        mpz_mod(c->i, c->i, fp->p);
    }
    assert (c->i->_mp_size <= fp->psize);
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);
    for (i = c->i->_mp_size; i < fp->psize; i++) {
        c->i->_mp_d[i] = 0;
    }
    c->i->_mp_size = fp->psize;
    return;
}

void mpFp_set_ui(mpFp_t c, unsigned long int a, mpz_t p) {
    mpFp_field_ptr fp;
    fp = _mpFp_field_lookup(p);
    return mpFp_set_ui_fp(c, a, fp);
}

int mpFp_cmp(mpFp_t a, mpFp_t b) {
    mpFp_field_ptr fp;
    PARANOID_ASSERT(a->fp == b->fp);
    fp = a->fp;

    return mpn_cmp(a->i->_mp_d, b->i->_mp_d, fp->psize);
}

int mpFp_cmp_ui(mpFp_t a, unsigned long b) {
    mpFp_field_ptr fp;
    mp_limb_t b_limb;
    int cmp;
    int i;
    fp = a->fp;
    b_limb = b;

    cmp = !(b_limb == a->i->_mp_d[0]);
    for (i = 1; i < fp->psize; i++) {
        cmp |= !(0 == a->i->_mp_d[i]);
    }
    return cmp;
}

void mpFp_neg(mpFp_t c, mpFp_t a) {
    mpFp_field_ptr fp;
    mp_limb_t borrow;
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);

    borrow = mpn_sub_n(c->i->_mp_d, fp->p->_mp_d, a->i->_mp_d, fp->psize);
    PARANOID_ASSERT(borrow == 0);

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_add(mpFp_t c, mpFp_t a, mpFp_t b) {
    mpFp_field_ptr fp;
    mp_limb_t carry, borrow;
    PARANOID_ASSERT(a->fp == b->fp);
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);

    carry = mpn_add_n(c->i->_mp_d, a->i->_mp_d, b->i->_mp_d, fp->psize);
    if ((carry != 0) || (mpn_cmp(c->i->_mp_d, fp->p->_mp_d, fp->psize) >= 0)) {
        borrow = mpn_sub_n(c->i->_mp_d, c->i->_mp_d, fp->p->_mp_d, fp->psize);
        PARANOID_ASSERT(borrow == carry);
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_add_ui(mpFp_t c, mpFp_t a, unsigned long int b) {
    mpFp_field_ptr fp;
    mp_limb_t carry, borrow;
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);

    // borrow/reuse "borrow" as input to add
    borrow = b;
    if (__GMP_UNLIKELY(fp->psize == 1)) {
        borrow = b % fp->p->_mp_d[0];
    }
    carry = mpn_add_1(c->i->_mp_d, a->i->_mp_d, fp->psize, borrow);
    if (__GMP_UNLIKELY((carry != 0) || (mpn_cmp(c->i->_mp_d, fp->p->_mp_d, fp->psize)) >= 0)) {
        borrow = mpn_sub_n(c->i->_mp_d, c->i->_mp_d, fp->p->_mp_d, fp->psize);
        PARANOID_ASSERT(borrow == carry);
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_sub(mpFp_t c, mpFp_t a, mpFp_t b) {
    mpFp_field_ptr fp;
    mp_limb_t carry, borrow;
    PARANOID_ASSERT(a->fp == b->fp);
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);

    borrow = mpn_sub_n(c->i->_mp_d, a->i->_mp_d, b->i->_mp_d, fp->psize);
    if (borrow != 0) {
        carry = mpn_add_n(c->i->_mp_d, fp->p->_mp_d, c->i->_mp_d, fp->psize);
        PARANOID_ASSERT(carry == 1);
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_sub_ui(mpFp_t c, mpFp_t a, unsigned long int b) {
    mpFp_field_ptr fp;
    mp_limb_t carry, borrow;
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;
    //mpz_realloc(c->i, fp->p2size);
    mpFp_realloc(c);

    // borrow/reuse "carry" as input to sub
    carry = b;
    if (__GMP_UNLIKELY(fp->psize == 1)) {
        carry = b % fp->p->_mp_d[0];
    }
    borrow = mpn_sub_1(c->i->_mp_d, a->i->_mp_d, fp->psize, carry);
    if (__GMP_UNLIKELY(borrow != 0)) {
        carry = mpn_add_n(c->i->_mp_d, fp->p->_mp_d, c->i->_mp_d, fp->psize);
        PARANOID_ASSERT(carry == 1);
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

int mpFp_inv(mpFp_t c, mpFp_t a) {
    int i, rstatus;
    mpz_t t;
    mpFp_field_ptr fp;
    mp_limb_t tl[_MPFP_MAX_LIMBS*2];
    fp = a->fp;
    PARANOID_ASSERT(a->fp == c->fp);
    PARANOID_ASSERT(a->i->_mp_size == fp->psize);
    PARANOID_ASSERT(fp->psize <= _MPFP_MAX_LIMBS);
    mpFp_realloc(c);
    t->_mp_d = tl;
    t->_mp_size = fp->psize;
    t->_mp_alloc = fp->p2size;

    for (i = 0; i < fp->psize; i++){
        tl[i] = a->i->_mp_d[i];
    }

    for (i = (fp->psize - 1); i >= 0; i-- ) {
        if (tl[i] != 0) {
            break;
        }
        t->_mp_size = i;
    }

    if (t->_mp_size == 0) return -1;

    // mpz_invert returns 0 on failure... reverse status
    rstatus = mpz_invert(c->i, t, fp->p);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return (rstatus == 0);
}

#if 1
void mpFp_mul(mpFp_t c, mpFp_t a, mpFp_t b) {
    int i;
    mpFp_field_ptr fp;
    mpz_t t;
    mp_limb_t tl[_MPFP_MAX_LIMBS*2];
    fp = a->fp;
    PARANOID_ASSERT(fp->psize <= _MPFP_MAX_LIMBS);
    PARANOID_ASSERT(a->fp == b->fp);
    PARANOID_ASSERT(a->fp == c->fp);
    mpFp_realloc(c);

    mpn_mul_n(tl, a->i->_mp_d, b->i->_mp_d, fp->psize);
    t->_mp_d = tl;
    t->_mp_size = fp->p2size;
    t->_mp_alloc = fp->p2size;
    // mpn_tdiv_qr(u->_mp_d, c->i->_mp_d, 0, t->_mp_d, fp->p2size, fp->p->_mp_d, fp->psize);
    mpz_mod(c->i, t, fp->p);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_mul_ui(mpFp_t c, mpFp_t a, unsigned long int b) {
    int i;
    mpFp_field_ptr fp;
    mpz_t t;
    mp_limb_t b_limb;
    mp_limb_t tl[_MPFP_MAX_LIMBS*2];
    fp = a->fp;
    PARANOID_ASSERT(fp->psize <= _MPFP_MAX_LIMBS);
    PARANOID_ASSERT(a->fp == c->fp);
    mpFp_realloc(c);

    b_limb = b;
    b_limb = mpn_mul_1(tl, a->i->_mp_d, fp->psize, b_limb);
    tl[fp->psize] = b_limb;
    t->_mp_d = tl;
    t->_mp_size = fp->psize + 1;
    t->_mp_alloc = fp->p2size;
    // mpn_tdiv_qr(u->_mp_d, c->i->_mp_d, 0, t->_mp_d, fp->p2size, fp->p->_mp_d, fp->psize);
    mpz_mod(c->i, t, fp->p);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}
#else
void mpFp_mul(mpFp_t c, mpFp_t a, mpFp_t b) {
    int i;
    mpFp_field_ptr fp;
    PARANOID_ASSERT(a->fp == b->fp);
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;

    mpz_mul(c->i, a->i, b->i);
    mpz_mod(c->i, c->i, fp->p);
    mpFp_realloc(c);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_mul_ui(mpFp_t c, mpFp_t a, unsigned long int b) {
    int i;
    mpFp_field_ptr fp;
    PARANOID_ASSERT(a->fp == c->fp);
    fp = a->fp;

    mpz_mul_ui(c->i, a->i, b);
    mpz_mod(c->i, c->i, fp->p);
    mpFp_realloc(c);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}
#endif

void mpFp_sqr(mpFp_t c, mpFp_t a) {
    int i;
    mpFp_field_ptr fp;
    mpz_t t;
    mp_limb_t tl[_MPFP_MAX_LIMBS*2];
    fp = a->fp;
    PARANOID_ASSERT(fp->psize <= _MPFP_MAX_LIMBS);
    PARANOID_ASSERT(a->fp == c->fp);
    mpFp_realloc(c);

    mpn_sqr(tl, a->i->_mp_d, fp->psize);
    t->_mp_d = tl;
    t->_mp_size = fp->p2size;
    t->_mp_alloc = fp->p2size;
    // mpn_tdiv_qr(u->_mp_d, c->i->_mp_d, 0, t->_mp_d, fp->p2size, fp->p->_mp_d, fp->psize);
    mpz_mod(c->i, t, fp->p);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_pow_ui(mpFp_t c, mpFp_t a, unsigned long int b) {
    int i;
    mpFp_field_ptr fp;
    fp = a->fp;
    PARANOID_ASSERT(a->fp == c->fp);
    mpFp_realloc(c);

    mpz_powm_ui(c->i, a->i, b, fp->p);
    if (__GMP_UNLIKELY(c->i->_mp_size < fp->psize)) {
        for (i = c->i->_mp_size; i < fp->psize; i++) {
            c->i->_mp_d[i] = 0;
        }
    }

    c->i->_mp_size = fp->psize;
    //c->fp = fp;
    return;
}

void mpFp_swap(mpFp_t a, mpFp_t b) {
    int i;
    mpFp_field_ptr fp;
    mp_limb_t   t;
    fp = a->fp;
    PARANOID_ASSERT(fp != NULL);
    PARANOID_ASSERT(a->fp == b->fp);

    for (i = 0; i < fp->psize; i++) {
        t = a->i->_mp_d[i];
        a->i->_mp_d[i] = b->i->_mp_d[i];
        b->i->_mp_d[i] = t;
    }
}

void mpFp_cswap(mpFp_t a, mpFp_t b, int swap) {
    mpFp_field_ptr fp;
    int i;
    mp_limb_t tl[2][_MPFP_MAX_LIMBS*2];
    fp = a->fp;
    PARANOID_ASSERT(fp != NULL);
    PARANOID_ASSERT(fp->psize <= _MPFP_MAX_LIMBS);
    PARANOID_ASSERT(a->fp == b->fp);
    swap = (swap != 0);
    assert(swap >= 0);
    assert(swap < 2);

    for (i = 0; i < fp->psize; i++) {
        tl[0][i] = a->i->_mp_d[i];
        tl[1][i] = b->i->_mp_d[i];
    }

    for (i = 0; i < fp->psize; i++) {
        a->i->_mp_d[i] = tl[0+swap][i];
        b->i->_mp_d[i] = tl[1-swap][i];
    }
    return;
}

void mpz_set_mpFp(mpz_t c, mpFp_t a) {
    mpFp_field_ptr fp;
    int i = 0;
    fp = a->fp;
    mpz_realloc(c, fp->p2size);
    assert (a->i->_mp_size == fp->psize);

    for (i = 0; i < fp->psize; i++) {
        c->_mp_d[i] = a->i->_mp_d[i];
    }
    c->_mp_size = fp->psize;
    for (i = fp->psize-1; i >= 0; i--) {
        if (c->_mp_d[i] != 0) {
            break;
        }
        c->_mp_size = i;
    }
    return;
}

void mpFp_urandom(mpFp_t a, mpz_t p) {
    mpz_t aa;
    mpz_init(aa);
    mpz_urandom(aa, p);
    mpFp_set_mpz(a, aa, p);
    mpz_clear(aa);
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
    int i, s;
    mpz_t t, q, opi, ropi;
    mpz_init(opi);
    mpz_set_mpFp(opi, op);
    // determine whether i is a quadratic residue (mod p)
    if (mpz_legendre(opi, op->fp->p) != 1) return -1;
    mpz_init(t);
    mpz_init(q);
    mpz_init(ropi);
    // tonelli shanks algorithm
    mpz_sub_ui(q, op->fp->p, 1);
    s = 0;
    assert(mpz_cmp_ui(q, 0) != 0);
    while (mpz_tstbit(q, 0) == 0) {
        mpz_tdiv_q_ui(q, q, 2);
        s += 1;
    }
    if (s == 1) {
        // p = 3 mod 4 case, sqrt by exponentiation
        mpz_add_ui(t, op->fp->p, 1);
        mpz_tdiv_q_ui(t, t, 4);
        rop->fp = op->fp;
        mpFp_realloc(rop);
        mpz_powm(rop->i, opi, t, op->fp->p);
    } else {
        int m, i;
        mpz_t z, c, r, b;
        mpz_init(z);
        mpz_init(c);
        mpz_init(r);
        mpz_init(b);
        mpz_set_ui(z, 2);
        while(mpz_legendre(z, op->fp->p) != -1) {
            mpz_add_ui(z, z, 1);
            assert (mpz_cmp(z, op->fp->p) < 0);
        }
        mpz_powm(c, z, q, op->fp->p);
        mpz_add_ui(t, q, 1);
        mpz_tdiv_q_ui(t, t, 2);
        mpz_powm(r, opi, t, op->fp->p);
        mpz_powm(t, opi, q, op->fp->p);
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
                mpz_powm_ui(z, z, 2, op->fp->p);
                //mpz_add_ui(i, i, 1); 
            }
            mpz_powm_ui(b, c, 1 << (m - i - 1), op->fp->p);
            mpz_mul(r, r, b);
            mpz_mod(r, r, op->fp->p);
            mpz_powm_ui(c, b, 2, op->fp->p);
            mpz_mul(t, t, c);
            mpz_mod(t, t, op->fp->p);
            //mpz_set(m, i);
            m = i;
        }
        rop->fp = op->fp;
        mpFp_realloc(rop);
        mpz_set(rop->i, r);
        mpz_clear(b);
        mpz_clear(r);
        mpz_clear(c);
        mpz_clear(z);
    }
    mpz_clear(q);
    mpz_clear(t);
    mpz_clear(opi);

    if (__GMP_UNLIKELY(rop->i->_mp_size < op->fp->psize)) {
        for (i = rop->i->_mp_size; i < op->fp->psize; i++) {
            rop->i->_mp_d[i] = 0;
        }
    }

    rop->i->_mp_size = op->fp->psize;
    return 0;
}

int  mpFp_tstbit(mpFp_t op, int bit) {
    return mpz_tstbit(op->i, bit);
}
