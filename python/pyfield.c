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
#include <pyfield.h>
#include <pygmplong.h>
#include <Python.h>
#include <structmember.h>

#define _MPFP_MAX_LIMBS   (32)

static inline void mpFp_realloc(mpFp_t c) {
    if (__GMP_UNLIKELY(c->i->_mp_alloc < c->fp->p2size)) {
        mpz_realloc(c->i, c->fp->p2size);
    }
}

PyDoc_STRVAR(FieldElement__doc__,
"FieldElement implements a prime field element type.\n");

// allocate the object
static PyObject *FieldElement_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	// create the new Parameterss object
	FieldElement *self = (FieldElement *)type->tp_alloc(type, 0);
	// make sure it actually worked
	if (!self) {
		PyErr_SetString(PyExc_TypeError, "could not create FieldElement object.");
		return NULL;
	}
	self->ready = 0;

	// cast and return
	return (PyObject *)self;
}

// Usage options:
// FieldElement(i(value)=PyLongObject, p(field prime)=PyLongObject)
static int FieldElement_init(FieldElement *self, PyObject *args, PyObject *kwargs) {
	PyLongObject *i = NULL;
	PyLongObject *p = NULL;
	mpz_t impz;
	mpz_t pmpz;
	int status;

	if (!PyArg_ParseTuple(args, "OO", &i, &p)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing FieldElement_init arguments");
		return -1;
	}

	mpz_init(pmpz);
	status = _pylong_to_mpz_unsigned(p, pmpz);
	if (status != 0) {
		PyErr_SetString(PyExc_TypeError, "Error parsing FieldElement_init modulus");
		mpz_clear(pmpz);
		return -1;
	}

	mpz_init(impz);
    status = _pylong_to_mpz(i, impz);
	if (status != 0) {
		PyErr_SetString(PyExc_TypeError, "Error parsing FieldElement_init value");
    	mpz_clear(impz);
		mpz_clear(pmpz);
		return -1;
	}

    mpFp_init(self->fe, pmpz);
    mpFp_set_mpz_fp(self->fe, impz, self->fe->fp);

    mpz_clear(impz);
    mpz_clear(pmpz);

	// you're ready!
	self->ready = 1;
	// all's clear
	return 0;
}

