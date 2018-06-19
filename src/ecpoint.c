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
#include <ecc/ecpoint.h>
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _MPECP_BASE_BITS    (8)

// defining _MPECP_MPFP_NOMALLOC uses fixed structures for mpFp elements
// it is of course critical that *_realloc is never called, so _mp_alloc
// should be set to >= fp->p2size to avoid realloc being called from
// mpFp_realloc()
#define _MPECP_MPFP_NOMALLOC
#ifdef _MPECP_MPFP_NOMALLOC

// this definition should match the one in field.c
#define _MPFP_MAX_LIMBS   (32)

typedef mp_limb_t __local_limb_t[_MPFP_MAX_LIMBS];

#endif // #ifdef _MPECP_MPFP_NOMALLOC

static char *_hexlut = "0123456789ABCDEF";

static inline int _mpECP_n_base_pt_levels(mpECP_t pt) {
    return (pt->cvp->bits + pt->base_bits - 1) / pt->base_bits;
}

static inline int _mpECP_n_base_pt_level_size(mpECP_t pt) {
    return (1 << pt->base_bits);
}

static inline int _mpECP_n_base_pts(mpECP_t pt) {
    return _mpECP_n_base_pt_levels(pt) * _mpECP_n_base_pt_level_size(pt);
}

static void _mpECP_base_pts_cleanup(mpECP_t pt) {
    int i, npts;
    assert(pt->base_pt != NULL);
    npts = _mpECP_n_base_pts(pt);
    for (i = 0; i < npts; i++) {
        mpECP_clear(&(pt->base_pt[i]));
    }
    free(pt->base_pt);
    pt->base_pt = NULL;
    pt->base_bits = 0;
}

static int _known_curve_type(mpECurve_t cv) {
    return (cv->type == EQTypeShortWeierstrass) || 
        (cv->type == EQTypeEdwards) ||
        (cv->type == EQTypeMontgomery) ||
        (cv->type == EQTypeTwistedEdwards);
}

void mpECP_init(mpECP_t pt, mpECurve_t cv) {
    pt->cvp = cv;
    mpFp_init_fp(pt->x, cv->fp);
    mpFp_init_fp(pt->y, cv->fp);
    mpFp_init_fp(pt->z, cv->fp);
    pt->base_bits = 0;
    pt->base_pt = NULL;
    return;
}

void mpECP_clear(mpECP_t pt) {
    if (pt->base_bits != 0) _mpECP_base_pts_cleanup(pt);
    mpFp_clear(pt->x);
    mpFp_clear(pt->y);
    mpFp_clear(pt->z);
    pt->cvp = NULL;
    return;
}

void mpECP_set(mpECP_t rpt, mpECP_t pt) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    rpt->cvp = pt->cvp;
    rpt->is_neutral = pt->is_neutral;
    mpFp_set(rpt->x, pt->x);
    mpFp_set(rpt->y, pt->y);
    mpFp_set(rpt->z, pt->z);
    return;
}

static inline void _transform_mo_to_ws(mpECP_t pt) {
    assert (pt->cvp->type == EQTypeMontgomery);
    // u = x/B + A/3, v = y/B
    mpFp_mul(pt->x, pt->x, pt->cvp->coeff.mo.Binv);
    mpFp_add(pt->x, pt->x, pt->cvp->coeff.mo.Adiv3);
    mpFp_mul(pt->y, pt->y, pt->cvp->coeff.mo.Binv);
}

void mpECP_set_mpz(mpECP_t rpt, mpz_t x, mpz_t y, mpECurve_t cv) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    rpt->cvp = &cv[0];
    rpt->is_neutral = 0;
    mpFp_set_mpz_fp(rpt->x, x, cv->fp);
    mpFp_set_mpz_fp(rpt->y, y, cv->fp);
    mpFp_set_ui_fp(rpt->z, 1, cv->fp);
    if (cv->type == EQTypeMontgomery) _transform_mo_to_ws(rpt);
    return;
}

void mpECP_set_mpFp(mpECP_t rpt, mpFp_t x, mpFp_t y, mpECurve_t cv) {
    assert(cv->fp == x->fp);
    assert(cv->fp == y->fp);
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    rpt->cvp = &cv[0];
    rpt->is_neutral = 0;
    mpFp_set(rpt->x, x);
    mpFp_set(rpt->y, y);
    mpFp_set_ui_fp(rpt->z, 1, cv->fp);
    if (cv->type == EQTypeMontgomery) _transform_mo_to_ws(rpt);
    return;
}

void mpECP_set_neutral(mpECP_t rpt, mpECurve_t cv) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    rpt->cvp = &cv[0];
    switch (cv->type) {
    case EQTypeShortWeierstrass:
        rpt->is_neutral = 1;
        mpFp_set_ui_fp(rpt->x, 0, cv->fp);
        mpFp_set_ui_fp(rpt->y, 1, cv->fp);
        mpFp_set_ui_fp(rpt->z, 0, cv->fp);
        return;
    case EQTypeEdwards:
        // return the neutral element (which is a valid curve point 0,c)
        rpt->is_neutral = 0;
        mpFp_set_ui_fp(rpt->z, 1, cv->fp);
        mpFp_set_ui_fp(rpt->x, 0, cv->fp);
        mpFp_set(rpt->y, cv->coeff.ed.c);
        return;
    case EQTypeMontgomery:
        rpt->is_neutral = 1;
        mpFp_set_ui_fp(rpt->x, 0, cv->fp);
        mpFp_set_ui_fp(rpt->y, 1, cv->fp);
        mpFp_set_ui_fp(rpt->z, 0, cv->fp);
        return;
    case EQTypeTwistedEdwards:
        // return the neutral element (which is a valid curve point 0,1)
        rpt->is_neutral = 0;
        mpFp_set_ui_fp(rpt->x, 0, cv->fp);
        mpFp_set_ui_fp(rpt->y, 1, cv->fp);
        mpFp_set_ui_fp(rpt->z, 1, cv->fp);
        return;
    default:
        assert(_known_curve_type(cv));
    }
}

void _mpECP_to_affine(mpECP_t pt) {
    mpFp_t zinv;
    if (mpFp_cmp_ui(pt->z, 1) == 0) {
        return;
    }
    mpFp_init_fp(zinv, pt->cvp->fp);
    switch (pt->cvp->type) {
        case EQTypeMontgomery:
            // Montgomery curve point internal representation is short-WS
        case EQTypeShortWeierstrass:
            // RCB uses projective coords, so fall through to same xform as Ed
#ifndef _MPECP_USE_RCB
            {
                mpFp_t t;
                if (pt->is_neutral != 0) break;
                // Jacobian coords x = X/Z**2 y = Y/Z**3);
                mpFp_init_fp(t, pt->cvp->fp);
                mpFp_inv(zinv, pt->z);
                mpFp_sqr(t, zinv);
                mpFp_mul(pt->x, pt->x, t);
                mpFp_pow_ui(t, zinv, 3);
                mpFp_mul(pt->y, pt->y, t);
                mpFp_set_ui_fp(pt->z, 1, pt->cvp->fp);
                mpFp_clear(t);
            }
            break;
#endif
        case EQTypeEdwards:
            // Projective x = X/Z y = Y/Z
        case EQTypeTwistedEdwards:
            // Projective x = X/Z y = Y/Z
            mpFp_inv(zinv, pt->z);
            mpFp_mul(pt->x, pt->x, zinv);
            mpFp_mul(pt->y, pt->y, zinv);
            mpFp_set_ui_fp(pt->z, 1, pt->cvp->fp);
            break;
        default:
            assert(_known_curve_type(pt->cvp));
    }
    mpFp_clear(zinv);
    return;
}

