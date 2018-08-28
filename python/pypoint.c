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
#include <pyfield.h>
#include <pygmplong.h>
#include <pypoint.h>
#include <Python.h>
#include <structmember.h>

PyDoc_STRVAR(ECPoint__doc__,
"ECPoint implements a general interface to elliptic curve points and basic "
"aritmetic operations on points.\n");

// allocate the object
static PyObject *ECPoint_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	// create the new Parameterss object
	ECPoint *self = (ECPoint *)type->tp_alloc(type, 0);
	// make sure it actually worked
	if (!self) {
		PyErr_SetString(PyExc_TypeError, "could not create ECPoint object.");
		return NULL;
	}
	self->ready = 0;

	// cast and return
	return (PyObject *)self;
}

// Usage options:
// ECPoint(curve=ECurve, (optional)initial=String)
//     where the bytes contain a compressed/uncompressed point value (or X coord
//     for Montgomery curve points) as a string literal, e.g. "026AF23456..."
// ECPoint(curve=ECurve, (optional)initial=Tuple(X, Y))
//     where the Tuple can be (PyLong, PyLong) or (FieldElement, FieldElement)
// by default point is initialized as the neutral element ("at infinity")
static int ECPoint_init(ECPoint *self, PyObject *args, PyObject *kwargs) {
    PyObject *curve;
    PyObject *initial = NULL;
	int status;

	if (!PyArg_ParseTuple(args, "O|O", &curve, &initial)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECPoint_init arguments");
		return -1;
	}

	if(!PyObject_TypeCheck((PyObject *)curve, &ECurveType)) {
		PyErr_SetString(PyExc_TypeError, "ECPoint_init: expected ECurve type as argument 1");
		return -1;
	}

	mpECP_init(self->ecp, ((ECurve *)curve)->ec);
	self->cv = (ECurve *)curve;
	Py_INCREF(curve);
	if (initial == NULL) {
	    mpECP_set_neutral(self->ecp, ((ECurve *)curve)->ec);
    } else {
        if (PyTuple_Check(initial)) {
            PyObject *ix, *iy;

            if (PyTuple_Size(initial) != 2) {
        		PyErr_SetString(PyExc_TypeError, "initializer tuple size mismatch");
        		return -1;
            }
            
            ix = PyTuple_GetItem(initial, 0);
            assert(ix != NULL);
            iy = PyTuple_GetItem(initial, 1);
            assert(iy != NULL);
            
            if (PyLong_Check(ix)) {
                mpz_t ixmpz, iympz;

                if (!PyLong_Check(iy)) {
        		    PyErr_SetString(PyExc_TypeError, "initializer tuple type mismatch");
        		    return -1;
                }
                
                mpz_init(ixmpz);
                _pylong_to_mpz((PyLongObject *)ix, ixmpz);
                mpz_init(iympz);
                _pylong_to_mpz((PyLongObject *)iy, iympz);

                mpECP_set_mpz(self->ecp, ixmpz, iympz, ((ECurve *)curve)->ec);
                
                mpz_clear(iympz);
                mpz_clear(ixmpz);
            } else {
                if ((!PyObject_TypeCheck(ix, &FieldElementType)) || 
                    (!PyObject_TypeCheck(iy, &FieldElementType))) {
        		    PyErr_SetString(PyExc_TypeError, "initializer tuple type mismatch");
        		    return -1;
                }

                if ((((FieldElement *)ix)->fe->fp != ((ECurve *)curve)->ec->fp) || 
                    (((FieldElement *)iy)->fe->fp != ((ECurve *)curve)->ec->fp)) {
        		    PyErr_SetString(PyExc_ValueError, "initializer tuple field order mismatch");
        		    return -1;
                }

                mpECP_set_mpFp(self->ecp, ((FieldElement *)ix)->fe,
                    ((FieldElement *)iy)->fe, ((ECurve *)curve)->ec);
            }
        } else {
            if (PyUnicode_Check(initial)) {
                char *buffer;

                buffer = (char *)PyUnicode_AsUTF8(initial);
                status = mpECP_set_str(self->ecp,buffer,((ECurve *)curve)->ec);
                if (status != 0) {
            		PyErr_SetString(PyExc_ValueError, "invalid curve point");
            		return -1;
                }
            } else if (PyBytes_Check(initial)) { 
            	unsigned char *buffer;
            	Py_ssize_t bsize;

                status = PyBytes_AsStringAndSize(initial, (char **)&buffer, &bsize);
                if (status != 0) {
            		PyErr_SetString(PyExc_ValueError, "unable to convert bytes initializer");
            		return -1;
                }
                status = mpECP_set_bytes(self->ecp,buffer, (size_t)bsize, ((ECurve *)curve)->ec);
                if (status != 0) {
            		PyErr_SetString(PyExc_ValueError, "invalid curve point");
            		return -1;
                }
            } else {
        		PyErr_SetString(PyExc_TypeError, "initializer type mismatch, expected unicode string");
        		return -1;
                //char *buffer;
                //int len;
                //if (!PyBytes_Check(initial) {
        		//    PyErr_SetString(PyExc_TypeError, "initializer type mismatch, expected bytes");
        		//    return -1;
                //}

                //status = PyBytes_AsStringAndSize(initial, &buffer, &len);
                //if (status != 0) {
        		//    PyErr_SetString(PyExc_ValueError, "initializer type mismatch, unable to convert bytes");
        		//    return -1;
                //}
                
                //if (buffer[0] == 0x04) {
                    
                //}
            }
        }
    }

	// you're ready!
	self->ready = 1;
	// all's clear
	return 0;
}

