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

from ECC import FieldElement, ECurve
from example_der import der_encode_privkey
from example_pem import pem_wrap
from argparse import ArgumentParser
import base64
import sys

desc = ('ecdh-gen generates a private key for elliptic curve cryptography '
        'based on a specific curve. The private key value is simply a '
        'random number ')

parser = ArgumentParser(description=desc)
parser.add_argument('-l', '--list_curves', action='store_true', help='list supported curves')
parser.add_argument('-c', '--curve', default='secp256k1', help='choose standard curve')
clargs = parser.parse_args()

if clargs.list_curves:
    clist = ECurve._CurveNames()
    for c in clist:
        print(c)
    sys.exit(0)

curve = ECurve(clargs.curve)
privkey = int(FieldElement.urandom(curve.p))

DERkey = der_encode_privkey(privkey, curve)

print(pem_wrap(DERkey, 'ECDH PRIVATE KEY'))
