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
#include <ecurve.h>
#include <field.h>
#include <gmp.h>
#include <ecpoint.h>
#include <stdio.h>
#include <string.h>

void mpECP_init(mpECP_t pt) {
    mpFp_init(pt->x);
    mpFp_init(pt->y);
    mpFp_init(pt->z);
    mpECurve_init(pt->cv);
    return;
}

void mpECP_clear(mpECP_t pt) {
    mpFp_clear(pt->x);
    mpFp_clear(pt->y);
    mpFp_clear(pt->z);
    mpECurve_clear(pt->cv);
    return;
}

void mpECP_set(mpECP_t rop, mpECP_t op) {
    mpECurve_set(rop->cv, op->cv);
    rop->is_infinite = op->is_infinite;
    mpFp_set(rop->x, op->x);
    mpFp_set(rop->y, op->y);
    mpFp_set(rop->z, op->z);
    return;
}

void mpECP_set_mpz(mpECP_t rop, mpz_t x, mpz_t y, mpECurve_t cv) {
    mpECurve_set(rop->cv, cv);
    rop->is_infinite = 0;
    mpFp_set_mpz(rop->x, x, cv->p);
    mpFp_set_mpz(rop->y, y, cv->p);
    mpFp_set_ui(rop->z, 1, cv->p);
    return;
}

void mpECP_set_mpFp(mpECP_t rop, mpFp_t x, mpFp_t y, mpECurve_t cv) {
    mpECurve_set(rop->cv, cv);
    rop->is_infinite = 0;
    mpFp_set(rop->x, x);
    mpFp_set(rop->y, y);
    mpFp_set_ui(rop->z, 1, cv->p);
    return;
}

void mpECP_set_infinite(mpECP_t rpt, mpECurve_t cv) {
    mpECurve_set(rpt->cv, cv);
    switch (cv->type) {
    case EQTypeEdwards:
        // return the neutral element (which is a valid curve point 0,c)
        rpt->is_infinite = 1;
        mpFp_set_ui(rpt->z, 1, cv->p);
        mpFp_set_ui(rpt->x, 0, cv->p);
        mpFp_set_mpz(rpt->y, cv->coeff.ed.c, cv->p);
        return;
    case EQTypeShortWeierstrass:
        rpt->is_infinite = 1;
        mpFp_set_ui(rpt->x, 0, cv->p);
        mpFp_set_ui(rpt->y, 1, cv->p);
        mpFp_set_ui(rpt->z, 0, cv->p);
        return;
    case EQTypeMontgomery:
        rpt->is_infinite = 1;
        mpFp_set_ui(rpt->x, 1, cv->p);
        mpFp_set_ui(rpt->z, 0, cv->p);
        return;
    default:
        assert((cv->type == EQTypeShortWeierstrass) || (cv->type == EQTypeEdwards) || (cv->type == EQTypeMontgomery));
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
            if (pt->is_infinite != 0) break;
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
        default:
            assert((pt->cv->type == EQTypeShortWeierstrass) || (pt->cv->type == EQTypeEdwards) || (pt->cv->type == EQTypeMontgomery));
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
            mpFp_set(y, pt->y);
            break;
        case EQTypeMontgomery: {
                // y coordinate is not preserved, can calculate if you insist
                // (please don't insist)
                // B * y**2 = x**3 + A * x**2 + x
                mpFp_t Binv, A, t;
                mpFp_init(Binv);
                mpFp_init(A);
                mpFp_init(t);
                // right side of curve equation
                mpFp_pow_ui(t, pt->x, 3);
                mpFp_pow_ui(A, pt->x, 2);
                mpFp_set_mpz(Binv, pt->cv->coeff.mo.A, pt->cv->p);
                mpFp_mul(A, A, Binv);
                mpFp_add(t, t, A);
                mpFp_add(t, t, pt->x);
                // divide by B
                mpFp_set_mpz(Binv, pt->cv->coeff.mo.B, pt->cv->p);
                mpFp_inv(Binv, Binv);
                mpFp_mul(t, t, Binv);
                // take square root - ignore +/-
                mpFp_sqrt(y, t);
                mpFp_clear(t);
                mpFp_clear(A);
                mpFp_clear(Binv);
            }
            break;
        default:
            assert((pt->cv->type == EQTypeShortWeierstrass) || (pt->cv->type == EQTypeEdwards) || (pt->cv->type == EQTypeMontgomery));
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
            assert((pt->cv->type == EQTypeShortWeierstrass) || (pt->cv->type == EQTypeEdwards) || (pt->cv->type == EQTypeMontgomery));
    }
    return;
}