static inline void _transform_ws_to_mo_x(mpFp_t x, mpECP_t pt) {
    //assert (pt->cvp->type == EQTypeMontgomery)
    //_mpECP_to_affine(pt);
    // transform to Mo
    // x = Bu-A/3, y = Bv
    mpFp_mul(x, pt->x, pt->cvp->coeff.mo.B);
    mpFp_sub(x, x, pt->cvp->coeff.mo.Adiv3);
    return;
}

static inline void _transform_ws_to_mo_y(mpFp_t y, mpECP_t pt) {
    //assert (pt->cvp->type == EQTypeMontgomery)
    //_mpECP_to_affine(pt);
    // transform to Mo
    // x = Bu-A/3, y = Bv
    mpFp_mul(y, pt->y, pt->cvp->coeff.mo.B);
    return;
}

void mpFp_set_mpECP_affine_x(mpFp_t x, mpECP_t pt) {
    _mpECP_to_affine(pt);
    if (pt->cvp->type == EQTypeMontgomery) {
        _transform_ws_to_mo_x(x, pt);
    } else {
        mpFp_set(x, pt->x);
    }
    return;
}

void mpFp_set_mpECP_affine_y(mpFp_t y, mpECP_t pt) {
    _mpECP_to_affine(pt);
    if (pt->cvp->type == EQTypeMontgomery) {
        _transform_ws_to_mo_y(y, pt);
    } else {
        mpFp_set(y, pt->y);
    }
    return;
}

void mpz_set_mpECP_affine_x(mpz_t x, mpECP_t pt) {
    _mpECP_to_affine(pt);
    if (pt->cvp->type == EQTypeMontgomery) {
        mpFp_t t;
        mpFp_init_fp(t, pt->cvp->fp);
        _transform_ws_to_mo_x(t, pt);
        mpz_set_mpFp(x, t);
        mpFp_clear(t);
    } else {
        mpz_set_mpFp(x, pt->x);
    }
    return;
}

void mpz_set_mpECP_affine_y(mpz_t y, mpECP_t pt) {
    _mpECP_to_affine(pt);
    if (pt->cvp->type == EQTypeMontgomery) {
        mpFp_t t;
        mpFp_init_fp(t, pt->cvp->fp);
        _transform_ws_to_mo_y(t, pt);
        mpz_set_mpFp(y, t);
        mpFp_clear(t);
    } else {
        mpz_set_mpFp(y, pt->y);
    }
    return;
}

static inline int _bytelen(int bits) {
    return ((bits + 7) >> 3);
}

int mpECP_set_str(mpECP_t rpt, char *s, mpECurve_t cv) {
    size_t slen;
    size_t blen;
    int i;
    int status;
    unsigned char *buf;

    slen = strlen(s);
    if ((slen & 0x01) != 0) return -1;
    blen = slen >> 1;
    buf = (unsigned char *)malloc(blen * sizeof(char));
    assert(buf != NULL);

    for (i = 0; i < blen; i++) {
        unsigned int btmp;
        sscanf(&s[2 * i], "%2X", &btmp);
        assert(btmp >= 0);
        assert(btmp < 256);
        buf[i] = (unsigned char)btmp;
        //printf("%02X", buf[i]);
    }
    //printf("\n");
    
    status = mpECP_set_bytes(rpt, buf, blen, cv);
    free(buf);
    return status;
}

int mpECP_set_bytes(mpECP_t rpt, unsigned char *b, size_t blen, mpECurve_t cv) {
    int bytes;

    bytes = _bytelen(cv->bits);
    if (blen < (1 + bytes)) return -1;
    switch (b[0]) {
        case 0:
            mpECP_set_neutral(rpt, cv);
            return 0;
        case 4: {
                if (blen != (1 + (2 * bytes))) return -1;
                mpz_t x, y;
                mpz_init(x);
                mpz_init(y);
                mpz_import(x, bytes, 1, sizeof(unsigned char), 1, 0, &(b[1]));
                mpz_import(y, bytes, 1, sizeof(unsigned char), 1, 0, &(b[1 + bytes]));
                mpECP_set_mpz(rpt, x, y, cv);
                mpz_clear(y);
                mpz_clear(x);
            }
            return 0;
        case 2:
        case 3: {
                mpz_t xz;
                mpFp_t x, y, t;
                int error, odd;
                if (blen != (1 + bytes)) return -1;
                mpz_init(xz);
                mpFp_init_fp(x, cv->fp);
                mpFp_init_fp(y, cv->fp);
                mpFp_init_fp(t, cv->fp);
                mpz_import(xz, bytes, 1, sizeof(unsigned char), 1, 0, &b[1]);
                mpFp_set_mpz_fp(x, xz, cv->fp);
                switch (cv->type) {
                    case EQTypeShortWeierstrass: {
                            // y**2 = x**3 + ax + b
                            mpFp_mul(t, cv->coeff.ws.a, x);
                            mpFp_add(y, cv->coeff.ws.b, t);
                            mpFp_pow_ui(t, x, 3);
                            mpFp_add(y, y, t);
                            error = mpFp_sqrt(y, y);
                            if (error != 0) {
                                mpFp_clear(t);
                                mpFp_clear(y);
                                mpFp_clear(x);
                                mpz_clear(xz);
                                return -1;
                            }
                        }
                        break;
                    case EQTypeEdwards: {
                            mpFp_t c2, x2;
                            mpFp_init_fp(c2, cv->fp);
                            mpFp_init_fp(x2, cv->fp);
                            // x**2 + y**2 = c**2 (1 + d * x**2 * y**2)
                            // y**2 - C**2 * d * x**2 * y**2 = c**2 - x**2
                            // y**2 = (c**2 - x**2) / (1 - c**2 * d * x**2)
                            mpFp_mul(c2, cv->coeff.ed.c, cv->coeff.ed.c);
                            mpFp_mul(t, cv->coeff.ed.d, c2);
                            mpFp_pow_ui(x2, x, 2);
                            mpFp_mul(t, t, x2);
                            mpFp_set_ui_fp(y, 1, cv->fp);
                            mpFp_sub(t, y, t);
                            mpFp_sub(y, c2, x2);
                            mpFp_inv(t, t);
                            mpFp_mul(t, t, y);
                            error = mpFp_sqrt(y, t);
                            mpFp_clear(x2);
                            mpFp_clear(c2);
                            if (error != 0) {
                                mpFp_clear(t);
                                mpFp_clear(y);
                                mpFp_clear(x);
                                mpz_clear(xz);
                                return -1;
                            }
                        }
                        break;
                    case EQTypeTwistedEdwards: {
                            mpFp_t a, x2;
                            mpFp_init_fp(a, cv->fp);
                            mpFp_init_fp(x2, cv->fp);
                            // a * x**2 + y**2 = 1 + d * x**2 * y**2
                            // y**2 - d * x**2 * y**2 = 1 - a * x**2
                            // y**2 = (1 - a * x**2) / (1 - d * x**2)
                            mpFp_set(a, cv->coeff.te.a);
                            mpFp_set(t, cv->coeff.te.d);
                            mpFp_pow_ui(x2, x, 2);
                            mpFp_mul(t, t, x2);
                            mpFp_set_ui_fp(y, 1, cv->fp);
                            mpFp_sub(t, y, t);
                            mpFp_mul(x2, x2, a);
                            mpFp_sub(y, y, x2);
                            mpFp_inv(t, t);
                            mpFp_mul(t, t, y);
                            error = mpFp_sqrt(y, t);
                            mpFp_clear(x2);
                            mpFp_clear(a);
                            if (error != 0) {
                                mpFp_clear(t);
                                mpFp_clear(y);
                                mpFp_clear(x);
                                mpz_clear(xz);
                                return -1;
                            }
                        }
                        break;
                    case EQTypeMontgomery: {
                            int error;
                            // B * y**2 = x**3 + A * x**2 + x
                            mpFp_t s;
                            mpFp_init_fp(s, cv->fp);
                            mpFp_mul(s, x, x);
                            mpFp_mul(t, s, x);
                            mpFp_mul(s, s, cv->coeff.mo.A);
                            mpFp_add(s, s, t);
                            mpFp_add(s, s, x);
                            mpFp_mul(s, s, cv->coeff.mo.Binv);
                            error = mpFp_sqrt(y, s);
                            mpFp_clear(s);
                            if (error != 0) {
                                mpFp_clear(t);
                                mpFp_clear(y);
                                mpFp_clear(x);
                                mpz_clear(xz);
                                return -1;
                            }
                        }
                        break;
                    default:
                        assert(_known_curve_type(cv));
                        return -1;
                }
                odd = mpz_tstbit(y->i, 0);
                // '3' implies odd, '2' even... negate if not matched
                if ((b[0] & 0x01) != odd) {
                    mpFp_neg(y, y);
                }
                mpECP_set_mpFp(rpt, x, y, cv);
                mpFp_clear(t);
                mpFp_clear(y);
                mpFp_clear(x);
                mpz_clear(xz);
            }
            return 0;
        default:
            return -1;
    }
    assert(0);
}

