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

const p41417_str = "3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef"
const x41417_str = "1a334905141443300218c0631c326e5fcd46369f44c03ec7f57ff35498a4ab4d6d6ba111301a73faa8537c64c4fd3812f3cbc595"

func TestNewFieldElement(t *testing.T) {
	v, _ := new(big.Int).SetString(x41417_str, 16)
	if v == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}
	p, _ := new(big.Int).SetString(p41417_str, 16)
	if p == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}
	fe := NewFieldElement(v, p)
	if fe == nil {
		fmt.Println("Error: nil returned from Gen")
		t.FailNow()
	}
	fe = NewFieldElementUI(1, p)
	if fe == nil {
		fmt.Println("Error: nil returned from Gen")
		t.FailNow()
	}
}

func TestConvertFieldElementToInt(t *testing.T) {
	v, _ := new(big.Int).SetString(x41417_str, 16)
	if v == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}
	p, _ := new(big.Int).SetString(p41417_str, 16)
	if p == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}
	fe := NewFieldElement(v, p)
	if fe == nil {
		fmt.Println("Error: nil returned from Gen")
		t.FailNow()
	}
	i := fe.AsInt()
	n := fe.Order()
	if v.Cmp(i) != 0 {
		fmt.Println("Error: value != i")
		t.FailNow()
	}
	if p.Cmp(n) != 0 {
		fmt.Println("Error: order != p")
		t.FailNow()
	}
	fe = NewFieldElementUI(1, p)
	if fe == nil {
		fmt.Println("Error: nil returned from Gen")
		t.FailNow()
	}
	i = fe.AsInt()
	n = fe.Order()
	if i.Cmp(big.NewInt(1)) != 0 {
		fmt.Println("Error: value != i")
		t.FailNow()
	}
	if p.Cmp(n) != 0 {
		fmt.Println("Error: order != p")
		t.FailNow()
	}
	fe = NewFieldElementUI(0, p)
	if fe == nil {
		fmt.Println("Error: nil returned from Gen")
		t.FailNow()
	}
	i = fe.AsInt()
	n = fe.Order()
	if i.Cmp(big.NewInt(0)) != 0 {
		fmt.Println("Error: value != i")
		t.FailNow()
	}
	if p.Cmp(n) != 0 {
		fmt.Println("Error: order != p")
		t.FailNow()
	}
}

func TestConvertFieldElementUrandom(t *testing.T) {
	var fe []*FieldElement

	p, _ := new(big.Int).SetString(p41417_str, 16)
	if p == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}

	for i := 0; i < 100; i++ {
		fe = append(fe, NewFieldElementURandom(p))
	}

	for i := 0; i < 100; i++ {
		for j := (i + 1); j < 100; j++ {
			if fe[i].Cmp(fe[j]) == 0 {
				fmt.Println("Error: match on random values")
				t.FailNow()
			}
		}
	}
}

