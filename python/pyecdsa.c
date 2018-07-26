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
#include <pyecdsa.h>
#include <pyfield.h>
#include <pygmplong.h>
#include <pypoint.h>
#include <Python.h>
#include <structmember.h>

// since we want to call back into a python hash function to generate the
// message digest we need to pass both a pointer to the message and the
// python hash function module in place of msg pointer
typedef struct {
    PyObject *hashmodule;
    PyObject *msg;
} _hash_and_msg_t;

// c routine to call the python hash function - note presumes that the
// the msg has been replaced by _hash_and_msg_t to encode both the python
// hash function and the message bytes (python bytes object)
static void _python_hash_wrapper(unsigned char *hash, unsigned char *msg, size_t sz){
    _hash_and_msg_t *ham;
    PyObject *mdict;
    PyObject *hnew;
    PyObject *tuple;
    PyObject *pyhash;
    PyObject *hbytes;
    int status;
    Py_ssize_t hsz;
    unsigned char *digest;
    
    ham = (_hash_and_msg_t *)msg;
    
    // no way to error out here, calling function must ensure message is bytes
    assert(PyBytes_Check(ham->msg));

    // call hash.new(msg) to initialize and process message
    mdict = PyModule_GetDict(ham->hashmodule);
    assert(PyDict_Check(mdict));
    hnew = PyDict_GetItemString(mdict, "new");
    assert(PyFunction_Check(hnew));
    assert(PyCallable_Check(hnew));
    Py_INCREF(hnew);

    tuple = PyTuple_New(1);
    assert(tuple != NULL);
    Py_INCREF(ham->msg);
    status = PyTuple_SetItem(tuple, 0, ham->msg);
    assert(status == 0);
    pyhash = PyObject_CallObject(hnew, tuple);
    assert(pyhash != NULL);
    Py_DECREF(ham->msg);
    Py_DECREF(hnew);

    Py_INCREF(pyhash);
    hbytes = PyObject_CallMethod(pyhash, "digest", NULL);
    assert(PyBytes_Check(hbytes));
    Py_DECREF(pyhash);

    hsz = PyBytes_Size(hbytes);
    status = PyBytes_AsStringAndSize(hbytes, (char **)&digest, &hsz);
    assert(status == 0);

    memcpy(hash, digest, hsz);
    return;
}

PyDoc_STRVAR(ECDSASignature__doc__,
"ECDSASignature implements encoding of ECDSA Signature values.\n");

// allocate the object
static PyObject *ECDSASignature_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    // create the new Parameterss object
    ECDSASignature *self = (ECDSASignature *)type->tp_alloc(type, 0);
    // make sure it actually worked
    if (!self) {
        PyErr_SetString(PyExc_TypeError, "could not create ECDSASignature object.");
        return NULL;
    }
    self->ready = 0;

    // cast and return
    return (PyObject *)self;
}

// Usage options:
// ECDSASignature(scheme=ECDSASignatureScheme, signature=BytesObject)
static int ECDSASignature_init(ECDSASignature *self, PyObject *args, PyObject *kwargs) {
    ECDSASignatureScheme *ss;
    PyObject *ssbytes;
    unsigned char *bytes;
    Py_ssize_t bsz;
    int status;

    if (!PyArg_ParseTuple(args, "OO", (PyObject *)&ss, &ssbytes)) {
        PyErr_SetString(PyExc_TypeError, "Error parsing ECDSASignature_init arguments");
        return -1;
    }

    if (!PyObject_TypeCheck((PyObject *)ss, &ECDSASignatureSchemeType)) {
        PyErr_SetString(PyExc_TypeError, "ECDSASignature_init: expected ECDSASignatureScheme type as argument 1");
        return -1;
    }

    if (PyBytes_Check(ssbytes)) {
        status = PyBytes_AsStringAndSize(ssbytes, (char **)&bytes, &bsz);
        if (status != 0) {
            PyErr_SetString(PyExc_ValueError, "ECDSASignature_init: cannot parse bytes object");
            return -1;
        }

        status =  mpECDSASignature_init_import_bytes(self->sig, ss->sscheme, bytes, bsz);
        if (status != 0) {
            PyErr_SetString(PyExc_ValueError, "ECDSASignature_init: cannot initialize from bytes object");
            return -1;
        }
    } else {
        char *sstr;

        if (!PyUnicode_Check(ssbytes)) {
            PyErr_SetString(PyExc_TypeError, "ECDSASignature_init: expected bytes or unicode type as argument 2");
            return -1;
        }

        sstr = PyUnicode_AsUTF8(ssbytes);

        status =  mpECDSASignature_init_import_str(self->sig, ss->sscheme, sstr);
        if (status != 0) {
            PyErr_SetString(PyExc_ValueError, "ECDSASignature_init: cannot initialize from string object");
            return -1;
        }
    }

    Py_INCREF((PyObject *)ss);
    self->ss = ss;

    // you're ready!
    self->ready = 1;
    // all's clear
    return 0;
}

