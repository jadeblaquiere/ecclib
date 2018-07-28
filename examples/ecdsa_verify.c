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
#include <string.h>

void wrap_libsodium_sha512(unsigned char *hash, unsigned char *msg, size_t sz) {
    int status;
    status = crypto_hash_sha512(hash, msg, (unsigned long long)sz);
    assert(status == 0);
    return;
}

int main(int argc, char **argv) {
    char *filename = NULL;
    char *keyfile = NULL;
    FILE *fPtr = stdin;
    FILE *kPtr = NULL;
    poptContext pc;
    struct poptOption po[] = {
        {"pubkey", 'k', POPT_ARG_STRING, &keyfile, 0, "signer public key file", "<file path>"},
        {"file", 'f', POPT_ARG_STRING, &filename, 0, "read input from filepath instead of stdin", "<file path>"},
        POPT_AUTOHELP
        {NULL}
    };
    mpECurve_t cv;
    mpECP_t pK;
    mpECDSAHashfunc_t H;
    mpECDSASignatureScheme_t ss;
    mpECDSASignature_t sig;
    char *der;
    char *dercp;
    size_t sz;
    int result;

    unsigned char *msg;
    size_t msglen;
    unsigned char *sigbytes;
    size_t siglen;

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
        fprintf(stderr,"<Error>: signer_key is a required parameter\n");
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
    }

    // HERE IS WHERE THE ACTUAL EXAMPLE STARTS... everything before is
    // processing and very limited validation of command line options
    
    der = read_buffer_from_file(fPtr, &sz);
    if (der == NULL) {
        fprintf(stderr,"<Error>: unable to read input file\n");
        exit(1);
    }
    fclose(fPtr);
    
    dercp = malloc(sz*sizeof(char));
    memcpy(dercp, der, sz);

    sigbytes = (unsigned char *)read_b64wrapped_from_buffer(der, "ECDSA SIGNATURE", &siglen);
    if (sigbytes == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 signature data\n");
        exit(1);
    }
 
    msg = (unsigned char *)read_b64wrapped_from_buffer(dercp, "ECDSA SIGNED MESSAGE", &msglen);
    if (msg == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 message data\n");
        exit(1);
    }
    free(dercp);
    free(der);

    der = read_b64wrapped_from_file(kPtr, "ECDH PUBLIC KEY", &sz);
    if (der == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 data\n");
        exit(1);
    }
    fclose(kPtr);
 
    if (_ecdh_der_init_import_pubkey(pK, cv, der, sz) != 0) {
        fprintf(stderr,"<Error>: Unable to import private key\n");
        exit(1);
    }
    
    //memset(der, 0, sz);
    free(der);

    mpECDSAHashfunc_init(H);
    H->dohash = wrap_libsodium_sha512;
    H->hsz = crypto_hash_sha512_BYTES;

    result = mpECDSASignatureScheme_init(ss, cv, H);
    if (result != 0) {
        fprintf(stderr,"<Error>: Initialization of Signature Scheme failed\n");
        exit(1);
    }

    result = mpECDSASignature_init_import_bytes(sig, ss, sigbytes, siglen);
    if (result != 0) {
        fprintf(stderr,"<Error>: Signature Corrupt\n");
        exit(1);
    }

    result = mpECDSASignature_verify_cmp(sig, pK, msg, msglen);
    if (result != 0) {
        fprintf(stderr, "<Error>: Signature Not Valid for Message\n");
        exit(1);
    }
    
    fwrite(msg, sizeof(unsigned char), msglen, stdout);

    free(sigbytes);
    free(msg);
    mpECDSASignature_clear(sig);
    mpECDSASignatureScheme_clear(ss);
    mpECDSAHashfunc_clear(H);
    mpECP_clear(pK);
    mpECurve_clear(cv);

    return 0;
}
