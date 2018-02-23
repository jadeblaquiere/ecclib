//BSD 3-Clause License
//
//Copyright (c) 2017, jadeblaquiere
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
#include <relic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BENCH_SZ    (50)

int main(int argc, char** argv) {
    int i, j, k;
    fp_t n[BENCH_SZ];
    bn_t nb[BENCH_SZ];
    ep_t pt[BENCH_SZ];
    ec_t rpt;
    double cpu_time;
    double mul_rate;
    int64_t start_time, stop_time;

    //fp_prime_init();
    //ep_curve_init();
    if (core_init() != STS_OK) {
		core_clean();
		return 1;
    }
    
    conf_print();
    
    if (ec_param_set_any() != STS_OK) {
		core_clean();
		return 1;
	}
	
	ec_param_print();

    //printf("\"curve\", \"num_iter\", \"time\", \"rate\",\n");
    printf("init called\n");

    for (i = SECG_P160; i <= SS_P1536; i++) {
    //    fp_param_set(i);
    //printf("fp_param_set called\n");
        TRY {
            ep_param_set(i);
        } CATCH_ANY {
    		continue;
    	}
        printf("ep_param_set called\n");
        ep_param_print();
        for (j = 0; j < BENCH_SZ; j++) {
            ep_rand(pt[j]);
            fp_rand(n[j]);
            fp_prime_back(nb[j],n[j]);
        }

        start_time = clock();
        for (j = 0; j < BENCH_SZ; j++) {
            for (k = 0; k < BENCH_SZ; k++) {
                ep_mul(rpt, pt[j], nb[k]);
            }
        }
        stop_time = clock();

        cpu_time = (double)(stop_time - start_time) / ((double)CLOCKS_PER_SEC);  
        mul_rate = (double)(BENCH_SZ * BENCH_SZ) / cpu_time;
        printf("\"curve\", %d, %lf, %lf,\n", (int)(BENCH_SZ*BENCH_SZ),cpu_time, mul_rate);

        i += 1;
    }
    return 0;
}
