//BSD 3-Clause License
//
//Copyright (c) 2018, jadeblaquiere
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// this code relies strongly on assert() which would be suppressed
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <gmp.h>
#include <pycurve.h>
#include <pygmplong.h>
#include <Python.h>
#include <structmember.h>

PyDoc_STRVAR(ECurve__doc__,
"ECurve implements a general interface to elliptic curve parameters of multiple types.\n");

// allocate the object
static PyObject *ECurve_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	// create the new Parameterss object
	ECurve *self = (ECurve *)type->tp_alloc(type, 0);
	// make sure it actually worked
	if (!self) {
		PyErr_SetString(PyExc_TypeError, "could not create ECurve object.");
		return NULL;
	}
	self->ready = 0;

	// cast and return
	return (PyObject *)self;
}

// Usage options:
// ECurve(name=PyString)
static int ECurve_init(ECurve *self, PyObject *args, PyObject *kwargs) {
    char *name;
	int status;

	if (!PyArg_ParseTuple(args, "s", &name)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECurve_init arguments");
		return -1;
	}

	mpECurve_init(self->ec);
    status = mpECurve_set_named(self->ec, name);
    
    if (status != 0) {
		PyErr_SetString(PyExc_ValueError, "Unable to find curve matching name");
		return -1;
    }

	// you're ready!
	self->ready = 1;
	// all's clear
	return 0;
}

// deallocates the object when done
static void ECurve_dealloc(ECurve *self) {
	// clear the internal element
	if (self->ready){
		mpECurve_clear(self->ec);
	}

	// free the object
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *ECurve_richcompare(PyObject *a, PyObject *b, int op) {
	int result = 0;

	assert(PyObject_TypeCheck(a, &ECurveType) != 0);

	if (!PyObject_TypeCheck((PyObject *)b, &ECurveType)) {
		Py_RETURN_NOTIMPLEMENTED;
	}

	switch (op) {
	case Py_EQ:
		if (mpECurve_cmp(((ECurve *)a)->ec, ((ECurve *)b)->ec) == 0) {
			result = 1;
		}
		break;
	case Py_NE:
		if (mpECurve_cmp(((ECurve *)a)->ec, ((ECurve *)b)->ec) != 0) {
			result = 1;
		}
		break;
	default:
		Py_RETURN_NOTIMPLEMENTED;
	}

	if (result != 0) {
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject *ECurve_check_point(PyObject *cv, PyObject *args) {
    PyObject *x, *y;
    mpz_t xmpz, ympz;
    int valid;
    int status;

	assert(PyObject_TypeCheck(cv, &ECurveType) != 0);

	if (!PyArg_ParseTuple(args, "OO", &x, &y)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECurve_init arguments");
		return NULL;
	}

    mpz_init(xmpz);
    mpz_init(ympz);

    status = _pylong_to_mpz_unsigned((PyLongObject *)x, xmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)y, ympz);
    if (status != 0) goto ConversionError;

    valid = mpECurve_point_check(((ECurve *)cv)->ec, xmpz, ympz);

    mpz_clear(ympz);
    mpz_clear(xmpz);

    if (valid == 0) Py_RETURN_FALSE;
    Py_RETURN_TRUE;

ConversionError:
    mpz_clear(ympz);
    mpz_clear(xmpz);
    PyErr_SetString(PyExc_ValueError, "Error converting to (unsigned) GMP mpz_t");
    return NULL;
}

//void mpECurve_set_mpz_ws(mpECurve_t cv, mpz_t p, mpz_t a, mpz_t b, mpz_t n,
//                      mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits);

PyDoc_STRVAR(ECurve_create_ws__doc__, 
	"ShortWeierstrass(p, a, b, n , h, gx, gy, bits) -> Ecurve\n\n"
	"Static Method to generate an elliptic curve using Short Weierstrass notation "
	"(y**2 = x**3 + a * x + b) with order n, cofactor h and generator point"
	"(gx, gy) where p is of size bits.");

// this is a Class Method, returns an instance of the type
// ECurve.ShortWeierstrass(p=Int, a=Int, b=Int, n=Int, h=Int, gx=Int, gy=Int, bits=Int)
static PyObject *ECurve_create_ws(PyObject *none, PyObject *args) {
    PyObject *p, *a, *b, *n, *h, *gx, *gy;
    ECurve *rop;
    mpz_t pmpz, ampz, bmpz, nmpz, hmpz, gxmpz, gympz;
    int bits;
    int status;

	if (!PyArg_ParseTuple(args, "OOOOOOOi", &p, &a, &b, &n, &h, &gx, &gy, &bits)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECurve_init arguments");
		return NULL;
	}

    mpz_init(pmpz);
    mpz_init(ampz);
    mpz_init(bmpz);
    mpz_init(nmpz);
    mpz_init(hmpz);
    mpz_init(gxmpz);
    mpz_init(gympz);

    status = _pylong_to_mpz_unsigned((PyLongObject *)p, pmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)a, ampz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)b, bmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)n, nmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)h, hmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gx, gxmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gy, gympz);
    if (status != 0) goto ConversionError;
    
	rop = (ECurve *)ECurve_new( &ECurveType, NULL, NULL);
	mpECurve_init(rop->ec);
    mpECurve_set_mpz_ws(rop->ec, pmpz, ampz, bmpz, nmpz, hmpz, gxmpz, gympz, bits);

    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(bmpz);
    mpz_clear(ampz);
    mpz_clear(pmpz);

    return (PyObject *)rop;

