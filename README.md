#ecclib

ecclib provides basic Elliptic Curve math operations in a library which loosely
follows the conventions of the GNU Multiple Precision (GMP) math library.

ecclib provides basic handling of math within the multiplicative field of
integers modulo an odd prime p (i.e. the ring Z/pZ, also denoted Zp) and extends that
capability to elliptic curve points over that ring, i.e. E(Fp).

Basic operations:
* import/export
* element (x,y) composition/decomposition
* modular inversion

Supported multiplicative group operations:
* addition
* subtraction
* negation
* multiplication
* exponentiation
* square root

Supported elliptic curve point mathematical operations:
* addition
* substraction
* doubling
* negation
* scalar multiplication
* scalar base multiplication

Supported elliptic curve types:
* short Weierstrass
* Edwards
* Montgomery (partial)
