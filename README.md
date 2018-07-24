# ecclib [![Build Status](https://travis-ci.org/jadeblaquiere/ecclib.svg?branch=master)](https://travis-ci.org/jadeblaquiere/ecclib) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/007eab0568ae41049a1a9b94dfcce494)](https://www.codacy.com/app/jadeblaquiere/ecclib?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=jadeblaquiere/ecclib&amp;utm_campaign=Badge_Grade)

ecclib provides basic Elliptic Curve math operations in a library which loosely
follows the conventions of the GNU Multiple Precision (GMP) math library. For
details on GMP, see: https://gmplib.org/.

ecclib provides basic handling of math within the multiplicative field of
integers modulo an odd prime p (i.e. the ring Z/pZ, also denoted Zp) and extends that
capability to elliptic curve points over that ring, i.e. E(Fp).

The implementation leverages complete addition formulae for all curve types, The Edwards
and Twisted Edwards addition laws are complete by default. The short Weierstrass
implementation uses the Renes, Costello and Batina complete addition formula (i.e.
add and double are the exact same algorithm). In order to provide the full set of
operations Montgomery curves are transformed to short Weierstrass and use the same
formula. In all cases the multiplication is via the Brier-Joye ladder.

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
* subtraction
* doubling
* negation
* scalar multiplication
* scalar base multiplication

Supported elliptic curve types:
* short Weierstrass
* Edwards
* twisted Edwards
* Montgomery (implemented internally via transform to short Weierstrass form)

Supported cryptography operations
* ECDSA

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

Note: if you want to run the benchmarks you'll need to install libsodium and
also pass the `--enable-benchmarks` option to configure. Building the examples
is enabled via `--enable-examples`. 

## Python bindings

Once you have installed the underlying C libraries you can install the python
API interface. This interface is only tested with python3. The python API can
be installed (and tested) with commands like:

```
$ sudo pip3 install --upgrade .
$ python3 ./tests/pytest_field.py
$ python3 ./tests/pytest_ecurve.py
$ python3 ./tests/pytest_ecpoint.py
```

## Go bindings

Once the underlying C libraries are installed, you can install the golang
API (which attempts to follow the semantics of math/big) to use the library in
Go programs.

```
$ go get github.com/jadeblaquiere/ecclib/ecgo
```