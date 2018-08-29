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
// #include <ecc/ecelgamal.h>
// #include <ecc/ecurve.h>
// #include <stdlib.h>
// #include <string.h>
//
// mpECElgamalCiphertext_ptr malloc_ECElgamalCiphertext() {
//     return (mpECElgamalCiphertext_ptr)malloc(sizeof(_mpECElgamalCiphertext_t));
// }
//
// void free_ECElgamalCiphertext(mpECElgamalCiphertext_ptr csig) {
//     free(csig);
// }
//
// mpECP_ptr C_of(mpECElgamalCiphertext_ptr ctxt) {
//     return ctxt->C;
// }
//
// mpECP_ptr D_of(mpECElgamalCiphertext_ptr ctxt) {
//     return ctxt->D;
// }
//
import "C"

import (
	//"bytes"
	//"crypto"
	//"encoding/base64"
	//"encoding/hex"
	//"encoding/binary"
	//"errors"
	//"fmt"
	//"hash"
	//"io/ioutil"
	"math/big"
	//"os"
	//"reflect"
	"runtime"
	//"strconv"
	//"strings"
	//"time"
	//"unsafe"
)

type ECElgamalCiphertext struct {
	egc *C._mpECElgamalCiphertext_t
	cv  *Curve
}

func ECElgamalEncrypt(pK *Point, ptxt *Point) (ctxt *ECElgamalCiphertext) {
	if C.mpECurve_cmp(pK.cv.ec, ptxt.cv.ec) != 0 {
		return nil
	}
	ctxt = new(ECElgamalCiphertext)
	ctxt.egc = C.malloc_ECElgamalCiphertext()
	ctxt.cv = pK.cv
	status := C.mpECElgamal_init_encrypt(ctxt.egc, pK.ecp, ptxt.ecp)
	runtime.SetFinalizer(ctxt, ecelgamalciphertext_clear)
	if status != C.int(0) {
		return nil
	}
	return ctxt
}

func ecelgamalciphertext_clear(ctxt *ECElgamalCiphertext) {
	C.mpECElgamal_clear(ctxt.egc)
	C.free_ECElgamalCiphertext(ctxt.egc)
}

func (ctxt *ECElgamalCiphertext) Decrypt(sK *big.Int) (ptxt *Point) {
	sKfe := NewFieldElement(sK, ctxt.cv.GetAttr("n"))
	ptxt = NewPointNeutral(ctxt.cv)
	status := C.mpECElgamal_init_decrypt(ptxt.ecp, sKfe.fe, ctxt.egc)
	if status != C.int(0) {
		return nil
	}
	return ptxt
}

func (ctxt *ECElgamalCiphertext) C() (c *Point) {
	c = NewPointNeutral(ctxt.cv)
	C.mpECP_set(c.ecp, C.C_of(ctxt.egc))
	return c
}

func (ctxt *ECElgamalCiphertext) D() (d *Point) {
	d = NewPointNeutral(ctxt.cv)
	C.mpECP_set(d.ecp, C.D_of(ctxt.egc))
	return d
}

func NewECElgamalCiphertext(c, d *Point) (ctxt *ECElgamalCiphertext) {
	if C.mpECurve_cmp(c.cv.ec, d.cv.ec) != 0 {
		return nil
	}
	ctxt = new(ECElgamalCiphertext)
	ctxt.cv = c.cv
	ctxt.egc = C.malloc_ECElgamalCiphertext()
	C.mpECP_init(C.C_of(ctxt.egc), c.cv.ec)
	C.mpECP_set(C.C_of(ctxt.egc), c.ecp)
	C.mpECP_init(C.D_of(ctxt.egc), d.cv.ec)
	C.mpECP_set(C.D_of(ctxt.egc), d.ecp)
	runtime.SetFinalizer(ctxt, ecelgamalciphertext_clear)
	return ctxt
}
