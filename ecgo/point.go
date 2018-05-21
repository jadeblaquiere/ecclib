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
// #include <ecpoint.h>
// #include <field.h>
// #include <gmp.h>
// #include <stdlib.h>
//
// typedef _mpECP_t *mpECP_ptr;
//
// mpECP_ptr malloc_ECPoint() {
//     return (mpECP_ptr)malloc(sizeof(_mpECP_t));
// }
//
// __mpz_struct *_pgx_of_cv(mpECurve_t cv) {
//     return cv->G[0];
// }
//
// __mpz_struct *_pgy_of_cv(mpECurve_t cv) {
//     return cv->G[1];
// }
//
// int _mpECP_supports_base_mul(mpECP_t pt) {
//     return pt->base_bits != 0;
// }
//
// char *mpECP_alloc_out_str(mpECP_t pt, int compress) {
//     int leng;
//     char *buf;
//
//     leng = mpECP_out_strlen(pt, compress);
//     buf = (char *)malloc((leng + 1)*sizeof(char));
//     assert(buf != NULL);
//     mpECP_out_str(buf, pt, compress);
//     return buf;
// }
//
// char *mpECP_alloc_out_bytes(mpECP_t pt, int compress) {
//     int leng;
//     char *buf;
//
//     leng = mpECP_out_bytelen(pt, compress);
//     buf = (char *)malloc(leng*sizeof(char));
//     assert(buf != NULL);
//     mpECP_out_bytes(buf, pt, compress);
//     return buf;
// }
//
// unsigned char *_toUCP(void *p) {
//     return (unsigned char *)p;
// }
//
//
// void free_ECPoint(mpECP_ptr pt) {
//     free(pt);
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

// Max size of Fp values is based on _MPFP_MAX_LIMBS * sizeof(mp_limb_t)
// const _MPFP_Max_Bytes = (32 * 8)

type Point struct {
	ecp *C._mpECP_t
	cv  *Curve
}

func NewPointXY(x, y *big.Int, c *Curve) (z *Point) {
	var xmpz, ympz C.__mpz_struct

	C.mpz_init(&xmpz)
	C.mpz_init(&ympz)

	bigint_to_c_mpz_signed(&xmpz, x)
	bigint_to_c_mpz_signed(&ympz, y)

	z = new(Point)
	z.ecp = C.malloc_ECPoint()
	z.cv = c
	C.mpECP_init(z.ecp, c.ec)
	runtime.SetFinalizer(z, point_clear)
	C.mpECP_set_mpz(z.ecp, &xmpz, &ympz, c.ec)

	C.mpz_clear(&xmpz)
	C.mpz_clear(&ympz)
	return z
}

func NewPointNeutral(c *Curve) (z *Point) {

	z = new(Point)
	z.ecp = C.malloc_ECPoint()
	z.cv = c
	C.mpECP_init(z.ecp, c.ec)
	runtime.SetFinalizer(z, point_clear)
	C.mpECP_set_neutral(z.ecp, c.ec)

	return z
}

func NewPointGenerator(c *Curve) (z *Point) {

	z = new(Point)
	z.ecp = C.malloc_ECPoint()
	z.cv = c
	C.mpECP_init(z.ecp, c.ec)
	runtime.SetFinalizer(z, point_clear)
	C.mpECP_set_mpz(z.ecp, C._pgx_of_cv(c.ec), C._pgy_of_cv(c.ec), c.ec)

	return z
}

func NewPointURandom(c *Curve) (z *Point) {

	z = new(Point)
	z.ecp = C.malloc_ECPoint()
	z.cv = c
	C.mpECP_init(z.ecp, c.ec)
	runtime.SetFinalizer(z, point_clear)
	C.mpECP_urandom(z.ecp, c.ec)

	return z
}

func NewPointFromString(gst string, c *Curve) (z *Point) {

	z = new(Point)
	z.ecp = C.malloc_ECPoint()
	z.cv = c
	C.mpECP_init(z.ecp, c.ec)
	runtime.SetFinalizer(z, point_clear)
	cstr := C.CString(gst)
	status := C.mpECP_set_str(z.ecp, cstr, c.ec)
	if status != C.int(0) {
		return nil
	}
	C.free(unsafe.Pointer(cstr))

	return z
}