int  mpECP_out_bytelen(mpECP_t pt, int compress) {
    int bytes;
    bytes = _bytelen(pt->cvp->bits);
    // use common prefix (02, 03, 04) and then either X or X and Y
    if (compress == 0) {
        return (1 + (2 * bytes));
    }
    return (1 + bytes);
}

int  mpECP_out_strlen(mpECP_t pt, int compress) {
    int bytes;
    bytes = _bytelen(pt->cvp->bits);
    // use common prefix (02, 03, 04) and then either X or X and Y
    if (compress == 0) {
        return (2 + (4 * bytes));
    }
    return (2 + (2 * bytes));
}

void mpECP_out_str(char *s, mpECP_t pt, int compress) {
    int i;
    int bytes;

    bytes = mpECP_out_bytelen(pt, compress);
    mpECP_out_bytes((unsigned char *)s, pt, compress);
    //for (i = 0; i < bytes; i++){
    //    printf("%02X", ((unsigned char *)s)[i]);
    //}
    //printf("\n");
    for (i = (bytes-1); i >= 0; i--) {
        unsigned int value;
        value = ((unsigned char *)s)[i];
        assert(value >= 0);
        assert(value < 256);
        s[2 * i] = _hexlut[(value >> 4)];
        s[2 * i + 1] = _hexlut[(value & 0x0F)];
    }
    s[2 * bytes] = 0;
    return;
}

void mpECP_out_bytes(unsigned char *s, mpECP_t pt, int compress) {
    int bytes;
    size_t blen;

    _mpECP_to_affine(pt);
    bytes = _bytelen(pt->cvp->bits);
    if (pt->is_neutral) {
        int i;

        if (compress == 0) bytes *= 2;
        for (i = 0; i < (bytes + 1); i++) {
            s[i] = 0;
        }
        return;
    }
    if (compress != 0) {
        mpz_t odd;
        mpz_init(odd);
        mpz_set_mpFp(odd, pt->y);
        mpz_mod_ui(odd, odd, 2);
        if (mpz_cmp_ui(odd, 1) == 0) {
            s[0] = 3;
        } else {
            s[0] = 2;
        }
        mpz_clear(odd);
    } else {
        s[0] = 4;
    }
    mpz_t xz;
    mpz_init(xz);
    if (pt->cvp->type == EQTypeMontgomery) {
        mpFp_t x;
        mpFp_init_fp(x, pt->cvp->fp);
        _transform_ws_to_mo_x(x, pt);
        mpz_set_mpFp(xz, x);
        mpFp_clear(x);
    } else {
        mpz_set_mpFp(xz, pt->x);
    }
    mpz_export(&(s[1]), &blen, 1, sizeof(unsigned char), 1, 0, xz);
    assert(blen <= bytes);
    if (blen < bytes) {
        int i;
        int shift;

        shift = bytes - blen;
        for (i = (blen - 1); i >= 0; i-- ) {
            s[1 + i + shift] = s[1+i];
        }
        for (i = 0; i < shift; i++) {
            s[1 + i] = 0;
        }
    }
    
    if (compress == 0) {
        mpz_t yz;

        mpz_init(yz);
        if (pt->cvp->type == EQTypeMontgomery) {
            mpFp_t y;
            mpFp_init_fp(y, pt->cvp->fp);
            _transform_ws_to_mo_y(y, pt);
            mpz_set_mpFp(yz, y);
            mpFp_clear(y);
        } else {
            mpz_set_mpFp(yz, pt->y);
        }
        mpz_export(&(s[1 + bytes]), &blen, 1, sizeof(unsigned char), 1, 0, yz);
        assert(blen <= bytes);
        if (blen < bytes) {
            int i;
            int shift;
    
            shift = bytes - blen;
            for (i = (blen - 1); i >= 0; i-- ) {
                s[1 + i + shift + bytes] = s[1 + i + bytes];
            }
            for (i = 0; i < shift; i++) {
                s[1 + i + bytes] = 0;
            }
        }
        mpz_clear(yz);
    }
    mpz_clear(xz);
    //printf("exported as %s\n", s);
    return;
}

void mpECP_neg(mpECP_t rpt, mpECP_t pt) {
    if (pt->is_neutral != 0) {
        mpECP_set_neutral(rpt, pt->cvp);
        return;
    }
    switch (pt->cvp->type) {
        case EQTypeMontgomery:
            // Montgomery curve point internal representation is short-WS
        case EQTypeShortWeierstrass:
            mpECP_set(rpt, pt);
            mpFp_neg(rpt->y, pt->y);
            break;
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            mpECP_set(rpt, pt);
            mpFp_neg(rpt->x, pt->x);
            break;
        default:
            assert(_known_curve_type(pt->cvp));
    }
    return;
}