// deallocates the object when done
static void FieldElement_dealloc(FieldElement *self) {
	// clear the internal element
	if (self->ready){
		mpFp_clear(self->fe);
	}

	// free the object
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *FieldElement_op_int(FieldElement *self, PyObject *none) {
	PyObject *rop;
	mpz_t t;

	mpz_init(t);

	// convert mpz to bytes, -1 implies little endian (wordwise and bytewise), 0 implies no "nails"
	mpz_set_mpFp(t, self->fe);
	rop = _mpz_to_pylong(t);

	mpz_clear(t);

	return rop;
}

static PyObject *FieldElement_op_add_pylong(FieldElement *op1, PyLongObject *op2) {
	FieldElement *rop;
	mpz_t op2z;
	mpFp_t op2f;

	mpz_init(op2z);
	mpFp_init_fp(op2f, op1->fe->fp);

	_pylong_to_mpz(op2, op2z);
	mpFp_set_mpz_fp(op2f, op2z, op1->fe->fp);

	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_add(rop->fe, op1->fe, op2f);

    mpFp_clear(op2f);
    mpz_clear(op2z);

    return (PyObject *)rop;
}

static PyObject *FieldElement_op_add(FieldElement *op1, FieldElement *op2) {
	FieldElement *rop;

	if(!PyObject_TypeCheck((PyObject *)op1, &FieldElementType)) {
		if(PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
			return FieldElement_op_add(op2, op1);
		}
		PyErr_SetString(PyExc_TypeError, "expected FieldElement, got something else.");
		return NULL;
	}
	
	if(PyLong_Check((PyObject *)op2)) {
		return FieldElement_op_add_pylong(op1, (PyLongObject *)op2);
	}
	
	if(!PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
		Py_RETURN_NOTIMPLEMENTED;
	}
	
	if (op1->fe->fp != op2->fe->fp) {
		PyErr_SetString(PyExc_ValueError, "FieldElement order mismatch");
		return NULL;
	}
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_add(rop->fe, op1->fe, op2->fe);
    return (PyObject *)rop;
}

static PyObject *FieldElement_op_sub_pylong(FieldElement *op1, PyLongObject *op2) {
	FieldElement *rop;
	mpz_t op2z;
	mpFp_t op2f;

	mpz_init(op2z);
	mpFp_init_fp(op2f, op1->fe->fp);

	_pylong_to_mpz(op2, op2z);
	mpFp_set_mpz_fp(op2f, op2z, op1->fe->fp);

	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_sub(rop->fe, op1->fe, op2f);

    mpFp_clear(op2f);
    mpz_clear(op2z);

    return (PyObject *)rop;
}

static PyObject *FieldElement_op_sub(FieldElement *op1, FieldElement *op2) {
	FieldElement *rop;

	if(!PyObject_TypeCheck((PyObject *)op1, &FieldElementType)) {
		if(PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
			PyObject *nop1;
			PyObject *nop2;
			nop1 = (Py_TYPE(op1))->tp_as_number->nb_negative((PyObject *)op1);
			nop2 = (Py_TYPE(op2))->tp_as_number->nb_negative((PyObject *)op2);
			if ((nop1 == NULL) || (nop2 == NULL)) {
				return NULL;
			}
			rop = (FieldElement *)FieldElement_op_sub((FieldElement *)nop2, (FieldElement *)nop1);
			Py_DECREF(nop1);
			Py_DECREF(nop2);
			return (PyObject *)rop;
		}
		PyErr_SetString(PyExc_TypeError, "expected FieldElement, got something else.");
		return NULL;
	}
	
	if(PyLong_Check((PyObject *)op2)) {
		return FieldElement_op_sub_pylong(op1, (PyLongObject *)op2);
	}
	
	if(!PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
		Py_RETURN_NOTIMPLEMENTED;
	}
	
	if (op1->fe->fp != op2->fe->fp) {
		PyErr_SetString(PyExc_ValueError, "FieldElement order mismatch");
		return NULL;
	}
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_sub(rop->fe, op1->fe, op2->fe);
    return (PyObject *)rop;
}

static PyObject *FieldElement_op_mul_pylong(FieldElement *op1, PyLongObject *op2) {
	FieldElement *rop;
	mpz_t op2z;
	mpFp_t op2f;

	mpz_init(op2z);
	mpFp_init_fp(op2f, op1->fe->fp);

	_pylong_to_mpz(op2, op2z);
	mpFp_set_mpz_fp(op2f, op2z, op1->fe->fp);

	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_mul(rop->fe, op1->fe, op2f);

    mpFp_clear(op2f);
    mpz_clear(op2z);

    return (PyObject *)rop;
}

static PyObject *FieldElement_op_mul(FieldElement *op1, FieldElement *op2) {
	FieldElement *rop;

	if(!PyObject_TypeCheck((PyObject *)op1, &FieldElementType)) {
		if(PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
			return FieldElement_op_mul(op2, op1);
		}
		PyErr_SetString(PyExc_TypeError, "expected FieldElement, got something else.");
		return NULL;
	}
	
	if(PyLong_Check((PyObject *)op2)) {
		return FieldElement_op_mul_pylong(op1, (PyLongObject *)op2);
	}
	
	if(!PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
		Py_RETURN_NOTIMPLEMENTED;
	}
	
	if (op1->fe->fp != op2->fe->fp) {
		PyErr_SetString(PyExc_ValueError, "FieldElement order mismatch");
		return NULL;
	}
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_mul(rop->fe, op1->fe, op2->fe);
    return (PyObject *)rop;
}

static PyObject *FieldElement_op_neg(FieldElement *op1) {
	FieldElement *rop;
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_neg(rop->fe, op1->fe);
    return (PyObject *)rop;
}

static PyObject *FieldElement_op_multiplicative_inverse(FieldElement *op1, PyObject *none) {
	FieldElement *rop;
	int status;
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    status = mpFp_inv(rop->fe, op1->fe);
    if (__GMP_UNLIKELY(status != 0)) {
		mpz_clear(rop->fe->i);
    	Py_RETURN_NONE;
    }
    return (PyObject *)rop;
}

static PyObject *FieldElement_op_sqrt(FieldElement *op1, PyObject *none) {
	FieldElement *rop;
	int status;
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = op1->fe->fp;
    mpFp_realloc(rop->fe);
    status = mpFp_sqrt(rop->fe, op1->fe);
    if (status != 0) {
		mpz_clear(rop->fe->i);
    	Py_RETURN_NONE;
    }
    return (PyObject *)rop;
}

// Usage options: (classmethod)
// FieldElement.urandom(p=PyLongObject)
PyDoc_STRVAR(FieldElement_urandom__doc__, 
	"urandom(p) -> FieldElement\n\n"
	"Static Method to generate a random element in the prime field Fp defined by p. "
	"The field order p must be an integer (or long integer) value. "
	"urandom uses /dev/urandom to obtain a cryptographically secure random.");
static PyObject *FieldElement_urandom(PyObject *none, PyObject *plong) {
	FieldElement *rop;
	mpz_t pmpz;
	int status;

	mpz_init(pmpz);

	status = _pylong_to_mpz_unsigned((PyLongObject *)plong, pmpz);
	if (status != 0) {
		PyErr_SetString(PyExc_TypeError, "Error parsing FieldElement_init modulus");
		mpz_clear(pmpz);
		return NULL;
	}

	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
    mpFp_urandom(rop->fe, pmpz);

    mpz_clear(pmpz);

	// you're ready!
	rop->ready = 1;
	// all's clear
	return (PyObject *)rop;
}

static PyObject *FieldElement_richcompare(PyObject *a, PyObject *b, int op) {
	int result = 0;

	assert(PyObject_TypeCheck(a, &FieldElementType) != 0);

	if (PyLong_Check(b)) {
		mpz_t bmpz;
		int status;
		
		mpz_init(bmpz);
		status = _pylong_to_mpz((PyLongObject *)b, bmpz);
		assert(status == 0);
		if (status != 0) {
			PyErr_SetString(PyExc_TypeError, "Error converting integer to mpz_t");
			return NULL;
		}
		switch (op) {
		case Py_EQ:
			if (mpFp_cmp_mpz(((FieldElement *)a)->fe, bmpz) == 0) {
				result = 1;
			}
			break;
		case Py_NE:
			if (mpFp_cmp_mpz(((FieldElement *)a)->fe, bmpz) != 0) {
				result = 1;
			}
			break;
		default:
			Py_RETURN_NOTIMPLEMENTED;
		}
		mpz_clear(bmpz);
	} else {
		if (!PyObject_TypeCheck((PyObject *)b, &FieldElementType)) {
			Py_RETURN_NOTIMPLEMENTED;
		}
		switch (op) {
		case Py_EQ:
			if (mpFp_cmp(((FieldElement *)a)->fe, ((FieldElement *)b)->fe) == 0) {
				result = 1;
			}
			break;
		case Py_NE:
			if (mpFp_cmp(((FieldElement *)a)->fe, ((FieldElement *)b)->fe) != 0) {
				result = 1;
			}
			break;
		default:
			Py_RETURN_NOTIMPLEMENTED;
		}
	}

	if (result != 0) {
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyObject *FieldElement_op_power(FieldElement *a, FieldElement *b, PyObject *c) {
	FieldElement *rop;
	mpz_t bmpz;
	unsigned long bui;
	int status;
	
	if (c != Py_None) {
		PyErr_SetString(PyExc_ValueError, "Optional 3rd argument to pow() not supported for Type");
		return NULL;
	}

	mpz_init(bmpz);
	status = _pylong_to_mpz_unsigned((PyLongObject *)b, bmpz);
	if (status != 0) {
		PyErr_SetString(PyExc_ValueError, "2rd argument to pow() must be of integer(/long) type");
		mpz_clear(bmpz);
		return NULL;
	}

	bui = mpz_get_ui(bmpz);
	if (mpz_cmp_ui(bmpz, bui) != 0) {
		PyErr_SetString(PyExc_ValueError, "2rd argument to pow() too large");
		mpz_clear(bmpz);
		return NULL;
	}
	mpz_clear(bmpz);
	
	rop = (FieldElement *)FieldElement_new( &FieldElementType, NULL, NULL);
	mpz_init(rop->fe->i);
    rop->fe->fp = a->fe->fp;
    mpFp_realloc(rop->fe);
    mpFp_pow_ui(rop->fe, a->fe, bui);
    return (PyObject *)rop;
}

static PyObject *FieldElement_repr(PyObject *a) {
	PyObject *i;
	PyObject *p;
	PyObject *rep;
	assert(PyObject_TypeCheck(a, &FieldElementType) != 0);
	
	i = _mpz_to_pylong(((FieldElement *)a)->fe->i);
	assert(i != NULL);
	p = _mpz_to_pylong(((FieldElement *)a)->fe->fp->p);
	assert(p != NULL);
	rep = PyUnicode_FromFormat("ECC.FieldElement(%S, %S)", i, p);
	Py_DECREF(p);
	Py_DECREF(i);
	return rep;
}

static PyMemberDef FieldElement_members[] = {
	{NULL}
};

static PyMethodDef FieldElement_methods[] = {
	//{"getvalue", (PyCFunction)FieldElement_getvalue, METH_NOARGS, "get value of element as an integer"},
	{"inverse", (PyCFunction)FieldElement_op_multiplicative_inverse, METH_NOARGS, "return modular multiplicative inverse of value. return None if no inverse exists."},
	{"sqrt", (PyCFunction)FieldElement_op_sqrt, METH_NOARGS, "return square root of value. returns None if the value is not a quadratic residue."},
	{"urandom", (PyCFunction)FieldElement_urandom, METH_O|METH_STATIC, FieldElement_urandom__doc__},
	{NULL}
};

PyNumberMethods FieldElement_num_meths = {
	(binaryfunc)FieldElement_op_add,		//binaryfunc nb_add;
	(binaryfunc)FieldElement_op_sub,		//binaryfunc nb_subtract;
	(binaryfunc)FieldElement_op_mul,		//binaryfunc nb_multiply;
	0,				//binaryfunc nb_remainder;
	0,				//binaryfunc nb_divmod;
	(ternaryfunc)FieldElement_op_power,		//ternaryfunc nb_power;
	(unaryfunc)FieldElement_op_neg,		//unaryfunc nb_negative;
	0,				//unaryfunc nb_positive;
	0,				//unaryfunc nb_absolute;
	0,	//inquiry nb_bool;
	(unaryfunc)0,	//unaryfunc nb_invert;
	0,				//binaryfunc nb_lshift;
	0,				//binaryfunc nb_rshift;
	0,				//binaryfunc nb_and;
	0,				//binaryfunc nb_xor;
	0,				//binaryfunc nb_or;
	(unaryfunc)FieldElement_op_int,		//unaryfunc nb_int;
	0,				//void *nb_reserved;
	0,				//unaryfunc nb_float;

	0,				//binaryfunc nb_inplace_add;
	0,				//binaryfunc nb_inplace_subtract;
	0,				//binaryfunc nb_inplace_multiply;
	0,				//binaryfunc nb_inplace_remainder;
	0,				//ternaryfunc nb_inplace_power;
	0,				//binaryfunc nb_inplace_lshift;
	0,				//binaryfunc nb_inplace_rshift;
	0,				//binaryfunc nb_inplace_and;
	0,				//binaryfunc nb_inplace_xor;
	0,				//binaryfunc nb_inplace_or;
	0,
	0,				//binaryfunc nb_floor_divide;
	0,		//binaryfunc nb_true_divide;
	0,				//binaryfunc nb_inplace_floor_divide;
	0,				//binaryfunc nb_inplace_true_divide;
};

PyTypeObject FieldElementType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ECC.FieldElement",                     /*tp_name*/
	sizeof(FieldElement),                     /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)FieldElement_dealloc,         /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,			                        /*tp_reserved*/
	FieldElement_repr,                                  /*tp_repr*/
	&FieldElement_num_meths,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	0,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	FieldElement__doc__,                      /* tp_doc */
	0,		                            /* tp_traverse */
	0,		                            /* tp_clear */
	FieldElement_richcompare,		                            /* tp_richcompare */
	0,		                            /* tp_weaklistoffset */
	0,		                            /* tp_iter */
	0,		                            /* tp_iternext */
	FieldElement_methods,                     /* tp_methods */
	FieldElement_members,                     /* tp_members */
	0,                                  /* tp_getset */
	0,                                  /* tp_base */
	0,                                  /* tp_dict */
	0,                                  /* tp_descr_get */
	0,                                  /* tp_descr_set */
	0,                                  /* tp_dictoffset */
	(initproc)FieldElement_init,              /* tp_init */
	0,                                  /* tp_alloc */
	FieldElement_new,                         /* tp_new */
};
