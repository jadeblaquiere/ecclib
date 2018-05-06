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
#include <b64/cencode.h>
#include <b64/cdecode.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_b64wrapped_to_file(FILE *fPtr, char* bindata, int sz, char *wrap) {
    char *enc;
    char *c;
    int i;
    base64_encodestate b64state;

    assert(bindata != NULL);
    assert(wrap != NULL);
    assert(fPtr != NULL);
    assert(sz > 0);

    // allocate buffer big enough for b64 encoding > 4/3 * sz
    enc = (char *)malloc((2 * sz) * sizeof(char));
    i = 0;
    c = enc;
    base64_init_encodestate(&b64state);
    i = base64_encode_block(bindata, sz, c, &b64state);
    c += i;
    i = base64_encode_blockend(c, &b64state);
    c += i;
    *c = 0;
    //printf("DER encoded privkey (%d bytes)=\n", sz);
    fprintf(fPtr, "-----BEGIN %s-----\n", wrap);
    fprintf(fPtr, "%s", enc);
    fprintf(fPtr, "-----END %s-----\n", wrap);
    free(enc);
    return 0;
}

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

char *read_b64wrapped_from_file(FILE *fPtr, char *wrap, int *sz) {
    char *filedata;
    char *bindata;
    char *b64data;
    char *begin_tag;
    char *end_tag;
    size_t read;
    size_t len;
    base64_decodestate b64state;
    int result;
    _readbuf_t *head;
    _readbuf_t *next;

    len = strlen(wrap) + strlen("BEGIN ") + 1;
    begin_tag = (char *)malloc(len * sizeof(char));
    sprintf(begin_tag, "BEGIN %s", wrap);

    len = strlen(wrap) + strlen("END ") + 1;
    end_tag = (char *)malloc(len * sizeof(char));
    sprintf(end_tag, "END %s", wrap);

    // read file into linked list of chunks
    head = (_readbuf_t *)malloc(sizeof(_readbuf_t));
    next = head;
    next->next = (_readbuf_t *)NULL;
    len = 0;
    
    while(true) {
        read = fread(next->buffer, sizeof(char), 16000, fPtr);
        len += read;
        next->sz = read;
        if (feof(fPtr)) {
            break;
        }
        next->next = (_readbuf_t *)malloc(sizeof(_readbuf_t));
        next = next->next;
        next->next = NULL;
    }
    if (len == 0) goto error_cleanup1;
    
    // concatenate into a single buffer
    filedata = (char *)malloc((len + 1) * sizeof(char));
    next = head;
    b64data = filedata;
    while (next != NULL) {
        bcopy(next->buffer, b64data, next->sz);
        b64data += next->sz;
        next = next->next;
    }
    filedata[len] = 0;

    // find BEGIN/END tags (and base64 data in the middle)
    b64data = strtok(filedata, "-----");
    if (b64data == NULL) goto error_cleanup2;

    while (strcmp(b64data, begin_tag) != 0) {
        b64data = strtok(NULL, "-----");
        if (b64data == NULL) goto error_cleanup2;
    }

    b64data = strtok(NULL, "-----");
    if (b64data == NULL) goto error_cleanup2;
    bindata = strtok(NULL, "-----");
    if (bindata == NULL) goto error_cleanup2;

    // expect end tag, error if not found;
    if (strcmp(bindata, end_tag) != 0) goto error_cleanup1;

    // over-allocate buffer, only need ~3/4 len
    len = strlen(b64data);
    bindata = (char *)malloc(len * sizeof(char));

    base64_init_decodestate(&b64state);
    result = base64_decode_block(b64data, len, bindata, &b64state);

    if (result == 0) {
        free(bindata);
        goto error_cleanup2;
    }

    free(filedata);
    _free_readbuf(head);
    free(end_tag);
    free(begin_tag);
    *sz = result;
    return bindata;

error_cleanup2:
    free(filedata);

error_cleanup1:
    _free_readbuf(head);
    free(end_tag);
    free(begin_tag);
    return (char *)NULL;
}