static PyObject *ECPoint_str(PyObject *self) {
    char *buffer;
    int bsize;
    PyObject *pointstr;
    ECPoint *ecself;

	assert(PyObject_TypeCheck(self, &ECPointType) != 0);
	ecself = (ECPoint *)self;

	bsize = mpECP_out_strlen(ecself->ecp, 1);
	buffer = (char *)malloc(2*bsize*sizeof(char));
	mpECP_out_str(buffer, ecself->ecp, 1);
	pointstr = PyUnicode_FromStringAndSize(buffer, bsize);
	free(buffer);

	return pointstr;
}

static PyObject *ECPoint_binary(PyObject *self) {
    unsigned char *buffer;
    int bsize;
    PyObject *pointbin;
    ECPoint *ecself;

	assert(PyObject_TypeCheck(self, &ECPointType) != 0);
	ecself = (ECPoint *)self;

	bsize = mpECP_out_bytelen(ecself->ecp, 1);
	buffer = (unsigned char *)malloc(2*bsize*sizeof(char));
	mpECP_out_bytes(buffer, ecself->ecp, 1);
	pointbin = PyBytes_FromStringAndSize((char*)buffer, bsize);
	free(buffer);

	return pointbin;
}

static PyObject *ECPoint_binary_full(PyObject *self) {
    unsigned char *buffer;
    int bsize;
    PyObject *pointbin;
    ECPoint *ecself;

	assert(PyObject_TypeCheck(self, &ECPointType) != 0);
	ecself = (ECPoint *)self;

	bsize = mpECP_out_bytelen(ecself->ecp, 0);
	buffer = (unsigned char *)malloc(2*bsize*sizeof(char));
	mpECP_out_bytes(buffer, ecself->ecp, 0);
	pointbin = PyBytes_FromStringAndSize((char*)buffer, bsize);
	free(buffer);

	return pointbin;
}

static PyObject *ECPoint_repr(PyObject *self) {
    PyObject *pointstr;
    PyObject *cvstr;
    ECPoint *ecself;

	assert(PyObject_TypeCheck(self, &ECPointType) != 0);
	ecself = (ECPoint *)self;

    pointstr = ECPoint_str(self);

    cvstr = Py_TYPE((PyObject *)(ecself->cv))->tp_repr((PyObject *)(ecself->cv));

	return PyUnicode_FromFormat("ECC.ECPoint(%S, \'%S\')", cvstr, pointstr);
}

