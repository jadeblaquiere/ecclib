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
#include <ecpoint.h>
#include <ecurve.h>
#include <field.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _MPECP_BASE_BITS    (8)

static inline int _mpECP_n_base_pt_levels(mpECP_t pt) {
    return (pt->cv->bits + pt->base_bits - 1) / pt->base_bits;
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

void mpECP_init(mpECP_t pt) {
    mpFp_init(pt->x);
    mpFp_init(pt->y);
    mpFp_init(pt->z);
    mpECurve_init(pt->cv);
    pt->base_bits = 0;
    pt->base_pt = NULL;
    return;
}

void mpECP_clear(mpECP_t pt) {
    if (pt->base_bits != 0) _mpECP_base_pts_cleanup(pt);
    mpFp_clear(pt->x);
    mpFp_clear(pt->y);
    mpFp_clear(pt->z);
    mpECurve_clear(pt->cv);
    return;
}

void mpECP_set(mpECP_t rpt, mpECP_t pt) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    mpECurve_set(rpt->cv, pt->cv);
    rpt->is_neutral = pt->is_neutral;
    mpFp_set(rpt->x, pt->x);
    mpFp_set(rpt->y, pt->y);
    mpFp_set(rpt->z, pt->z);
    return;
}

void mpECP_set_mpz(mpECP_t rpt, mpz_t x, mpz_t y, mpECurve_t cv) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    mpECurve_set(rpt->cv, cv);
    rpt->is_neutral = 0;
    mpFp_set_mpz(rpt->x, x, cv->p);
    mpFp_set_mpz(rpt->y, y, cv->p);
    mpFp_set_ui(rpt->z, 1, cv->p);
    return;
}

void mpECP_set_mpFp(mpECP_t rpt, mpFp_t x, mpFp_t y, mpECurve_t cv) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    mpECurve_set(rpt->cv, cv);
    rpt->is_neutral = 0;
    mpFp_set(rpt->x, x);
    mpFp_set(rpt->y, y);
    mpFp_set_ui(rpt->z, 1, cv->p);
    return;
}

void mpECP_set_neutral(mpECP_t rpt, mpECurve_t cv) {
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    mpECurve_set(rpt->cv, cv);
    switch (cv->type) {
    case EQTypeShortWeierstrass:
        rpt->is_neutral = 1;
        mpFp_set_ui(rpt->x, 0, cv->p);
        mpFp_set_ui(rpt->y, 1, cv->p);
        mpFp_set_ui(rpt->z, 0, cv->p);
        return;
    case EQTypeEdwards:
        // return the neutral element (which is a valid curve point 0,c)
        rpt->is_neutral = 0;
        mpFp_set_ui(rpt->z, 1, cv->p);
        mpFp_set_ui(rpt->x, 0, cv->p);
        mpFp_set(rpt->y, cv->coeff.ed.c);
        return;
    case EQTypeMontgomery:
        rpt->is_neutral = 1;
        mpFp_set_ui(rpt->x, 1, cv->p);
        mpFp_set_ui(rpt->z, 0, cv->p);
        return;
    case EQTypeTwistedEdwards:
        // return the neutral element (which is a valid curve point 0,1)
        rpt->is_neutral = 0;
        mpFp_set_ui(rpt->x, 0, cv->p);
        mpFp_set_ui(rpt->y, 1, cv->p);
        mpFp_set_ui(rpt->z, 1, cv->p);
        return;
    default:
        assert(_known_curve_type(cv));
    }
}

