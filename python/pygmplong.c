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

#include <gmp.h>
#include <pygmplong.h>
#include <Python.h>

#define _MPFP_MAX_LIMBS   (32)

int _pylong_to_mpz(PyLongObject *pobj, mpz_t pmpz) {
	mp_limb_t pdata[_MPFP_MAX_LIMBS];
	size_t psz;
	int status;
	int negative;

	if (pobj == NULL) {
    	return -1;
	}

	if (!PyLong_Check(pobj)) {
		return -1;
	}

	// calculate the byte length of p
	psz = _PyLong_NumBits((PyObject*)pobj);
	//printf("psz = %d\n", psz);
	psz >>= 3;

	if (psz > sizeof(pdata)) {
		return -1;
	}
	
	// convert to bytes, initial 1 implies little endian, final 1 implies signed
	status = _PyLong_AsByteArray(pobj, (unsigned char *)pdata, psz+1, 1, 1);

	//{
	//	int i;
	//	printf("p= 0x");
	//	for (i = 0; i <= psz; i++) {
	//		printf("%02X", ((unsigned char *)pdata)[i]);
	//	}
	//	printf("\n");
	//}

	if (status != 0) {
		return -1;
	}

	// mpz_import is inherently unsigned... detect sign
	negative = (((unsigned char *)pdata)[psz] & 0x80) != 0;
	
	// take 1's complement if negative
	if (negative) {
		int i;
		for (i = 0; i <= psz; i++) {
			((unsigned char *)pdata)[i] ^= 0xFF;
		}
	}

	// convert bytes to mpz, -1 implies little endian (wordwise and bytewise), 0 implies no "nails"
	mpz_import(pmpz, psz+1, -1, sizeof(unsigned char), -1, 0, (void *)pdata);

	// convert to 2's complement and negate 	
	if (negative) {
		mpz_add_ui(pmpz, pmpz, 1);
		mpz_neg(pmpz, pmpz);
	}
	
	return 0;
}

int _pylong_to_mpz_unsigned(PyLongObject *pobj, mpz_t pmpz) {
	mp_limb_t pdata[_MPFP_MAX_LIMBS];
	size_t psz;
	int status;

	if (pobj == NULL) {
    	return -1;
	}

	if (!PyLong_Check(pobj)) {
		return -1;
	}

	// calculate the byte length of p
	psz = _PyLong_NumBits((PyObject*)pobj);
	psz >>= 3;

	if (psz > sizeof(pdata)) {
		return -1;
	}

	// convert to bytes, 1 implies little endian, 0 implies unsigned
	status = _PyLong_AsByteArray(pobj, (unsigned char *)pdata, psz+1, 1, 0);

	if (status != 0) {
		return -1;
	}

	// convert bytes to mpz, -1 implies little endian (wordwise and bytewise), 0 implies no "nails"
	mpz_import(pmpz, psz+1, -1, sizeof(unsigned char), -1, 0, (void *)pdata);
	
	return 0;
}

PyObject *_mpz_to_pylong(mpz_t pmpz) {
	mp_limb_t pdata[_MPFP_MAX_LIMBS];
	size_t countp;

	countp = sizeof(pdata);

	mpz_export((void *)pdata, &countp, -1, sizeof(unsigned char), -1, 0, pmpz);

	// convert from bytes, 1 implies little endian, 0 implies unsigned
	// note, the mpz zero repr (no data written to pdata and countp=0 is correctly handled)
	return _PyLong_FromByteArray((unsigned char*)pdata, countp, 1, (pmpz->_mp_size < 0));
}