static PyObject *ECDSASignature_str(PyObject *self) {
    ECDSASignature *ssig;
    char *sstr;
    PyObject *str;
    
    assert(PyObject_TypeCheck(self, &ECDSASignatureType) != 0);
    ssig = (ECDSASignature *)self;

    sstr = mpECDSASignature_export_str(ssig->sig);
    if (sstr == NULL) return NULL;
    str = PyUnicode_FromFormat("%s", sstr);
    free(sstr);

    return str;
}

static PyObject *ECDSASignature_binary(PyObject *self) {
    ECDSASignature *ssig;
    unsigned char *sbytes;
    size_t bsz;
    PyObject *bytes;
    
    assert(PyObject_TypeCheck(self, &ECDSASignatureType) != 0);
    ssig = (ECDSASignature *)self;

    sbytes = mpECDSASignature_export_bytes(ssig->sig, &bsz);
    if (sbytes == NULL) return NULL;
    bytes = PyBytes_FromStringAndSize((char *)sbytes, bsz);
    free(sbytes);

    return bytes;
}

static PyObject *ECDSASignature_repr(PyObject *self) {
    char *hashstr;
    PyObject *ssstr;
    ECDSASignature *ssig;

    assert(PyObject_TypeCheck(self, &ECDSASignatureType) != 0);
    ssig = (ECDSASignature *)self;

    ssstr = Py_TYPE((PyObject *)(ssig->ss))->tp_repr((PyObject *)(ssig->ss));
    hashstr = mpECDSASignature_export_str(ssig->sig);

    return PyUnicode_FromFormat("ECC.ECDSASignature(%S, \'%s\')", ssstr, hashstr);
}

