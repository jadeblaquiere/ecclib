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
from ECC import FieldElement, ECurve, ECPoint, ECDSASignatureScheme
from Crypto.Hash import SHA256, SHA384, SHA512
from binascii import unhexlify


class TestECDSA(unittest.TestCase):

    def setUp(self):
        self.cv = []
        self.cv.append(ECurve('secp256k1'))
        self.cv.append(ECurve('secp256r1'))
        self.cv.append(ECurve('E-222'))
        self.cv.append(ECurve('M-221'))
        self.cv.append(ECurve('Ed25519'))
        self.hm = []
        self.hm.append(SHA256)
        self.hm.append(SHA384)
        self.hm.append(SHA512)

    def test_basic_sign_verify(self):
        for c in self.cv:
            G = ECPoint(c, c.G)
            sK = FieldElement.urandom(c.n)
            pK = G * sK
            for h in self.hm:
                ss = ECDSASignatureScheme(c, h)
                sig = ss.Sign(sK, 'test')
                assert sig.Verify(pK, 'test')
                # print(repr(ss))


if __name__ == '__main__':
    unittest.main()