int mpECP_cmp(mpECP_t pt1, mpECP_t pt2) {
    if (mpECurve_cmp(pt1->cvp, pt2->cvp) != 0) return -1;
    if (pt1->is_neutral != 0) {
        if (pt2->is_neutral != 0) {
            return 0;
        }
        return -1;
    } else if (pt2->is_neutral != 0) {
        return -1;
    }
    switch (pt1->cvp->type) {
        case EQTypeMontgomery:
            // Montgomery curve point internal representation is short-WS
        case EQTypeShortWeierstrass:
            // RCB uses projective coords, so fall through to same xform as Ed
#ifndef _MPECP_USE_RCB
            {
                mpFp_t U1, U2;
                mpFp_init_fp(U1, pt1->cvp->fp);
                mpFp_init_fp(U2, pt1->cvp->fp);
                mpFp_pow_ui(U1, pt2->z, 2);
                mpFp_mul(U1, U1, pt1->x);
                mpFp_pow_ui(U2, pt1->z, 2);
                mpFp_mul(U2, U2, pt2->x);
                if (mpFp_cmp(U1, U2) != 0) return -1;
                mpFp_pow_ui(U1, pt2->z, 3);
                mpFp_mul(U1, U1, pt1->y);
                mpFp_pow_ui(U2, pt1->z, 3);
                mpFp_mul(U2, U2, pt2->y);
                if (mpFp_cmp(U1, U2) != 0) return -1;
                mpFp_clear(U2);
                mpFp_clear(U1);
            }
            break;
#endif
        case EQTypeEdwards:
        case EQTypeTwistedEdwards: {
                mpFp_t U1, U2;
                mpFp_init_fp(U1, pt1->cvp->fp);
                mpFp_init_fp(U2, pt1->cvp->fp);
                mpFp_mul(U1, pt2->z, pt1->x);
                mpFp_mul(U2, pt1->z, pt2->x);
                if (mpFp_cmp(U1, U2) != 0) return -1;
                mpFp_mul(U1, pt2->z, pt1->y);
                mpFp_mul(U2, pt1->z, pt2->y);
                if (mpFp_cmp(U1, U2) != 0) return -1;
                mpFp_clear(U2);
                mpFp_clear(U1);
            }
            break;
        default:
            assert(_known_curve_type(pt1->cvp));
    }
    return 0;
}

void mpECP_swap(mpECP_t pt2, mpECP_t pt1) {
    int t;
    struct _p_mpECP_t *t_base_pt;
    assert(mpECurve_cmp(pt1->cvp, pt2->cvp) == 0);
    mpFp_cswap(pt2->x, pt1->x, 1);
    mpFp_cswap(pt2->y, pt1->y, 1);
    mpFp_cswap(pt2->z, pt1->z, 1);
    t = pt2->is_neutral;
    pt2->is_neutral = pt1->is_neutral;
    pt1->is_neutral = t;
    t = pt2->base_bits;
    pt2->base_bits = pt1->base_bits;
    pt1->base_bits = t;
    t_base_pt = pt2->base_pt;
    pt2->base_pt = pt1->base_pt;
    pt1->base_pt = t_base_pt;
    return;
}

static void _mpECP_cswap_safe(mpECP_t pt2, mpECP_t pt1, int swap) {
    int a[2];
    swap = (swap != 0);

    mpFp_cswap(pt2->x, pt1->x, swap);
    mpFp_cswap(pt2->y, pt1->y, swap);
    mpFp_cswap(pt2->z, pt1->z, swap);

    a[0] = pt1->is_neutral;
    a[1] = pt2->is_neutral;
    pt2->is_neutral = a[1-swap];
    pt1->is_neutral = a[swap];

    return;
}

void mpECP_cswap(mpECP_t pt2, mpECP_t pt1, int swap) {
    assert(mpECurve_cmp(pt1->cvp, pt2->cvp) == 0);
    assert(pt1->base_bits == 0);
    assert(pt2->base_bits == 0);
    _mpECP_cswap_safe(pt2, pt1, swap);
}