// deallocates the object when done
static void ECDSASignature_dealloc(ECDSASignature *self) {
    // clear the internal element
    if (self->ready){
        mpECDSASignature_clear(self->sig);
        Py_DECREF((PyObject *)self->ss);
    }

    // free the object
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyDoc_STRVAR(ECDSASignature_verify__doc__, 
	"Sign(key, Message) -> ECDSASignature\n\n"
	"Sign a message using a secret key (scalar field value). Uses the curve "
	"and hash defined in the signature scheme.");

static PyObject *ECDSASignature_verify(PyObject *self, PyObject *args) {
    PyObject *key;
    PyObject *msg;
    ECDSASignature *ssis;
    ECPoint *kpt;
    PyObject *bytes;
    _hash_and_msg_t ham;
    int status;
    
    assert(PyObject_TypeCheck(self, &ECDSASignatureType) != 0);
    ssis = (ECDSASignature *)self;

	if (!PyArg_ParseTuple(args, "OO", &key, &msg)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECDSASignatureScheme_sign arguments");
		return NULL;
	}

    if(!PyObject_TypeCheck(key, &ECPointType)) {
		PyErr_SetString(PyExc_TypeError, "ECDSASignature_verify: 1st arg must be public key of ECPoint type");
		return NULL;
    }
    kpt = (ECPoint *)key;

    if (PyBytes_Check(msg)) {
        bytes = msg;
    } else {
        if (!PyUnicode_Check(msg)) {
    		PyErr_SetString(PyExc_TypeError, "ECDSASignature_verify: 2nd arg must be message of Bytes or Unicode type");
    		return NULL;
        }
        bytes = PyUnicode_AsUTF8String(msg);
        Py_INCREF(bytes);
    }
    
    ham.hashmodule = ssis->ss->hashmodule;
    ham.msg = bytes;

    status = mpECDSASignature_verify_cmp(ssis->sig, kpt->ecp, (unsigned char *)&ham, PyBytes_Size(bytes));

    Py_DECREF(bytes);
    if (status != 0) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    Py_INCREF(Py_True);
    return Py_True;
}

static PyMemberDef ECDSASignature_members[] = {
    {NULL}
};

static PyMethodDef ECDSASignature_methods[] = {
    //{"getvalue", (PyCFunction)ECPoint_getvalue, METH_NOARGS, "get value of element as an integer"},
	{"AsBytes", (PyCFunction)ECDSASignature_binary, METH_NOARGS, "dump raw (compressed) binary representation of the signature"},
    {"Verify", (PyCFunction)ECDSASignature_verify, METH_VARARGS, ECDSASignature_verify__doc__},
    {NULL}
};

PyTypeObject ECDSASignatureType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ECC.ECDSASignature",                     /*tp_name*/
    sizeof(ECDSASignature),                     /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    (destructor)ECDSASignature_dealloc,         /*tp_dealloc*/
    0,                                  /*tp_print*/
    0,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,			                        /*tp_reserved*/
    ECDSASignature_repr,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash */
    0,                                  /*tp_call*/
    ECDSASignature_str,                                  /*tp_str*/
    0,                                  /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    ECDSASignature__doc__,                      /* tp_doc */
    0,		                            /* tp_traverse */
    0,		                            /* tp_clear */
    0,		                            /* tp_richcompare */
    0,		                            /* tp_weaklistoffset */
    0,		                            /* tp_iter */
    0,		                            /* tp_iternext */
    ECDSASignature_methods,                     /* tp_methods */
    ECDSASignature_members,                     /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)ECDSASignature_init,              /* tp_init */
    0,                                  /* tp_alloc */
    ECDSASignature_new,                         /* tp_new */
};

PyDoc_STRVAR(ECDSASignatureScheme__doc__,
"ECDSASignatureScheme implements ECDSA Signature System using a chosen Elliptic "
"Curve and Hash Function.\n");

// allocate the object
static PyObject *ECDSASignatureScheme_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    // create the new Parameterss object
    ECDSASignatureScheme *self = (ECDSASignatureScheme *)type->tp_alloc(type, 0);
    // make sure it actually worked
    if (!self) {
        PyErr_SetString(PyExc_TypeError, "could not create ECDSASignatureScheme object.");
        return NULL;
    }
    self->ready = 0;

    // cast and return
    return (PyObject *)self;
}

