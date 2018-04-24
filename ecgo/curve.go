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

package ecgo

// #cgo LDFLAGS: -lecc -lgmp
// #include <ecurve.h>
// #include <field.h>
// #include <gmp.h>
// #include <stdlib.h>
//
// mpECurve_ptr malloc_ECurve() {
//     return (mpECurve_ptr)malloc(sizeof(_mpECurve_t));
// }
//
// int clist_len(char **list) {
//     int i = 0;
//     char **l;
//
//     if (list == NULL) return 0;
//     l = list;
//     while (*l != NULL) {
//         l++;
//         i++;
//     }
//     return i;
// }
//
// __mpz_struct *_p_of_cv(mpECurve_t cv) {
//     return cv->fp->p;
// }
//
// __mpz_struct *_n_of_cv(mpECurve_t cv) {
//     return cv->n;
// }
//
// __mpz_struct *_h_of_cv(mpECurve_t cv) {
//     return cv->h;
// }
//
// __mpz_struct *_gx_of_cv(mpECurve_t cv) {
//     return cv->G[0];
// }
//
// __mpz_struct *_gy_of_cv(mpECurve_t cv) {
//     return cv->G[1];
// }
//
// unsigned int _bits_of_cv(mpECurve_t cv) {
//     return cv->bits;
// }
//
// unsigned int _type_of_cv(mpECurve_t cv) {
//     return (unsigned int)cv->type;
// }
//
// _mpFp_struct *_ws_a_of_cv(mpECurve_t cv) {
//     return cv->coeff.ws.a;
// }
//
// _mpFp_struct *_ws_b_of_cv(mpECurve_t cv) {
//     return cv->coeff.ws.b;
// }
//
// _mpFp_struct *_ed_c_of_cv(mpECurve_t cv) {
//     return cv->coeff.ed.c;
// }
//
// _mpFp_struct *_ed_d_of_cv(mpECurve_t cv) {
//     return cv->coeff.ed.d;
// }
//
// _mpFp_struct *_mo_B_of_cv(mpECurve_t cv) {
//     return cv->coeff.mo.B;
// }
//
// _mpFp_struct *_mo_A_of_cv(mpECurve_t cv) {
//     return cv->coeff.mo.A;
// }
//
// _mpFp_struct *_te_a_of_cv(mpECurve_t cv) {
//     return cv->coeff.te.a;
// }
//
// _mpFp_struct *_te_d_of_cv(mpECurve_t cv) {
//     return cv->coeff.te.d;
// }
//
// void free_ECurve(mpECurve_ptr e) {
//     free(e);
// }
//
import "C"

import (
	//"bytes"
	//"encoding/base64"
	//"encoding/hex"
	//"encoding/binary"
	//"errors"
	//"fmt"
	//"io/ioutil"
	"math/big"
	//"os"
	//"reflect"
	"runtime"
	//"strconv"
	//"strings"
	//"time"
	"unsafe"
)

// curve types
const (
	EQTypeNone             = iota
	EQTypeUninitialized    = iota
	EQTypeShortWeierstrass = iota
	EQTypeEdwards          = iota
	EQTypeMontgomery       = iota
	EQTypeTwistedEdwards   = iota
)

// Max size of Fp values is based on _MPFP_MAX_LIMBS * sizeof(mp_limb_t)
// const _MPFP_Max_Bytes = (32 * 8)

type Curve struct {
	ec *C._mpECurve_t
}

func CurveNames() []string {
	clist := C._mpECurve_list_standard_curves()
	cllen := C.clist_len(clist)
	return goStrings(cllen, clist)
}

func NamedCurve(name string) (z *Curve) {
	z = new(Curve)
	z.ec = C.malloc_ECurve()
	C.mpECurve_init(z.ec)
	cname := C.CString(name)
	status := C.mpECurve_set_named(z.ec, cname)
	C.free(unsafe.Pointer(cname))

	runtime.SetFinalizer(z, curve_clear)
	if int(status) != 0 {
		return nil
	}
	return z
}

func curve_clear(z *Curve) {
	C.mpECurve_clear(z.ec)
	C.free_ECurve(z.ec)
}

func mpFp_to_bigint(fp *C._mpFp_struct) (r *big.Int) {
	var tmpz C.__mpz_struct

	C.mpz_init(&tmpz)
	C.mpz_set_mpFp(&tmpz, fp)
	r = c_mpz_to_bigint(&tmpz)
	C.mpz_clear(&tmpz)
	return r
}

func (z *Curve) Type() (r int) {
	return int(C._type_of_cv(z.ec))
}

func (z *Curve) Cmp(a *Curve) int {
	return int(C.mpECurve_cmp(z.ec, a.ec))
}

