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

import ast
import unittest
import ECC
from ECC import FieldElement, ECurve, ECPoint, ECElgamalCiphertext
from Crypto.Hash import SHA256, SHA384, SHA512
from binascii import hexlify, unhexlify


class TestECElgamal(unittest.TestCase):

    def setUp(self):
        self.cv = []
        self.cv.append(ECurve('secp256k1'))
        self.cv.append(ECurve('secp256r1'))
        self.cv.append(ECurve('E-222'))
        self.cv.append(ECurve('M-221'))
        self.cv.append(ECurve('Ed25519'))

    def test_encrypt_decrypt(self):
        pts = []
        for c in self.cv:
            G = ECPoint(c, c.G)
            sK = FieldElement.urandom(c.n)
            pK = G * sK
            ptxt = ECPoint.urandom(c)
            ctxt = ECElgamalCiphertext.encrypt(pK, ptxt)
            pdec = ctxt.decrypt(sK)
            assert (pdec == ptxt)
            Cpt = ctxt.C
            Dpt = ctxt.D
            ctxt2 = ECElgamalCiphertext(Cpt, Dpt)
            pdec2 = ctxt2.decrypt(sK)
            assert (pdec2 == ptxt)
            for i in range(0, 100):
                sK2 = FieldElement.urandom(c.n)
                pdec2 = ctxt.decrypt(sK2)
                assert (pdec2 != pdec)


if __name__ == '__main__':
    unittest.main()