// Usage options:
// ECDSASignatureScheme(curve=ECurve, hash=(hash which conforms to Crypto.Hash.hashalgo.HashAlgo API))
static int ECDSASignatureScheme_init(ECDSASignatureScheme *self, PyObject *args, PyObject *kwargs) {
    PyObject *curve;
    PyObject *hashmodule;
    PyObject *hashmodule_dict;
    PyObject *digest_size;
    mpECDSAHashfunc_t hf;
    long int hsz;
    int status;

    if (!PyArg_ParseTuple(args, "OO", &curve, &hashmodule)) {
        PyErr_SetString(PyExc_TypeError, "Error parsing ECDSASignatureScheme_init arguments");
        return -1;
    }

    if(!PyObject_TypeCheck((PyObject *)curve, &ECurveType)) {
        PyErr_SetString(PyExc_TypeError, "ECDSASignatureScheme_init: expected ECurve type as argument 1");
        return -1;
    }

    if (!PyModule_Check(hashmodule)) {
        PyErr_SetString(PyExc_TypeError, "Hash module must be a module");
        return -1;
    }

    // validate class variable for digest_size
    hashmodule_dict = PyModule_GetDict(hashmodule);
    if (!PyDict_Check(hashmodule_dict)) {
        PyErr_SetString(PyExc_TypeError, "GetDict for hash module did not return a Dictionary");
        return -1;
    }

    digest_size = PyDict_GetItemString(hashmodule_dict, "digest_size");

    if (!PyLong_Check(digest_size)) {
        PyErr_SetString(PyExc_TypeError, "Hash module digest_size is not an Integer(PyLong) type");
        return -1;
    }
    
    hsz = PyLong_AsLong(digest_size);
    if (hsz <= 0) {
        PyErr_SetString(PyExc_ValueError, "Hash module returned negative or zero digest_size");
        return -1;
    }

    // test hash module all the way through
    {
        PyObject *hnew;
        PyObject *tuple;
        PyObject *pyhash;
        PyObject *mbytes;
        PyObject *hbytes;
        Py_ssize_t bsz;

        // call hash.new(msg) to initialize and process message
        hnew = PyDict_GetItemString(hashmodule_dict, "new");
        if (!PyFunction_Check(hnew)) {
            PyErr_SetString(PyExc_TypeError, "Hash module.new() is not a function");
            return -1;
        }
    
        if (!PyCallable_Check(hnew)) {
            PyErr_SetString(PyExc_TypeError, "Hash module.new() function is not callable");
            return -1;
        }
        Py_INCREF(hnew);

        tuple = PyTuple_New(1);
        if (tuple == NULL) {
            PyErr_SetString(PyExc_TypeError, "Create Tuple Failed !?");
            return -1;
        }

        mbytes = PyBytes_FromStringAndSize("test", 4);
        if (mbytes == NULL) {
            PyErr_SetString(PyExc_TypeError, "Create Bytes from String Failed !?");
            return -1;
        }
        Py_INCREF(mbytes);

        status = PyTuple_SetItem(tuple, 0, mbytes);
        if (status != 0) {
            PyErr_SetString(PyExc_TypeError, "Tuple SetItem Failed !?");
            return -1;
        }

        pyhash = PyObject_CallObject(hnew, tuple);
        if (pyhash == NULL) {
            PyErr_SetString(PyExc_TypeError, "Call Hash returned NULL");
            return -1;
        }

        Py_DECREF(mbytes);
        Py_DECREF(hnew);
        
        Py_INCREF(pyhash);
        hbytes = PyObject_CallMethod(pyhash, "digest", NULL);
        if (!PyBytes_Check(hbytes)) {
            PyErr_SetString(PyExc_TypeError, "Hash object item digest() did not return bytes");
            return -1;
        }
        Py_DECREF(pyhash);
        
        bsz = PyBytes_Size(hbytes);
        if (bsz != hsz) {
            PyErr_SetString(PyExc_TypeError, "Hash bytes size mismatch !?");
            return -1;
        }
    }

    Py_INCREF(hashmodule);
    self->hashmodule = hashmodule;
    hf->dohash = _python_hash_wrapper;
    hf->hsz = hsz;

    Py_INCREF(curve);
    self->cv = (ECurve *)curve;

    status = mpECDSASignatureScheme_init(self->sscheme, self->cv->ec, hf);
    if (status != 0) {
        Py_DECREF(curve);
        Py_DECREF(hashmodule);
        return -1;
    }

    // you're ready!
    self->ready = 1;
    // all's clear
    return 0;
}

static PyObject *ECDSASignatureScheme_repr(PyObject *self) {
    PyObject *hashstr;
    PyObject *cvstr;
    ECDSASignatureScheme *ss;

    assert(PyObject_TypeCheck(self, &ECDSASignatureSchemeType) != 0);
    ss = (ECDSASignatureScheme *)self;

    cvstr = Py_TYPE((PyObject *)(ss->cv))->tp_repr((PyObject *)(ss->cv));
    hashstr = Py_TYPE((PyObject *)(ss->hashmodule))->tp_repr((PyObject *)(ss->hashmodule));

    return PyUnicode_FromFormat("ECC.ECDSASignatureScheme(%S, %S)", cvstr, hashstr);
}