void mpECP_add(mpECP_t rpt, mpECP_t pt1, mpECP_t pt2) {
#ifdef _MPECP_USE_RCB
    mpFp_ptr aa, bb;
#endif
    assert(mpECurve_cmp(pt1->cvp, pt2->cvp) == 0);
    if (pt1->is_neutral != 0) {
        if (pt2->is_neutral != 0) {
            mpECP_set_neutral(rpt, pt1->cvp);
            return;
        } else {
            mpECP_set(rpt, pt2);
            return;
        }
    } else if (pt2->is_neutral != 0) {
        mpECP_set(rpt, pt1);
        return;
    }
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
#ifdef _MPECP_USE_RCB
    aa = pt1->cvp->coeff.ws.a;
    bb = pt1->cvp->coeff.ws.b;
#endif
    switch (pt1->cvp->type) {
        case EQTypeMontgomery:
            // Montgomery curve point internal representation is short-WS
#ifdef _MPECP_USE_RCB
            aa = pt1->cvp->coeff.mo.ws_a;
            bb = pt1->cvp->coeff.mo.ws_b;
#endif
        case EQTypeShortWeierstrass: {
            // RCB uses projective coords, so fall through to same xform as Ed
#ifdef _MPECP_USE_RCB
                //assert(0); // might want to implement something here ;)
                // 2015 Renes-Costello-Batina "Algorithm 1"
                // from https://eprint.iacr.org/2015/1060.pdf
                mpFp_t t0, t1, t2, t3, t4, t5, b3;
#ifdef _MPECP_MPFP_NOMALLOC
                __local_limb_t lt0, lt1, lt2, lt3, lt4, lt5, lb3;
                t0->i->_mp_d = lt0; t0->i->_mp_size = 0; t0->i->_mp_alloc = _MPFP_MAX_LIMBS; t0->fp = pt1->cvp->fp;
                t1->i->_mp_d = lt1; t1->i->_mp_size = 0; t1->i->_mp_alloc = _MPFP_MAX_LIMBS; t1->fp = pt1->cvp->fp;
                t2->i->_mp_d = lt2; t2->i->_mp_size = 0; t2->i->_mp_alloc = _MPFP_MAX_LIMBS; t2->fp = pt1->cvp->fp;
                t3->i->_mp_d = lt3; t3->i->_mp_size = 0; t3->i->_mp_alloc = _MPFP_MAX_LIMBS; t3->fp = pt1->cvp->fp;
                t4->i->_mp_d = lt4; t4->i->_mp_size = 0; t4->i->_mp_alloc = _MPFP_MAX_LIMBS; t4->fp = pt1->cvp->fp;
                t5->i->_mp_d = lt5; t5->i->_mp_size = 0; t5->i->_mp_alloc = _MPFP_MAX_LIMBS; t5->fp = pt1->cvp->fp;
                b3->i->_mp_d = lb3; b3->i->_mp_size = 0; b3->i->_mp_alloc = _MPFP_MAX_LIMBS; b3->fp = pt1->cvp->fp;
#else
                mpFp_init_fp(t0, pt1->cvp->fp);
                mpFp_init_fp(t1, pt1->cvp->fp);
                mpFp_init_fp(t2, pt1->cvp->fp);
                mpFp_init_fp(t3, pt1->cvp->fp);
                mpFp_init_fp(t4, pt1->cvp->fp);
                mpFp_init_fp(t5, pt1->cvp->fp);
                mpFp_init_fp(b3, pt1->cvp->fp);
#endif
                // TODO: Precalculate b3 and store in coeff.ws.b3
                mpFp_add(b3, bb, bb);
                mpFp_add(b3, b3, bb);

                // 1. t0 <- X1 * X2
                // 2. t1 <- Y1 * Y2
                // 3. t2 <- Z1 * Z2
                // 4. t3 <- X1 + Y1
                // 5. t4 <- X2 + Y2
                // 6. t3 <- t3 * t4
                // 7. t4 <- t0 + t1
                // 8. t3 <- t3 - t4
                // 9. t4 <- X1 + Z1
                //10. t5 <- X2 + Z2
                //11. t4 <- t4 * t5
                //12. t5 <- t0 + t2
                //13. t4 <- t4 - t5
                //14. t5 <- Y1 + Z1
                //15. X3 <- Y2 + Z2
                //16. t5 <- t5 * X3
                //17. X3 <- t1 + t2
                //18. t5 <- t5 - X3
                //19. Z3 <-  a * t4
                //20. X3 <- b3 * t2
                //21. Z3 <- X3 + Z3
                //22. X3 <- t1 - Z3
                //23. Z3 <- t1 + Z3
                //24. Y3 <- X3 * Z3
                //25. t1 <- t0 + t0
                //26. t1 <- t1 + t0
                //27. t2 <-  a * t2
                //28. t4 <- b3 * t4
                //29. t1 <- t1 + t2
                //30. t2 <- t0 - t2
                //31. t2 <-  a * t2
                //32. t4 <- t4 + t2
                //33. t0 <- t1 * t4
                //34. Y3 <- Y3 + t0
                //35. t0 <- t5 * t4
                //36. X3 <- t3 * X3
                //37. X3 <- X3 - t0
                //38. t0 <- t3 * t1
                //39. Z3 <- t5 * Z3
                //40. Z3 <- Z3 + t0

                // 1. t0 <- X1 * X2
                mpFp_mul(t0, pt1->x, pt2->x);
                // 2. t1 <- Y1 * Y2
                mpFp_mul(t1, pt1->y, pt2->y);
                // 3. t2 <- Z1 * Z2
                mpFp_mul(t2, pt1->z, pt2->z);
                // 4. t3 <- X1 + Y1
                mpFp_add(t3, pt1->x, pt1->y);
                // 5. t4 <- X2 + Y2
                mpFp_add(t4, pt2->x, pt2->y);
                // 6. t3 <- t3 * t4
                mpFp_mul(t3, t3, t4);
                // 7. t4 <- t0 + t1
                mpFp_add(t4, t0, t1);
                // 8. t3 <- t3 - t4
                mpFp_sub(t3, t3, t4);
                // 9. t4 <- X1 + Z1
                mpFp_add(t4, pt1->x, pt1->z);
                //10. t5 <- X2 + Z2
                mpFp_add(t5, pt2->x, pt2->z);
                //11. t4 <- t4 * t5
                mpFp_mul(t4, t4, t5);
                //12. t5 <- t0 + t2
                mpFp_add(t5, t0, t2);
                //13. t4 <- t4 - t5
                mpFp_sub(t4, t4, t5);
                //14. t5 <- Y1 + Z1
                mpFp_add(t5, pt1->y, pt1->z);
                //15. X3 <- Y2 + Z2
                mpFp_add(rpt->x, pt2->y, pt2->z);
                //16. t5 <- t5 * X3
                mpFp_mul(t5, t5, rpt->x);
                //17. X3 <- t1 + t2
                mpFp_add(rpt->x, t1, t2);
                //18. t5 <- t5 - X3
                mpFp_sub(t5, t5, rpt->x);
                //19. Z3 <-  a * t4
                mpFp_mul(rpt->z, aa, t4);
                //20. X3 <- b3 * t2
                mpFp_mul(rpt->x, b3, t2);
                //21. Z3 <- X3 + Z3
                mpFp_add(rpt->z, rpt->x, rpt->z);
                //22. X3 <- t1 - Z3
                mpFp_sub(rpt->x, t1, rpt->z);
                //23. Z3 <- t1 + Z3
                mpFp_add(rpt->z, t1, rpt->z);
                //24. Y3 <- X3 * Z3
                mpFp_mul(rpt->y, rpt->x, rpt->z);
                //25. t1 <- t0 + t0
                mpFp_add(t1, t0, t0);
                //26. t1 <- t1 + t0
                mpFp_add(t1, t1, t0);
                //27. t2 <-  a * t2
                mpFp_mul(t2, aa, t2);
                //28. t4 <- b3 * t4
                mpFp_mul(t4, b3, t4);
                //29. t1 <- t1 + t2
                mpFp_add(t1, t1, t2);
                //30. t2 <- t0 - t2
                mpFp_sub(t2, t0, t2);
                //31. t2 <-  a * t2
                mpFp_mul(t2, aa, t2);
                //32. t4 <- t4 + t2
                mpFp_add(t4, t4, t2);
                //33. t0 <- t1 * t4
                mpFp_mul(t0, t1, t4);
                //34. Y3 <- Y3 + t0
                mpFp_add(rpt->y, rpt->y, t0);
                //35. t0 <- t5 * t4
                mpFp_mul(t0, t5, t4);
                //36. X3 <- t3 * X3
                mpFp_mul(rpt->x, t3, rpt->x);
                //37. X3 <- X3 - t0
                mpFp_sub(rpt->x, rpt->x, t0);
                //38. t0 <- t3 * t1
                mpFp_mul(t0, t3, t1);
                //39. Z3 <- t5 * Z3
                mpFp_mul(rpt->z, t5, rpt->z);
                //40. Z3 <- Z3 + t0
                mpFp_add(rpt->z, rpt->z, t0);

                rpt->cvp = pt1->cvp;

                if (mpFp_cmp_ui(rpt->z, 0) == 0) {
                    mpECP_set_neutral(rpt, pt1->cvp);
                } else {
                    rpt->is_neutral = 0;
                }

#ifndef _MPECP_MPFP_NOMALLOC
                mpFp_clear(b3);
                mpFp_clear(t5);
                mpFp_clear(t4);
                mpFp_clear(t3);
                mpFp_clear(t2);
                mpFp_clear(t1);
                mpFp_clear(t0);
#endif
#else
                // 2007 Bernstein-Lange formula
                // from : http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#addition-add-2007-bl
                // Z1Z1 = Z1**2
                // Z2Z2 = Z2**2
                // U1 = X1*Z2Z2
                // U2 = X2*Z1Z1
                // S1 = Y1*Z2*Z2Z2
                // S2 = Y2*Z1*Z1Z1
                // H = U2-U1
                // I = (2*H)**2
                // J = H*I
                // r = 2*(S2-S1) - use S2 as temp in place of r
                // V = U1*I 
                // X3 = r**2-J-2*V
                // Y3 = r*(V-X3)-2*S1*J
                // Z3 = ((Z1+Z2)**2-Z1Z1-Z2Z2)*H
                mpFp_t Z1Z1, Z2Z2, U1, U2, S1, S2, H, I, J, V;
                mpFp_init_fp(Z1Z1, pt1->cvp->fp);
                mpFp_init_fp(Z2Z2, pt1->cvp->fp);
                mpFp_init_fp(U1, pt1->cvp->fp);
                mpFp_init_fp(U2, pt1->cvp->fp);
                mpFp_init_fp(S1, pt1->cvp->fp);
                mpFp_init_fp(S2, pt1->cvp->fp);
                mpFp_init_fp(H, pt1->cvp->fp);
                mpFp_init_fp(I, pt1->cvp->fp);
                mpFp_init_fp(J, pt1->cvp->fp);
                mpFp_init_fp(V, pt1->cvp->fp);
                // Z1Z1 = Z1**2
                mpFp_pow_ui(Z1Z1, pt1->z, 2);
                // Z2Z2 = Z2**2
                mpFp_pow_ui(Z2Z2, pt2->z, 2);
                // U1 = X1*Z2Z2
                mpFp_mul(U1, Z2Z2, pt1->x);
                // U2 = X2*Z1Z1
                mpFp_mul(U2, Z1Z1, pt2->x);
                // S1 = Y1*Z2*Z2Z2
                mpFp_mul(S1, pt2->z, Z2Z2);
                mpFp_mul(S1, S1, pt1->y);
                // S2 = Y2*Z1*Z1Z1
                mpFp_mul(S2, pt1->z, Z1Z1);
                mpFp_mul(S2, S2, pt2->y);
                if (mpFp_cmp(U1, U2) == 0) {
                    if (mpFp_cmp(S1, S2) == 0) {
                        // pt1 == pt2, so use doubling formula
                        mpECP_double(rpt, pt1);
                    } else {
                        // pt1 == -pt2 ? validate, return is_neutral
                        mpFp_neg(I, S2);
                        assert(mpFp_cmp(I, S1) == 0);
                        mpECP_set_neutral(rpt, pt1->cvp);
                    }
                } else {
                    // H = U2-U1
                    mpFp_sub(H, U2, U1);
                    // I = (2*H)**2
                    mpFp_mul_ui(I, H, 2);
                    mpFp_pow_ui(I, I, 2);
                    // J = H*I
                    mpFp_mul(J, H, I);
                    // S2 is used as r below here
                    // S2_r = 2*(S2-S1) - use S2 as temp in place of r
                    mpFp_sub(S2, S2, S1);
                    mpFp_mul_ui(S2, S2, 2);
                    // V = U1*I 
                    mpFp_mul(V, U1, I);
                    // I, U2 is used as temp below here
                    // X3 = S2_r**2-J-2*V
                    mpFp_pow_ui(U2, S2, 2);
                    mpFp_sub(U2, U2, J);
                    mpFp_mul_ui(I, V, 2);
                    mpFp_sub(rpt->x, U2, I);
                    // Y3 = S2_r*(V-X3)-2*S1*J
                    mpFp_sub(I, V, rpt->x);
                    mpFp_mul(I, I, S2);
                    mpFp_mul(S1, S1, J);
                    mpFp_mul_ui(S1, S1, 2);
                    mpFp_sub(rpt->y, I, S1);
                    // Z3 = ((Z1+Z2)**2-Z1Z1-Z2Z2)*H
                    mpFp_add(I, pt1->z, pt2->z);
                    mpFp_pow_ui(I, I, 2);
                    mpFp_sub(I, I, Z1Z1);
                    mpFp_sub(I, I, Z2Z2);
                    mpFp_mul(rpt->z, I, H);
                    rpt->cvp = pt1->cvp;
                    rpt->is_neutral = 0;
                }
                // done... clean up temporary variables
                mpFp_clear(V);
                mpFp_clear(J);
                mpFp_clear(I);
                mpFp_clear(H);
                mpFp_clear(S2);
                mpFp_clear(S1);
                mpFp_clear(U2);
                mpFp_clear(U1);
                mpFp_clear(Z2Z2);
                mpFp_clear(Z1Z1);
#endif
                return;
            }
            break;
        case EQTypeEdwards: {
                // 2007 Bernstein-Lange formula
                // http://www.hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#addition-add-2007-bl
                // A = Z1*Z2
                // B = A**2
                // C = X1*X2
                // D = Y1*Y2
                // E = d*C*D
                // F = B-E
                // G = B+E
                // X3 = A*F*((X1+Y1)*(X2+Y2)-C-D)
                // Y3 = A*G*(D-C)
                // Z3 = c*F*G
                mpFp_t A, B, C, D, E, F, G;
#ifdef _MPECP_MPFP_NOMALLOC
                __local_limb_t lA, lB, lC, lD, lE, lF, lG;
                A->i->_mp_d = lA; A->i->_mp_size = 0; A->i->_mp_alloc = _MPFP_MAX_LIMBS; A->fp = pt1->cvp->fp;
                B->i->_mp_d = lB; B->i->_mp_size = 0; B->i->_mp_alloc = _MPFP_MAX_LIMBS; B->fp = pt1->cvp->fp;
                C->i->_mp_d = lC; C->i->_mp_size = 0; C->i->_mp_alloc = _MPFP_MAX_LIMBS; C->fp = pt1->cvp->fp;
                D->i->_mp_d = lD; D->i->_mp_size = 0; D->i->_mp_alloc = _MPFP_MAX_LIMBS; D->fp = pt1->cvp->fp;
                E->i->_mp_d = lE; E->i->_mp_size = 0; E->i->_mp_alloc = _MPFP_MAX_LIMBS; E->fp = pt1->cvp->fp;
                F->i->_mp_d = lF; F->i->_mp_size = 0; F->i->_mp_alloc = _MPFP_MAX_LIMBS; F->fp = pt1->cvp->fp;
                G->i->_mp_d = lG; G->i->_mp_size = 0; G->i->_mp_alloc = _MPFP_MAX_LIMBS; G->fp = pt1->cvp->fp;
#else
                mpFp_init_fp(A, pt1->cvp->fp);
                mpFp_init_fp(B, pt1->cvp->fp);
                mpFp_init_fp(C, pt1->cvp->fp);
                mpFp_init_fp(D, pt1->cvp->fp);
                mpFp_init_fp(E, pt1->cvp->fp);
                mpFp_init_fp(F, pt1->cvp->fp);
                mpFp_init_fp(G, pt1->cvp->fp);
#endif

                // A = Z1*Z2
                mpFp_mul(A, pt1->z, pt2->z);
                // B = A**2
                mpFp_pow_ui(B, A, 2);
                // C = X1*X2
                mpFp_mul(C, pt1->x, pt2->x);
                // D = Y1*Y2
                mpFp_mul(D, pt1->y, pt2->y);
                // E = d*C*D
                mpFp_mul(E, pt1->cvp->coeff.ed.d, C);
                mpFp_mul(E, E, D);
                // F = B-E
                mpFp_sub(F, B, E);
                // G = B+E
                mpFp_add(G, B, E);
                // B, E used as temp below here
                // X3 = A*F*((X1+Y1)*(X2+Y2)-C-D)
                mpFp_add(B, pt1->x, pt1->y);
                mpFp_add(E, pt2->x, pt2->y);
                mpFp_mul(B, B, E);
                mpFp_sub(B, B, C);
                mpFp_sub(B, B, D);
                mpFp_mul(B, B, F);
                mpFp_mul(rpt->x, B, A);
                // Y3 = A*G*(D-C)
                mpFp_sub(B, D, C);
                mpFp_mul(B, B, G);
                mpFp_mul(rpt->y, B, A);
                // Z3 = c*F*G
                mpFp_mul(B, pt1->cvp->coeff.ed.c, G);
                mpFp_mul(rpt->z, B, F);
                mpECurve_set(rpt->cvp, pt1->cvp);
                rpt->is_neutral = 0;

#ifndef _MPECP_MPFP_NOMALLOC
                mpFp_clear(G);
                mpFp_clear(F);
                mpFp_clear(E);
                mpFp_clear(D);
                mpFp_clear(C);
                mpFp_clear(B);
                mpFp_clear(A);
#endif
                return;
            }
            break;
        case EQTypeTwistedEdwards: {
                // 2008 Bernstein–Birkner–Joye–Lange–Peters formula
                // http://www.hyperelliptic.org/EFD/g1p/auto-twisted-projective.html#addition-add-2008-bbjlp
                // A = Z1*Z2
                // B = A**2
                // C = X1*X2
                // D = Y1*Y2
                // E = d*C*D
                // F = B-E
                // G = B+E
                // X3 = A*F*((X1+Y1)*(X2+Y2)-C-D)
                // Y3 = A*G*(D-a*C)
                // Z3 = F*G
                mpFp_t A, B, C, D, E, F, G;
#ifdef _MPECP_MPFP_NOMALLOC
                __local_limb_t lA, lB, lC, lD, lE, lF, lG;
                A->i->_mp_d = lA; A->i->_mp_size = 0; A->i->_mp_alloc = _MPFP_MAX_LIMBS; A->fp = pt1->cvp->fp;
                B->i->_mp_d = lB; B->i->_mp_size = 0; B->i->_mp_alloc = _MPFP_MAX_LIMBS; B->fp = pt1->cvp->fp;
                C->i->_mp_d = lC; C->i->_mp_size = 0; C->i->_mp_alloc = _MPFP_MAX_LIMBS; C->fp = pt1->cvp->fp;
                D->i->_mp_d = lD; D->i->_mp_size = 0; D->i->_mp_alloc = _MPFP_MAX_LIMBS; D->fp = pt1->cvp->fp;
                E->i->_mp_d = lE; E->i->_mp_size = 0; E->i->_mp_alloc = _MPFP_MAX_LIMBS; E->fp = pt1->cvp->fp;
                F->i->_mp_d = lF; F->i->_mp_size = 0; F->i->_mp_alloc = _MPFP_MAX_LIMBS; F->fp = pt1->cvp->fp;
                G->i->_mp_d = lG; G->i->_mp_size = 0; G->i->_mp_alloc = _MPFP_MAX_LIMBS; G->fp = pt1->cvp->fp;
#else
                mpFp_init_fp(A, pt1->cvp->fp);
                mpFp_init_fp(B, pt1->cvp->fp);
                mpFp_init_fp(C, pt1->cvp->fp);
                mpFp_init_fp(D, pt1->cvp->fp);
                mpFp_init_fp(E, pt1->cvp->fp);
                mpFp_init_fp(F, pt1->cvp->fp);
                mpFp_init_fp(G, pt1->cvp->fp);
#endif

                // A = Z1*Z2
                mpFp_mul(A, pt1->z, pt2->z);
                // B = A**2
                mpFp_mul(B, A, A);
                // C = X1*X2
                mpFp_mul(C, pt1->x, pt2->x);
                // D = Y1*Y2
                mpFp_mul(D, pt1->y, pt2->y);
                // E = d*C*D
                mpFp_mul(E, pt1->cvp->coeff.te.d, C);
                mpFp_mul(E, E, D);
                // F = B-E
                mpFp_sub(F, B, E);
                // G = B+E
                mpFp_add(G, B, E);
                // B, E used as temp below here
                // X3 = A*F*((X1+Y1)*(X2+Y2)-C-D)
                mpFp_add(B, pt1->x, pt1->y);
                mpFp_add(E, pt2->x, pt2->y);
                mpFp_mul(B, B, E);
                mpFp_sub(B, B, C);
                mpFp_sub(B, B, D);
                mpFp_mul(B, B, F);
                mpFp_mul(rpt->x, B, A);
                // Y3 = A*G*(D-a*C)
                mpFp_mul(C, C, pt1->cvp->coeff.te.a);
                mpFp_sub(B, D, C);
                mpFp_mul(B, B, G);
                mpFp_mul(rpt->y, B, A);
                // Z3 = F*G
                mpFp_mul(rpt->z, G, F);
                mpECurve_set(rpt->cvp, pt1->cvp);
                rpt->is_neutral = 0;

#ifndef _MPECP_MPFP_NOMALLOC
                mpFp_clear(G);
                mpFp_clear(F);
                mpFp_clear(E);
                mpFp_clear(D);
                mpFp_clear(C);
                mpFp_clear(B);
                mpFp_clear(A);
#endif
                return;
            }
            break;
        default:
            assert(_known_curve_type(pt1->cvp));
    }
    assert(0);
}

