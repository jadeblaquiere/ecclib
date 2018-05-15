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

from ECC import FieldElement, ECurve, ECPoint
from example_der import der_decode_privkey, der_decode_message
from example_pem import pem_wrap, pem_unwrap
from argparse import ArgumentParser
import base64
import binascii
import sys
from hashlib import sha256
import asn1
import pysodium

desc = ('ecdh_dec decrypts a message ecnrypted with the ChaCha20 stream cipher '
        'with a shared ephemeral (ECDHE) key.')

parser = ArgumentParser(description=desc)
parser.add_argument('-k', '--privkey', default=None, help='file path for file containing private key')
parser.add_argument('-f', '--file', default=None, help='read message plaintext from file instead of stdin')
clargs = parser.parse_args()

if clargs.privkey is None:
    Inkey = sys.stdin.read()
else:
    with open(clargs.privkey, 'r') as keyfile:
        Inkey=keyfile.read()

DERkey = pem_unwrap(Inkey, 'ECDH PRIVATE KEY')
if DERkey == None:
    sys.exit('unable to decode ECDH PRIVATE KEY in base64 PEM format')

try:
    (privkey, curve) = der_decode_privkey(DERkey)
except ValueError:
    sys.exit('Error: Unable to import private key, aborting.')

if privkey is None:
    sys.exit('Error: Unable to import public key, aborting.')

if clargs.file is None:
    inCtxt = sys.stdin.read()
else:
    with open(clargs.file, 'r') as msgfile:
        inCtxt=msgfile.read()

ctder = pem_unwrap(inCtxt, 'ECDHE_XSALSA20 ENCRYPTED MESSAGE')
if ctder is None:
    sys.exit('unable to decode ECDHE_XSALSA20 ENCRYPTED MESSAGE in base64 PEM format')

(ePubkeybytes, nonce, ctext) = der_decode_message(ctder)

try:
    ePubkey = ECPoint(curve, ePubkeybytes)
except:
    sys.exit('Pubkey value invalid for curve')

# generate a random (ephemeral) private key
SharedPt = ePubkey * privkey
sbytes = SharedPt.compressed()
key = sha256(sbytes).digest()

ptext = pysodium.crypto_stream_xor(ctext, len(ctext), nonce, key)

print(ptext.decode(), end='')
