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
// #include <ecc/ecdsa.h>
// #include <ecc/ecurve.h>
// #include <stdlib.h>
// #include <string.h>
//
// //typedef _mpECDSASignatureScheme_t *mpECDSASignatureScheme_ptr;
//
// mpECDSASignatureScheme_ptr malloc_ECDSASignatureScheme() {
//     return (mpECDSASignatureScheme_ptr)malloc(sizeof(_mpECDSASignatureScheme_t));
// }
//
// void free_ECDSASignatureScheme(mpECDSASignatureScheme_ptr css) {
//     free(css);
// }
//
// //typedef _mpECDSASignature_t *mpECDSASignature_ptr;
//
// mpECDSASignature_ptr malloc_ECDSASignature() {
//     return (mpECDSASignature_ptr)malloc(sizeof(_mpECDSASignature_t));
// }
//
// void free_ECDSASignature(mpECDSASignature_ptr csig) {
//     free(csig);
// }
//
// // this is a bit of a cheat as golang wrapper will pre-hash the message
// void cgo_dohash(unsigned char *hash, unsigned char *msg, size_t msz) {
//     memcpy(hash, msg, msz);
//     return;
// }
//
// extern unsigned char *_toUCP(void *p);
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
	"hash"
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

type ECDSASignatureScheme struct {
	css *C._mpECDSASignatureScheme_t
	cv  *Curve
	h   hash.Hash
}

type ECDSASignature struct {
	csig *C._mpECDSASignature_t
	ss   *ECDSASignatureScheme
}

func NewECDSASignatureScheme(c *Curve, h hash.Hash) (ss *ECDSASignatureScheme) {
	var hdesc C._mpECDSAHashfunc_t

	ss = new(ECDSASignatureScheme)
	ss.css = C.malloc_ECDSASignatureScheme()
	ss.cv = c
	ss.h = h
	hdesc.dohash = C._ecdsa_dohash(C.cgo_dohash)
	hdesc.hsz = C.size_t(h.Size())
	C.mpECDSASignatureScheme_init(ss.css, c.ec, &hdesc)
	runtime.SetFinalizer(ss, signaturescheme_clear)
	return ss
}

func signaturescheme_clear(ss *ECDSASignatureScheme) {
	C.mpECDSASignatureScheme_clear(ss.css)
	C.free_ECDSASignatureScheme(ss.css)
}

func (ss *ECDSASignatureScheme) Sign(sK *big.Int, message []byte) *ECDSASignature {
	ss.h.Reset()
	ss.h.Write(message)
	mhash := C.CBytes(ss.h.Sum(nil))
	// ghash := ss.h.Sum(nil)
	// fmt.Fprintf(os.Stderr, "hash = %s\n", hex.EncodeToString(ghash))
	hsz := C.size_t(ss.h.Size())
	sig := new(ECDSASignature)
	sig.csig = C.malloc_ECDSASignature()
	sKfe := NewFieldElement(sK, ss.cv.GetAttr("n"))
	status := C.mpECDSASignature_init_Sign(sig.csig, ss.css, sKfe.fe, C._toUCP(mhash), hsz)
	C.free(mhash)
	if status != C.int(0) {
		C.free_ECDSASignature(sig.csig)
		return nil
	}
	sig.ss = ss
	return sig
}

func (sig *ECDSASignature) Verify(pK *Point, message []byte) bool {
	sig.ss.h.Reset()
	sig.ss.h.Write(message)
	mhash := C.CBytes(sig.ss.h.Sum(nil))
	hsz := C.size_t(sig.ss.h.Size())
	status := C.mpECDSASignature_verify_cmp(sig.csig, pK.ecp, C._toUCP(mhash), hsz)
	C.free(mhash)
	if status == C.int(0) {
		return true
	}
	return false
}

func (sig *ECDSASignature) AsBytes() []byte {
	var sz C.size_t
	cbytes := unsafe.Pointer(C.mpECDSASignature_export_bytes(sig.csig, &sz))
	gobytes := C.GoBytes(cbytes, C.int(sz))
	C.memset(cbytes, 0, sz)
	C.free(cbytes)
	return gobytes
}

func (ss *ECDSASignatureScheme) SignatureFromBytes(gobytes []byte) *ECDSASignature {
	sig := new(ECDSASignature)
	sig.csig = C.malloc_ECDSASignature()
	sig.ss = ss
	cbytes := C.CBytes(gobytes)
	sz := C.size_t(len(gobytes))
	status := C.mpECDSASignature_init_import_bytes(sig.csig, ss.css, C._toUCP(cbytes), sz)
	C.memset(cbytes, 0, sz)
	C.free(cbytes)
	if status != 0 {
		C.free_ECDSASignature(sig.csig)
		return nil
	}
	return sig
}