ConversionError:
    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(bmpz);
    mpz_clear(ampz);
    mpz_clear(pmpz);
    PyErr_SetString(PyExc_ValueError, "Error converting to (unsigned) GMP mpz_t");
    return NULL;
}

//void mpECurve_set_mpz_ed(mpECurve_t cv, mpz_t p, mpz_t c, mpz_t d, mpz_t n,
//                      mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits);

PyDoc_STRVAR(ECurve_create_ed__doc__, 
	"Edwards(p, c, d, n , h, gx, gy, bits) -> Ecurve\n\n"
	"Static Method to generate an elliptic curve using Edwards notation "
	"(x**2 + y**2 = c**2 * (1 + d * x**2 * y**2)) with order n, cofactor h and"
	"generator point (gx, gy) where p is of size bits.");

// this is a Class Method, returns an instance of the type
// ECurve.Edwards(p=Int, c=Int, d=Int, n=Int, h=Int, gx=Int, gy=Int, bits=Int)
static PyObject *ECurve_create_ed(PyObject *none, PyObject *args) {
    PyObject *p, *c, *d, *n, *h, *gx, *gy;
    ECurve *rop;
    mpz_t pmpz, cmpz, dmpz, nmpz, hmpz, gxmpz, gympz;
    int bits;
    int status;

	if (!PyArg_ParseTuple(args, "OOOOOOOi", &p, &c, &d, &n, &h, &gx, &gy, &bits)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECurve_init arguments");
		return NULL;
	}

    mpz_init(pmpz);
    mpz_init(cmpz);
    mpz_init(dmpz);
    mpz_init(nmpz);
    mpz_init(hmpz);
    mpz_init(gxmpz);
    mpz_init(gympz);

    status = _pylong_to_mpz_unsigned((PyLongObject *)p, pmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)c, cmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)d, dmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)n, nmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)h, hmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gx, gxmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gy, gympz);
    if (status != 0) goto ConversionError;
    
	rop = (ECurve *)ECurve_new( &ECurveType, NULL, NULL);
	mpECurve_init(rop->ec);
    mpECurve_set_mpz_ed(rop->ec, pmpz, cmpz, dmpz, nmpz, hmpz, gxmpz, gympz, bits);

    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(dmpz);
    mpz_clear(cmpz);
    mpz_clear(pmpz);

    return (PyObject *)rop;

ConversionError:
    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(dmpz);
    mpz_clear(cmpz);
    mpz_clear(pmpz);
    PyErr_SetString(PyExc_ValueError, "Error converting to (unsigned) GMP mpz_t");
    return NULL;
}

//void mpECurve_set_mpz_mo(mpECurve_t cv, mpz_t p, mpz_t B, mpz_t A, mpz_t n,
//                      mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits);

PyDoc_STRVAR(ECurve_create_mo__doc__, 
	"Montgomery(p, B, A, n , h, gx, gy, bits) -> Ecurve\n\n"
	"Static Method to generate an elliptic curve using Montgomery notation "
	"(B * y**2 = x**3 + A * x**2 + x) with order n, cofactor h and"
	"generator point (gx, gy) where p is of size bits.");

