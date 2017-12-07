# public domain implementation of ed25519 from:
# https://ed25519.cr.yp.to/python/ed25519.py

import hashlib

b = 416
# 0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
q = 2**414 - 17
# 0x7ffffffffffffffffffffffffffffffffffffffffffffffffffeb3cc92414cf706022b36f1c0338ad63cf181b0e71a5e106af79
l = 0x7ffffffffffffffffffffffffffffffffffffffffffffffffffeb3cc92414cf706022b36f1c0338ad63cf181b0e71a5e106af79

def H(m):
  return hashlib.sha512(m).digest()

def expmod(b,e,m):
  if e == 0: return 1
  t = expmod(b,e//2,m)**2 % m
  if e & 1: t = (t*b) % m
  return t

def inv(x):
  return expmod(x,q-2,q)

c = 1
d = 3617
I = expmod(2,(q-1)//4,q)

def xrecover(y):
  xx = (y*y-1) * inv(d*y*y+1)
  x = expmod(xx,(q+3)//8,q)
  if (x*x - xx) % q != 0: x = (x*I) % q
  if x % 2 != 0: x = q-x
  return x

Bx = 0x1a334905141443300218c0631c326e5fcd46369f44c03ec7f57ff35498a4ab4d6d6ba111301a73faa8537c64c4fd3812f3cbc595
By = 0x22
#By = 4 * inv(5)
#Bx = xrecover(By)
B = [Bx % q,By % q]

def edwards(P,Q):
  x1 = P[0]
  y1 = P[1]
  x2 = Q[0]
  y2 = Q[1]
  x3 = (x1*y2+x2*y1) * inv(c * (1+d*x1*x2*y1*y2))
  y3 = (y1*y2-x1*x2) * inv(c * (1-d*x1*x2*y1*y2))
  return [x3 % q,y3 % q]

def scalarmult(P,e):
  if e == 0: return [0,1]
  Q = scalarmult(P,e//2)
  Q = edwards(Q,Q)
  if e & 1: Q = edwards(Q,P)
  return Q

def encodeint(y):
  bits = [(y >> i) & 1 for i in range(b)]
  return ''.join([chr(sum([bits[i * 8 + j] << j for j in range(8)])) for i in range(b//8)])

def encodepoint(P):
  x = P[0]
  y = P[1]
  bits = [(y >> i) & 1 for i in range(b - 1)] + [x & 1]
  return ''.join([chr(sum([bits[i * 8 + j] << j for j in range(8)])) for i in range(b//8)])

def bit(h,i):
  return (ord(h[i//8]) >> (i%8)) & 1

def publickey(sk):
  h = H(sk)
  a = 2**(b-2) + sum(2**i * bit(h,i) for i in range(3,b-2))
  A = scalarmult(B,a)
  return encodepoint(A)

def Hint(m):
  h = H(m)
  return sum(2**i * bit(h,i) for i in range(2*b))

def signature(m,sk,pk):
  h = H(sk)
  a = 2**(b-2) + sum(2**i * bit(h,i) for i in range(3,b-2))
  r = Hint(''.join([h[i] for i in range(b//8,b//4)]) + m)
  R = scalarmult(B,r)
  S = (r + Hint(encodepoint(R) + pk + m) * a) % l
  return encodepoint(R) + encodeint(S)

def isoncurve(P):
  x = P[0]
  y = P[1]
  return (x*x + y*y - (c * (1 + d*x*x*y*y))) % q == 0

def decodeint(s):
  return sum(2**i * bit(s,i) for i in range(0,b))

def decodepoint(s):
  y = sum(2**i * bit(s,i) for i in range(0,b-1))
  x = xrecover(y)
  if x & 1 != bit(s,b-1): x = q-x
  P = [x,y]
  if not isoncurve(P): raise Exception("decoding point that is not on curve")
  return P

def checkvalid(s,m,pk):
  if len(s) != b//4: raise Exception("signature length is wrong")
  if len(pk) != b//8: raise Exception("public-key length is wrong")
  R = decodepoint(s[0:b//8])
  A = decodepoint(pk)
  S = decodeint(s[b//8:b//4])
  h = Hint(encodepoint(R) + pk + m)
  if scalarmult(B,S) != edwards(R,scalarmult(A,h)):
    raise Exception("signature does not pass verification")
    
if __name__ == '__main__':
    from Crypto.Random import random
    n = l
    p = q
    print("p = 0x%0104X" % (q))
    print("n = 0x%0104X" % (l))
    print("c = 1")
    print("d = 0x%0104X" % (d))
    if not isoncurve(B): raise Exception("Generator point not on curve? huh?")
    print("G = (0x%0104X, 0x%0104X)" % (B[0], B[1]))
    for i in range(0,10):
        nq = i
        nQ = scalarmult(B, nq)
        if (nQ[0] == 0) and (nQ[1] == 1):
            print("{\"Curve41417\", \"0x%0104X\", \"00%0104X\"}," % (nq, nQ[0]))
        elif (nQ[1] % 2 ) == 1:
            print("{\"Curve41417\", \"0x%0104X\", \"03%0104X\"}," % (nq, nQ[0]))
        else:
            print("{\"Curve41417\", \"0x%0104X\", \"02%0104X\"}," % (nq, nQ[0]))
    for i in range(0,10):
        nq = random.randint(2, q-1)
        nQ = scalarmult(B, nq)
        if (nQ[0] == 0) and (nQ[1] == 1):
            print("{\"Curve41417\", \"0x%0104X\", \"00%0104X\"}," % (nq, nQ[0]))
        elif (nQ[1] % 2 ) == 1:
            print("{\"Curve41417\", \"0x%0104X\", \"03%0104X\"}," % (nq, nQ[0]))
        else:
            print("{\"Curve41417\", \"0x%0104X\", \"02%0104X\"}," % (nq, nQ[0]))
    for i in range(0,10):
        nq = q - i - 1
        nQ = scalarmult(B, nq)
        if (nQ[0] == 0) and (nQ[1] == 1):
            print("{\"Curve41417\", \"0x%0104X\", \"00%0104X\"}," % (nq, nQ[0]))
        elif (nQ[1] % 2 ) == 1:
            print("{\"Curve41417\", \"0x%0104X\", \"03%0104X\"}," % (nq, nQ[0]))
        else:
            print("{\"Curve41417\", \"0x%0104X\", \"02%0104X\"}," % (nq, nQ[0]))
