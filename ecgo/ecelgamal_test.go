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
	//"crypto/sha256"
	//"crypto/sha512"
	//"encoding/hex"
	"fmt"
	"testing"
)

func TestElgamalEncryptDecrypt(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve found", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
		n := cv.GetAttr("n")
		sK := NewFieldElementURandom(n).AsInt()
		G := NewPointGenerator(cv)
		pK := NewPointNeutral(cv).Mul(G, sK)
		ptxt := NewPointURandom(cv)
		ctxt := ECElgamalEncrypt(pK, ptxt)
		ptxt2 := ctxt.Decrypt(sK)
		if ptxt.Cmp(ptxt2) != 0 {
			fmt.Println("Decrypt(Encrypt(M)) != M")
			t.FailNow()
		}
		C := ctxt.C()
		D := ctxt.D()
		ctxt2 := NewECElgamalCiphertext(C, D)
		ptxt3 := ctxt2.Decrypt(sK)
		if ptxt.Cmp(ptxt3) != 0 {
			fmt.Println("Decrypt(Encrypt(M)) != M")
			t.FailNow()
		}
		for i := 0; i < 50; i++ {
			sK2 := NewFieldElementURandom(n).AsInt()
			ptxt4 := ctxt.Decrypt(sK2)
			if ptxt.Cmp(ptxt4) == 0 {
				fmt.Println("Message decrypted sucessfully for random key")
				t.FailNow()
			}
		}
	}
}