// this is a Class Method, returns an instance of the type
// ECurve.Montgomery(p=Int, B=Int, A=Int, n=Int, h=Int, gx=Int, gy=Int, bits=Int)
static PyObject *ECurve_create_mo(PyObject *none, PyObject *args) {
    PyObject *p, *B, *A, *n, *h, *gx, *gy;
    ECurve *rop;
    mpz_t pmpz, Bmpz, Ampz, nmpz, hmpz, gxmpz, gympz;
    int bits;
    int status;

	if (!PyArg_ParseTuple(args, "OOOOOOOi", &p, &B, &A, &n, &h, &gx, &gy, &bits)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECurve_init arguments");
		return NULL;
	}

    mpz_init(pmpz);
    mpz_init(Bmpz);
    mpz_init(Ampz);
    mpz_init(nmpz);
    mpz_init(hmpz);
    mpz_init(gxmpz);
    mpz_init(gympz);

    status = _pylong_to_mpz_unsigned((PyLongObject *)p, pmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)B, Bmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)A, Ampz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)n, nmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)h, hmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gx, gxmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gy, gympz);
    if (status != 0) goto ConversionError;
    
	rop = (ECurve *)ECurve_new( &ECurveType, NULL, NULL);
	mpECurve_init(rop->ec);
    mpECurve_set_mpz_mo(rop->ec, pmpz, Bmpz, Ampz, nmpz, hmpz, gxmpz, gympz, bits);

    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(Ampz);
    mpz_clear(Bmpz);
    mpz_clear(pmpz);

    return (PyObject *)rop;

ConversionError:
    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(Ampz);
    mpz_clear(Bmpz);
    mpz_clear(pmpz);
    PyErr_SetString(PyExc_ValueError, "Error converting to (unsigned) GMP mpz_t");
    return NULL;
}

//void mpECurve_set_mpz_te(mpECurve_t cv, mpz_t p, mpz_t a, mpz_t d, mpz_t n,
//                      mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits);

PyDoc_STRVAR(ECurve_create_te__doc__, 
	"TwistedEdwards(p, a, d, n , h, gx, gy, bits) -> Ecurve\n\n"
	"Static Method to generate an elliptic curve using Twisted Edwards notation "
	"(a * x**2 + y**2 = 1 + d * x**2 * y**2) with order n, cofactor h and"
	"generator point (gx, gy) where p is of size bits.");

// this is a Class Method, returns an instance of the type
// ECurve.TwistedEdwards(p=Int, a=Int, d=Int, n=Int, h=Int, gx=Int, gy=Int, bits=Int)
static PyObject *ECurve_create_te(PyObject *none, PyObject *args) {
    PyObject *p, *a, *d, *n, *h, *gx, *gy;
    ECurve *rop;
    mpz_t pmpz, ampz, dmpz, nmpz, hmpz, gxmpz, gympz;
    int bits;
    int status;

	if (!PyArg_ParseTuple(args, "OOOOOOOi", &p, &a, &d, &n, &h, &gx, &gy, &bits)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECurve_init arguments");
		return NULL;
	}

    mpz_init(pmpz);
    mpz_init(ampz);
    mpz_init(dmpz);
    mpz_init(nmpz);
    mpz_init(hmpz);
    mpz_init(gxmpz);
    mpz_init(gympz);

    status = _pylong_to_mpz_unsigned((PyLongObject *)p, pmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)a, ampz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)d, dmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)n, nmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)h, hmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gx, gxmpz);
    if (status != 0) goto ConversionError;

    status = _pylong_to_mpz_unsigned((PyLongObject *)gy, gympz);
    if (status != 0) goto ConversionError;
    
	rop = (ECurve *)ECurve_new( &ECurveType, NULL, NULL);
	mpECurve_init(rop->ec);
    mpECurve_set_mpz_te(rop->ec, pmpz, ampz, dmpz, nmpz, hmpz, gxmpz, gympz, bits);

    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(dmpz);
    mpz_clear(ampz);
    mpz_clear(pmpz);

    return (PyObject *)rop;

ConversionError:
    mpz_clear(gympz);
    mpz_clear(gxmpz);
    mpz_clear(hmpz);
    mpz_clear(nmpz);
    mpz_clear(dmpz);
    mpz_clear(ampz);
    mpz_clear(pmpz);
    PyErr_SetString(PyExc_ValueError, "Error converting to (unsigned) GMP mpz_t");
    return NULL;
}

static PyObject *ECurve_list_curve_names(PyObject *type, PyObject *none) {
    char **list;
    PyObject *clist;
    int i = 0;

    assert(none == NULL);
    
    list = _mpECurve_list_standard_curves();
    assert(list != NULL);

    clist = PyList_New(0);
    assert(clist != NULL);
    while (list[i] != NULL) {
        int status;
        PyObject *pstr;

        pstr = PyUnicode_FromString(list[i]);
        assert(pstr != NULL);
        status = PyList_Append(clist, pstr);
        Py_DECREF(pstr);
        if (status != 0) {
            Py_DECREF(clist);
            return NULL;
        }

        i++;
    }
    return clist;
}