void _mpECP_to_affine(mpECP_t pt) {
    mpFp_t zinv, t;
    if (mpFp_cmp_ui(pt->z, 1) == 0) {
        return;
    }
    mpFp_init(zinv);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
            if (pt->is_neutral != 0) break;
            // Jacobian coords x = X/Z**2 y = Y/Z**3);
            mpFp_init(t);
            mpFp_inv(zinv, pt->z);
            mpFp_pow_ui(t, zinv, 2);
            mpFp_mul(pt->x, pt->x, t);
            mpFp_pow_ui(t, zinv, 3);
            mpFp_mul(pt->y, pt->y, t);
            mpFp_set_ui(pt->z, 1, pt->cv->p);
            mpFp_clear(t);
            break;
        case EQTypeEdwards:
            // Projective x = X/Z y = Y/Z
            mpFp_inv(zinv, pt->z);
            mpFp_mul(pt->x, pt->x, zinv);
            mpFp_mul(pt->y, pt->y, zinv);
            mpFp_set_ui(pt->z, 1, pt->cv->p);
            break;
        case EQTypeMontgomery:
            // Projective (XZ) x = X/Z
            mpFp_inv(zinv, pt->z);
            mpFp_mul(pt->x, pt->x, zinv);
            mpFp_set_ui(pt->z, 1, pt->cv->p);
            break;
        case EQTypeTwistedEdwards:
            // Projective x = X/Z y = Y/Z
            mpFp_inv(zinv, pt->z);
            mpFp_mul(pt->x, pt->x, zinv);
            mpFp_mul(pt->y, pt->y, zinv);
            mpFp_set_ui(pt->z, 1, pt->cv->p);
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    mpFp_clear(zinv);
    return;
}

void mpFp_set_mpECP_affine_x(mpFp_t x, mpECP_t pt) {
    _mpECP_to_affine(pt);
    mpFp_set(x, pt->x);
    return;
}

void mpFp_set_mpECP_affine_y(mpFp_t y, mpECP_t pt) {
    _mpECP_to_affine(pt);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            mpFp_set(y, pt->y);
            break;
        case EQTypeMontgomery: {
                // y coordinate is not preserved, can calculate if you insist
                // (please don't insist, you shouldn't need this value)
                // B * y**2 = x**3 + A * x**2 + x
                mpFp_t Binv, A, t;
                mpFp_init(Binv);
                mpFp_init(A);
                mpFp_init(t);
                // right side of curve equation
                mpFp_pow_ui(t, pt->x, 3);
                mpFp_pow_ui(A, pt->x, 2);
                mpFp_mul(A, A, pt->cv->coeff.mo.A);
                mpFp_add(t, t, A);
                mpFp_add(t, t, pt->x);
                // divide by B
                mpFp_inv(Binv, pt->cv->coeff.mo.B);
                mpFp_mul(t, t, Binv);
                // take square root - ignore +/-
                mpFp_sqrt(y, t);
                mpFp_clear(t);
                mpFp_clear(A);
                mpFp_clear(Binv);
            }
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    return;
}

void mpz_set_mpECP_affine_x(mpz_t x, mpECP_t pt) {
    _mpECP_to_affine(pt);
    mpz_set_mpFp(x, pt->x);
    return;
}

void mpz_set_mpECP_affine_y(mpz_t y, mpECP_t pt) {
    _mpECP_to_affine(pt);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            mpz_set_mpFp(y, pt->y);
            break;
        case EQTypeMontgomery: {
                mpFp_t t;
                mpFp_init(t);
                mpFp_set_mpECP_affine_y(t, pt);
                mpz_set_mpFp(y, t);
                mpFp_clear(t);
            }
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    return;
}

static inline int _bytelen(int bits) {
    return ((bits + 7) >> 3);
}

