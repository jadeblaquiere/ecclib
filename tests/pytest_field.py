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
from ECC import FieldElement


class TestFieldElement(unittest.TestCase):

    def setUp(self):
        self.p = (2 ** 414) - 17

    def test_as_int(self):
        e = FieldElement(0, self.p)
        self.assertEqual(int(e), 0)
        f = FieldElement(1, self.p)
        self.assertEqual(int(f), 1)
        g = FieldElement(-1, self.p)
        self.assertEqual(int(g), self.p - 1)

    def test_urandom(self):
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            self.assertGreaterEqual(int(e), 0)
            self.assertLess(int(e), self.p)

    def test_add(self):
        e = FieldElement(0, self.p)
        f = FieldElement(1, self.p)
        g = e + f
        self.assertEqual(int(g), 1)
        e = FieldElement(1, self.p)
        g = e + f
        self.assertEqual(int(g), 2)
        e = FieldElement(-1, self.p)
        g = e + f
        self.assertEqual(int(g), 0)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            f = FieldElement.urandom(self.p)
            g = (int(e) + int(f)) % self.p
            self.assertEqual(int(e + f), g)
            self.assertEqual(int(f + e), g)

    def test_neg(self):
        e = FieldElement(0, self.p)
        g = -e
        self.assertEqual(int(g), 0)
        e = FieldElement(1, self.p)
        g = -e
        self.assertEqual(int(g), self.p - 1)
        e = FieldElement(-1, self.p)
        g = -e
        self.assertEqual(int(g), 1)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            g = (-int(e)) % self.p
            self.assertEqual(int(-e), g)

    def test_sub(self):
        e = FieldElement(0, self.p)
        f = FieldElement(1, self.p)
        g = e - f
        self.assertEqual(int(g), self.p - 1)
        e = FieldElement(1, self.p)
        g = e - f
        self.assertEqual(int(g), 0)
        f = FieldElement(-1, self.p)
        g = e - f
        self.assertEqual(int(g), 2)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            f = FieldElement.urandom(self.p)
            g = (int(e) - int(f)) % self.p
            self.assertEqual(int(e - f), g)
            g = (-g) % self.p
            self.assertEqual(int(f - e), g)

    def test_mul(self):
        e = FieldElement(0, self.p)
        f = FieldElement(1, self.p)
        g = e * f
        self.assertEqual(int(g), 0)
        e = FieldElement(1, self.p)
        g = e * f
        self.assertEqual(int(g), 1)
        f = FieldElement(-1, self.p)
        g = e * f
        self.assertEqual(int(g), self.p - 1)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            f = FieldElement.urandom(self.p)
            g = (int(e) * int(f)) % self.p
            self.assertEqual(int(e * f), g)
            self.assertEqual(int(f * e), g)

    def test_inv(self):
        e = FieldElement(0, self.p)
        g = e.inverse()
        self.assertEqual(g, None)
        f = FieldElement(1, self.p)
        g = f.inverse()
        self.assertEqual(int(g), 1)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            f = e.inverse()
            if f is None:
                self.assertEqual(int(e), 0)
            else:
                g = e * f
                self.assertEqual(int(g), 1)

    def test_sqrt(self):
        e = FieldElement(0, self.p)
        g = e.sqrt()
        self.assertEqual(g, None)
        f = FieldElement(1, self.p)
        g = f.inverse()
        self.assertEqual(int(g * g), 1)
        self.assertEqual(int(g * g), f)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            f = e.sqrt()
            if f is not None:
                g = f * f
                self.assertEqual(int(g), int(e))
                self.assertEqual(g, e)

    def test_pow(self):
        e = FieldElement(0, self.p)
        g = pow(e, 2)
        self.assertEqual(g, 0)
        g = e ** 2
        self.assertEqual(g, 0)
        f = FieldElement(1, self.p)
        g = pow(f, 3)
        self.assertEqual(g, 1)
        g = f ** 3
        self.assertEqual(g, 1)
        for _ in range(0, 100):
            e = FieldElement.urandom(self.p)
            for j in range(0, 10):
                f = pow(e, j)
                g = pow(int(e), j, self.p)
                self.assertEqual(f, g)

    def test_repr(self):
        e = FieldElement(0, self.p)
        g = repr(e)
        self.assertEqual(eval(g), e)
        f = FieldElement(1, self.p)
        g = repr(f)
        self.assertEqual(eval(g), 1)
        for _ in range(0, 10000):
            e = FieldElement.urandom(self.p)
            f = repr(e)
            g = eval(f)
            self.assertEqual(e, g)


if __name__ == '__main__':
    unittest.main()