static inline int _bytelen(int bits) {
    return ((bits + 7) >> 3);
}

int mpECP_set_str(mpECP_t rpt, char *s, mpECurve_t cv) {
    int bytes;
    bytes = _bytelen(cv->bits);
    switch (cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
            if (strlen(s) < (2 + (2*bytes))) return -1;
            if (s[0] != '0') return -1;
            switch (s[1]) {
                case '0':
                    mpECP_set_infinite(rpt, cv);
                    return 0;
                case '4': {
                        if (strlen(s) != (2 + (4 * bytes))) return -1;
                        mpz_t x, y;
                        mpz_init(x);
                        mpz_init(y);
                        gmp_sscanf(&s[2+(2*bytes)],"%ZX", y);
                        s[2+(2*bytes)] = 0;
                        gmp_sscanf(&s[2],"%ZX", x);
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
                            mpFp_set_mpz(y, cv->coeff.ws.b, cv->p);
                            mpFp_set_mpz(t, cv->coeff.ws.a, cv->p);
                            mpFp_mul(t, t, x);
                            mpFp_add(y, y, t);
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
                        } else {
                            mpFp_t c2, x2;
                            mpFp_init(c2);
                            mpFp_init(x2);
                            // x**2 + y**2 = c**2 (1 + d * x**2 * y**2)
                            // y**2 - C**2 * d * x**2 * y**2 = c**2 - x**2
                            // y**2 = (c**2 - x**2) / (1 - c**2 * d * x**2)
                            mpFp_set_mpz(c2, cv->coeff.ed.c, cv->p);
                            mpFp_pow_ui(c2, c2, 2);
                            mpFp_set_mpz(t, cv->coeff.ed.d, cv->p);
                            mpFp_mul(t, t, c2);
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
                    mpECP_set_infinite(rpt, cv);
                } else {
                    mpECurve_set(rpt->cv, cv);
                    rpt->is_infinite = 0; 
                    mpFp_set_mpz(rpt->x, x, cv->p);
                    mpFp_set_ui(rpt->z, 1, cv->p);
                }
                mpz_clear(x);
            }
            return 0;
        default:
            assert((cv->type == EQTypeShortWeierstrass) || (cv->type == EQTypeEdwards) || (cv->type == EQTypeMontgomery));
    }
    assert(0);
}

int  mpECP_out_strlen(mpECP_t pt, int compress) {
    int bytes;
    bytes = _bytelen(pt->cv->bits);
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
        case EQTypeEdwards:
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
            assert((pt->cv->type == EQTypeShortWeierstrass) || (pt->cv->type == EQTypeEdwards) || (pt->cv->type == EQTypeMontgomery));
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
            s[0] = '0';
            if (pt->is_infinite) {
                if (compress == 0) bytes *= 2;
                for (i = 1; i < ((2*bytes) + 2); i++) {
                    s[i] = '0';
                }
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
            assert((pt->cv->type == EQTypeShortWeierstrass) || (pt->cv->type == EQTypeEdwards) || (pt->cv->type == EQTypeMontgomery));
    }
    //printf("exported as %s\n", s);
    return;
}

void mpECP_neg(mpECP_t rpt, mpECP_t pt) {
    if (pt->is_infinite != 0) {
        mpECP_set_infinite(rpt, pt->cv);
        return;
    }
    switch (pt->cv->type) {
        case EQTypeShortWeierstrass:
            mpECP_set(rpt, pt);
            mpFp_neg(rpt->y, pt->y);
            break;
        case EQTypeEdwards:
            mpECP_set(rpt, pt);
            mpFp_neg(rpt->x, pt->x);
            break;
        case EQTypeMontgomery:
            // negation is not relevant in XZ projective representation
            break;
        default:
            assert((pt->cv->type == EQTypeShortWeierstrass) || (pt->cv->type == EQTypeEdwards) || (pt->cv->type == EQTypeMontgomery));
    }
    return;
}


int mpECP_cmp(mpECP_t pt1, mpECP_t pt2) {
    if (mpECurve_cmp(pt1->cv, pt2->cv) != 0) return -1;
    if (pt1->is_infinite != 0) {
        if (pt2->is_infinite != 0) {
            return 0;
        }
        return -1;
    } else if (pt2->is_infinite != 0) {
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
        case EQTypeEdwards: {
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
            assert((pt1->cv->type == EQTypeShortWeierstrass) || (pt1->cv->type == EQTypeEdwards) || (pt1->cv->type == EQTypeMontgomery));
    }
    return 0;
}