int mpECP_set_str(mpECP_t rpt, char *s, mpECurve_t cv) {
    int bytes;
    char *buffer;
    
    bytes = _bytelen(cv->bits);
    buffer = (char *)malloc((strlen(s) + 1) * sizeof(char));
    assert(buffer != NULL);
    strcpy(buffer, s);
    switch (cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            if (strlen(s) < (2 + (2*bytes))) return -1;
            if (s[0] != '0') return -1;
            switch (s[1]) {
                case '0':
                    mpECP_set_neutral(rpt, cv);
                    return 0;
                case '4': {
                        if (strlen(s) != (2 + (4 * bytes))) return -1;
                        mpz_t x, y;
                        mpz_init(x);
                        mpz_init(y);
                        gmp_sscanf(&s[2+(2*bytes)],"%ZX", y);
                        buffer[2+(2*bytes)] = 0;
                        gmp_sscanf(&buffer[2],"%ZX", x);
                        mpECP_set_mpz(rpt, x, y, cv);
                        mpz_clear(y);
                        mpz_clear(x);
                    }
                    return 0;
                case '2':
                case '3': {
                        mpz_t xz;
                        mpFp_t x, y;
                        mpFp_t t;
                        int error, odd;
                        if (strlen(s) != (2 + (2 * bytes))) return -1;
                        mpz_init(xz);
                        mpFp_init(x);
                        mpFp_init(y);
                        mpFp_init(t);
                        gmp_sscanf(&s[2],"%ZX", xz);
                        mpFp_set_mpz(x, xz, cv->p);
                        if (cv->type == EQTypeShortWeierstrass) {
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
                        } else if (cv->type == EQTypeEdwards) {
                            mpFp_t c2, x2;
                            mpFp_init(c2);
                            mpFp_init(x2);
                            // x**2 + y**2 = c**2 (1 + d * x**2 * y**2)
                            // y**2 - C**2 * d * x**2 * y**2 = c**2 - x**2
                            // y**2 = (c**2 - x**2) / (1 - c**2 * d * x**2)
                            mpFp_mul(c2, cv->coeff.ed.c, cv->coeff.ed.c);
                            mpFp_mul(t, cv->coeff.ed.d, c2);
                            mpFp_pow_ui(x2, x, 2);
                            mpFp_mul(t, t, x2);
                            mpFp_set_ui(y, 1, cv->p);
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
                        } else {
                            mpFp_t a, x2;
                            mpFp_init(a);
                            mpFp_init(x2);
                            // a * x**2 + y**2 = 1 + d * x**2 * y**2
                            // y**2 - d * x**2 * y**2 = 1 - a * x**2
                            // y**2 = (1 - a * x**2) / (1 - d * x**2)
                            mpFp_set(a, cv->coeff.te.a);
                            mpFp_set(t, cv->coeff.te.d);
                            mpFp_pow_ui(x2, x, 2);
                            mpFp_mul(t, t, x2);
                            mpFp_set_ui(y, 1, cv->p);
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
                        odd = mpz_tstbit(y->i, 0);
                        // '3' implies odd, '2' even... negate if not matched
                        if ((((s[1] - '2') + odd) & 1) == 1) {
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
            break;
        case EQTypeMontgomery: {
                mpz_t x;
                mpz_init(x);
                if (strlen(s) != (2* bytes)) return -1;
                gmp_sscanf(s,"%ZX", x);
                if (mpz_cmp_ui(x,0) == 0) {
                    mpECP_set_neutral(rpt, cv);
                } else {
                    mpECurve_set(rpt->cv, cv);
                    rpt->is_neutral = 0; 
                    mpFp_set_mpz(rpt->x, x, cv->p);
                    mpFp_set_ui(rpt->z, 1, cv->p);
                }
                mpz_clear(x);
            }
            return 0;
        default:
            assert(_known_curve_type(cv));
    }
    assert(0);
}

int  mpECP_out_strlen(mpECP_t pt, int compress) {
    int bytes;
    bytes = _bytelen(pt->cv->bits);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            // use common prefix (02, 03, 04) and then either X or X and Y
            if (compress == 0) {
                return (2 + (4 * bytes));
            } else {
                return (2 + (2 * bytes));
            }
            break;
        case EQTypeMontgomery:
            // Montgomery compressed representation, following Curve25519 model
            // is simply the x coordinate in hexadecimal
            return (2 * bytes);
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    assert(0);
}

void mpECP_out_str(char *s, mpECP_t pt, int compress) {
    int i, bytes;
    char format[32];
    _mpECP_to_affine(pt);
    bytes = _bytelen(pt->cv->bits);
    // print format for n-bit (big endian) hexadecimal numbers (w/o '0x')
    sprintf(format,"%%0%dZX", (bytes * 2));
    //printf("mp_ECP_out_str format: %s\n", format);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            s[0] = '0';
            if (pt->is_neutral) {
                if (compress == 0) bytes *= 2;
                for (i = 1; i < ((2*bytes) + 2); i++) {
                    s[i] = '0';
                }
                s[2 + (2 * bytes)] = 0;
                return;
            }
            if (compress != 0) {
                mpz_t odd;
                mpz_init(odd);
                mpz_set_mpFp(odd, pt->y);
                mpz_mod_ui(odd, odd, 2);
                if (mpz_cmp_ui(odd, 1) == 0) {
                    s[1] = '3';
                } else {
                    s[1] = '2';
                }
                mpz_clear(odd);
            } else {
                s[1] = '4';
            }
            //s[2] = 0;
            //printf("prefix = %s\n", s);
            //gmp_printf(format, pt->x->i);
            //printf("\n");
            gmp_sprintf(&s[2], format, pt->x->i);
            //printf("as string = %s\n", s);
            if (compress == 0) {
                gmp_sprintf(&s[2 + (2 * bytes)], format, pt->y->i);
                s[2 + (4 * bytes)] = 0;
            } else {
                s[2 + (2 * bytes)] = 0;
            }
            break;
        case EQTypeMontgomery:
            gmp_sprintf(&s[0], format, pt->x->i);
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    //printf("exported as %s\n", s);
    return;
}

void mpECP_neg(mpECP_t rpt, mpECP_t pt) {
    if (pt->is_neutral != 0) {
        mpECP_set_neutral(rpt, pt->cv);
        return;
    }
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
            mpECP_set(rpt, pt);
            mpFp_neg(rpt->y, pt->y);
            break;
        case EQTypeEdwards:
        case EQTypeTwistedEdwards:
            mpECP_set(rpt, pt);
            mpFp_neg(rpt->x, pt->x);
            break;
        case EQTypeMontgomery:
            // negation is not relevant in XZ projective representation
            assert(0);
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    return;
}

int mpECP_cmp(mpECP_t pt1, mpECP_t pt2) {
    if (mpECurve_cmp(pt1->cv, pt2->cv) != 0) return -1;
    if (pt1->is_neutral != 0) {
        if (pt2->is_neutral != 0) {
            return 0;
        }
        return -1;
    } else if (pt2->is_neutral != 0) {
        return -1;
    }
    switch (pt1->cv->type) {
        case EQTypeShortWeierstrass: {
                mpFp_t U1, U2;
                mpFp_init(U1);
                mpFp_init(U2);
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
        case EQTypeEdwards:
        case EQTypeTwistedEdwards: {
                mpFp_t U1, U2;
                mpFp_init(U1);
                mpFp_init(U2);
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
        case EQTypeMontgomery: {
                mpFp_t U1, U2;
                mpFp_init(U1);
                mpFp_init(U2);
                mpFp_mul(U1, pt2->z, pt1->x);
                mpFp_mul(U2, pt1->z, pt2->x);
                if (mpFp_cmp(U1, U2) != 0) return -1;
                mpFp_clear(U2);
                mpFp_clear(U1);
            }
            break;
        default:
            assert(_known_curve_type(pt1->cv));
    }
    return 0;
}

void mpECP_swap(mpECP_t pt2, mpECP_t pt1) {
    int t;
    struct _p_mpECP_t *t_base_pt;
    assert(mpECurve_cmp(pt1->cv, pt2->cv) == 0);
    mpFp_swap(pt2->x, pt1->x);
    mpFp_swap(pt2->y, pt1->y);
    mpFp_swap(pt2->z, pt1->z);
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

void mpECP_cswap(mpECP_t pt2, mpECP_t pt1, int swap) {
    volatile unsigned long s, ns;
    int a, b;
    assert(mpECurve_cmp(pt1->cv, pt2->cv) == 0);
    assert(pt1->base_bits == 0);
    assert(pt2->base_bits == 0);
    
    if (swap != 0) {
        s = 1;
        ns = 0;
    } else {
        s = 0;
        ns = 1;
    }
    
    mpFp_cswap(pt2->x, pt1->x, swap);
    mpFp_cswap(pt2->y, pt1->y, swap);
    mpFp_cswap(pt2->z, pt1->z, swap);
    
    a = (s * pt1->is_neutral) + (ns * pt2->is_neutral);
    b = (ns * pt1->is_neutral) + (s * pt2->is_neutral);
    
    pt2->is_neutral = a;
    pt1->is_neutral = b;

    return;
}

void mpECP_add(mpECP_t rpt, mpECP_t pt1, mpECP_t pt2) {
    assert(mpECurve_cmp(pt1->cv, pt2->cv) == 0);
    if (pt1->is_neutral != 0) {
        if (pt2->is_neutral != 0) {
            mpECP_set_neutral(rpt, pt1->cv);
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
    switch (pt1->cv->type) {
        case EQTypeShortWeierstrass: {
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
                mpFp_init(Z1Z1);
                mpFp_init(Z2Z2);
                mpFp_init(U1);
                mpFp_init(U2);
                mpFp_init(S1);
                mpFp_init(S2);
                mpFp_init(H);
                mpFp_init(I);
                mpFp_init(J);
                mpFp_init(V);
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
                        mpECP_set_neutral(rpt, pt1->cv);
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
                    mpECurve_set(rpt->cv, pt1->cv);
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
                mpFp_init(A);
                mpFp_init(B);
                mpFp_init(C);
                mpFp_init(D);
                mpFp_init(E);
                mpFp_init(F);
                mpFp_init(G);
                
                // A = Z1*Z2
                mpFp_mul(A, pt1->z, pt2->z);
                // B = A**2
                mpFp_pow_ui(B, A, 2);
                // C = X1*X2
                mpFp_mul(C, pt1->x, pt2->x);
                // D = Y1*Y2
                mpFp_mul(D, pt1->y, pt2->y);
                // E = d*C*D
                mpFp_mul(E, pt1->cv->coeff.ed.d, C);
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
                mpFp_mul(B, pt1->cv->coeff.ed.c, G);
                mpFp_mul(rpt->z, B, F);
                mpECurve_set(rpt->cv, pt1->cv);
                rpt->is_neutral = 0;
                
                mpFp_clear(G);
                mpFp_clear(F);
                mpFp_clear(E);
                mpFp_clear(D);
                mpFp_clear(C);
                mpFp_clear(B);
                mpFp_clear(A);
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
                mpFp_init(A);
                mpFp_init(B);
                mpFp_init(C);
                mpFp_init(D);
                mpFp_init(E);
                mpFp_init(F);
                mpFp_init(G);
                
                // A = Z1*Z2
                mpFp_mul(A, pt1->z, pt2->z);
                // B = A**2
                mpFp_mul(B, A, A);
                // C = X1*X2
                mpFp_mul(C, pt1->x, pt2->x);
                // D = Y1*Y2
                mpFp_mul(D, pt1->y, pt2->y);
                // E = d*C*D
                mpFp_mul(E, pt1->cv->coeff.te.d, C);
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
                mpFp_mul(C, C, pt1->cv->coeff.te.a);
                mpFp_sub(B, D, C);
                mpFp_mul(B, B, G);
                mpFp_mul(rpt->y, B, A);
                // Z3 = F*G
                mpFp_mul(rpt->z, G, F);
                mpECurve_set(rpt->cv, pt1->cv);
                rpt->is_neutral = 0;
                
                mpFp_clear(G);
                mpFp_clear(F);
                mpFp_clear(E);
                mpFp_clear(D);
                mpFp_clear(C);
                mpFp_clear(B);
                mpFp_clear(A);
                return;
            }
            break;
        case EQTypeMontgomery:
            break;
        default:
            assert(_known_curve_type(pt1->cv));
    }
    assert(0);
}

void mpECP_double(mpECP_t rpt, mpECP_t pt) {
    if (pt->is_neutral != 0) {
        mpECP_set_neutral(rpt, pt->cv);
        return;
    }
    if (rpt->base_bits != 0) _mpECP_base_pts_cleanup(rpt);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass: {
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
                mpFp_init(XX);
                mpFp_init(YY);
                mpFp_init(YYYY);
                mpFp_init(ZZ);
                mpFp_init(S);
                mpFp_init(M);
                mpFp_init(T);
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
                mpFp_mul(M, M, pt->cv->coeff.ws.a);
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
                mpECurve_set(rpt->cv, pt->cv);
                rpt->is_neutral = 0;
                return;
            }
            break;
        case EQTypeEdwards:
        case EQTypeTwistedEdwards: {
               // Edwards addition law is complete, so can use add for double
                mpECP_add(rpt, pt, pt);
                return;
            }
            break;
        case EQTypeMontgomery:
            break;
        default:
            assert(_known_curve_type(pt->cv));
    }
    assert(0);
}

void mpECP_sub(mpECP_t rpt, mpECP_t pt1, mpECP_t pt2) {
    assert(mpECurve_cmp(pt1->cv, pt2->cv) == 0);
    if (pt2->is_neutral != 0) {
        if (pt1->is_neutral != 0) {
            mpECP_set_neutral(rpt, pt1->cv);
        } else {
            mpECP_set(rpt, pt1);
        }
    } else {
        mpECP_t n;
        mpECP_init(n);
        mpECP_neg(n, pt2);
        mpECP_add(rpt, pt1, n);
        mpECP_clear(n);
    }
    return;
}

void mpECP_scalar_mul(mpECP_t rpt, mpECP_t pt, mpFp_t sc) {
    int i, b;
    mpECP_t R0, R1;
    mpECP_init(R0);
    mpECP_init(R1);
    mpECP_set_neutral(R0, pt->cv);
    mpECP_set(R1, pt);
    // scalar should be modulo the order of the curve
    assert(mpz_cmp(sc->p, pt->cv->n) == 0);
    for (i = pt->cv->bits - 1; i >= 0 ; i--) {
        b = mpFp_tstbit(sc, i);
        //printf("bit %d = %d\n", i, b);
        mpECP_cswap(R0, R1, b);
        mpECP_add(R1, R1, R0);
        mpECP_double(R0, R0);
        mpECP_cswap(R0, R1, b);
    }
    mpECP_set(rpt, R0);
    mpECP_clear(R1);
    mpECP_clear(R0);
    return;
}

void mpECP_scalar_mul_mpz(mpECP_t rpt, mpECP_t pt, mpz_t sc) {
    mpFp_t s;
    mpFp_init(s);
    mpFp_set_mpz(s, sc, pt->cv->n);
    mpECP_scalar_mul(rpt, pt, s);
    mpFp_clear(s);
    return;
}

void mpECP_scalar_base_mul_setup(mpECP_t pt) {
    int i, j, k, npts, nlevels, levelsz;
    mpECP_t last, a, b;
    struct _p_mpECP_t *base_pt;
    if (pt->base_bits != 0) {
        // already set up... 
        return;
    }
    mpECP_init(last);
    mpECP_init(a);
    mpECP_init(b);
    assert(pt->base_bits == 0);
    pt->base_bits = _MPECP_BASE_BITS;
    npts = _mpECP_n_base_pts(pt);
    base_pt = (struct _p_mpECP_t *)malloc(npts * sizeof(struct _p_mpECP_t));
    for (i = 0; i < npts; i++) {
        mpECP_init(&base_pt[i]);
    }
    nlevels = _mpECP_n_base_pt_levels(pt);
    levelsz = _mpECP_n_base_pt_level_size(pt);
    // printf("setup: levels = %d, levelsz = %d\n", nlevels, levelsz);
    mpECP_set(last, pt);
    for (j = 0; j < nlevels; j++) {
        for (i = 0; i < levelsz; i++) {
            mpECP_set(a, last);
            mpECP_set_neutral(b, pt->cv);
            k = i;
            while (k > 0) {
                if ((k % 2) == 1) {
                    mpECP_add(b, b, a);
                }
                mpECP_double(a, a);
                k >>= 1;
            }
            mpECP_set(&base_pt[(j * levelsz) + i], b);
        }
        for (i = 0 ; i < pt->base_bits; i++) {
            mpECP_double(last, last);
        }
    }
    pt->base_pt = base_pt;
    mpECP_clear(b);
    mpECP_clear(a);
    mpECP_clear(last);
    return;
}

void mpECP_scalar_base_mul(mpECP_t rpt, mpECP_t pt, mpFp_t sc) {
    int j, k, nlevels, levelsz;
    mpz_t s, kmpz;
    mpECP_t a;
    assert (mpz_cmp(sc->p, pt->cv->n) == 0);
    mpECP_scalar_base_mul_setup(pt);
    mpz_init(s);
    mpz_init(kmpz);
    mpECP_init(a);
    mpECP_set_neutral(a, pt->cv);
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
    mpFp_init(sc);
    mpFp_set_mpz(sc, s, pt->cv->n);
    mpECP_scalar_base_mul(rpt, pt, sc);
    mpFp_clear(sc);
    return;
}


void mpECP_urandom(mpECP_t rpt, mpECurve_t cv) {
    mpFp_t a;
    mpECP_t g;
    mpFp_init(a);
    mpECP_init(g);
    mpECP_set_mpz(g, cv->G[0], cv->G[1], cv);
    mpFp_urandom(a, cv->n);
    mpECP_scalar_mul(rpt, g, a);
    mpECP_clear(g);
    mpFp_clear(a);
    return;
}
