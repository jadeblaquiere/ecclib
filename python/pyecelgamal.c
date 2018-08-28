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
#include <pyecelgamal.h>
#include <pyfield.h>
#include <pygmplong.h>
#include <pypoint.h>
#include <Python.h>
#include <structmember.h>

PyDoc_STRVAR(ECElgamalCiphertext__doc__,
"ECElgamalCiphertext implements encoding of cipherext values for the elliptic"
"curve variant of the Elgamal encryption system.\n");

// allocate the object
static PyObject *ECElgamalCiphertext_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    // create the new Parameterss object
    ECElgamalCiphertext *self = (ECElgamalCiphertext *)type->tp_alloc(type, 0);
    // make sure it actually worked
    if (!self) {
        PyErr_SetString(PyExc_TypeError, "could not create ECElgamalCiphertext object.");
        return NULL;
    }
    self->ready = 0;

    // cast and return
    return (PyObject *)self;
}

// Usage options:
// ECElgamalCiphertext(C=ECPoint, D=ECPoint)
static int ECElgamalCiphertext_init(ECElgamalCiphertext *self, PyObject *args, PyObject *kwargs) {
    ECPoint *C;
    ECPoint *D;

    if (!PyArg_ParseTuple(args, "OO", (PyObject *)&C, &D)) {
        PyErr_SetString(PyExc_TypeError, "Error parsing ECElgamalCiphertext_init arguments");
        return -1;
    }

    if (!PyObject_TypeCheck((PyObject *)C, &ECPointType)) {
        PyErr_SetString(PyExc_TypeError, "ECElgamalCiphertext_init: expected ECPoint type as argument 1 (C)");
        return -1;
    }

    if (!PyObject_TypeCheck((PyObject *)D, &ECPointType)) {
        PyErr_SetString(PyExc_TypeError, "ECElgamalCiphertext_init: expected ECPoint type as argument 2 (D)");
        return -1;
    }
    
    if (mpECurve_cmp(C->ecp->cvp, D->ecp->cvp) != 0) {
        PyErr_SetString(PyExc_ValueError, "ECElgamalCiphertext_init: C and D are not on the same curve");
        return -1;
    }

    Py_INCREF((PyObject *)C->cv);
    self->cv = C->cv;

    mpECP_init(self->ctxt->C, C->ecp->cvp);
    mpECP_set(self->ctxt->C, C->ecp);
    mpECP_init(self->ctxt->D, D->ecp->cvp);
    mpECP_set(self->ctxt->D, D->ecp);

    // you're ready!
    self->ready = 1;
    // all's clear
    return 0;
}

static PyObject *ECElgamalCiphertext_str(PyObject *self) {
    ECElgamalCiphertext *egc;
    char *Cstr;
    char *Dstr;
    size_t sz;
    PyObject *str;
    
    assert(PyObject_TypeCheck(self, &ECElgamalCiphertextType) != 0);
    egc = (ECElgamalCiphertext *)self;

    sz = mpECP_out_strlen(egc->ctxt->C, 1);
    Cstr = malloc((sz + 1)*sizeof(char));
    assert(Cstr != NULL);
    Dstr = malloc((sz + 1)*sizeof(char));
    assert(Dstr != NULL);
    mpECP_out_str(Cstr, egc->ctxt->C, 1);
    mpECP_out_str(Dstr, egc->ctxt->D, 1);
    str = PyUnicode_FromFormat("(%s, %s)", Cstr, Dstr);
    free(Dstr);
    free(Cstr);

    return str;
}

static PyObject *ECElgamalCiphertext_repr(PyObject *self) {
    ECPoint *Cpt;
    ECPoint *Dpt;
    PyObject *Cstr;
    PyObject *Dstr;
    ECElgamalCiphertext *egc;

    assert(PyObject_TypeCheck(self, &ECElgamalCiphertextType) != 0);
    egc = (ECElgamalCiphertext *)self;

	Cpt = (ECPoint *)PyECPoint_FromECP(egc->ctxt->C);
	Dpt = (ECPoint *)PyECPoint_FromECP(egc->ctxt->D);
    Cstr = Py_TYPE((PyObject *)Cpt)->tp_repr((PyObject *)Cpt);
    Dstr = Py_TYPE((PyObject *)Dpt)->tp_repr((PyObject *)Dpt);

    return PyUnicode_FromFormat("ECC.ECElgamalCiphertext(%S, %S)", Cstr, Dstr);
}