void mpECP_double(mpECP_t rpt, mpECP_t pt) {
    if (pt->is_neutral != 0) {
        mpECP_set_neutral(rpt, pt->cvp);
        return;
    }
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    switch (pt->cvp->type) {
        case EQTypeMontgomery:
            // Montgomery curve point internal representation is short-WS
        case EQTypeShortWeierstrass:
            // RCB add is complete... fall through and call add
#ifndef _MPECP_USE_RCB
            {
                // 2007 Bernstein-Lange formula
                // from : http://www.hyperelliptic.org/EFD/g1p/auto-shortw-jacobian.html#doubling-dbl-2007-bl
                // XX = X1**2
                // YY = Y1**2
                // YYYY = YY**2
                // ZZ = Z1**2
                // S = 2*((X1+YY)**2-XX-YYYY)
                // M = 3*XX+a*ZZ**2
                // T = M**2-2*S
                // X3 = T
                // Y3 = M*(S-T)-8*YYYY
                // Z3 = (Y1+Z1)**2-YY-ZZ
                mpFp_t XX, YY, YYYY, ZZ, S, M, T;
                mpFp_init_fp(XX, pt->cvp->fp);
                mpFp_init_fp(YY, pt->cvp->fp);
                mpFp_init_fp(YYYY, pt->cvp->fp);
                mpFp_init_fp(ZZ, pt->cvp->fp);
                mpFp_init_fp(S, pt->cvp->fp);
                mpFp_init_fp(M, pt->cvp->fp);
                mpFp_init_fp(T, pt->cvp->fp);
                // XX = X1**2
                mpFp_pow_ui(XX, pt->x, 2);
                // YY = Y1**2
                mpFp_pow_ui(YY, pt->y, 2);
                // YYYY = YY**2
                mpFp_pow_ui(YYYY, YY, 2);
                // ZZ = Z1**2
                mpFp_pow_ui(ZZ, pt->z, 2);
                // S = 2*((X1+YY)**2-XX-YYYY)
                mpFp_add(S, pt->x, YY);
                mpFp_pow_ui(S, S, 2);
                mpFp_sub(S, S, XX);
                mpFp_sub(S, S, YYYY);
                mpFp_mul_ui(S, S, 2);
                // M = 3*XX+a*ZZ**2
                mpFp_pow_ui(M, ZZ, 2);
                if (pt->cvp->type == EQTypeMontgomery) {
                    mpFp_mul(M, M, pt->cvp->coeff.mo.ws_a);
                } else {
                    mpFp_mul(M, M, pt->cvp->coeff.ws.a);
                }
                mpFp_mul_ui(T, XX, 3);
                mpFp_add(M, M, T);
                // T = M**2-2*S, XX is temp var from here down
                mpFp_pow_ui(T, M, 2);
                mpFp_mul_ui(XX, S, 2);
                mpFp_sub(T, T, XX);
                // X3 = T
                mpFp_set(rpt->x, T);
                // Y3 = M*(S-T)-8*YYYY
                mpFp_mul_ui(YYYY, YYYY, 8);
                mpFp_sub(S, S, T);
                mpFp_mul(M, M, S);
                mpFp_sub(S, M, YYYY);
                // Z3 = (Y1+Z1)**2-YY-ZZ
                mpFp_add(M, pt->y, pt->z);
                mpFp_pow_ui(M, M, 2);
                mpFp_sub(M, M, YY);
                mpFp_sub(rpt->z, M, ZZ);
                mpFp_set(rpt->y, S);
                //
                // done... clean up temporary variables
                mpFp_clear(T);
                mpFp_clear(M);
                mpFp_clear(S);
                mpFp_clear(ZZ);
                mpFp_clear(YYYY);
                mpFp_clear(YY);
                mpFp_clear(XX);
                mpECurve_set(rpt->cvp, pt->cvp);
                rpt->is_neutral = 0;
                return;
            }
            break;
#endif
        case EQTypeEdwards:
        case EQTypeTwistedEdwards: {
               // Edwards addition law is complete, so can use add for double
                mpECP_add(rpt, pt, pt);
                return;
            }
            break;
        default:
            assert(_known_curve_type(pt->cvp));
    }
    assert(0);
}

