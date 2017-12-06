# Copyright (c) 2017, Joseph deBlaquiere <jadeblaquiere@yahoo.com>
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

from ecpy.point import Point, Generator
import ecpy.curves as curves
from Crypto.Random import random

#_curve = curves.curve_secp112r1
#_curve = curves.curve_secp224k1
_curve = curves.curve_secp256k1
#_curve = curves.curve_secp384r1
#_curve = curves.curve_bauer9

Generator.set_curve(_curve)

if __name__ == '__main__':
    n = _curve['p']
    
    Gaffine = _curve['G']
    G = Generator.init(Gaffine[0],Gaffine[1])
    q = [];
    for i in range(0, 10):
        nq = i
        nQ = G * nq;
        print("{\"0x%064X\", \"%s\"}," % (nq, nQ.compress().decode()))
    for i in range(0, 10):
        nq = random.randint(2, n-1)
        nQ = G * nq;
        print("{\"0x%064X\", \"%s\"}," % (nq, nQ.compress().decode()))
    for i in range(0, 10):
        nq = n - i - 1
        nQ = G * nq;
        print("{\"0x%064X\", \"%s\"}," % (nq, nQ.compress().decode()))
