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
#include <b64file.h>
#include <check.h>
#include <ecc.h>
#include <ecdh_der.h>
#include <limits.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>

char *default_curve = "secp256k1";

void print_supported_curves(FILE *fPtr) {
    char **clist;
    char **cnext;

    clist = _mpECurve_list_standard_curves();
    fprintf(fPtr, "Supported Elliptic Curves:\n");
    for (cnext = clist; *cnext != NULL; cnext++) {
        fprintf(fPtr, "    %s\n", *cnext);
    }
}

int main(int argc, char **argv) {
    char *curvename = NULL;
    int list_curves = 0;
    poptContext pc;
    struct poptOption po[] = {
        {"curve", 'c', POPT_ARG_STRING, &curvename, 0, "elliptic curve name, default = secp256k1", "<curve name>"},
        {"list-curves", 'l', POPT_ARG_NONE, &list_curves, 0, "list the available curve names and exit", "<>"},
        POPT_AUTOHELP
        {NULL}
    };
    mpECurve_t cv;
    mpz_t pmpz;
    char *der;
    size_t sz;
    int result;

    // attach gmp realloc/free functions to clear memory before free
    _enable_gmp_safe_clean();

    // pc is the context for all popt-related functions
    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    //poptSetOtherOptionHelp(pc, "[ARG...]");

    {
        // process options and handle each val returned
        int val;
        while ((val = poptGetNextOpt(pc)) >= 0) {
        //printf("poptGetNextOpt returned val %d\n", val);
        }
        if (val != -1) {
            fprintf(stderr,"<Error processing args>\n");
            poptPrintUsage(pc, stderr, 0);
            exit(1);
        }
    }

    if (list_curves != 0) {
        print_supported_curves(stdout);
        exit(0);
    }

    if (curvename == NULL) {
        curvename = default_curve;
    }

    // HERE IS WHERE THE ACTUAL EXAMPLE STARTS... everything before is
    // processing and very limited validation of command line options

    mpECurve_init(cv);
    if (mpECurve_set_named(cv, curvename) != 0) {
        fprintf(stderr,"<Error: Curve not found for name \"%s\">\n", curvename);
        print_supported_curves(stderr);
        exit(1);
    }

    // random value between 0 and cv->n (curve order)
    mpz_init(pmpz);
    mpz_urandom(pmpz, cv->n);

    // encode to ASN.1 DER format
    der = _ecdh_der_export_privkey(pmpz, cv, &sz);
    assert(der != NULL);

    result = write_b64wrapped_to_file(stdout, der, sz, "ECDH PRIVATE KEY");
    if (result != 0) {
        fprintf(stderr, "<WriteError>: Error writing output\n");
        exit(1);
    }

    free(der);
    mpz_clear(pmpz);
    mpECurve_clear(cv);

    return 0;
}
