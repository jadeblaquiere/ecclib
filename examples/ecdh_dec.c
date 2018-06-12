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
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char *filename = NULL;
    char *keyfile = NULL;
    FILE *fPtr = stdin;
    FILE *kPtr = NULL;
    poptContext pc;
    struct poptOption po[] = {
        {"privkey", 'k', POPT_ARG_STRING, &keyfile, 0, "recipient private key file", "<file path>"},
        {"file", 'f', POPT_ARG_STRING, &filename, 0, "read input from filepath instead of stdin", "<file path>"},
        POPT_AUTOHELP
        {NULL}
    };
    mpECurve_t cv;
    mpz_t pmpz;
    mpECP_t Gpt;
    mpECP_t Ppt;
    mpECP_t pQpt;
    char *der;
    size_t sz;
    int result;

    unsigned char *mtxt;
    int msz;
    unsigned char *ntxt;
    int nsz;
    unsigned char *ctxt;
    int csz;
    unsigned char *ptxt;
    int psz;

    unsigned char shared_hash[crypto_stream_KEYBYTES];
    unsigned char nonce[crypto_stream_NONCEBYTES];

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

    if (keyfile == NULL) {
        fprintf(stderr,"<Error>: to_key is a required parameter\n");
        exit(1);
    }

    kPtr = fopen(keyfile, "r");
    if (kPtr == NULL) {
        fprintf(stderr,"<Error>: file open failed for file \"%s\"\n", keyfile);
        exit(1);
    }

    if (filename != NULL) {
        fPtr = fopen(filename, "r");
        if (fPtr == NULL) {
            fprintf(stderr,"<Error>: file open failed for file \"%s\"\n", filename);
            exit(1);
        }
    }

    // HERE IS WHERE THE ACTUAL EXAMPLE STARTS... everything before is
    // processing and very limited validation of command line options

    der = read_b64wrapped_from_file(kPtr, "ECDH PRIVATE KEY", &sz);
    if (der == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 data for private key\n");
        exit(1);
    }
 
    mpz_init(pmpz);
    mpECurve_init(cv);
    if (_ecdh_der_import_privkey(pmpz, cv, der, sz) != 0) {
        fprintf(stderr,"<Error>: Unable to import private key\n");
        exit(1);
    }
    
    free(der);
    der = read_b64wrapped_from_file(fPtr, "ECDHE_XSALSA20 ENCRYPTED MESSAGE", &sz);
    if (der == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 data for message\n");
        exit(1);
    }
    
    result = _ecdhe_der_init_import_message(&ptxt, &psz, &ntxt, &nsz, &ctxt, &csz, der, sz);
    if (result != 0) {
        fprintf(stderr,"<Error>: Unable to import message as DER data\n");
        exit(1);
    }
    
    // instantiate generator point for curve
    mpECP_init(Gpt, cv);
    mpECP_set_mpz(Gpt, cv->G[0], cv->G[1], cv);

    // public (ephemeral key point = private key scalar * G
    mpECP_init(Ppt, cv);
    mpECP_set_bytes(Ppt, ptxt, psz, cv);

    // shared (ECDH) key using 
    mpECP_init(pQpt, cv);
    mpECP_scalar_mul_mpz(pQpt, Ppt, pmpz);

    assert(sizeof(nonce) == 24);
    assert(sizeof(shared_hash) == 32);

    // hash shared key to get a 256-bit key for encryption w/ChaCha
    {
        int len = 0;
        unsigned char *k_bytes;
        len = mpECP_out_bytelen(pQpt, 1);
        k_bytes = (unsigned char *)malloc(len * sizeof(unsigned char));
        mpECP_out_bytes(k_bytes, pQpt, 1);

        crypto_hash_sha256(shared_hash, k_bytes, len);
        free (k_bytes);
    }

    // decrypt message (xor with cipher stream)
    // int crypto_stream_xchacha20_xor(unsigned char *c, const unsigned char *m,eseer *n,
    //                            const unsigned char *k);
    mtxt = (unsigned char *)malloc(csz*sizeof(unsigned char));
    msz = csz;
    // crypto_stream_xor is XSalsa20 with a 192-bit nonce and 64-bit counter
    crypto_stream_xor(mtxt, ctxt, csz, ntxt, shared_hash);

    // write decrypted message to stdout
    fwrite(mtxt, sizeof(unsigned char), msz, stdout);

    mpECP_clear(pQpt);
    mpECP_clear(Ppt);
    mpECP_clear(Gpt);
    mpz_clear(pmpz);
    mpECurve_clear(cv);

    return 0;
}