func TestFieldElementBasicMath(t *testing.T) {
	var err error

	field := big.NewInt(79)
	fsum := NewFieldElementUI(0, field)
	rsum := NewFieldElementUI(0, field)
	fdiff := NewFieldElementUI(0, field)
	rdiff := NewFieldElementUI(0, field)
	fmul := NewFieldElementUI(0, field)
	rmul := NewFieldElementUI(0, field)
	ainv := NewFieldElementUI(0, field)
	mulinv := NewFieldElementUI(0, field)
	sqrt := NewFieldElementUI(0, field)
	exp := NewFieldElementUI(0, field)
	cc := NewFieldElementUI(0, field)
	fsum2 := big.NewInt(0)
	fdiff2 := big.NewInt(0)
	fmul2 := big.NewInt(0)
	sqrt2 := big.NewInt(0)
	exp2 := big.NewInt(0)
	c := big.NewInt(0)

	for i := 0; i < 200; i++ {
		a := big.NewInt(int64(i))
		aa := NewFieldElement(a, field)
		for j := 0; j < 200; j++ {
			b := big.NewInt(int64(j))
			bb := NewFieldElement(b, field)
			fsum.Add(aa, bb)
			rsum.Add(bb, aa)
			fdiff.Sub(aa, bb)
			rdiff.Sub(bb, aa)
			fmul.Mul(aa, bb)
			rmul.Mul(bb, aa)
			c.Add(a, b)
			fsum2.Mod(c, field)
			c.Sub(a, b)
			fdiff2.Mod(c, field)
			c.Mul(a, b)
			fmul2.Mod(c, field)
			if fsum.Cmp(rsum) != 0 {
				fmt.Println("Error: addition doesn't commute?")
				t.FailNow()
			}
			dd := NewFieldElementUI(0, field)
			dd.Neg(rdiff)
			if fdiff.Cmp(dd) != 0 {
				fmt.Println("Error: subtraction doesn't commute (* -1)?")
				t.FailNow()
			}
			if fmul.Cmp(rmul) != 0 {
				fmt.Println("Error: multiplication doesn't commute?")
				t.FailNow()
			}
			b = fsum.AsInt()
			if b.Cmp(fsum2) != 0 {
				fmt.Println("Error: aa + bb != (a + b) % p?")
				t.FailNow()
			}
			b = fdiff.AsInt()
			if b.Cmp(fdiff2) != 0 {
				fmt.Println("Error: aa - bb != (a - b) % p?")
				t.FailNow()
			}
			b = fmul.AsInt()
			if b.Cmp(fmul2) != 0 {
				fmt.Println("Error: aa * bb != (a * b) % p?")
				t.FailNow()
			}
			_, err = ainv.Inv(aa)
			if err == nil {
				mulinv.Mul(aa, ainv)
				if mulinv.Cmp(NewFieldElementUI(1, field)) != 0 {
					fmt.Println("Error: aa * inv(aa) != 1?")
					t.FailNow()
				}
			} else {
				if aa.Cmp(NewFieldElementUI(0, field)) != 0 {
					fmt.Println("Inverse doesn't exist for nonzero a?")
					t.FailNow()
				}
			}
			_, err = sqrt.Sqrt(fmul)
			if err == nil {
				if sqrt2.ModSqrt(fmul2, field) == nil {
					fmt.Println("Error: big.Int.ModSqrt failed when FE.Sqrt succeeded?")
					t.FailNow()
				}
				if sqrt2.Cmp(sqrt.AsInt()) != 0 {
					fmt.Println("Error: FE.sqrt() != big.Int.ModSqrt()")
					t.FailNow()
				}
				cc.Mul(sqrt, sqrt)
				if fmul.Cmp(cc) != 0 {
					fmt.Println("Error: sqrt(x) ** 2 != x?")
					t.FailNow()
				}
			}
			exp.Exp(aa, uint64(j))
			exp2.Exp(a, big.NewInt(int64(j)), field)
			if exp2.Cmp(exp.AsInt()) != 0 {
				fmt.Println("Error: FE.exp() != big.Int.exp()")
				t.FailNow()
			}
		}
	}
}

