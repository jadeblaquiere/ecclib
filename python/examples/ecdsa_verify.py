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

from ECC import ECDSASignatureScheme, ECDSASignature
from example_der import der_decode_pubkey
from example_pem import pem_unwrap
from argparse import ArgumentParser
import sys
from Crypto.Hash import SHA512


desc = ('ecdh_enc generate a shared ephemeral (ECDHE) key and encrypts a '
        'message using the XSalsa20 stream cipher using that key. Output is '
        'DER encoded and PEM-wrapped')

parser = ArgumentParser(description=desc)
parser.add_argument('-k', '--pubkey', default=None, help='file path for file containing private key')
parser.add_argument('-f', '--file', default=None, help='read message plaintext from file instead of stdin')
clargs = parser.parse_args()

if clargs.pubkey is None:
    sys.exit('Error: -k / --pubkey option for public key file is mandatory.')

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
    signed_msg = sys.stdin.read()
else:
    with open(clargs.file, 'r') as msgfile:
        signed_msg=msgfile.read()

if (signed_msg is None) or (len(signed_msg) == 0):
    sys.exit('Error: Signed input length 0, aborting.')

message = pem_unwrap(signed_msg, 'ECDSA SIGNED MESSAGE')
if message == None:
    sys.exit('unable to decode ECDSA SIGNED MESSAGE in base64 PEM format')

sigbytes = pem_unwrap(signed_msg, 'ECDSA SIGNATURE')
if message == None:
    sys.exit('unable to decode ECDSA SIGNATURE in base64 PEM format')

ss = ECDSASignatureScheme(curve, SHA512)
sig = ECDSASignature(ss, sigbytes)
if sig.Verify(Pubkey, message) != True:
    sys.exit('verify signature failed')

print(message.decode(), end='')