void mpECP_sub(mpECP_t rpt, mpECP_t pt1, mpECP_t pt2) {
    assert(mpECurve_cmp(pt1->cvp, pt2->cvp) == 0);
    if (pt2->is_neutral != 0) {
        if (pt1->is_neutral != 0) {
            mpECP_set_neutral(rpt, pt1->cvp);
        } else {
            mpECP_set(rpt, pt1);
        }
    } else {
        mpECP_t n;
        mpECP_init(n, pt1->cvp);
        mpECP_neg(n, pt2);
        mpECP_add(rpt, pt1, n);
        mpECP_clear(n);
    }
    return;
}

void mpECP_scalar_mul(mpECP_t rpt, mpECP_t pt, mpFp_t sc) {
    int i, b;
    mpECP_t R0, R1;
    mpECP_init(R0, pt->cvp);
    mpECP_init(R1, pt->cvp);
    mpECP_set_neutral(R0, pt->cvp);
    mpECP_set(R1, pt);
    // scalar should be modulo the order of the curve
    assert(mpz_cmp(sc->fp->p, pt->cvp->n) == 0);
    for (i = pt->cvp->bits - 1; i >= 0 ; i--) {
        b = mpFp_tstbit(sc, i);
        _mpECP_cswap_safe(R0, R1, b);
        mpECP_add(R1, R1, R0);
        mpECP_double(R0, R0);
        _mpECP_cswap_safe(R0, R1, b);
    }
    mpECP_set(rpt, R0);
    mpECP_clear(R1);
    mpECP_clear(R0);
    return;
}

