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

from ECC import FieldElement, ECurve, ECPoint, ECElgamalCiphertext
from example_der import der_decode_pubkey, der_encode_ecelgamal_ctxt
from example_pem import pem_wrap, pem_unwrap
from argparse import ArgumentParser
import base64
import sys
from hashlib import sha256
import asn1
import pysodium

desc = ('ecelgamal_enc encrypts an ecdh public key (point) with a public '
        'key (point)')

parser = ArgumentParser(description=desc)
parser.add_argument('-k', '--pubkey', default=None, help='file path for file containing public key')
parser.add_argument('-f', '--file', default=None, help='read message plaintext from file instead of stdin')
clargs = parser.parse_args()

if clargs.pubkey is None:
    sys.exit('Error: -k / --key option for public key file is mandatory.')

if clargs.pubkey is None:
    Inkey = sys.stdin.read()
else:
    with open(clargs.pubkey, 'r') as keyfile:
        Inkey=keyfile.read()

DERkey = pem_unwrap(Inkey, 'ECDH PUBLIC KEY')
if DERkey == None:
    sys.exit('unable to decode ECDH PUBLIC KEY in base64 PEM format')

try:
    (Pubkey, curve) = der_decode_pubkey(DERkey)
except ValueError:
    sys.exit('Error: Unable to import private key, aborting.')

if Pubkey is None:
    sys.exit('Error: Unable to import public key, aborting.')

if clargs.file is None:
    ptxt_wrapped = sys.stdin.read()
else:
    with open(clargs.file, 'r') as msgfile:
        ptxt_wrapped=msgfile.read()

if (ptxt_wrapped is None) or (len(ptxt_wrapped) == 0):
    sys.exit('Error: Plaintext length 0, aborting.')

DERptxt = pem_unwrap(ptxt_wrapped, 'ECDH PUBLIC KEY')
if DERptxt == None:
    sys.exit('unable to decode plaintext (ECDH PUBLIC KEY) in base64 PEM format')

try:
    (Plaintext, curve2) = der_decode_pubkey(DERptxt)
except ValueError:
    sys.exit('Error: Unable to import private key, aborting.')

if Plaintext is None:
    sys.exit('Error: Unable to import public key, aborting.')

if curve != curve2:
    sys.exit('Error: key and plaintext must be on the same curve, aborting.')

ctext = ECElgamalCiphertext.encrypt(Pubkey, Plaintext)
DERmsg = der_encode_ecelgamal_ctxt(ctext, curve)

print(pem_wrap(DERmsg, 'ECELGAMAL ENCRYPTED MESSAGE'))