// deallocates the object when done
static void ECPoint_dealloc(ECPoint *self) {
	// clear the internal element
	if (self->ready){
		mpECP_clear(self->ecp);
		Py_DECREF((PyObject *)self->cv);
	}

	// free the object
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *ECPoint_richcompare(PyObject *a, PyObject *b, int op) {
	int result = 0;

	assert(PyObject_TypeCheck(a, &ECPointType) != 0);

	if (!PyObject_TypeCheck((PyObject *)b, &ECPointType)) {
		Py_RETURN_NOTIMPLEMENTED;
	}

	switch (op) {
	case Py_EQ:
		if (mpECP_cmp(((ECPoint *)a)->ecp, ((ECPoint *)b)->ecp) == 0) {
			result = 1;
		}
		break;
	case Py_NE:
		if (mpECP_cmp(((ECPoint *)a)->ecp, ((ECPoint *)b)->ecp) != 0) {
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

static PyObject *ECPoint_op_add(ECPoint *op1, ECPoint *op2) {
	ECPoint *rop;

	if ((!PyObject_TypeCheck((PyObject *)op1, &ECPointType)) ||
		(!PyObject_TypeCheck((PyObject *)op2, &ECPointType))) {
		Py_RETURN_NOTIMPLEMENTED;
	}
	
	if (mpECurve_cmp(op1->ecp->cvp, op2->ecp->cvp) != 0) {
		PyErr_SetString(PyExc_ValueError, "ECPoint_op_add curve mismatch");
		return NULL;
	}
	
	rop = (ECPoint *)ECPoint_new(&ECPointType, NULL, NULL);
	mpECP_init(rop->ecp, op1->ecp->cvp);
    rop->cv = op1->cv;
    Py_INCREF(rop->cv);
    mpECP_add(rop->ecp, op1->ecp, op2->ecp);
	rop->ready = 1;
    return (PyObject *)rop;
}

static PyObject *ECPoint_op_sub(ECPoint *op1, ECPoint *op2) {
	ECPoint *rop;

	if ((!PyObject_TypeCheck((PyObject *)op1, &ECPointType)) ||
		(!PyObject_TypeCheck((PyObject *)op2, &ECPointType))) {
		Py_RETURN_NOTIMPLEMENTED;
	}
	
	if (mpECurve_cmp(op1->ecp->cvp, op2->ecp->cvp) != 0) {
		PyErr_SetString(PyExc_ValueError, "ECPoint_op_add curve mismatch");
		return NULL;
	}
	
	rop = (ECPoint *)ECPoint_new(&ECPointType, NULL, NULL);
	mpECP_init(rop->ecp, op1->ecp->cvp);
    rop->cv = op1->cv;
    Py_INCREF(rop->cv);
    mpECP_sub(rop->ecp, op1->ecp, op2->ecp);
	rop->ready = 1;
    return (PyObject *)rop;
}

static PyObject *ECPoint_op_neg(ECPoint *op1) {
	ECPoint *rop;

	if (!PyObject_TypeCheck((PyObject *)op1, &ECPointType)) {
		PyErr_SetString(PyExc_TypeError, "unsupported operand type for ECPoint negation(unary -)");
		return NULL;
	}
	
	rop = (ECPoint *)ECPoint_new(&ECPointType, NULL, NULL);
	mpECP_init(rop->ecp, op1->ecp->cvp);
    rop->cv = op1->cv;
    Py_INCREF(rop->cv);
    mpECP_neg(rop->ecp, op1->ecp);
	rop->ready = 1;
    return (PyObject *)rop;
}

static PyObject *ECPoint_op_mul(PyObject *op1, PyObject *op2) {
	ECPoint *rop, *p;
	mpz_t scalar;
	int negative = 0;

	if(!PyObject_TypeCheck(op1, &ECPointType)) {
		if(PyObject_TypeCheck((PyObject *)op2, &ECPointType)) {
			return ECPoint_op_mul(op2, op1);
		}
		PyErr_SetString(PyExc_TypeError, "expected ECPointType, got something else.");
		return NULL;
	}
	p = (ECPoint *)op1;

	mpz_init(scalar);
	if(PyLong_Check(op2)) {
	    int status;

		status = _pylong_to_mpz((PyLongObject *)op2, scalar);
		assert(status == 0);
		if (mpz_cmp_ui(scalar, 0) < 0) {
		    negative = 1;
		    mpz_neg(scalar, scalar);
		}
	} else if(PyObject_TypeCheck((PyObject *)op2, &FieldElementType)) {
	    mpz_set_mpFp(scalar, ((FieldElement *)op2)->fe);
    } else {
		Py_RETURN_NOTIMPLEMENTED;
	}

	rop = (ECPoint *)ECPoint_new( &ECPointType, NULL, NULL);
	mpECP_init(rop->ecp, p->ecp->cvp);
    rop->cv = p->cv;
    Py_INCREF(rop->cv);
    if (rop->ecp->base_bits != 0) {
        mpECP_scalar_base_mul_mpz(rop->ecp, p->ecp, scalar);
    } else {
        mpECP_scalar_mul_mpz(rop->ecp, p->ecp, scalar);
    }
    if (negative != 0) {
        mpECP_neg(rop->ecp, rop->ecp);
    }
    mpz_clear(scalar);
	rop->ready = 1;
    return (PyObject *)rop;
}

// Usage options: (classmethod)
// FieldElement.urandom(p=PyLongObject)
PyDoc_STRVAR(ECPoint_urandom__doc__, 
	"urandom(cv) -> ECPoint\n\n"
	"Static Method to generate a random element on the elliptic curve cv. "
	"urandom uses /dev/urandom to obtain a cryptographically secure random.");
static PyObject *ECPoint_urandom(PyObject *none, ECurve *curve) {
	ECPoint *rop;
	
	assert(PyObject_TypeCheck((PyObject *)curve, &ECurveType));

	rop = (ECPoint *)ECPoint_new( &ECPointType, NULL, NULL);
    mpECP_urandom(rop->ecp, curve->ec);
    rop->cv = curve;
    Py_INCREF(rop->cv);

	// you're ready!
	rop->ready = 1;
	// all's clear
	return (PyObject *)rop;
}

static PyObject *ECPoint_affine(PyObject *p, PyObject *none) {
    PyObject *rx, *ry, *rt;
    mpz_t zx, zy;
    int status;
	
	assert(PyObject_TypeCheck((PyObject *)p, &ECPointType));
	
	mpz_init(zx);
	mpz_init(zy);
	
	mpz_set_mpECP_affine_x(zx, ((ECPoint *)p)->ecp);
	mpz_set_mpECP_affine_y(zy, ((ECPoint *)p)->ecp);

	rx = _mpz_to_pylong(zx);
	ry = _mpz_to_pylong(zy);
	
	mpz_clear(zx);
	mpz_clear(zy);

	rt = PyTuple_New(2);
    assert(rt != NULL);

    status = PyTuple_SetItem(rt, 0, rx);
    assert(status == 0);
    Py_INCREF(rx);
    status = PyTuple_SetItem(rt, 1, ry);
    assert(status == 0);
    Py_INCREF(ry);

	return (PyObject *)rt;
}

static PyObject *ECPoint_set_basemult(PyObject *p, PyObject *none) {
	assert(PyObject_TypeCheck((PyObject *)p, &ECPointType));

    mpECP_scalar_base_mul_setup(((ECPoint *)p)->ecp);
    
    Py_RETURN_NONE;
}

PyObject *PyECPoint_FromECP(mpECP_t pt) {
	ECPoint *rop;

	rop = (ECPoint *)ECPoint_new( &ECPointType, NULL, NULL);
	mpECP_init(rop->ecp, pt->cvp);
	mpECP_set(rop->ecp, pt);
	rop->cv = (ECurve *)PyECurve_FromECurve(pt->cvp);
	Py_INCREF(rop->cv);
	rop->ready = 1;
	return (PyObject *)rop;
}

static PyMemberDef ECPoint_members[] = {
	{NULL}
};

static PyMethodDef ECPoint_methods[] = {
	//{"getvalue", (PyCFunction)ECPoint_getvalue, METH_NOARGS, "get value of element as an integer"},
	{"urandom", (PyCFunction)ECPoint_urandom, METH_O | METH_STATIC, ECPoint_urandom__doc__},
	{"affine", (PyCFunction)ECPoint_affine, METH_NOARGS, "returns point affine coords (x,y) as tuple, affine representation of \"point at infinity\" is undefined"},
	{"compressed", (PyCFunction)ECPoint_binary, METH_NOARGS, "dump raw (compressed) binary representation of the point"},
	{"uncompressed", (PyCFunction)ECPoint_binary_full, METH_NOARGS, "dump raw (compressed) binary representation of the point"},
	{"setup_basemult", (PyCFunction)ECPoint_set_basemult, METH_NOARGS, "set up LUTs for accelerated point multiplication (base point multiplication)"},
	{NULL}
};

PyNumberMethods ECPoint_num_meths = {
	(binaryfunc)ECPoint_op_add,		//binaryfunc nb_add;
	(binaryfunc)ECPoint_op_sub,		//binaryfunc nb_subtract;
	(binaryfunc)ECPoint_op_mul,		//binaryfunc nb_multiply;
	0,				//binaryfunc nb_remainder;
	0,				//binaryfunc nb_divmod;
	(ternaryfunc)0,		//ternaryfunc nb_power;
	(unaryfunc)ECPoint_op_neg,		//unaryfunc nb_negative;
	0,				//unaryfunc nb_positive;
	0,				//unaryfunc nb_absolute;
	0,	//inquiry nb_bool;
	(unaryfunc)0,	//unaryfunc nb_invert;
	0,				//binaryfunc nb_lshift;
	0,				//binaryfunc nb_rshift;
	0,				//binaryfunc nb_and;
	0,				//binaryfunc nb_xor;
	0,				//binaryfunc nb_or;
	(unaryfunc)0,		//unaryfunc nb_int;
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

PyTypeObject ECPointType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"ECC.ECPoint",                     /*tp_name*/
	sizeof(ECPoint),                     /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)ECPoint_dealloc,         /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,			                        /*tp_reserved*/
	ECPoint_repr,                                  /*tp_repr*/
	&ECPoint_num_meths,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	ECPoint_str,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	ECPoint__doc__,                      /* tp_doc */
	0,		                            /* tp_traverse */
	0,		                            /* tp_clear */
	ECPoint_richcompare,		                            /* tp_richcompare */
	0,		                            /* tp_weaklistoffset */
	0,		                            /* tp_iter */
	0,		                            /* tp_iternext */
	ECPoint_methods,                     /* tp_methods */
	ECPoint_members,                     /* tp_members */
	0,                                  /* tp_getset */
	0,                                  /* tp_base */
	0,                                  /* tp_dict */
	0,                                  /* tp_descr_get */
	0,                                  /* tp_descr_set */
	0,                                  /* tp_dictoffset */
	(initproc)ECPoint_init,              /* tp_init */
	0,                                  /* tp_alloc */
	ECPoint_new,                         /* tp_new */
};
