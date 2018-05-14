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
import asn1
import sys
import binascii

def _int_to_bebytes(ivalue):
    ivalue = int(ivalue)
    assert(ivalue >= 0)
    bytelen = max(1, (ivalue.bit_length() + 7) >> 3)
    pformat = '%%0%dX' % (bytelen * 2)
    return binascii.unhexlify(pformat % ivalue)


def _bebytes_to_int(bvalue):
    hvalue = binascii.hexlify(bvalue)
    return int(hvalue, 16)


def ensure_tag(decoder, expected):
    tag = decoder.peek()
    if tag.nr != expected:
        raise ValueError("Error in DER format, expected tag %d, got %d" %
                         (expected, tag.nr))


def _der_encode_curve(curve, encoder):
    ctenum = 0
    cp0 = 0
    cp1 = 0
    ctype = curve.ctype
    if ctype == 'ShortWeierstrass':
        ctenum = 2
        cp0 = curve.a
        cp1 = curve.b
    elif ctype == 'Edwards':
        ctenum = 3
        cp0 = curve.c
        cp1 = curve.d
    elif ctype == 'Montgomery':
        ctenum = 4
        cp0 = curve.B
        cp1 = curve.A
    elif ctype == 'TwistedEdwards':
        ctenum = 5
        cp0 = curve.a
        cp1 = curve.d
    else:
        raise ValueError('Unknown Curve Type')

    # curve description
    encoder.enter(asn1.Numbers.Sequence)
    # field order
    encoder.write(_int_to_bebytes(curve.p), asn1.Numbers.OctetString)
    # curve type (enum)
    encoder.write(ctenum, asn1.Numbers.Enumerated)
    # curve equation coefficients
    encoder.enter(asn1.Numbers.Sequence)
    encoder.write(_int_to_bebytes(cp0), asn1.Numbers.OctetString)
    encoder.write(_int_to_bebytes(cp1), asn1.Numbers.OctetString)
    encoder.leave()
    # curve order
    encoder.write(_int_to_bebytes(curve.n), asn1.Numbers.OctetString)
    # curve cofactor
    encoder.write(_int_to_bebytes(curve.h), asn1.Numbers.OctetString)
    # curve generator point
    encoder.enter(asn1.Numbers.Sequence)
    encoder.write(_int_to_bebytes(curve.G[0]), asn1.Numbers.OctetString)
    encoder.write(_int_to_bebytes(curve.G[1]), asn1.Numbers.OctetString)
    encoder.leave()
    # curve field bitsize
    encoder.write(curve.bits, asn1.Numbers.Integer)
    encoder.leave()

def der_encode_privkey(privkey, curve):
    # encode key, curve to ASN1 DER format
    encoder = asn1.Encoder()
    encoder.start()
    encoder.enter(asn1.Numbers.Sequence)
    # privkey
    encoder.write(_int_to_bebytes(privkey), asn1.Numbers.OctetString)
    _der_encode_curve(curve, encoder)
    encoder.leave()
    return encoder.output()

def _der_decode_curve(decoder):
    # curve description
    ensure_tag(decoder, asn1.Numbers.Sequence)
    decoder.enter()
    # field order
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, p = decoder.read()
    p = _bebytes_to_int(p)
    # curve type (enum)
    ensure_tag(decoder, asn1.Numbers.Enumerated)
    tag, ctype = decoder.read()
    # curve equation coefficients
    ensure_tag(decoder, asn1.Numbers.Sequence)
    decoder.enter()
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, cp0 = decoder.read()
    cp0 = _bebytes_to_int(cp0)
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, cp1 = decoder.read()
    cp1 = _bebytes_to_int(cp1)
    decoder.leave()
    # curve order
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, n = decoder.read()
    n = _bebytes_to_int(n)
    # curve cofactor
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, h = decoder.read()
    h = _bebytes_to_int(h)
    # generator point
    ensure_tag(decoder, asn1.Numbers.Sequence)
    decoder.enter()
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, gx = decoder.read()
    gx = _bebytes_to_int(gx)
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, gy = decoder.read()
    gy = _bebytes_to_int(gy)
    decoder.leave()
    # curve field bitsize
    ensure_tag(decoder, asn1.Numbers.Integer)
    tag, bits = decoder.read()
    decoder.leave()
    if ctype == 2:
        curve = ECurve.ShortWeierstrass(p, cp0, cp1, n, h, gx, gy, bits)
    elif ctype == 3:
        curve = ECurve.Edwards(p, cp0, cp1, n, h, gx, gy, bits)
    elif ctype == 4:
        curve = ECurve.Montgomery(p, cp0, cp1, n, h, gx, gy, bits)
    elif ctype == 5:
        curve = ECurve.TwistedEdwards(p, cp0, cp1, n, h, gx, gy, bits)
    else:
        raise ValueError('Unknown Curve Type')
    return curve

def der_decode_privkey(DERbytes):
    decoder = asn1.Decoder()
    decoder.start(DERbytes)
    ensure_tag(decoder, asn1.Numbers.Sequence)
    decoder.enter()
    # privkey
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, privkey = decoder.read()
    privkey = _bebytes_to_int(privkey)
    curve = _der_decode_curve(decoder)
    decoder.leave()
    return (privkey, curve)

def der_encode_pubkey(pubkey, curve):
    # encode key, curve to ASN1 DER format
    encoder = asn1.Encoder()
    encoder.start()
    encoder.enter(asn1.Numbers.Sequence)
    # pubkey
    encoder.write(pubkey.compressed(), asn1.Numbers.OctetString)
    # curve
    _der_encode_curve(curve, encoder)
    encoder.leave()
    return encoder.output()

def der_decode_pubkey(DERbytes):
    decoder = asn1.Decoder()
    decoder.start(DERbytes)
    ensure_tag(decoder, asn1.Numbers.Sequence)
    decoder.enter()
    # pubkey
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, pubbytes = decoder.read()
    curve = _der_decode_curve(decoder)
    # curve
    pubkey = ECPoint(curve, pubbytes)
    decoder.leave()
    return (pubkey, curve)

def der_encode_message(pubkey, nonce, ctext):
    # encode key, curve to ASN1 DER format
    encoder = asn1.Encoder()
    encoder.start()
    encoder.enter(asn1.Numbers.Sequence)
    # pubkey
    encoder.write(pubkey.compressed(), asn1.Numbers.OctetString)
    # nonce
    encoder.write(nonce, asn1.Numbers.OctetString)
    # ciphertext
    encoder.write(ctext, asn1.Numbers.OctetString)
    encoder.leave()
    return encoder.output()

def der_decode_message(DERbytes):
    decoder = asn1.Decoder()
    decoder.start(DERbytes)
    ensure_tag(decoder, asn1.Numbers.Sequence)
    decoder.enter()
    # pubkey
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, pubbytes = decoder.read()
    # nonce
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, nonce = decoder.read()
    # ciphertext
    ensure_tag(decoder, asn1.Numbers.OctetString)
    tag, ctext = decoder.read()
    decoder.leave()
    return (pubbytes, nonce, ctext)
    