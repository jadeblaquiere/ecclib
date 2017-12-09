# ecclib

ecclib provides basic Elliptic Curve math operations in a library which loosely
follows the conventions of the GNU Multiple Precision (GMP) math library. For
details on GMP, see: https://gmplib.org/.

ecclib provides basic handling of math within the multiplicative field of
integers modulo an odd prime p (i.e. the ring Z/pZ, also denoted Zp) and extends that
capability to elliptic curve points over that ring, i.e. E(Fp).

## capabilities

Basic operations:
* import/export
* element (x,y) composition/decomposition

Supported multiplicative group operations:
* addition
* subtraction
* negation
* multiplication
* exponentiation
* modular inversion
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
* Twisted Edwards
* Montgomery (implemented internally via transform to short Weierstrass form)

## build and install

libecc follows the traditional GNU autotools build process so it _should_
build without issue on most platforms where autotools is present. GMP is the
only dependency. 

Example build and install sequence:

```
$ autoreconf --install
$ ./configure --prefix=/usr
$ make
$ make check
$ sudo make install
```