// deallocates the object when done
static void ECElgamalCiphertext_dealloc(ECElgamalCiphertext *self) {
    // clear the internal element
    if (self->ready){
        mpECElgamal_clear(self->ctxt);
        Py_DECREF((PyObject *)self->cv);
    }

    // free the object
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyDoc_STRVAR(ECElgamalCiphertext_encrypt__doc__, 
	"ECElgamalCiphertext_encrypt(public_key, plaintext) -> ECElgamalCiphertext\n\n"
	"Encrypt a plaintext ECPoint with a public key ECPoint.");

// Usage options:
// ECElgamalCiphertext_encrypt(pK=ECPoint, ptxt=ECPoint)
static PyObject *ECElgamalCiphertext_encrypt(PyObject *none, PyObject *args, PyObject *kwargs) {
    ECElgamalCiphertext *rop;
    ECPoint *key;
    ECPoint *ptxt;
    int status;

    if (!PyArg_ParseTuple(args, "OO", (PyObject *)&key, &ptxt)) {
        PyErr_SetString(PyExc_TypeError, "Error parsing ECElgamalCiphertext_encrypt arguments");
        return NULL;
    }

    if (!PyObject_TypeCheck((PyObject *)key, &ECPointType)) {
        PyErr_SetString(PyExc_TypeError, "ECElgamalCiphertext_init: expected ECPoint type as argument 1 (public key)");
        return NULL;
    }

    if (!PyObject_TypeCheck((PyObject *)ptxt, &ECPointType)) {
        PyErr_SetString(PyExc_TypeError, "ECElgamalCiphertext_init: expected ECPoint type as argument 2 (plaintext)");
        return NULL;
    }
    
    if (mpECurve_cmp(key->ecp->cvp, ptxt->ecp->cvp) != 0) {
        PyErr_SetString(PyExc_ValueError, "ECElgamalCiphertext_init: public key and plaintext are not on the same curve");
        return NULL;
    }

	rop = (ECElgamalCiphertext *)ECElgamalCiphertext_new( &ECElgamalCiphertextType, NULL, NULL);

    status = mpECElgamal_init_encrypt(rop->ctxt, key->ecp, ptxt->ecp);
    if (status != 0) {
        PyErr_SetString(PyExc_RuntimeError, "ECElgamalCiphertext_init: unexpected error encrypting");
        return NULL;
    }

    Py_INCREF((PyObject *)key->cv);
    rop->cv = key->cv;
    
    // you're ready!
    rop->ready = 1;
    // all's clear
    return (PyObject *)rop;
}

PyDoc_STRVAR(ECElgamalCiphertext_decrypt__doc__, 
	"ECElgamalCiphertext_decrypt(secret_key) -> ECPoint\n\n"
	"Decrypt a ciphertext to recover the plaintext ECPoint via a secret key.");

// Usage options:
// ECElgamalCiphertext_decrypt(sK=(FieldElement or PyLong))
static PyObject *ECElgamalCiphertext_decrypt(PyObject *self, PyObject *sK) {
    ECPoint *rop;
    mpFp_ptr feK;
    int status;
    ECElgamalCiphertext *egc;

    assert(PyObject_TypeCheck(self, &ECElgamalCiphertextType) != 0);
    egc = (ECElgamalCiphertext *)self;

    if (PyObject_TypeCheck(sK, &FieldElementType)) {
        feK = ((FieldElement *)sK)->fe;

        if (mpz_cmp(feK->fp->p, egc->ctxt->C->cvp->n) != 0) {
            PyErr_SetString(PyExc_ValueError, "ECElgamalCiphertext_init: key order incorrect for ciphertext");
            return NULL;
        }
    } else if (PyLong_Check(sK)) {
        mpz_t impz;

        feK = (mpFp_ptr)malloc(sizeof(mpFp_t));
    	mpz_init(impz);
        status = _pylong_to_mpz((PyLongObject *)sK, impz);
    	if (status != 0) {
    		PyErr_SetString(PyExc_TypeError, "Error converting long to FieldElement");
        	mpz_clear(impz);
    		return NULL;
    	}

        mpFp_init(feK, egc->ctxt->C->cvp->n);
        mpFp_set_mpz(feK, impz, egc->ctxt->C->cvp->n);
        mpz_clear(impz);
    } else {
        PyErr_SetString(PyExc_TypeError, "ECElgamalCiphertext_init: decryption key must be a FieldElement or Integer");
        return NULL;
    }

	rop = (ECPoint *)PyECPoint_FromECP(egc->ctxt->C);

    status = mpECElgamal_init_decrypt(rop->ecp, feK, egc->ctxt);
    if (status != 0) {
        PyErr_SetString(PyExc_RuntimeError, "ECElgamalCiphertext_init: unexpected error decrypting");
        return NULL;
    }

    if (PyLong_Check(sK)) {
        mpFp_clear(feK);
        free(feK);
    }

    // you're ready!
    rop->ready = 1;
    // all's clear
    return (PyObject *)rop;
}

static PyObject *ECElgamalCiphertext_getattr(PyObject *o, char *attr_name) {
    ECElgamalCiphertext *egc;

    assert(PyObject_TypeCheck(o, &ECElgamalCiphertextType) != 0);
    egc = (ECElgamalCiphertext *)o;

    if (strcmp(attr_name, "C") == 0) {
        return PyECPoint_FromECP(egc->ctxt->C);
    }

    if (strcmp(attr_name, "D") == 0) {
        return PyECPoint_FromECP(egc->ctxt->D);
    }

    PyErr_SetString(PyExc_ValueError, "ECurve_getattr: Unknown attribute");
    return NULL;
}

static PyObject *ECElgamalCiphertext_getattro(PyObject *o, PyObject *attr_name) {
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

    return ECElgamalCiphertext_getattr(o, attr_nm_c);
}

static PyMemberDef ECElgamalCiphertext_members[] = {
    {NULL}
};

static PyMethodDef ECElgamalCiphertext_methods[] = {
    //{"getvalue", (PyCFunction)ECPoint_getvalue, METH_NOARGS, "get value of element as an integer"},
	{"encrypt", (PyCFunction)ECElgamalCiphertext_encrypt, METH_VARARGS | METH_STATIC, ECElgamalCiphertext_encrypt__doc__},
	{"decrypt", (PyCFunction)ECElgamalCiphertext_decrypt, METH_O, ECElgamalCiphertext_decrypt__doc__},
    {NULL}
};

PyTypeObject ECElgamalCiphertextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ECC.ECElgamalCiphertext",                     /*tp_name*/
    sizeof(ECElgamalCiphertext),                     /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    (destructor)ECElgamalCiphertext_dealloc,         /*tp_dealloc*/
    0,                                  /*tp_print*/
    ECElgamalCiphertext_getattr,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,			                        /*tp_reserved*/
    ECElgamalCiphertext_repr,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash */
    0,                                  /*tp_call*/
    ECElgamalCiphertext_str,                                  /*tp_str*/
    ECElgamalCiphertext_getattro,                                  /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    ECElgamalCiphertext__doc__,                      /* tp_doc */
    0,		                            /* tp_traverse */
    0,		                            /* tp_clear */
    0,		                            /* tp_richcompare */
    0,		                            /* tp_weaklistoffset */
    0,		                            /* tp_iter */
    0,		                            /* tp_iternext */
    ECElgamalCiphertext_methods,                     /* tp_methods */
    ECElgamalCiphertext_members,                     /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)ECElgamalCiphertext_init,              /* tp_init */
    0,                                  /* tp_alloc */
    ECElgamalCiphertext_new,                         /* tp_new */
};
