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
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <ecc/mpzurandom.h>
#include <ecc/safememory.h>
#include <gmp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *test_prime_fields[] = { "65521", "131071", "4294967291", "8589934583", "18446744073709551557", "36893488147419103183",
    "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF",
    "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed",
    "0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef",
    "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"};

#define ARRAY_SZ    (2000)

static FILE *_f_urandom = NULL;

unsigned long int ui_urandom(unsigned long int max) {
    int sz_read;
    unsigned long int value;
    if (_f_urandom == NULL) {
        _f_urandom = fopen("/dev/urandom", "rb");
        assert(_f_urandom != NULL);
    }
    // bytes intentionally long to ensure uniformity, will truncate with modulo
    sz_read = fread(&value, sizeof(unsigned long int), 1, _f_urandom);
    assert(sz_read == 1);
    if (max > 0) {
        value = value % max;
    }
    return value;
}


int main(void) {
    int i, j;
    int ii;
    int nfields;
    // static as w/large values of ARRAY_SZ the stack exceeds ulimit allowance
    static mpFp_t a[ARRAY_SZ];
    static mpFp_t b[ARRAY_SZ];
    static mpz_t aaa[ARRAY_SZ];
    static mpz_t bbb[ARRAY_SZ];
    static mpz_t ccc[ARRAY_SZ];
    unsigned long int bui[ARRAY_SZ];
    mpFp_t c;
    mpFp_t cc;
    mpz_t aa, bb, d, e, p;
    //mpFp_field fp;
    int64_t start_time, stop_time;
    double fp_rate, mpz_rate;

    mpz_init(aa);
    mpz_init(bb);
    mpz_init(d);
    mpz_init(e);
    mpz_init(p);
    //mpFp_field_init(fp);

    // attach gmp realloc/free functions to clear memory before free
    _enable_gmp_safe_clean();

    nfields = sizeof(test_prime_fields)/sizeof(test_prime_fields[0]);

    for (j = 0 ; j < nfields; j++) {
        mpz_set_str(p,test_prime_fields[j], 0);

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_init(a[i], p);
            mpFp_init(b[i], p);
            mpz_init(aaa[i]);
            mpz_init(bbb[i]);
            mpz_init(ccc[i]);
        }
        mpFp_init(c, p);
        mpFp_init(cc, p);

        for (i = 0; i < ARRAY_SZ; i++) {
            //printf("i=%d\n",i);
            mpFp_urandom(a[i],p);
            mpFp_urandom(b[i],p);
            mpz_set_mpFp(aaa[i], a[i]);
            mpz_set_mpFp(bbb[i], b[i]);
            bui[i] = ui_urandom(0);
        }

        gmp_printf("Testing ADD for field 0x%ZX\n", p);

        // ADDITION
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_add(c, a[i], b[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_add(d, aa, bb);
            mpz_mod(d, d, p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_add_ui(c, a[i], bui[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_add_ui(d, aa, bui[i]);
            mpz_mod(d, d, p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        //gmp_printf("c[n] = 0x%ZX\n", aa);
        //gmp_printf("d[n] = 0x%ZX\n", d);

        mpz_add(d, c->fp->p, c->fp->pc);

        // corner cases for add ... carry conditions and add to -1, 0, 1 (mod p)

        // a+b = carry + 1, result = pc + 1
        mpz_set_mpFp(aa, a[0]);
        mpz_sub(bb, d, aa);
        mpz_add_ui(bb, bb, 1);
        mpFp_set_mpz(b[0], bb, p);

        // a+b = carry + 0, result = pc
        mpz_set_mpFp(aa, a[1]);
        mpz_sub(bb, d, aa);
        mpFp_set_mpz(b[1], bb, p);

        // a + b = carry - 1, result does not carry, but still > p
        mpz_set_mpFp(aa, a[2]);
        mpz_sub(bb, d, aa);
        mpz_sub_ui(bb, bb, 1);
        mpFp_set_mpz(b[2], bb, p);

        // a + b = p + 1, result = 1
        mpz_set_mpFp(aa, a[3]);
        mpz_sub(bb, p, aa);
        mpz_add_ui(bb, bb, 1);
        mpFp_set_mpz(b[3], bb, p);

        // a + b = p, result = 0
        mpz_set_mpFp(aa, a[4]);
        mpz_sub(bb, p, aa);
        mpFp_set_mpz(b[4], bb, p);

        // a + p = p - 1, result = p - 1
        mpz_set_mpFp(aa, a[5]);
        mpz_sub(bb, p, aa);
        mpz_sub_ui(bb, bb, 1);
        mpFp_set_mpz(b[5], bb, p);

        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        for (i = 0; i < 6; i++) {
            mpFp_add(c, a[i], b[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_add(d, aa, bb);
            mpz_mod(d, d, p);

            //printf("a[%d]: limbs = %d, limballoc = %d, limbsize = %lu\n", i, a[i]->i->_mp_size, a[i]->i->_mp_alloc, sizeof(a[i]->i->_mp_d[0]));
            //printf("b[%d]: limbs = %d, limballoc = %d, limbsize = %lu\n", i, b[i]->i->_mp_size, b[i]->i->_mp_alloc, sizeof(b[i]->i->_mp_d[0]));
            //printf("c: limbs = %d, limballoc = %d, limbsize = %lu\n", c->i->_mp_size, c->i->_mp_alloc, sizeof(c->i->_mp_d[0]));
            //printf("d: limbs = %d, limballoc = %d, limbsize = %lu\n", d->_mp_size, d->_mp_alloc, sizeof(d->_mp_d[0]));

            mpz_set_mpFp(aa, c);

            //gmp_printf("a[%d] = 0x%ZX\n", i, aa);
            //gmp_printf("b[%d] = 0x%ZX\n", i, bb);
            //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
            //gmp_printf("d[%d] = 0x%ZX\n", i, d);

            assert(mpz_cmp(d, aa) == 0);
        }
    
        mpFp_set(c, a[0]);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_add(c, c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_add(d, d, aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        printf("mpFp ADD rate = %g adds/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  ADD rate  = %g adds/sec\n", mpz_rate);
        assert(mpz_cmp(d, aa) == 0);
    
        mpFp_set_ui(c, bui[0], p);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_add_ui(c, c, bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_add_ui(d, d, bui[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        printf("mpFp ADD_UI rate = %g adds/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  ADD_UI rate  = %g adds/sec\n", mpz_rate);
        assert(mpz_cmp(d, aa) == 0);
    
        // SUBTRACTION
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sub(c, a[i], b[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_sub(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }
    
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sub_ui(c, a[i], bui[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_sub_ui(d, aa, bui[i]);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }
    
        // corner cases for sub ... borrow conditions -> -1, 0, 1 (mod p)
    
        // a-b = 1, result = 1
        mpz_set_mpFp(aa, a[0]);
        mpz_set(bb, aa);
        mpz_sub_ui(bb, bb, 1);
        mpFp_set_mpz(b[0], bb, p);
    
        // a-b = 0, result = 0
        mpz_set_mpFp(aa, a[1]);
        mpz_set(bb, aa);
        mpFp_set_mpz(b[1], bb, p);
    
        // a - b = - 1, result = p - (b - a) = p - 1
        mpz_set_mpFp(aa, a[2]);
        mpz_set(bb, aa);
        mpz_add_ui(bb, bb, 1);
        mpFp_set_mpz(b[2], bb, p);
    
        for (i = 0; i < 3; i++) {
            mpFp_sub(c, a[i], b[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_sub(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }
    
        mpFp_set(c, a[0]);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_sub(c, c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_sub(d, d, aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp SUB rate = %g subs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  SUB rate  = %g subs/sec\n", mpz_rate);

        mpFp_set_ui(c, bui[0], p);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_sub_ui(c, c, bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_sub_ui(d, d, bui[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp SUB_UI rate = %g subs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  SUB_UI rate  = %g subs/sec\n", mpz_rate);

        // Multiplication
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_mul(c, a[i], b[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, b[i]);
            mpz_mul(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_mul_ui(c, a[i], bui[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_mul_ui(d, aa, bui[i]);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        mpFp_set(c, a[0]);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_mul(c, c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mul(d, d, aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp MUL rate = %g muls/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  MUL rate  = %g muls/sec\n", mpz_rate);
        
        mpFp_set_ui(c, bui[0], p);
        mpz_set_mpFp(d, c);
        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpFp_mul_ui(c, c, bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mul_ui(d, d, bui[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp MUL_UI rate = %g muls/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  MUL_UI rate  = %g muls/sec\n", mpz_rate);
        
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mul(ccc[i], aaa[i], aaa[i-1]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 1; i < ARRAY_SZ; i++) {
            mpz_mod(ccc[i], ccc[i], p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        printf("mpz  MUL(noMOD) rate = %g ops/sec\n", fp_rate);
        printf("mpz  MOD(noMUL) rate  = %g ops/sec\n", mpz_rate);

        // Exponentiation
        for (i = 0; i < (ARRAY_SZ >> 4); i++) {
            mpFp_pow_ui(c, a[i], bui[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_powm_ui(d, aa, bui[i], p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);

            mpFp_pow_mpz(c, a[i], bbb[i]);

            mpz_set_mpFp(aa, a[i]);
            mpz_powm(d, aa, bbb[i], p);

            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);
        }

        start_time = clock();
        for (i = 0; i < (ARRAY_SZ >> 4); i++) {
            mpFp_pow_ui(c, a[i], bui[i]);
        }
        stop_time = clock();
        fp_rate = ((double)(ARRAY_SZ >> 4) * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 0; i < (ARRAY_SZ >> 4); i++) {
            mpz_powm_ui(d, aaa[i], bui[i], p);
        }
        stop_time = clock();
        mpz_rate = ((double)(ARRAY_SZ >> 4) * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp POWM rate = %g exps/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  POWM rate = %g exps/sec\n", mpz_rate);
        
        // Squaring
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sqr(c, a[i]);
    
            mpz_set_mpFp(aa, a[i]);
            mpz_set_mpFp(bb, a[i]);
            mpz_mul(d, aa, bb);
            mpz_mod(d, d, p);
    
            mpz_set_mpFp(aa, c);
            assert(mpz_cmp(d, aa) == 0);

            mpz_powm_ui(d, bb, 2, p);
            assert(mpz_cmp(d, aa) == 0);
        }

        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sqr(c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpz_mul(d, aaa[i], aaa[i]);
            mpz_mod(d, d, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp SQR rate = %g sqrs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  SQR rate = %g sqrs/sec\n", mpz_rate);
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpz_powm_ui(d, aaa[i], 2, p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        printf("mpz  SQR rate = %g sqrs/sec (using powm_ui)\n", mpz_rate);
        
        // Inverse
        for (i = 0; i < ARRAY_SZ; i++) {
            int status, rstatus;
            status = mpFp_inv(c, a[i]);
    
            mpz_set_mpFp(aa, a[i]);
            rstatus = mpz_invert(d, aa, p);

            assert(rstatus != status);
            if (status == 0) {
                mpz_set_mpFp(aa, c);
                assert(mpz_cmp(d, aa) == 0);
            }
        }

        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_inv(c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpz_invert(d, aaa[i], p);
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        mpz_set_mpFp(aa, c);
        //gmp_printf("c[%d] = 0x%ZX\n", i, aa);
        //gmp_printf("d[%d] = 0x%ZX\n", i, d);
        assert(mpz_cmp(d, aa) == 0);
        printf("mpFp INV rate = %g sqrs/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  INV rate = %g sqrs/sec\n", mpz_rate);

        // Conditional Swap
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpFp_set(c, a[i]);
            mpFp_cswap(a[i], b[i], ii);
            if (ii == 0) {
                assert(mpFp_cmp(c, a[i]) == 0);
            } else {
                assert(mpFp_cmp(c, b[i]) == 0);
            }
            mpFp_cswap(a[i], b[i], ii);
        }

        //carry = mpn_add_n(c->_mp_d, a->_mp_d, b->_mp_d, psize);
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpFp_cswap(a[i], b[i], ii);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpFp_cswap(a[i], b[i], ii);
        }
        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpz_set(d, aaa[i]);
            mpz_set(e, ccc[i]);
            if (ii == 0) {
                mpz_set(aaa[i], d);
                mpz_set(ccc[i], e);
            } else {
                mpz_set(aaa[i], e);
                mpz_set(ccc[i], d);
            }
        }
        stop_time = clock();
        mpz_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        for (i = 0; i < ARRAY_SZ; i++) {
            ii = i % 2;
            mpz_set(d, aaa[i]);
            mpz_set(e, ccc[i]);
            if (ii == 0) {
                mpz_set(aaa[i], d);
                mpz_set(ccc[i], e);
            } else {
                mpz_set(aaa[i], e);
                mpz_set(ccc[i], d);
            }
        }
        printf("mpFp CSWAP rate = %g swaps/sec (%g X)\n", fp_rate, (fp_rate / mpz_rate));
        printf("mpz  CSWAP rate = %g swaps/sec\n", mpz_rate);
        
        // Squaring first, then sqrt
        for (i = 0; i < ARRAY_SZ; i++) {
            int status;

            status = mpFp_sqrt(c, a[i]);
            
            if (status == 0) {
                mpFp_sqr(cc, c);
                assert(mpFp_cmp(cc, a[i]) == 0);
                mpFp_neg(cc, c);
                mpFp_sqr(cc, cc);
                assert(mpFp_cmp(cc, a[i]) == 0);
            }
        }

        start_time = clock();
        for (i = 0; i < ARRAY_SZ; i++) {
            mpFp_sqrt(c, a[i]);
        }
        stop_time = clock();
        fp_rate = ((double)ARRAY_SZ * CLOCKS_PER_SEC)/((double)(stop_time - start_time));
        printf("mpFp SQRT rate = %g sqrts/sec\n", fp_rate);

        mpFp_clear(cc);
        mpFp_clear(c);
        for (i = ARRAY_SZ-1; i >= 0; i--) {
            mpz_clear(ccc[i]);
            mpz_clear(bbb[i]);
            mpz_clear(aaa[i]);
            mpFp_clear(b[i]);
            mpFp_clear(a[i]);
        }
    }

    //mpFp_field_clear(fp);
    //mpz_clear(pc);
    mpz_clear(p);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(bb);
    mpz_clear(aa);
    
    return 0;
}