func TestMathUrandom(t *testing.T) {
	var err error
	var fe []*FieldElement
	var bi []*big.Int

	field, _ := new(big.Int).SetString(p41417_str, 16)
	if field == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}

	for i := 0; i < 100; i++ {
		fe = append(fe, NewFieldElementURandom(field))
		bi = append(bi, fe[i].AsInt())
	}

	fsum := NewFieldElementUI(0, field)
	rsum := NewFieldElementUI(0, field)
	fdiff := NewFieldElementUI(0, field)
	rdiff := NewFieldElementUI(0, field)
	fmul := NewFieldElementUI(0, field)
	rmul := NewFieldElementUI(0, field)
	ainv := NewFieldElementUI(0, field)
	mulinv := NewFieldElementUI(0, field)
	sqrt := NewFieldElementUI(0, field)
	exp := NewFieldElementUI(0, field)
	cc := NewFieldElementUI(0, field)
	fsum2 := big.NewInt(0)
	fdiff2 := big.NewInt(0)
	fmul2 := big.NewInt(0)
	sqrt2 := big.NewInt(0)
	exp2 := big.NewInt(0)
	c := big.NewInt(0)

	for i := 0; i < 100; i++ {
		a := bi[i]
		aa := fe[i]
		for j := 0; j < 100; j++ {
			b := bi[j]
			bb := fe[j]
			fsum.Add(aa, bb)
			rsum.Add(bb, aa)
			fdiff.Sub(aa, bb)
			rdiff.Sub(bb, aa)
			fmul.Mul(aa, bb)
			rmul.Mul(bb, aa)
			c.Add(a, b)
			fsum2.Mod(c, field)
			c.Sub(a, b)
			fdiff2.Mod(c, field)
			c.Mul(a, b)
			fmul2.Mod(c, field)
			if fsum.Cmp(rsum) != 0 {
				fmt.Println("Error: addition doesn't commute?")
				t.FailNow()
			}
			dd := NewFieldElementUI(0, field)
			dd.Neg(rdiff)
			if fdiff.Cmp(dd) != 0 {
				fmt.Println("Error: subtraction doesn't commute (* -1)?")
				t.FailNow()
			}
			if fmul.Cmp(rmul) != 0 {
				fmt.Println("Error: multiplication doesn't commute?")
				t.FailNow()
			}
			b = fsum.AsInt()
			if b.Cmp(fsum2) != 0 {
				fmt.Println("Error: aa + bb != (a + b) % p?")
				t.FailNow()
			}
			b = fdiff.AsInt()
			if b.Cmp(fdiff2) != 0 {
				fmt.Println("Error: aa - bb != (a - b) % p?")
				t.FailNow()
			}
			b = fmul.AsInt()
			if b.Cmp(fmul2) != 0 {
				fmt.Println("Error: aa * bb != (a * b) % p?")
				t.FailNow()
			}
			_, err = ainv.Inv(aa)
			if err == nil {
				mulinv.Mul(aa, ainv)
				if mulinv.Cmp(NewFieldElementUI(1, field)) != 0 {
					fmt.Println("Error: aa * inv(aa) != 1?")
					t.FailNow()
				}
			} else {
				if aa.Cmp(NewFieldElementUI(0, field)) != 0 {
					fmt.Println("Inverse doesn't exist for nonzero a?")
					t.FailNow()
				}
			}
			_, err = sqrt.Sqrt(fmul)
			if err == nil {
				if sqrt2.ModSqrt(fmul2, field) == nil {
					fmt.Println("Error: big.Int.ModSqrt failed when FE.Sqrt succeeded?")
					t.FailNow()
				}
				if sqrt2.Cmp(sqrt.AsInt()) != 0 {
					fmt.Println("Error: FE.sqrt() != big.Int.ModSqrt()")
					t.FailNow()
				}
				cc.Mul(sqrt, sqrt)
				if fmul.Cmp(cc) != 0 {
					fmt.Println("Error: sqrt(x) ** 2 != x?")
					t.FailNow()
				}
			}
			exp.Exp(aa, uint64(j))
			exp2.Exp(a, big.NewInt(int64(j)), field)
			if exp2.Cmp(exp.AsInt()) != 0 {
				fmt.Println("Error: FE.exp() != big.Int.exp()")
				t.FailNow()
			}
		}
	}
}

func TestTstBitUrandom(t *testing.T) {
	var fe []*FieldElement

	p, _ := new(big.Int).SetString(p41417_str, 16)
	if p == nil {
		fmt.Println("Error: nil returned from SetString")
		t.FailNow()
	}

	for i := 0; i < 100; i++ {
		fe = append(fe, NewFieldElementURandom(p))
	}

	for i := 0; i < 100; i++ {
		for j := 0; j < 414; j++ {
			bi := fe[i].AsInt()
			if fe[i].Bit(j) != bi.Bit(j) {
				fmt.Println("Error: FE.Bit() != big.Int.Bit()")
				t.FailNow()
			}
		}
	}
}