func (z *Curve) GetAttr(attr string) (r *big.Int) {

	ctype := int(C._type_of_cv(z.ec))
	//r = big.NewInt(0)
	switch attr {
	case "p":
		r = c_mpz_to_bigint(C._p_of_cv(z.ec))
	case "n":
		r = c_mpz_to_bigint(C._n_of_cv(z.ec))
	case "h":
		r = c_mpz_to_bigint(C._h_of_cv(z.ec))
	case "gx":
		r = c_mpz_to_bigint(C._gx_of_cv(z.ec))
	case "gy":
		r = c_mpz_to_bigint(C._gy_of_cv(z.ec))
	case "bits":
		r = new(big.Int)
		bits := uint64(C._bits_of_cv(z.ec))
		r.SetUint64(bits)
	case "ws_a":
		if ctype != EQTypeShortWeierstrass {
			return nil
		}
		r = mpFp_to_bigint(C._ws_a_of_cv(z.ec))
	case "ws_b":
		if ctype != EQTypeShortWeierstrass {
			return nil
		}
		r = mpFp_to_bigint(C._ws_b_of_cv(z.ec))
	case "ed_c":
		if ctype != EQTypeEdwards {
			return nil
		}
		r = mpFp_to_bigint(C._ed_c_of_cv(z.ec))
	case "ed_d":
		if ctype != EQTypeEdwards {
			return nil
		}
		r = mpFp_to_bigint(C._ed_d_of_cv(z.ec))
	case "mo_B":
		if ctype != EQTypeMontgomery {
			return nil
		}
		r = mpFp_to_bigint(C._mo_B_of_cv(z.ec))
	case "mo_A":
		if ctype != EQTypeMontgomery {
			return nil
		}
		r = mpFp_to_bigint(C._mo_A_of_cv(z.ec))
	case "te_a":
		if ctype != EQTypeTwistedEdwards {
			return nil
		}
		r = mpFp_to_bigint(C._te_a_of_cv(z.ec))
	case "te_d":
		if ctype != EQTypeTwistedEdwards {
			return nil
		}
		r = mpFp_to_bigint(C._te_d_of_cv(z.ec))
	default:
		return nil
	}
	return r
}

func (z *Curve) PointCheck(gx, gy *big.Int) bool {
	var gxmpz, gympz C.__mpz_struct
	var err error
	var r bool

	r = false

	C.mpz_init(&gxmpz)
	C.mpz_init(&gympz)

	err = bigint_to_c_mpz_unsigned(&gxmpz, gx)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gympz, gy)
	if err != nil {
		goto cleanup
	}

	r = (C.mpECurve_point_check(z.ec, &gxmpz, &gympz) != 0)
cleanup:
	C.mpz_clear(&gympz)
	C.mpz_clear(&gxmpz)
	return r
}

func ShortWeierstrassCurve(p, a, b, n, h, gx, gy *big.Int, bits uint) *Curve {
	var pmpz, ampz, bmpz, nmpz, hmpz, gxmpz, gympz C.__mpz_struct
	var err error
	var z *Curve
	var status int

	z = nil

	C.mpz_init(&pmpz)
	C.mpz_init(&ampz)
	C.mpz_init(&bmpz)
	C.mpz_init(&nmpz)
	C.mpz_init(&hmpz)
	C.mpz_init(&gxmpz)
	C.mpz_init(&gympz)

	err = bigint_to_c_mpz_unsigned(&pmpz, p)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&ampz, a)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&bmpz, b)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&nmpz, n)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&hmpz, h)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gxmpz, gx)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gympz, gy)
	if err != nil {
		goto cleanup
	}

	z = new(Curve)
	z.ec = C.malloc_ECurve()
	C.mpECurve_init(z.ec)
	status = int(C.mpECurve_set_mpz_ws(z.ec, &pmpz, &ampz, &bmpz, &nmpz,
		&hmpz, &gxmpz, &gympz, C.uint(bits)))
	if status != 0 {
		C.mpECurve_clear(z.ec)
		C.free_ECurve(z.ec)
		z = nil
	}
cleanup:
	C.mpz_clear(&gympz)
	C.mpz_clear(&gxmpz)
	C.mpz_clear(&hmpz)
	C.mpz_clear(&nmpz)
	C.mpz_clear(&bmpz)
	C.mpz_clear(&ampz)
	C.mpz_clear(&pmpz)
	return z
}

func EdwardsCurve(p, c, d, n, h, gx, gy *big.Int, bits uint) *Curve {
	var pmpz, cmpz, dmpz, nmpz, hmpz, gxmpz, gympz C.__mpz_struct
	var err error
	var z *Curve
	var status int

	z = nil

	C.mpz_init(&pmpz)
	C.mpz_init(&cmpz)
	C.mpz_init(&dmpz)
	C.mpz_init(&nmpz)
	C.mpz_init(&hmpz)
	C.mpz_init(&gxmpz)
	C.mpz_init(&gympz)

	err = bigint_to_c_mpz_unsigned(&pmpz, p)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&cmpz, c)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&dmpz, d)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&nmpz, n)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&hmpz, h)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gxmpz, gx)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gympz, gy)
	if err != nil {
		goto cleanup
	}

	z = new(Curve)
	z.ec = C.malloc_ECurve()
	C.mpECurve_init(z.ec)
	status = int(C.mpECurve_set_mpz_ed(z.ec, &pmpz, &cmpz, &dmpz, &nmpz,
		&hmpz, &gxmpz, &gympz, C.uint(bits)))
	if status != 0 {
		C.mpECurve_clear(z.ec)
		C.free_ECurve(z.ec)
		z = nil
	}