// deallocates the object when done
static void ECDSASignatureScheme_dealloc(ECDSASignatureScheme *self) {
    // clear the internal element
    if (self->ready){
        mpECDSASignatureScheme_clear(self->sscheme);
        Py_DECREF((PyObject *)self->cv);
        Py_DECREF((PyObject *)self->hashmodule);
    }

    // free the object
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyDoc_STRVAR(ECDSASignatureScheme_sign__doc__, 
	"Sign(key, Message) -> ECDSASignature\n\n"
	"Sign a message using a secret key (scalar field value). Uses the curve "
	"and hash defined in the signature scheme.");

static PyObject *ECDSASignatureScheme_sign(PyObject *self, PyObject *args) {
    PyObject *key;
    PyObject *msg;
    ECDSASignatureScheme *ss;
    ECDSASignature *psig;
    FieldElement *kfe;
    PyObject *bytes;
    _hash_and_msg_t ham;
    int status;
    
    assert(PyObject_TypeCheck(self, &ECDSASignatureSchemeType) != 0);
    ss = (ECDSASignatureScheme *)self;

	if (!PyArg_ParseTuple(args, "OO", &key, &msg)) {
		PyErr_SetString(PyExc_TypeError, "Error parsing ECDSASignatureScheme_sign arguments");
		return NULL;
	}

    if(!PyObject_TypeCheck(key, &FieldElementType)) {
		PyErr_SetString(PyExc_TypeError, "ECDSASignatureScheme_sign: 1st arg must be secret key of FieldElement type");
		return NULL;
    }
    kfe = (FieldElement *)key;

    if (PyBytes_Check(msg)) {
        bytes = msg;
    } else {
        if (!PyUnicode_Check(msg)) {
    		PyErr_SetString(PyExc_TypeError, "ECDSASignatureScheme_sign: 2nd arg must be message of Bytes or Unicode type");
    		return NULL;
        }
        bytes = PyUnicode_AsUTF8String(msg);
        Py_INCREF(bytes);
    }

    psig = (ECDSASignature *)ECDSASignature_new(&ECDSASignatureType, NULL, NULL);
    
    ham.hashmodule = ss->hashmodule;
    ham.msg = bytes;

    status = mpECDSASignature_init_Sign(psig->sig, ss->sscheme, kfe->fe, (unsigned char *)&ham, PyBytes_Size(bytes));
    Py_DECREF(bytes);
    if (status != 0) {
        return NULL;
    }
    psig->ss = ss;
    Py_INCREF(ss);
    psig->ready = 1;

    return (PyObject *)psig;
}

static PyMemberDef ECDSASignatureScheme_members[] = {
    {NULL}
};

static PyMethodDef ECDSASignatureScheme_methods[] = {
    //{"getvalue", (PyCFunction)ECPoint_getvalue, METH_NOARGS, "get value of element as an integer"},
    {"Sign", (PyCFunction)ECDSASignatureScheme_sign, METH_VARARGS, ECDSASignatureScheme_sign__doc__},
    {NULL}
};

PyTypeObject ECDSASignatureSchemeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ECC.ECDSASignatureScheme",                     /*tp_name*/
    sizeof(ECDSASignatureScheme),                     /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    (destructor)ECDSASignatureScheme_dealloc,         /*tp_dealloc*/
    0,                                  /*tp_print*/
    0,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,			                        /*tp_reserved*/
    ECDSASignatureScheme_repr,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash */
    0,                                  /*tp_call*/
    0,                                  /*tp_str*/
    0,                                  /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    ECDSASignatureScheme__doc__,                      /* tp_doc */
    0,		                            /* tp_traverse */
    0,		                            /* tp_clear */
    0,		                            /* tp_richcompare */
    0,		                            /* tp_weaklistoffset */
    0,		                            /* tp_iter */
    0,		                            /* tp_iternext */
    ECDSASignatureScheme_methods,                     /* tp_methods */
    ECDSASignatureScheme_members,                     /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)ECDSASignatureScheme_init,              /* tp_init */
    0,                                  /* tp_alloc */
    ECDSASignatureScheme_new,                         /* tp_new */
};

