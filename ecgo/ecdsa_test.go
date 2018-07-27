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

import (
	"crypto/sha256"
	//"encoding/hex"
	"fmt"
	"testing"
)

func TestNewSignatureScheme(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve found", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
		ss := NewECDSASignatureScheme(cv, sha256.New())
		if ss.css.cvp != cv.ec {
			fmt.Println("Error: curve mismatch?")
			t.FailNow()
		}
	}
}

func TestSignVerify(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve found", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
		G := NewPointGenerator(cv)
		ss := NewECDSASignatureScheme(cv, sha256.New())
		if ss.css.cvp != cv.ec {
			fmt.Println("Error: curve mismatch?")
			t.FailNow()
		}
		sK := NewFieldElementURandom(cv.GetAttr("n"))
		pK := NewPointNeutral(cv)
		pK.Mul(G, sK.AsInt())
		sig := ss.Sign(sK, []byte("test"))
		if sig == nil {
			fmt.Println("Error: nil Signature returned from Sign")
			t.FailNow()
		}
		sigbytes := sig.AsBytes()
		//hexdigest := hex.EncodeToString(sigbytes)
		//fmt.Printf("signature of \"test\" on curve %s is %s\n", cvn, hexdigest)
		valid := sig.Verify(pK, []byte("test"))
		if valid != true {
			fmt.Println("Error: Signature verify failed")
			t.FailNow()
		}
		sig2 := ss.SignatureFromBytes(sigbytes)
		if sig2 == nil {
			fmt.Println("Error: Signature import failed")
			t.FailNow()
		}
		valid2 := sig2.Verify(pK, []byte("test"))
		if valid2 != true {
			fmt.Println("Error: Signature verify failed")
			t.FailNow()
		}
		for i := 0; i < 100; i++ {
			sK2 := NewFieldElementURandom(cv.GetAttr("n"))
			pK2 := NewPointNeutral(cv)
			pK2.Mul(G, sK2.AsInt())
			if sig.Verify(pK2, []byte("test")) {
				fmt.Println("Error: Signature verify worked for random key")
				t.FailNow()
			}
		}
	}
}