cleanup:
	C.mpz_clear(&gympz)
	C.mpz_clear(&gxmpz)
	C.mpz_clear(&hmpz)
	C.mpz_clear(&nmpz)
	C.mpz_clear(&dmpz)
	C.mpz_clear(&cmpz)
	C.mpz_clear(&pmpz)
	return z
}

func MontgomeryCurve(p, B, A, n, h, gx, gy *big.Int, bits uint) *Curve {
	var pmpz, Bmpz, Ampz, nmpz, hmpz, gxmpz, gympz C.__mpz_struct
	var err error
	var z *Curve
	var status int

	z = nil

	C.mpz_init(&pmpz)
	C.mpz_init(&Bmpz)
	C.mpz_init(&Ampz)
	C.mpz_init(&nmpz)
	C.mpz_init(&hmpz)
	C.mpz_init(&gxmpz)
	C.mpz_init(&gympz)

	err = bigint_to_c_mpz_unsigned(&pmpz, p)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&Bmpz, B)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&Ampz, A)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&nmpz, n)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&hmpz, h)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gxmpz, gx)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gympz, gy)
	if err != nil {
		goto cleanup
	}

	z = new(Curve)
	z.ec = C.malloc_ECurve()
	C.mpECurve_init(z.ec)
	status = int(C.mpECurve_set_mpz_mo(z.ec, &pmpz, &Bmpz, &Ampz, &nmpz,
		&hmpz, &gxmpz, &gympz, C.uint(bits)))
	if status != 0 {
		C.mpECurve_clear(z.ec)
		C.free_ECurve(z.ec)
		z = nil
	}
cleanup:
	C.mpz_clear(&gympz)
	C.mpz_clear(&gxmpz)
	C.mpz_clear(&hmpz)
	C.mpz_clear(&nmpz)
	C.mpz_clear(&Ampz)
	C.mpz_clear(&Bmpz)
	C.mpz_clear(&pmpz)
	return z
}

func TwistedEdwardsCurve(p, a, d, n, h, gx, gy *big.Int, bits uint) *Curve {
	var pmpz, ampz, dmpz, nmpz, hmpz, gxmpz, gympz C.__mpz_struct
	var err error
	var z *Curve
	var status int

	z = nil

	C.mpz_init(&pmpz)
	C.mpz_init(&ampz)
	C.mpz_init(&dmpz)
	C.mpz_init(&nmpz)
	C.mpz_init(&hmpz)
	C.mpz_init(&gxmpz)
	C.mpz_init(&gympz)

	err = bigint_to_c_mpz_unsigned(&pmpz, p)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&ampz, a)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&dmpz, d)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&nmpz, n)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&hmpz, h)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gxmpz, gx)
	if err != nil {
		goto cleanup
	}
	err = bigint_to_c_mpz_unsigned(&gympz, gy)
	if err != nil {
		goto cleanup
	}

	z = new(Curve)
	z.ec = C.malloc_ECurve()
	C.mpECurve_init(z.ec)
	status = int(C.mpECurve_set_mpz_te(z.ec, &pmpz, &ampz, &dmpz, &nmpz,
		&hmpz, &gxmpz, &gympz, C.uint(bits)))
	if status != 0 {
		C.mpECurve_clear(z.ec)
		C.free_ECurve(z.ec)
		z = nil
	}
cleanup:
	C.mpz_clear(&gympz)
	C.mpz_clear(&gxmpz)
	C.mpz_clear(&hmpz)
	C.mpz_clear(&nmpz)
	C.mpz_clear(&dmpz)
	C.mpz_clear(&ampz)
	C.mpz_clear(&pmpz)
	return z
}

// basic list of strings handling function
func goStrings(argc C.int, argv **C.char) []string {

	length := int(argc)
	tmpslice := (*[1 << 30]*C.char)(unsafe.Pointer(argv))[:length:length]
	gostrings := make([]string, length)
	for i, s := range tmpslice {
		gostrings[i] = C.GoString(s)
	}
	return gostrings
}

// this is actually just a test hook, but can use "import C" in test
func validateTypeEnums() bool {
	if int(EQTypeNone) != int(C.EQTypeNone) {
		return false
	}
	if int(EQTypeUninitialized) != int(C.EQTypeUninitialized) {
		return false
	}
	if int(EQTypeShortWeierstrass) != int(C.EQTypeShortWeierstrass) {
		return false
	}
	if int(EQTypeEdwards) != int(C.EQTypeEdwards) {
		return false
	}
	if int(EQTypeMontgomery) != int(C.EQTypeMontgomery) {
		return false
	}
	if int(EQTypeTwistedEdwards) != int(C.EQTypeTwistedEdwards) {
		return false
	}
	return true
}
