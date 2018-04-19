# BSD 3-Clause License
#
# Copyright (c) 2018, Joseph deBlaquiere <jadeblaquiere@yahoo.com>
# All rights reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of ecpy nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import unittest
import ECC
from ECC import FieldElement, ECurve, ECPoint


class TestECPoint(unittest.TestCase):

    def setUp(self):
        self.cv = []
        self.cv.append(ECurve('secp256k1'))
        self.cv.append(ECurve('secp256r1'))
        self.cv.append(ECurve('E-222'))
        self.cv.append(ECurve('M-221'))
        self.cv.append(ECurve('Ed25519'))

    def test_create_neutral(self):
        pts = []
        for c in self.cv:
            pts.append(ECPoint(c))

    def test_create_generator(self):
        pts = []
        for c in self.cv:
            G = c.G
            pts.append(ECPoint(c, G))

    def test_repr(self):
        pts = []
        for c in self.cv:
            G = c.G
            pG = ECPoint(c, G)
            pts.append(pG)
            self.assertEqual(c.G, pG.affine())
        for p in pts:
            pGr = repr(p)
            p2 = eval(pGr)
            self.assertEqual(p, p2)

    def test_add_neutral(self):
        for c in self.cv:
            G = c.G
            pG = ECPoint(c, G)
            pN = ECPoint(c)
            p1 = pG + pN
            p2 = pN + pG
            self.assertEqual(p1, pG)
            self.assertEqual(p2, pG)
            self.assertNotEqual(p1, pN)

    def test_scalar_mult(self):
        for c in self.cv:
            G = c.G
            pG = ECPoint(c, G)
            p0 = ECPoint(c)
            for i in range(0, 10):
                pI = pG * i
                pII = i * pG
                pIn = pG * (-i)
                self.assertEqual(pI, pII)
                self.assertEqual(pI + pIn, p0)
                pA = p0
                for _ in range(0, i):
                    pA += pG
                self.assertEqual(pI, pA)

    def test_scalar_base_mult(self):
        for c in self.cv:
            G = c.G
            pG = ECPoint(c, G)
            pGb = ECPoint(c, G)
            pGb.setup_basemult()
            self.assertEqual(pG, pGb)
            self.assertIsNot(pG, pGb)
            for i in range(0, 10):
                pI = pG * i
                pII = pGb * i
                self.assertEqual(pI, pII)

    # there is a finite (but extremely small) probability that this test
    # fails because two random draws are identical (or 0; or 1)
    # but that probabibilty is essentially 102 / c.n, where c.n is
    # >100 bits long..., so if it happens once in your life you are... lucky!
    def test_urandom(self):
        for c in self.cv:
            G = c.G
            pG = ECPoint(c, G)
            pN = ECPoint(c)
            pR = []
            for i in range(0, 100):
                pR.append(ECPoint.urandom(c))
            for i in range(0, 100):
                pC = ECPoint(c, str(pR[i]))
                self.assertEqual(pC, pR[i])
                self.assertNotEqual(pC, pG)
                self.assertNotEqual(pC, pN)
                for j in range(i+1, 100):
                    self.assertNotEqual(pC, pR[j])

    def test_ecdh(self):
        for c in self.cv:
            n = c.n
            pG = ECPoint(c, c.G)
            a = FieldElement.urandom(n)
            b = FieldElement.urandom(n)
            self.assertNotEqual(a, b)
            A = a * pG
            B = b * pG
            self.assertNotEqual(A, B)
            self.assertEqual(A * b, a * B)


if __name__ == '__main__':
    unittest.main()
