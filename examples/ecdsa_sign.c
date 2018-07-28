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

typedef struct _readbuf {
    char    buffer[16000];
    struct _readbuf *next;
    int     sz;
} _readbuf_t;

static void _free_readbuf(_readbuf_t *next) {
    if (next->next != NULL) _free_readbuf(next->next);
    free(next);
    return;
}

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
        {"privkey", 'k', POPT_ARG_STRING, &keyfile, 0, "signer private key file", "<file path>"},
        {"file", 'f', POPT_ARG_STRING, &filename, 0, "read input from filepath instead of stdin", "<file path>"},
        {"file", 'f', POPT_ARG_STRING, &filename, 0, "read input from filepath instead of stdin", "<file path>"},
        POPT_AUTOHELP
        {NULL}
    };
    mpECurve_t cv;
    mpz_t pmpz;
    mpFp_t sK;
    mpECDSAHashfunc_t H;
    mpECDSASignatureScheme_t ss;
    mpECDSASignature_t sig;
    char *der;
    size_t sz;
    int result;

    unsigned char *msg;
    size_t msglen;

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
        free(filename);
    }

    // read entire plaintext input into memory
    {
        size_t len, rlen;
        _readbuf_t *head;
        _readbuf_t *next;
        unsigned char *buf;

        // read file into linked list of chunks
        head = (_readbuf_t *)malloc(sizeof(_readbuf_t));
        next = head;
        next->next = (_readbuf_t *)NULL;
        len = 0;

        while(1) {
            rlen = fread(next->buffer, sizeof(char), 16000, fPtr);
            len += rlen;
            next->sz = rlen;
            if (feof(fPtr)) {
                break;
            }
            next->next = (_readbuf_t *)malloc(sizeof(_readbuf_t));
            next = next->next;
            next->next = NULL;
        }
        if (len == 0) {
            fprintf(stderr,"<Error>: plaintext input zero length");
            exit(1);
        }

        // concatenate chunks into a single buffer
        msg = (unsigned char *)malloc((len + 1) * sizeof(char));
        next = head;
        buf = msg;
        while (next != NULL) {
            bcopy(next->buffer, buf, next->sz);
            buf += next->sz;
            next = next->next;
        }
        msg[len] = 0;
        msglen = len;
        _free_readbuf(head);
    }
    fclose(fPtr);

    // HERE IS WHERE THE ACTUAL EXAMPLE STARTS... everything before is
    // processing and very limited validation of command line options

    der = read_b64wrapped_from_file(kPtr, "ECDH PRIVATE KEY", &sz);
    if (der == NULL) {
        fprintf(stderr,"<ParseError>: unable to decode b64 data\n");
        exit(1);
    }
    fclose(kPtr);
 
    mpz_init(pmpz);
    mpECurve_init(cv);
    if (_ecdh_der_import_privkey(pmpz, cv, der, sz) != 0) {
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

    mpFp_init(sK, cv->n);
    mpFp_set_mpz(sK, pmpz, cv->n);
    result = mpECDSASignature_init_Sign(sig, ss, sK, msg, msglen);

    result = write_b64wrapped_to_file(stdout, (char *)msg, msglen, "ECDSA SIGNED MESSAGE");
    if (result != 0) {
        fprintf(stderr, "<WriteError>: Error writing output\n");
        exit(1);
    }

    der = (char *)mpECDSASignature_export_bytes(sig, &sz);

    result = write_b64wrapped_to_file(stdout, der, sz, "ECDSA SIGNATURE");
    if (result != 0) {
        fprintf(stderr, "<WriteError>: Error writing output\n");
        exit(1);
    }

    free(msg);
    free(der);
    mpECDSASignature_clear(sig);
    mpECDSASignatureScheme_clear(ss);
    mpECDSAHashfunc_clear(H);
    mpFp_clear(sK);
    mpz_clear(pmpz);
    mpECurve_clear(cv);

    return 0;
}
