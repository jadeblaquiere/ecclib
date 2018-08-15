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
#include <elgamal_der.h>
#include <limits.h>
#include <popt.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    char *filename = NULL;
    char *keyfile = NULL;
    FILE *fPtr = stdin;
    FILE *kPtr = NULL;
    poptContext pc;
    struct poptOption po[] = {
        {"pubkey", 'k', POPT_ARG_STRING, &keyfile, 0, "recipient public key file", "<file path>"},
        {"file", 'f', POPT_ARG_STRING, &filename, 0, "read input (public key file) from filepath instead of stdin", "<file path>"},
        POPT_AUTOHELP
        {NULL}
    };
    mpECurve_t cv1;
    mpECurve_t cv2;
    mpECP_t pK;
    mpECP_t ptxt;
    mpECElgamalCiphertext_t ctxt;
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
    poptFreeContext(pc);

    if (keyfile == NULL) {
        fprintf(stderr,"<Error>: to_key is a required parameter\n");
        exit(1);
    }

    kPtr = fopen(keyfile, "r");
    if (kPtr == NULL) {
        fprintf(stderr,"<Error>: file open failed for file \"%s\"\n", keyfile);
        exit(1);
    }
    free(keyfile);

    if (filename != NULL) {
        fPtr = fopen(filename, "r");
        if (fPtr == NULL) {
            fprintf(stderr,"<Error>: file open failed for file \"%s\"\n", filename);
            exit(1);
        }
        free(filename);
    }

    // HERE IS WHERE THE ACTUAL EXAMPLE STARTS... everything before is
    // processing and very limited validation of command line options

    der = read_b64wrapped_from_file(kPtr, "ECDH PUBLIC KEY", &sz);
    if (der == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 key data\n");
        exit(1);
    }
    fclose(kPtr);
 
    if (_ecdh_der_init_import_pubkey(pK, cv1, der, sz) != 0) {
        fprintf(stderr,"<Error>: Unable to import public key\n");
        exit(1);
    }
    free(der);
    
    der = read_b64wrapped_from_file(fPtr, "ECDH PUBLIC KEY", &sz);
    if (der == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 plaintext key data\n");
        exit(1);
    }
    fclose(fPtr);
 
    if (_ecdh_der_init_import_pubkey(ptxt, cv2, der, sz) != 0) {
        fprintf(stderr,"<Error>: Unable to import plaintext (public key)\n");
        exit(1);
    }
    free(der);

    if (mpECurve_cmp(cv1, cv2) != 0) {
        fprintf(stderr,"<Error>: public key point and plaintext point must be on the same curve\n");
        exit(1);
    }
    
    result = mpECElgamal_init_encrypt(ctxt, pK, ptxt);
    if (result != 0) {
        fprintf(stderr,"<Error>: encryption failed\n");
        exit(1);
    }

    // encode to ASN.1 DER format
    der = _ecelgamal_der_export_ciphertxt(ctxt, &sz);
    assert(der != NULL);

    result = write_b64wrapped_to_file(stdout, der, sz, "ECELGAMAL ENCRYPTED MESSAGE");
    if (result != 0) {
        fprintf(stderr, "<WriteError>: Error writing output\n");
        exit(1);
    }

    free(der);
    mpECElgamal_clear(ctxt);
    mpECP_clear(ptxt);
    mpECP_clear(pK);
    mpECurve_clear(cv2);
    mpECurve_clear(cv1);

    return 0;
}
