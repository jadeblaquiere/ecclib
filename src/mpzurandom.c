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

#include <assert.h>
#include <gmp.h>
#include <mpzurandom.h>
#include <stdio.h>
#include <stdlib.h>

static FILE *_f_urandom = NULL;

void mpz_urandom(mpz_t rop, mpz_t rand_max) {
    int bytes, sz_read;
    char *buffer;
    if (_f_urandom == NULL) {
        _f_urandom = fopen("/dev/urandom", "rb");
        assert(_f_urandom != NULL);
    }
    // bytes intentionally long to ensure uniformity, will truncate with modulo
    bytes = ((mpz_sizeinbase(rand_max,2) + 7) >> 3) * 2;
    buffer = (char *)malloc((bytes) * sizeof(char));
    sz_read = fread(buffer, sizeof(char), bytes, _f_urandom);
    assert(sz_read == bytes);
    mpz_import(rop, sz_read, 1, sizeof(char), 0, 0, buffer);
    mpz_mod(rop, rop, rand_max);
    free(buffer);
    return;
}
