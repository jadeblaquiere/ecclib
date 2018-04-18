#BSD 3-Clause License
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
from ECC import ECurve
#from binascii import hexlify, unhexlify

class TestECurve(unittest.TestCase):

    def test_named(self):
        cvws = ECurve('secp112r1')
        self.assertIsNotNone(cvws)
        cved = ECurve('E-222')
        self.assertIsNotNone(cved)
        cvmo = ECurve('M-221')
        self.assertIsNotNone(cvmo)
        cvte = ECurve('Ed25519')
        self.assertIsNotNone(cvte)
        with self.assertRaises(ValueError):
            cvne = ECurve('thisdoesnotexist')

    def test_parameters(self):
        cvws1 = ECurve('secp112r1')
        self.assertIsNotNone(cvws1)
        cvws2 = ECurve.ShortWeierstrass(0xDB7C2ABF62E35E668076BEAD208B,
            0xDB7C2ABF62E35E668076BEAD2088, 0x659EF8BA043916EEDE8911702B22,
            0xDB7C2ABF62E35E7628DFAC6561C5, 1,
            0x09487239995A5EE76B55F9C2F098, 0xA89CE5AF8724C0A23E0E0FF77500, 
            112)
        self.assertIsNotNone(cvws2)
        self.assertEqual(cvws1, cvws2)
        #
        cved1 = ECurve('E-222')
        self.assertIsNotNone(cved1)
        cved2 = ECurve.Edwards(0x3fffffffffffffffffffffffffffffffffffffffffffffffffffff8b,
            1, 160102,
            0xffffffffffffffffffffffffffff70cbc95e932f802f31423598cbf, 4,
            0x19b12bb156a389e55c9768c303316d07c23adab3736eb2bc3eb54e51, 0x1c, 
            222)
        self.assertIsNotNone(cved2)
        self.assertEqual(cved1, cved2)
        #
        cvmo1 = ECurve('M-221')
        self.assertIsNotNone(cvmo1)
        cvmo2 = ECurve.Montgomery(0x1ffffffffffffffffffffffffffffffffffffffffffffffffffffffd,
            1, 117050,
            0x40000000000000000000000000015a08ed730e8a2f77f005042605b, 8,
            0x4, 0xf7acdd2a4939571d1cef14eca37c228e61dbff10707dc6c08c5056d, 
            221)
        self.assertIsNotNone(cvmo2)
        self.assertEqual(cvmo1, cvmo2)
        #
        cvte1 = ECurve('Ed25519')
        self.assertIsNotNone(cvte1)
        cvte2 = ECurve.TwistedEdwards(0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED,
            0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC, 
            0x52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3,
            0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED,
            8,
            0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A,
            0x6666666666666666666666666666666666666666666666666666666666666658, 
            255)
        self.assertIsNotNone(cvte2)
        self.assertEqual(cvte1, cvte2)

    def test_point_check(self):
        cvws = ECurve('secp112r1')
        self.assertIsNotNone(cvws)
        status = cvws.PointIsValid(0x09487239995A5EE76B55F9C2F098, 
            0xA89CE5AF8724C0A23E0E0FF77500)
        self.assertEqual(status, True)
        status = cvws.PointIsValid(0x09487239995A5EE76B55F9C2F098, 
            0)
        self.assertEqual(status, False)
        #
        cved = ECurve('E-222')
        self.assertIsNotNone(cved)
        status = cved.PointIsValid(0x19b12bb156a389e55c9768c303316d07c23adab3736eb2bc3eb54e51, 
            0x1c)
        self.assertEqual(status, True)
        status = cved.PointIsValid(0x19b12bb156a389e55c9768c303316d07c23adab3736eb2bc3eb54e51, 
            0)
        self.assertEqual(status, False)
        #
        cvmo = ECurve('M-221')
        self.assertIsNotNone(cvmo)
        status = cvmo.PointIsValid(0x4, 
            0xf7acdd2a4939571d1cef14eca37c228e61dbff10707dc6c08c5056d)
        self.assertEqual(status, True)
        status = cvmo.PointIsValid(0x4, 
            0)
        self.assertEqual(status, False)
        #
        cvte = ECurve('Ed25519')
        self.assertIsNotNone(cvte)
        status = cvte.PointIsValid(0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A, 
            0x6666666666666666666666666666666666666666666666666666666666666658)
        self.assertEqual(status, True)
        status = cvte.PointIsValid(0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A, 
            0)
        self.assertEqual(status, False)

    def test_getattr(self):
        curves = ECurve._CurveNames()
        for c in curves:
            cva = ECurve(c)
            p = cva.p
            n = cva.n
            h = cva.h
            G = cva.G
            bits = cva.bits
            ctype = cva.ctype
            if ctype == 'ShortWeierstrass':
                a = cva.a
                b = cva.b
                cvb = ECurve.ShortWeierstrass(p, a, b, n, h, G[0], G[1], bits)
            elif ctype == 'Edwards':
                c = cva.c
                d = cva.d
                cvb = ECurve.Edwards(p, c, d, n, h, G[0], G[1], bits)
            elif ctype == 'Montgomery':
                B = cva.B
                A = cva.A
                cvb = ECurve.Montgomery(p, B, A, n, h, G[0], G[1], bits)
            else:
                self.assertEqual(ctype, "TwistedEdwards")
                a = cva.a
                d = cva.d
                cvb = ECurve.TwistedEdwards(p, a, d, n ,h, G[0], G[1], bits)
            self.assertEqual(cva, cvb)

if __name__ == '__main__':
    unittest.main()
