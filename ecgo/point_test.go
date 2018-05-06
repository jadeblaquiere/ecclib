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
	"bytes"
	"encoding/hex"
	"fmt"
	"math/big"
	"testing"
)

func TestInitPointNeutral(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve found", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
		pt := NewPointNeutral(cv)
		if pt == nil {
			fmt.Println("Error: nil returned from NewPointNeutral")
			t.FailNow()
		}
		if (cv.Type() == EQTypeShortWeierstrass) ||
			(cv.Type() == EQTypeMontgomery) {
			isNeutral := pt.IsNeutral()
			if isNeutral != true {
				fmt.Println("Error: neutral point is not neutral?")
				t.FailNow()
			}
		}
	}
}

func TestInitPointG(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve G Pt:", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
		pt := NewPointGenerator(cv)
		if pt == nil {
			fmt.Println("Error: nil returned from NewPointNeutral")
			t.FailNow()
		}
		isNeutral := pt.IsNeutral()
		if isNeutral == true {
			fmt.Println("Error: neutral point is not neutral?")
			t.FailNow()
		}
		x, y := pt.Affine()
		if x.Cmp(cv.GetAttr("gx")) != 0 {
			fmt.Println("Error: generator x point mismatch")
			t.FailNow()
		}
		if y.Cmp(cv.GetAttr("gy")) != 0 {
			fmt.Println("Error: generator y point mismatch")
			t.FailNow()
		}
	}
}

func TestMathPointURandom(t *testing.T) {
	clist := CurveNames()
	for _, cvn := range clist {
		//fmt.Println("Curve G Pt:", cvn)
		cv := NamedCurve(cvn)
		if cv == nil {
			fmt.Println("Error: nil returned from NamedCurve")
			t.FailNow()
		}
		npt := NewPointNeutral(cv)
		if npt == nil {
			fmt.Println("Error: nil returned from NewPointNeutral")
			t.FailNow()
		}
		pt := NewPointURandom(cv)
		if pt == nil {
			fmt.Println("Error: nil returned from NewPointNeutral")
			t.FailNow()
		}
		isNeutral := pt.IsNeutral()
		if isNeutral == true {
			fmt.Println("Error: neutral point is not neutral?")
			t.FailNow()
		}
		ptstr1 := pt.StringCompressed()
		//fmt.Println("compressed point: ", ptstr1)
		ptcopy1 := NewPointFromString(ptstr1, cv)
		if pt.Cmp(ptcopy1) != 0 {
			fmt.Println("Error: export/import compressed string failed")
			t.FailNow()
		}
		ptstr2 := pt.StringUncompressed()
		ptcopy2 := NewPointFromString(ptstr2, cv)
		if pt.Cmp(ptcopy2) != 0 {
			fmt.Println("Error: export/import compressed string failed")
			t.FailNow()
		}
		ptbstr1 := pt.BytesCompressed()
		//fmt.Println("compressed point: ", ptstr1)
		ptbcopy1 := NewPointFromBytes(ptbstr1, cv)
		if pt.Cmp(ptbcopy1) != 0 {
			fmt.Println("Error: export/import compressed string failed")
			t.FailNow()
		}
		ptbbstr1, err := hex.DecodeString(ptstr1)
		if err != nil {
			fmt.Println("Error: unable to decode hex point string")
			t.FailNow()
		}
		if bytes.Compare(ptbbstr1, ptbstr1) != 0 {
			fmt.Println("Error: mismatch between byte and hex string repr")
			t.FailNow()
		}
		ptbstr2 := pt.BytesUncompressed()
		ptbcopy2 := NewPointFromBytes(ptbstr2, cv)
		if pt.Cmp(ptbcopy2) != 0 {
			fmt.Println("Error: export/import compressed string failed")
			t.FailNow()
		}
		ptbbstr2, err := hex.DecodeString(ptstr2)
		if err != nil {
			fmt.Println("Error: unable to decode hex point string")
			t.FailNow()
		}
		if bytes.Compare(ptbbstr2, ptbstr2) != 0 {
			fmt.Println("Error: mismatch between byte and hex string repr")
			t.FailNow()
		}
		x, y := pt.Affine()
		if x.Cmp(cv.GetAttr("gx")) == 0 {
			fmt.Println("Error: random point matches generator X?")
			t.FailNow()
		}
		if y.Cmp(cv.GetAttr("gy")) == 0 {
			fmt.Println("Error: random point matches generator Y?")
			t.FailNow()
		}
		ptcopy3 := NewPointXY(x, y, cv)
		if pt.Cmp(ptcopy3) != 0 {
			fmt.Println("Error: copy via X, Y failed")
			t.FailNow()
		}
		pplusn := NewPointNeutral(cv)
		pplusn.Add(pplusn, pt)
		if pt.Cmp(pplusn) != 0 {
			fmt.Println("Error: pt + neutral/0 != pt")
			t.FailNow()
		}
		pneg := NewPointNeutral(cv).Neg(pt)
		if pt.Cmp(pneg) == 0 {
			fmt.Println("Error: pt == -pt")
			t.FailNow()
		}
		pneg.Add(pt, pneg)
		if pneg.Cmp(npt) != 0 {
			fmt.Println("Error: pt + (-pt) != neutral/0")
			t.FailNow()
		}
		pgen := NewPointNeutral(cv).Set(pt)
		pgen.SetupBaseMul()
		if pgen.Cmp(pt) != 0 {
			fmt.Println("Error: P != Pg when Pg = Set(P)")
			t.FailNow()
		}
		for i := 0; i < 10; i++ {
			ibig := big.NewInt(int64(i))
			pmul := NewPointNeutral(cv).Mul(pt, ibig)
			accum := NewPointNeutral(cv)
			for j := 0; j < i; j++ {
				accum.Add(accum, pt)
			}
			if pmul.Cmp(accum) != 0 {
				fmt.Println("Error: i * P != SUM(1 -> i, P)")
				t.FailNow()
			}
			pgmul := NewPointNeutral(cv).Mul(pgen, ibig)
			if pgmul.Cmp(pmul) != 0 {
				fmt.Println("Error: i * P != i * Pg when Pg = P")
				t.FailNow()
			}
		}
		pdoub1 := NewPointNeutral(cv).Add(pt, pt)
		pdoub2 := NewPointNeutral(cv).Mul(pt, big.NewInt(2))
		if pdoub1.Cmp(pdoub2) != 0 {
			fmt.Println("Error: P + P != 2 * P")
			t.FailNow()
		}
		p2pmin1 := NewPointNeutral(cv).Sub(pdoub1, pt)
		if p2pmin1.Cmp(pt) != 0 {
			fmt.Println("Error: 2 * P - P != P")
			t.FailNow()
		}
	}
}
