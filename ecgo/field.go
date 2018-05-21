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
// #include <field.h>
// #include <gmp.h>
// #include <stdlib.h>
//
// mpFp_ptr malloc_FieldElement() {
//     return (mpFp_ptr)malloc(sizeof(_mpFp_struct));
// }
//
// void free_FieldElement(mpFp_ptr e) {
//     free(e);
// }
//
// mpFp_field_ptr _fp_of_fe(mpFp_t p) {
//     return p->fp;
// }
//
// __mpz_struct *_p_of_fe(mpFp_t p) {
//     return p->fp->p;
// }
//
// int _local_mpz_sgn(mpz_t p) {
//     return mpz_sgn(p);
// }
//
// size_t _mpz_sizeinbytes(mpz_t a) {
//     return (mpz_sizeinbase(a, 2) + 7) >> 3;
// }
//
import "C"

import (
	//"bytes"
	//"encoding/base64"
	//"encoding/hex"
	//"encoding/binary"
	"errors"
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

// Max size of Fp values is based on _MPFP_MAX_LIMBS * sizeof(mp_limb_t)
const _MPFP_Max_Bytes = (32 * 8)

type FieldElement struct {
	fe *C._mpFp_struct
}

func bigint_to_c_mpz_unsigned(m *C.__mpz_struct, a *big.Int) error {
	abytes := a.Bytes()

	if a.Cmp(big.NewInt(0)) < 0 {
		return errors.New("negative input to bigint_to_c_mpz_unsigned")
	}

	if len(abytes) == 0 {
		C.mpz_set_ui(m, 0)
	} else {
		C.mpz_import(m, C.size_t(len(abytes)), 1, C.sizeof_char, 1, 0, unsafe.Pointer(&abytes[0]))
	}
	return nil
}

func bigint_to_c_mpz_signed(m *C.__mpz_struct, a *big.Int) {
	var negative bool

	if a.Cmp(big.NewInt(0)) < 0 {
		negative = true
		a.Neg(a)
	}

	abytes := a.Bytes()

	if len(abytes) == 0 {
		C.mpz_set_ui(m, 0)
	} else {
		C.mpz_import(m, C.size_t(len(abytes)), 1, C.sizeof_char, 1, 0, unsafe.Pointer(&abytes[0]))
	}

	if negative {
		C.mpz_neg(m, m)
	}
}

func c_mpz_to_bigint(m *C.__mpz_struct) (z *big.Int) {
	var bsz C.size_t
	var negative bool

	z = new(big.Int)
	sz := C._mpz_sizeinbytes(m)
	buf := make([]byte, sz, sz*2)
	if int(C._local_mpz_sgn(m)) < 0 {
		negative = true
		C.mpz_neg(m, m)
	}
	C.mpz_export(unsafe.Pointer(&buf[0]), &bsz, 1, C.sizeof_char, 1, 0, m)
	z.SetBytes(buf)
	if negative {
		z.Neg(z)
	}
	return z
}

func NewFieldElement(value, field *big.Int) (z *FieldElement) {
	var vmpz, pmpz C.__mpz_struct
	var err error

	C.mpz_init(&vmpz)
	C.mpz_init(&pmpz)

	if field.Cmp(big.NewInt(0)) == 0 {
		panic("cannot operate on field of order 0")
	}
	bigint_to_c_mpz_signed(&vmpz, value)
	err = bigint_to_c_mpz_unsigned(&pmpz, field)
	if err != nil {
		return nil
	}

	// fmt.Println("imported value as:")
	// C.print_mpz_hex(&vmpz)
	// fmt.Println("imported field as:")
	// C.print_mpz_hex(&pmpz)

	z = new(FieldElement)
	z.fe = C.malloc_FieldElement()
	C.mpFp_init(z.fe, &pmpz)
	C.mpFp_set_mpz(z.fe, &vmpz, &pmpz)

	C.mpz_clear(&pmpz)
	C.mpz_clear(&vmpz)
	runtime.SetFinalizer(z, fieldElement_clear)
	return z
}

func NewFieldElementUI(value uint64, field *big.Int) (z *FieldElement) {
	var pmpz C.__mpz_struct
	var err error

	if field.Cmp(big.NewInt(0)) == 0 {
		panic("cannot operate on field of order 0")
	}
	C.mpz_init(&pmpz)
	err = bigint_to_c_mpz_unsigned(&pmpz, field)
	if err != nil {
		return nil
	}

	// fmt.Println("imported field as:")
	// C.print_mpz_hex(&pmpz)

	z = new(FieldElement)
	z.fe = C.malloc_FieldElement()
	C.mpFp_init(z.fe, &pmpz)
	C.mpFp_set_ui(z.fe, C.ulong(value), &pmpz)

	C.mpz_clear(&pmpz)
	runtime.SetFinalizer(z, fieldElement_clear)
	return z
}

func NewFieldElementURandom(field *big.Int) (z *FieldElement) {
	var pmpz C.__mpz_struct
	var err error

	if field.Cmp(big.NewInt(0)) == 0 {
		panic("cannot operate on field of order 0")
	}
	C.mpz_init(&pmpz)
	err = bigint_to_c_mpz_unsigned(&pmpz, field)
	if err != nil {
		return nil
	}

	// fmt.Println("imported field as:")
	// C.print_mpz_hex(&pmpz)

	z = new(FieldElement)
	z.fe = C.malloc_FieldElement()
	C.mpFp_init(z.fe, &pmpz)
	C.mpFp_urandom(z.fe, &pmpz)

	C.mpz_clear(&pmpz)
	runtime.SetFinalizer(z, fieldElement_clear)
	return z
}

func fieldElement_clear(z *FieldElement) {
	C.mpFp_clear(z.fe)
	C.free_FieldElement(z.fe)
}

func (z *FieldElement) AsInt() (r *big.Int) {
	var impz C.__mpz_struct

	C.mpz_init(&impz)
	C.mpz_set_mpFp(&impz, z.fe)
	r = c_mpz_to_bigint(&impz)
	C.mpz_clear(&impz)
	return r
}

func (z *FieldElement) Order() (r *big.Int) {
	impz := C._p_of_fe(z.fe)
	r = c_mpz_to_bigint(impz)
	return r
}

func (z *FieldElement) Cmp(a *FieldElement) int {
	return int(C.mpFp_cmp(z.fe, a.fe))
}

func (z *FieldElement) Add(a, b *FieldElement) *FieldElement {
	if C._fp_of_fe(a.fe) != C._fp_of_fe(b.fe) {
		panic("FE.Add: field mismatch")
	}
	C.mpFp_add(z.fe, a.fe, b.fe)
	return z
}

func (z *FieldElement) Sub(a, b *FieldElement) *FieldElement {
	if C._fp_of_fe(a.fe) != C._fp_of_fe(b.fe) {
		panic("FE.Add: field mismatch")
	}
	C.mpFp_sub(z.fe, a.fe, b.fe)
	return z
}

func (z *FieldElement) Mul(a, b *FieldElement) *FieldElement {
	if C._fp_of_fe(a.fe) != C._fp_of_fe(b.fe) {
		panic("FE.Add: field mismatch")
	}
	C.mpFp_mul(z.fe, a.fe, b.fe)
	return z
}

func (z *FieldElement) Neg(a *FieldElement) *FieldElement {
	C.mpFp_neg(z.fe, a.fe)
	return z
}

func (z *FieldElement) Inv(a *FieldElement) (fe *FieldElement, err error) {
	status := C.mpFp_inv(z.fe, a.fe)
	if int(status) != 0 {
		return nil, errors.New("no inverse exists for element")
	}
	return z, nil
}

func (z *FieldElement) Sqrt(a *FieldElement) (fe *FieldElement, err error) {
	status := C.mpFp_sqrt(z.fe, a.fe)
	if int(status) != 0 {
		return nil, errors.New("element is not a quadratic residue")
	}
	return z, nil
}

func (z *FieldElement) Exp(a *FieldElement, e uint64) *FieldElement {
	C.mpFp_pow_ui(z.fe, a.fe, C.ulong(e))
	return z
}

func (z *FieldElement) Bit(i int) uint {
	return uint(C.mpFp_tstbit(z.fe, C.int(i)))
}