void mpECP_scalar_mul_mpz(mpECP_t rpt, mpECP_t pt, mpz_t sc) {
    mpFp_t s;
    mpFp_init_fp(s, pt->cvp->fp);
    mpFp_set_mpz(s, sc, pt->cvp->n);
    mpECP_scalar_mul(rpt, pt, s);
    mpFp_clear(s);
    return;
}

void mpECP_scalar_base_mul_setup(mpECP_t pt) {
    int i, j, npts, nlevels, levelsz;
    mpECP_t a, b;
    struct _p_mpECP_t *level_pt;
    struct _p_mpECP_t *base_pt;
    if (pt->base_bits != 0) {
        // already set up... 
        return;
    }
    mpECP_init(a, pt->cvp);
    mpECP_init(b, pt->cvp);
    assert(pt->base_bits == 0);
    pt->base_bits = _MPECP_BASE_BITS;
    npts = _mpECP_n_base_pts(pt);
    base_pt = (struct _p_mpECP_t *)malloc(npts * sizeof(struct _p_mpECP_t));
    for (i = 0; i < npts; i++) {
        mpECP_init(&base_pt[i], pt->cvp);
    }
    level_pt = (struct _p_mpECP_t *)malloc(pt->base_bits * sizeof(struct _p_mpECP_t));
    for (i = 0; i < pt->base_bits; i++) {
        mpECP_init(&(level_pt[i]), pt->cvp);
    }
    nlevels = _mpECP_n_base_pt_levels(pt);
    levelsz = _mpECP_n_base_pt_level_size(pt);
    // printf("setup: levels = %d, levelsz = %d\n", nlevels, levelsz);
    mpECP_set(a, pt);
    for (j = 0; j < nlevels; j++) {
        mpECP_set_neutral(b, pt->cvp);
        for (i = 0; i < levelsz; i++) {
            mpECP_set(&base_pt[(j * levelsz) + i], b);
            mpECP_add(b, b, a);
        }
        mpECP_set(a, b);
    }
    pt->base_pt = base_pt;
    mpECP_clear(b);
    mpECP_clear(a);
    return;
}

void mpECP_scalar_base_mul(mpECP_t rpt, mpECP_t pt, mpFp_t sc) {
    int j, k, nlevels, levelsz;
    mpz_t s, kmpz;
    mpECP_t a;
    assert (mpz_cmp(sc->fp->p, pt->cvp->n) == 0);
    if (pt->base_bits == 0) {
        mpECP_scalar_base_mul_setup(pt);
    }
    mpz_init(s);
    mpz_init(kmpz);
    mpECP_init(a, pt->cvp);
    mpECP_set_neutral(a, pt->cvp);
    mpz_set_mpFp(s, sc);
    nlevels = _mpECP_n_base_pt_levels(pt);
    levelsz = _mpECP_n_base_pt_level_size(pt);
    for (j = 0; j < nlevels; j++) {
        mpz_mod_ui(kmpz, s, levelsz);
        k = mpz_get_ui(kmpz);
        mpECP_add(a, a, &pt->base_pt[(j * levelsz) + k]);
        mpz_tdiv_q_ui(s, s, levelsz);
    }
    mpECP_set(rpt, a);
    mpECP_clear(a);
    mpz_clear(kmpz);
    mpz_clear(s);
    return;
}

void mpECP_scalar_base_mul_mpz(mpECP_t rpt, mpECP_t pt, mpz_t s) {
    mpFp_t sc;
    mpFp_init_fp(sc, pt->cvp->fp);
    mpFp_set_mpz(sc, s, pt->cvp->n);
    mpECP_scalar_base_mul(rpt, pt, sc);
    mpFp_clear(sc);
    return;
}

void mpECP_urandom(mpECP_t rpt, mpECurve_t cv) {
    mpFp_t a;
    mpECP_t g;
    mpFp_init_fp(a, cv->fp);
    mpECP_init(g, cv);
    mpECP_set_mpz(g, cv->G[0], cv->G[1], cv);
    mpFp_urandom(a, cv->n);
    mpECP_scalar_mul(rpt, g, a);
    mpECP_clear(g);
    mpFp_clear(a);
    return;
}
