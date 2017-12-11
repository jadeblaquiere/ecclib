# public domain implementation of ed25519 from:
# https://ed25519.cr.yp.to/python/ed25519.py

import hashlib

b = 256
# 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed
q = 2**255 - 19
# 0x1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed
l = 0x1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed

def H(m):
  return hashlib.sha512(m).digest()

def expmod(b,e,m):
  if e == 0: return 1
  t = expmod(b,e//2,m)**2 % m
  if e & 1: t = (t*b) % m
  return t

def inv(x):
  return expmod(x,q-2,q)

B = 1
A = 486662
I = expmod(2,(q-1)//4,q)

def xrecover(y):
  xx = (y*y-1) * inv(d*y*y+1)
  x = expmod(xx,(q+3)//8,q)
  if (x*x - xx) % q != 0: x = (x*I) % q
  if x % 2 != 0: x = q-x
  return x

Gx = 0x9
Gy = 0x20ae19a1b8a086b4e01edd2c7748d14c923d4d7e6d7c61b229e9c5a27eced3d9
#By = 4 * inv(5)
#Bx = xrecover(By)
G = [Gx % q,Gy % q, 0]

def montgomery(P,Q):
  x1 = P[0]
  y1 = P[1]
  i1 = P[2]
  x2 = Q[0]
  y2 = Q[1]
  i2 = Q[2]
  if (i1 != 0):
    return Q
  if (i2 != 0):
    return P
  if (x1 == x2) and (y1 == y2):
    x3 = ((B*((3*x1**2)+(2*A*x1)+1)**2) * inv((2*B*y1)**2)) - A - x1 - x1
    y3 = (((2*x1)+x1+A)*(3*x1**2+2*A*x1+1)) * inv(2*B*y1) - B*(3*x1**2+2*A*x1+1)**3 * inv((2*B*y1)**3) - y1
  else:
    x3 = (B*((y2-y1)**2) * inv((x2-x1)**2)) - A - x1 - x2
    y3 = (((2*x1)+x2+A) * (y2-y1) * inv(x2-x1)) - (B * ((y2-y1)**3) * inv((x2-x1)**3)) - y1
  return [x3 % q,y3 % q, 0]

def scalarmult(P,e):
  if e == 0: return [0,0,1]
  Q = scalarmult(P,e//2)
  Q = montgomery(Q,Q)
  if e & 1: Q = montgomery(Q,P)
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
  P = scalarmult(G,a)
  return encodepoint(P)

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
  i = P[2]
  if i != 0:
    return True
  # B*y**2=x**3+A*x**2+x
  return (B*y*y - (x*x*x + A*x*x + x)) % q == 0

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
  if scalarmult(B,S) != montgomery(R,scalarmult(A,h)):
    raise Exception("signature does not pass verification")
    
if __name__ == '__main__':
    from Crypto.Random import random
    n = l
    p = q
    print("p = 0x%064X" % (q))
    print("n = 0x%064X" % (l))
    print("B = 0x%064X" % (B))
    print("A = 0x%064X" % (A))
    if not isoncurve(G): raise Exception("Generator point not on curve? huh?")
    print("G = (0x%064X, 0x%064X)" % (G[0], G[1]))
    L = [0, 0, 1]
    for i in range(0,10):
        nq = i
        nQ = scalarmult(G, nq)
        aQ = [0, 0, 1]
        for j in range (1, i+1):
            aQ = montgomery(aQ, G)
        if (aQ[0] != nQ[0]) or (aQ[1] != nQ[1]) or (aQ[2] != nQ[2]):
            raise Exception("add not equal mult")
        if i > 0:
            Lp1 = montgomery(L, G)
            if (Lp1[0] != nQ[0]) or (Lp1[1] != nQ[1]) or (Lp1[2] != nQ[2]):
                raise Exception("X + G != (x + 1)G")
        if not isoncurve(nQ): raise Exception("point not on curve")
        if (nQ[2] == 1):
            print("{\"Curve25519\", \"0x%064X\", \"00%064X\"}," % (nq, nQ[0]))
        elif (nQ[1] % 2 ) == 1:
            print("{\"Curve25519\", \"0x%064X\", \"03%064X\"}," % (nq, nQ[0]))
        else:
            print("{\"Curve25519\", \"0x%064X\", \"02%064X\"}," % (nq, nQ[0]))
        L = nQ;
    for i in range(0,10):
        nq = random.randint(2, q-1)
        nQ = scalarmult(G, nq)
        if not isoncurve(nQ): raise Exception("point not on curve")
        if (nQ[2] == 1):
            print("{\"Curve25519\", \"0x%064X\", \"00%064X\"}," % (nq, nQ[0]))
        elif (nQ[1] % 2 ) == 1:
            print("{\"Curve25519\", \"0x%064X\", \"03%064X\"}," % (nq, nQ[0]))
        else:
            print("{\"Curve25519\", \"0x%064X\", \"02%064X\"}," % (nq, nQ[0]))
    for i in range(0,10):
        nq = q - i - 1
        nQ = scalarmult(G, nq)
        if i > 0:
            Np1 = montgomery(nQ, G)
            if (Np1[0] != L[0]) or (Np1[1] != L[1]) or (Np1[2] != L[2]):
                raise Exception("X + G != (x + 1)G")
        if not isoncurve(nQ): raise Exception("point not on curve")
        if (nQ[2] == 1):
            print("{\"Curve25519\", \"0x%064X\", \"00%064X\"}," % (nq, nQ[0]))
        elif (nQ[1] % 2 ) == 1:
            print("{\"Curve25519\", \"0x%064X\", \"03%064X\"}," % (nq, nQ[0]))
        else:
            print("{\"Curve25519\", \"0x%064X\", \"02%064X\"}," % (nq, nQ[0]))
        L = nQ;
