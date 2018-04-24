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
	"fmt"
	"math/big"
	"testing"
)

func TestInitNamedCurves(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve found", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
	}
}

func TestValidateTypeEnums(t *testing.T) {
	if !validateTypeEnums() {
		fmt.Println("Error: enum mismatch c<->go")
		t.FailNow()
	}
}

func TestCopyWSCurves(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		cv := NamedCurve(cvn)
		//fmt.Println("Copying curve", cvn)
		ctype := cv.Type()
		p := cv.GetAttr("p")
		if p == nil {
			fmt.Println("GetAttr failed for p")
			t.FailNow()
		}
		n := cv.GetAttr("n")
		if n == nil {
			fmt.Println("GetAttr failed for n")
			t.FailNow()
		}
		h := cv.GetAttr("h")
		if h == nil {
			fmt.Println("GetAttr failed for h")
			t.FailNow()
		}
		gx := cv.GetAttr("gx")
		if gx == nil {
			fmt.Println("GetAttr failed for gx")
			t.FailNow()
		}
		gy := cv.GetAttr("gy")
		if gy == nil {
			fmt.Println("GetAttr failed for gy")
			t.FailNow()
		}
		bits := cv.GetAttr("bits")
		if bits == nil {
			fmt.Println("GetAttr failed for bits")
			t.FailNow()
		}
		// not available in older golang... hold it out for now
		// if bits.IsUint64() != true {
		// 	   t.FailNow()
		// }
		if bits.Cmp(big.NewInt(1024)) > 0 {
			fmt.Println("bits out of range")
			t.FailNow()
		}
		if ctype == EQTypeShortWeierstrass {
			a := cv.GetAttr("ws_a")
			if a == nil {
				fmt.Println("GetAttr failed for a")
				t.FailNow()
			}
			b := cv.GetAttr("ws_b")
			if b == nil {
				fmt.Println("GetAttr failed for b")
				t.FailNow()
			}
			cvcp := ShortWeierstrassCurve(p, a, b, n, h, gx, gy, uint(bits.Uint64()))
			if cv.Cmp(cvcp) != 0 {
				fmt.Println("curves not equal?")
				t.FailNow()
			}
		}
		if ctype == EQTypeEdwards {
			c := cv.GetAttr("ed_c")
			if c == nil {
				fmt.Println("GetAttr failed for c")
				t.FailNow()
			}
			d := cv.GetAttr("ed_d")
			if d == nil {
				fmt.Println("GetAttr failed for d")
				t.FailNow()
			}
			cvcp := EdwardsCurve(p, c, d, n, h, gx, gy, uint(bits.Uint64()))
			if cv.Cmp(cvcp) != 0 {
				fmt.Println("curves not equal?")
				t.FailNow()
			}
		}
		if ctype == EQTypeMontgomery {
			B := cv.GetAttr("mo_B")
			if B == nil {
				fmt.Println("GetAttr failed for B")
				t.FailNow()
			}
			A := cv.GetAttr("mo_A")
			if A == nil {
				fmt.Println("GetAttr failed for A")
				t.FailNow()
			}
			cvcp := MontgomeryCurve(p, B, A, n, h, gx, gy, uint(bits.Uint64()))
			if cv.Cmp(cvcp) != 0 {
				fmt.Println("curves not equal?")
				t.FailNow()
			}
		}
		if ctype == EQTypeTwistedEdwards {
			a := cv.GetAttr("te_a")
			if a == nil {
				fmt.Println("GetAttr failed for a(te)")
				t.FailNow()
			}
			d := cv.GetAttr("te_d")
			if d == nil {
				fmt.Println("GetAttr failed for d(te)")
				t.FailNow()
			}
			cvcp := TwistedEdwardsCurve(p, a, d, n, h, gx, gy, uint(bits.Uint64()))
			if cv.Cmp(cvcp) != 0 {
				fmt.Println("curves not equal?")
				t.FailNow()
			}
		}
	}
}

func TestPointCheckGenerator(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		cv := NamedCurve(cvn)
		//fmt.Println("Validating Generator for curve", cvn)
		gx := cv.GetAttr("gx")
		if gx == nil {
			fmt.Println("GetAttr failed for gx")
			t.FailNow()
		}
		gy := cv.GetAttr("gy")
		if gy == nil {
			fmt.Println("GetAttr failed for gy")
			t.FailNow()
		}
		if !cv.PointCheck(gx, gy) {
			fmt.Println("Generator not on curve?")
			t.FailNow()
		}
	}
}