func NewPointFromBytes(gby []byte, c *Curve) (z *Point) {

	z = new(Point)
	z.ecp = C.malloc_ECPoint()
	z.cv = c
	C.mpECP_init(z.ecp, c.ec)
	runtime.SetFinalizer(z, point_clear)
	cby := C.CBytes(gby)
	l := len(gby)
	status := C.mpECP_set_bytes(z.ecp, C._toUCP(cby), C.int(l), c.ec)
	if status != C.int(0) {
		return nil
	}
	C.free(cby)

	return z
}

func point_clear(z *Point) {
	C.mpECP_clear(z.ecp)
	C.free_ECPoint(z.ecp)
}

func (z *Point) Affine() (x, y *big.Int) {
	var xmpz, ympz C.__mpz_struct

	C.mpz_init(&xmpz)
	C.mpz_init(&ympz)

	C.mpz_set_mpECP_affine_x(&xmpz, z.ecp)
	C.mpz_set_mpECP_affine_y(&ympz, z.ecp)

	x = c_mpz_to_bigint(&xmpz)
	y = c_mpz_to_bigint(&ympz)

	C.mpz_clear(&xmpz)
	C.mpz_clear(&ympz)

	return x, y
}

func (z *Point) IsNeutral() bool {
	pt := z.ecp
	isn := pt.is_neutral
	if int(isn) == 0 {
		return false
	}
	return true
}

func (z *Point) StringCompressed() string {
	cstr := C.mpECP_alloc_out_str(z.ecp, C.int(1))
	gstr := C.GoString(cstr)
	C.free(unsafe.Pointer(cstr))
	return gstr
}

func (z *Point) StringUncompressed() string {
	cstr := C.mpECP_alloc_out_str(z.ecp, C.int(0))
	gstr := C.GoString(cstr)
	C.free(unsafe.Pointer(cstr))
	return gstr
}

func (z *Point) BytesCompressed() []byte {
	blen := C.mpECP_out_bytelen(z.ecp, C.int(1))
	cstr := C.mpECP_alloc_out_bytes(z.ecp, C.int(1))
	gstr := C.GoBytes(unsafe.Pointer(cstr), blen)
	C.free(unsafe.Pointer(cstr))
	return gstr
}

func (z *Point) BytesUncompressed() []byte {
	blen := C.mpECP_out_bytelen(z.ecp, C.int(0))
	cstr := C.mpECP_alloc_out_bytes(z.ecp, C.int(0))
	gstr := C.GoBytes(unsafe.Pointer(cstr), blen)
	C.free(unsafe.Pointer(cstr))
	return gstr
}

func (z *Point) Cmp(a *Point) int {
	return int(C.mpECP_cmp(z.ecp, a.ecp))
}

func (z *Point) Add(a, b *Point) *Point {
	C.mpECP_add(z.ecp, a.ecp, b.ecp)
	z.cv = a.cv
	return z
}

func (z *Point) Sub(a, b *Point) *Point {
	C.mpECP_sub(z.ecp, a.ecp, b.ecp)
	z.cv = a.cv
	return z
}

func (z *Point) Neg(a *Point) *Point {
	C.mpECP_neg(z.ecp, a.ecp)
	z.cv = a.cv
	return z
}

func (z *Point) Set(a *Point) *Point {
	C.mpECP_set(z.ecp, a.ecp)
	z.cv = a.cv
	return z
}

func (z *Point) Mul(a *Point, b *big.Int) *Point {
	var bmpz C.__mpz_struct

	C.mpz_init(&bmpz)
	bigint_to_c_mpz_signed(&bmpz, b)

	if C._mpECP_supports_base_mul(a.ecp) != 0 {
		C.mpECP_scalar_base_mul_mpz(z.ecp, a.ecp, &bmpz)
	} else {
		C.mpECP_scalar_mul_mpz(z.ecp, a.ecp, &bmpz)
	}
	z.cv = a.cv

	return z
}

func (z *Point) SetupBaseMul() {
	C.mpECP_scalar_base_mul_setup(z.ecp)
	return
}