//typedef enum {EQTypeNone, EQTypeUninitialized, EQTypeShortWeierstrass, EQTypeEdwards, EQTypeMontgomery, EQTypeTwistedEdwards} _mpECurve_eq_type;
static PyObject *ECurve_repr(PyObject *a) {
	PyObject *p, *c1, *c2, *n, *h, *gx, *gy;
	unsigned int bits;
	PyObject *rep;
	assert(PyObject_TypeCheck(a, &ECurveType) != 0);
	
	p = _mpz_to_pylong(((ECurve *)a)->ec->fp->p);
	assert(p != NULL);
	n = _mpz_to_pylong(((ECurve *)a)->ec->n);
	assert(p != NULL);
	h = _mpz_to_pylong(((ECurve *)a)->ec->h);
	assert(p != NULL);
	gx = _mpz_to_pylong(((ECurve *)a)->ec->G[0]);
	assert(p != NULL);
	gy = _mpz_to_pylong(((ECurve *)a)->ec->G[1]);
	assert(p != NULL);
	bits = ((ECurve *)a)->ec->bits;
	c1 = NULL;
	c2 = NULL;
	switch (((ECurve *)a)->ec->type) {
    case EQTypeShortWeierstrass:
    	c1 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.ws.a);
    	assert(c1 != NULL);
    	c2 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.ws.b);
    	assert(c2 != NULL);
        rep = PyUnicode_FromFormat("ECC.ECurve.ShortWeierstrass(%S, %S, %S, %S, %S, %S, %S, %d)", p, c1, c2, n, h, gx, gy, bits);
        break;
    case EQTypeEdwards:
    	c1 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.ed.c);
    	assert(c1 != NULL);
    	c2 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.ed.d);
    	assert(c2 != NULL);
        rep = PyUnicode_FromFormat("ECC.ECurve.Edwards(%S, %S, %S, %S, %S, %S, %S, %d)", p, c1, c2, n, h, gx, gy, bits);
        break;
    case EQTypeMontgomery:
    	c1 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.mo.B);
    	assert(c1 != NULL);
    	c2 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.mo.A);
    	assert(c2 != NULL);
        rep = PyUnicode_FromFormat("ECC.ECurve.Montgomery(%S, %S, %S, %S, %S, %S, %S, %d)", p, c1, c2, n, h, gx, gy, bits);
        break;
    case EQTypeTwistedEdwards:
    	c1 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.te.a);
    	assert(c1 != NULL);
    	c2 = _mpFp_to_pylong(((ECurve *)a)->ec->coeff.te.d);
    	assert(c2 != NULL);
        rep = PyUnicode_FromFormat("ECC.ECurve.TwistedEdwards(%S, %S, %S, %S, %S, %S, %S, %d)", p, c1, c2, n, h, gx, gy, bits);
        break;
    default:
        PyErr_SetString(PyExc_AssertionError, "ECurve object has unknown/uninitialized type");
        rep = NULL;
	}
	Py_DECREF(gy);
	Py_DECREF(gx);
	Py_DECREF(h);
	Py_DECREF(n);
	if (c2 != NULL) Py_DECREF(c2);
	if (c1 != NULL) Py_DECREF(c1);
	Py_DECREF(p);
	return rep;
}

