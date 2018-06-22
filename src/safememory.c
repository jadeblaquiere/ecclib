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

#include <ecc/safememory.h>
#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void *_safe_clean_realloc(void *old, size_t oldsz, size_t newsz) {
    void *ret;

    if (__GMP_LIKELY(newsz <= oldsz)) {
        return old;
    }

    ret = malloc(newsz);
    if (__GMP_UNLIKELY(ret == NULL)) {
        fprintf (stderr, "ECCLIB: unable to allocate memory, aborting\n");
        exit(-1);
    }

    memcpy(ret, old, oldsz);
    memset(old, 0, oldsz);

    return ret;
}

void _safe_clean_free(void *old, size_t oldsz) {
    memset(old, 0, oldsz);
    free (old);
}

void _enable_gmp_safe_clean() {
    mp_set_memory_functions(NULL, _safe_clean_realloc, _safe_clean_free);
}