static PyObject *ECurve_getattr(PyObject *o, char *attr_name) {
    ECurve *cv;
    
	assert(PyObject_TypeCheck(o, &ECurveType) != 0);
	cv = (ECurve *)o;

    if (strcmp(attr_name, "p") == 0) {
        return _mpz_to_pylong(cv->ec->fp->p);
    }

    if (strcmp(attr_name, "n") == 0) {
        return _mpz_to_pylong(cv->ec->n);
    }

    if (strcmp(attr_name, "h") == 0) {
        return _mpz_to_pylong(cv->ec->h);
    }

    if (strcmp(attr_name, "G") == 0) {
        PyObject *gx;
        PyObject *gy;
        PyObject *t;
        int status;

        gx = _mpz_to_pylong(cv->ec->G[0]);
        gy = _mpz_to_pylong(cv->ec->G[1]);
        assert(gx != NULL);
        assert(gy != NULL);
        
        t = PyTuple_New(2);
        assert(t != NULL);

        status = PyTuple_SetItem(t, 0, gx);
        assert(status == 0);
        Py_INCREF(gx);
        status = PyTuple_SetItem(t, 1, gy);
        assert(status == 0);
        Py_INCREF(gy);

        return t;
    }
    
    if (strcmp(attr_name, "bits") == 0) {
        return PyLong_FromUnsignedLong(cv->ec->bits);
    }

    switch (cv->ec->type) {
    case EQTypeShortWeierstrass:
        if (strcmp(attr_name, "ctype") == 0) {
            return PyUnicode_FromString("ShortWeierstrass");
        }
        if (strcmp(attr_name, "a") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.ws.a);
        }
        if (strcmp(attr_name, "b") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.ws.b);
        }
        break;
    case EQTypeEdwards:
        if (strcmp(attr_name, "ctype") == 0) {
            return PyUnicode_FromString("Edwards");
        }
        if (strcmp(attr_name, "c") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.ed.c);
        }
        if (strcmp(attr_name, "d") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.ed.d);
        }
        break;
    case EQTypeMontgomery:
        if (strcmp(attr_name, "ctype") == 0) {
            return PyUnicode_FromString("Montgomery");
        }
        if (strcmp(attr_name, "B") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.mo.B);
        }
        if (strcmp(attr_name, "A") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.mo.A);
        }
        break;
    case EQTypeTwistedEdwards:
        if (strcmp(attr_name, "ctype") == 0) {
            return PyUnicode_FromString("TwistedEdwards");
        }
        if (strcmp(attr_name, "a") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.te.a);
        }
        if (strcmp(attr_name, "d") == 0) {
            return _mpFp_to_pylong(cv->ec->coeff.te.d);
        }
        break;
    default:
        break;
    }

    PyErr_SetString(PyExc_ValueError, "ECurve_getattr: Unknown attribute");
    return NULL;
}

static PyObject *ECurve_getattro(PyObject *o, PyObject *attr_name) {
    PyObject *tmp;
    char *attr_nm_c;

    if (!(tmp = PyObject_GenericGetAttr(o, attr_name))) {
        if (!PyErr_ExceptionMatches(PyExc_AttributeError))
            return NULL;
        PyErr_Clear();
    }
    else {
        return tmp;
    }

	assert(PyUnicode_Check(attr_name) != 0);
    attr_nm_c = PyUnicode_AsUTF8(attr_name);

    return ECurve_getattr(o, attr_nm_c);
}

static PyMemberDef ECurve_members[] = {
	{NULL}
};

static PyMethodDef ECurve_methods[] = {
	//{"getvalue", (PyCFunction)ECurve_getvalue, METH_NOARGS, "get value of element as an integer"},
	{"ShortWeierstrass", (PyCFunction)ECurve_create_ws, METH_VARARGS|METH_STATIC, ECurve_create_ws__doc__},
	{"Edwards", (PyCFunction)ECurve_create_ed, METH_VARARGS|METH_STATIC, ECurve_create_ed__doc__},
	{"Montgomery", (PyCFunction)ECurve_create_mo, METH_VARARGS|METH_STATIC, ECurve_create_mo__doc__},
	{"TwistedEdwards", (PyCFunction)ECurve_create_te, METH_VARARGS|METH_STATIC, ECurve_create_te__doc__},
	{"PointIsValid", (PyCFunction)ECurve_check_point, METH_VARARGS, "Return True if affine coordinates supplied are on the curve"},
	{"_CurveNames", (PyCFunction)ECurve_list_curve_names, METH_NOARGS|METH_CLASS, "List all named curves (intended for testing)"},
	{NULL}
};

PyTypeObject ECurveType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ECC.ECurve",                     /*tp_name*/
	sizeof(ECurve),                     /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)ECurve_dealloc,         /*tp_dealloc*/
	0,                                  /*tp_print*/
	ECurve_getattr,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,			                        /*tp_reserved*/
	ECurve_repr,                                  /*tp_repr*/
	0,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	0,                                  /*tp_str*/
	ECurve_getattro,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	ECurve__doc__,                      /* tp_doc */
	0,		                            /* tp_traverse */
	0,		                            /* tp_clear */
	ECurve_richcompare,		                            /* tp_richcompare */
	0,		                            /* tp_weaklistoffset */
	0,		                            /* tp_iter */
	0,		                            /* tp_iternext */
	ECurve_methods,                     /* tp_methods */
	ECurve_members,                     /* tp_members */
	0,                                  /* tp_getset */
	0,                                  /* tp_base */
	0,                                  /* tp_dict */
	0,                                  /* tp_descr_get */
	0,                                  /* tp_descr_set */
	0,                                  /* tp_dictoffset */
	(initproc)ECurve_init,              /* tp_init */
	0,                                  /* tp_alloc */
	ECurve_new,                         /* tp_new */
};
