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
#include <ecdh_der.h>
#include <portable_endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const asn1_static_node ecdhe_asn1_tab[];

static int myasn1_write_mpz_value(asn1_node node_root, const char *name, mpz_t n, int *sz) {
    char *buf;
    size_t blen = 0;
    int status;

    // write big-endian by word and byte (1, 1), 0 nails
    buf = mpz_export(NULL, &blen, 1, sizeof(char), 1, 0, n);
    if (blen < 1) {
        blen = 1;
        buf = (char *)malloc(1 * sizeof(char));
        buf[0] = 0;
    }

    status = asn1_write_value(node_root, name, buf, blen);
    free(buf);
    *sz += blen + 5;
    return status;
}

static int myasn1_read_octet_string_to_bytes(ASN1_TYPE root, const char *name, unsigned char **buf, int *sz) {
    unsigned char *buffer;
    int result;
    int len_test;
    int len_read;

    len_test = 0;
    result = asn1_read_value(root, name, NULL, &len_test);
    if (result != ASN1_MEM_ERROR) return -1;
    assert(len_test > 0);
    buffer = (unsigned char *)malloc((len_test + 1)*sizeof(char));

    len_read = len_test + 1;
    result = asn1_read_value(root, name, buffer, &len_read);
    assert(len_read == len_test);

    *sz = len_read;
    *buf = buffer;
    return 0;
}

static int myasn1_read_octet_string_to_mpz(ASN1_TYPE root, const char *name, mpz_t n) {
    unsigned char *buffer;
    int sz;

    if (myasn1_read_octet_string_to_bytes(root, name, &buffer, &sz) != 0) {
        free(buffer);
        return -1;
    }
    // read big endian word and byte (1, 1), 0 nails
    mpz_import(n, sz, 1, sizeof(unsigned char), 1, 0, buffer);
    free(buffer);
    return 0;
}

static int myasn1_read_integer_to_int(asn1_node root, const char *name, int *value) {
    int result, length, lread;
    uint32_t uvalue = 0;
    //char *buffer;

    assert(sizeof(uint32_t) == 4);
    // call read_value with NULL buffer to get length
    length = 0;
    result = asn1_read_value(root, name, NULL, &length);
    if (result != ASN1_MEM_ERROR) return -1;
    assert(length > 0);
    assert(length <= sizeof(int));
    lread = sizeof(int);
    result = asn1_read_value(root, name, &uvalue, &lread);
    assert(result == ASN1_SUCCESS);
    assert(lread == length);
    *value = (int)be32toh(uvalue);
    //printf("value = 0x%08X", *value);
    if (length < sizeof(int)) {
        *value >>= ((sizeof(int) - length) * 8) ;
    }
    //printf("adjusted value = %d\n", *value);
    return 0;
}

char *_ecdh_der_export_privkey(mpz_t privkey, mpECurve_t cv, size_t *sz) {
    ASN1_TYPE ecdhe_asn1 = ASN1_TYPE_EMPTY;
    ASN1_TYPE privkey_asn1 = ASN1_TYPE_EMPTY;
    char asnError[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
    char *der;
    mpz_t tmpz;
    int bsz = 0;
    int bufsz;
    int result;
    
    mpz_init(tmpz);

    // read ASN1 syntax
    result = asn1_array2tree(ecdhe_asn1_tab, &ecdhe_asn1, asnError);

    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
        return NULL;
    }

    // create an empty ASN1 message structure
    result = asn1_create_element(ecdhe_asn1, "ExampleASN.ECDHPrivateKey",
        &privkey_asn1);
    assert(result == 0);

    //printf("-----------------\n");
    //asn1_print_structure(stdout, privkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // private key value
    result = myasn1_write_mpz_value(privkey_asn1, "privkey", privkey, &bsz);
    assert(result == 0);

    // field order
    result = myasn1_write_mpz_value(privkey_asn1, "curve.p", cv->fp->p, &bsz);
    assert(result == 0);

    // curve order
    result = myasn1_write_mpz_value(privkey_asn1, "curve.n", cv->n, &bsz);
    assert(result == 0);

    // curve cofactor
    result = myasn1_write_mpz_value(privkey_asn1, "curve.h", cv->h, &bsz);
    assert(result == 0);

    // curve generator
    result = myasn1_write_mpz_value(privkey_asn1, "curve.g.x", cv->G[0], &bsz);
    assert(result == 0);
    result = myasn1_write_mpz_value(privkey_asn1, "curve.g.y", cv->G[1], &bsz);
    assert(result == 0);

    // curve type
    {
        unsigned char ctype;
        ctype = (unsigned char)cv->type;
        result = asn1_write_value(privkey_asn1, "curve.type", &ctype, sizeof(ctype));
        bsz += 6;
        assert(result == 0);
    }

    // curve bits
    {
        unsigned short int cbits;
        cbits = htobe16((unsigned short)cv->bits);
        result = asn1_write_value(privkey_asn1, "curve.bits", &cbits, sizeof(cbits));
        bsz += 7;
        assert(result == 0);
    }

    // curve parameters
    switch (cv->type) {
    case EQTypeShortWeierstrass:
        mpz_set_mpFp(tmpz, cv->coeff.ws.a);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.ws.b);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    case EQTypeEdwards:
        mpz_set_mpFp(tmpz, cv->coeff.ed.c);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.ed.d);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    case EQTypeMontgomery:
        mpz_set_mpFp(tmpz, cv->coeff.mo.B);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.mo.A);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    case EQTypeTwistedEdwards:
        mpz_set_mpFp(tmpz, cv->coeff.te.a);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.te.d);
        result = myasn1_write_mpz_value(privkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    default:
        return NULL;
    }

    //printf("-----------------\n");
    //asn1_print_structure(stdout, privkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // dump additional data in DER format
    bsz += 256;
    bufsz = bsz;
    der = (char *)malloc((bsz) * sizeof(char));
    result = asn1_der_coding(privkey_asn1, "", der, &bsz, asnError);
    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
    }
    assert(result == 0);
    assert(bsz < bufsz);
    *sz = bsz;

    mpz_clear(tmpz);
    asn1_delete_structure(&privkey_asn1);
    asn1_delete_structure(&ecdhe_asn1);
    return der;
}

int _ecdh_der_import_privkey(mpz_t privkey, mpECurve_t cv, char *der, size_t sz) {
    ASN1_TYPE ecdhe_asn1 = ASN1_TYPE_EMPTY;
    ASN1_TYPE privkey_asn1 = ASN1_TYPE_EMPTY;
    char asnError[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
    int result;
    mpz_t cv_p;
    mpz_t cv_n;
    mpz_t cv_h;
    mpz_t cv_gx;
    mpz_t cv_gy;
    mpz_t cv_p1;
    mpz_t cv_p2;
    int cv_type;
    int cv_bits;

    // read ASN1 syntax
    result = asn1_array2tree(ecdhe_asn1_tab, &ecdhe_asn1, asnError);

    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
        return -1;
    }

    // create an empty ASN1 message structure
    result = asn1_create_element(ecdhe_asn1, "ExampleASN.ECDHPrivateKey",
        &privkey_asn1);
    assert(result == 0);

    //printf("-----------------\n");
    //asn1_print_structure(stdout, privkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // read DER into ASN1 structure
    result = asn1_der_decoding(&privkey_asn1, der, sz, asnError);
    if (result != ASN1_SUCCESS) return -1;

    //printf("-----------------\n");
    //asn1_print_structure(stdout, privkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    mpz_init(cv_p);
    mpz_init(cv_n);
    mpz_init(cv_h);
    mpz_init(cv_gx);
    mpz_init(cv_gy);
    mpz_init(cv_p1);
    mpz_init(cv_p2);

    // private key value
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "privkey", privkey);
    if (result != 0) goto error_cleanup;

   // field order
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.p", cv_p);
    if (result != 0) goto error_cleanup;

    // curve order
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.n", cv_n);
    if (result != 0) goto error_cleanup;

    // curve cofactor
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.h", cv_h);
    if (result != 0) goto error_cleanup;

    // curve generator
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.g.x", cv_gx);
    if (result != 0) goto error_cleanup;
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.g.y", cv_gy);
    if (result != 0) goto error_cleanup;

    // curve parameters
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.param.p1", cv_p1);
    if (result != 0) goto error_cleanup;
    result = myasn1_read_octet_string_to_mpz(privkey_asn1, "curve.param.p2", cv_p2);
    if (result != 0) goto error_cleanup;

    // curve bits, type
    result = myasn1_read_integer_to_int(privkey_asn1, "curve.bits", &cv_bits);
    if (result != 0) goto error_cleanup;
    result = myasn1_read_integer_to_int(privkey_asn1, "curve.type", &cv_type);
    if (result != 0) goto error_cleanup;

    result = 0;

    switch (cv_type) {
    case EQTypeShortWeierstrass:
        result = mpECurve_set_mpz_ws(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    case EQTypeEdwards:
        result = mpECurve_set_mpz_ed(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    case EQTypeMontgomery:
        result = mpECurve_set_mpz_mo(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    case EQTypeTwistedEdwards:
        result = mpECurve_set_mpz_te(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    default:
        result = -1;
    }

error_cleanup:
    asn1_delete_structure(&privkey_asn1);
    asn1_delete_structure(&ecdhe_asn1);
    mpz_clear(cv_p2);
    mpz_clear(cv_p1);
    mpz_clear(cv_gy);
    mpz_clear(cv_gx);
    mpz_clear(cv_h);
    mpz_clear(cv_n);
    mpz_clear(cv_p);
    return result;
}

char *_ecdh_der_export_pubkey(mpECP_t pubkey, mpECurve_t cv, size_t *sz) {
    ASN1_TYPE ecdhe_asn1 = ASN1_TYPE_EMPTY;
    ASN1_TYPE pubkey_asn1 = ASN1_TYPE_EMPTY;
    char asnError[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
    char *der;
    mpz_t tmpz;
    int bsz = 0;
    int bufsz;
    int result;
    unsigned char *buf;
    
    mpz_init(tmpz);

    // read ASN1 syntax
    result = asn1_array2tree(ecdhe_asn1_tab, &ecdhe_asn1, asnError);

    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
        return NULL;
    }

    // create an empty ASN1 message structure
    result = asn1_create_element(ecdhe_asn1, "ExampleASN.ECDHPublicKey",
        &pubkey_asn1);
    assert(result == 0);

    //printf("-----------------\n");
    //asn1_print_structure(stdout, pubkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    bsz = mpECP_out_bytelen(pubkey, 1);
    buf = (unsigned char *)malloc(bsz*sizeof(unsigned char));
    mpECP_out_bytes(buf, pubkey, 1);

    // public key value
    result = asn1_write_value(pubkey_asn1, "pubkey", buf, bsz);
    assert(result == 0);
    memset(buf, 0, bsz);
    free(buf);
    
    bsz += 5;

    // field order
    result = myasn1_write_mpz_value(pubkey_asn1, "curve.p", cv->fp->p, &bsz);
    assert(result == 0);

    // curve order
    result = myasn1_write_mpz_value(pubkey_asn1, "curve.n", cv->n, &bsz);
    assert(result == 0);

    // curve cofactor
    result = myasn1_write_mpz_value(pubkey_asn1, "curve.h", cv->h, &bsz);
    assert(result == 0);

    // curve generator
    result = myasn1_write_mpz_value(pubkey_asn1, "curve.g.x", cv->G[0], &bsz);
    assert(result == 0);
    result = myasn1_write_mpz_value(pubkey_asn1, "curve.g.y", cv->G[1], &bsz);
    assert(result == 0);

    // curve type
    {
        unsigned char ctype;
        ctype = (unsigned char)cv->type;
        result = asn1_write_value(pubkey_asn1, "curve.type", &ctype, sizeof(ctype));
        bsz += 6;
        assert(result == 0);
    }

    // curve bits
    {
        unsigned short int cbits;
        cbits = htobe16((unsigned short)cv->bits);
        result = asn1_write_value(pubkey_asn1, "curve.bits", &cbits, sizeof(cbits));
        bsz += 7;
        assert(result == 0);
    }

    // curve parameters
    switch (cv->type) {
    case EQTypeShortWeierstrass:
        mpz_set_mpFp(tmpz, cv->coeff.ws.a);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.ws.b);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    case EQTypeEdwards:
        mpz_set_mpFp(tmpz, cv->coeff.ed.c);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.ed.d);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    case EQTypeMontgomery:
        mpz_set_mpFp(tmpz, cv->coeff.mo.B);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.mo.A);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    case EQTypeTwistedEdwards:
        mpz_set_mpFp(tmpz, cv->coeff.te.a);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p1", tmpz, &bsz);
        assert(result == 0);
        mpz_set_mpFp(tmpz, cv->coeff.te.d);
        result = myasn1_write_mpz_value(pubkey_asn1, "curve.param.p2", tmpz, &bsz);
        assert(result == 0);
        break;
    default:
        return NULL;
    }

    //printf("-----------------\n");
    //asn1_print_structure(stdout, pubkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // dump additional data in DER format
    bsz += 256;
    bufsz = bsz;
    der = (char *)malloc((bsz) * sizeof(char));
    result = asn1_der_coding(pubkey_asn1, "", der, &bsz, asnError);
    if (result != 0) {
        printf("%s", asnError);
    }
    assert(result == 0);
    assert(bsz < bufsz);
    *sz = bsz;

    mpz_clear(tmpz);
    asn1_delete_structure(&pubkey_asn1);
    asn1_delete_structure(&ecdhe_asn1);
    return der;
}

int _ecdh_der_init_import_pubkey(mpECP_t pubkey, mpECurve_t cv, char *der, size_t sz) {
    ASN1_TYPE ecdhe_asn1 = ASN1_TYPE_EMPTY;
    ASN1_TYPE pubkey_asn1 = ASN1_TYPE_EMPTY;
    char asnError[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
    int result;
    mpz_t cv_p;
    mpz_t cv_n;
    mpz_t cv_h;
    mpz_t cv_gx;
    mpz_t cv_gy;
    mpz_t cv_p1;
    mpz_t cv_p2;
    int cv_type;
    int cv_bits;
    unsigned char *rawpubkey;
    int rawpubkeysz;

    // read ASN1 syntax
    result = asn1_array2tree(ecdhe_asn1_tab, &ecdhe_asn1, asnError);

    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
        return -1;
    }

    // create an empty ASN1 message structure
    result = asn1_create_element(ecdhe_asn1, "ExampleASN.ECDHPublicKey",
        &pubkey_asn1);
    assert(result == 0);

    //printf("-----------------\n");
    //asn1_print_structure(stdout, privkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // read DER into ASN1 structure
    result = asn1_der_decoding(&pubkey_asn1, der, sz, asnError);
    if (result != ASN1_SUCCESS) return -1;

    //printf("-----------------\n");
    //asn1_print_structure(stdout, privkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    mpz_init(cv_p);
    mpz_init(cv_n);
    mpz_init(cv_h);
    mpz_init(cv_gx);
    mpz_init(cv_gy);
    mpz_init(cv_p1);
    mpz_init(cv_p2);

    // private key value
    result = myasn1_read_octet_string_to_bytes(pubkey_asn1, "pubkey", &rawpubkey, &rawpubkeysz);
    if (result != 0) goto error_cleanup;

   // field order
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.p", cv_p);
    if (result != 0) goto error_cleanup;

    // curve order
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.n", cv_n);
    if (result != 0) goto error_cleanup;

    // curve cofactor
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.h", cv_h);
    if (result != 0) goto error_cleanup;

    // curve generator
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.g.x", cv_gx);
    if (result != 0) goto error_cleanup;
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.g.y", cv_gy);
    if (result != 0) goto error_cleanup;

    // curve parameters
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.param.p1", cv_p1);
    if (result != 0) goto error_cleanup;
    result = myasn1_read_octet_string_to_mpz(pubkey_asn1, "curve.param.p2", cv_p2);
    if (result != 0) goto error_cleanup;

    // curve bits, type
    result = myasn1_read_integer_to_int(pubkey_asn1, "curve.bits", &cv_bits);
    if (result != 0) goto error_cleanup;
    result = myasn1_read_integer_to_int(pubkey_asn1, "curve.type", &cv_type);
    if (result != 0) goto error_cleanup;

    result = 0;

    mpECurve_init(cv);

    switch (cv_type) {
    case EQTypeShortWeierstrass:
        result = mpECurve_set_mpz_ws(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    case EQTypeEdwards:
        result = mpECurve_set_mpz_ed(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    case EQTypeMontgomery:
        result = mpECurve_set_mpz_mo(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    case EQTypeTwistedEdwards:
        result = mpECurve_set_mpz_te(cv, cv_p, cv_p1, cv_p2, cv_n, cv_h,
            cv_gx, cv_gy, (unsigned int)cv_bits);
        break;
    default:
        result = -1;
    }

    mpECP_init(pubkey, cv);
    if (result == 0) {
        result = mpECP_set_bytes(pubkey, rawpubkey, rawpubkeysz, cv);
    }
    free(rawpubkey);
    {
        mpz_t px;
        mpz_t py;

        mpz_init(px);
        mpz_init(py);
        mpz_set_mpECP_affine_x(px, pubkey);
        mpz_set_mpECP_affine_y(py, pubkey);
        if (!mpECurve_point_check(cv, px, py)) result = -1;
        mpz_clear(py);
        mpz_clear(px);
    }

error_cleanup:
    asn1_delete_structure(&pubkey_asn1);
    asn1_delete_structure(&ecdhe_asn1);
    mpz_clear(cv_p2);
    mpz_clear(cv_p1);
    mpz_clear(cv_gy);
    mpz_clear(cv_gx);
    mpz_clear(cv_h);
    mpz_clear(cv_n);
    mpz_clear(cv_p);
    return result;
}

char *_ecdhe_der_export_message(mpECP_t pubkey, unsigned char *nonce, size_t nsz, unsigned char *msg, size_t msz, size_t *sz) {
    ASN1_TYPE ecdhe_asn1 = ASN1_TYPE_EMPTY;
    ASN1_TYPE msg_asn1 = ASN1_TYPE_EMPTY;
    char asnError[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
    char *der;
    int bsz = 0;
    int bufsz;
    int result;
    unsigned char *buf;
    
    // read ASN1 syntax
    result = asn1_array2tree(ecdhe_asn1_tab, &ecdhe_asn1, asnError);

    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
        return NULL;
    }

    // create an empty ASN1 message structure
    result = asn1_create_element(ecdhe_asn1, "ExampleASN.ECDHEMessage",
        &msg_asn1);
    assert(result == 0);

    //printf("-----------------\n");
    //asn1_print_structure(stdout, msg_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    bsz = mpECP_out_bytelen(pubkey, 1);
    buf = (unsigned char *)malloc(bsz*sizeof(unsigned char));
    mpECP_out_bytes(buf, pubkey, 1);

    // public key value
    result = asn1_write_value(msg_asn1, "pubkeybytes", buf, bsz);
    assert(result == 0);
    free(buf);
    
    // public key value
    result = asn1_write_value(msg_asn1, "nonce", nonce, nsz);
    assert(result == 0);
    
    bsz += nsz + 5;
    
    // public key value
    result = asn1_write_value(msg_asn1, "ciphertext", msg, msz);
    assert(result == 0);
    
    bsz += msz + 5;

    //printf("-----------------\n");
    //asn1_print_structure(stdout, msg_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // dump data in DER format
    bsz += 256;
    bufsz = bsz;
    der = (char *)malloc((bsz) * sizeof(char));
    result = asn1_der_coding(msg_asn1, "", der, &bsz, asnError);
    if (result != 0) {
        printf("%s", asnError);
    }
    assert(result == 0);
    assert(bsz < bufsz);
    *sz = bsz;

    asn1_delete_structure(&msg_asn1);
    asn1_delete_structure(&ecdhe_asn1);
    return der;
}

int _ecdhe_der_init_import_message(unsigned char **ptxt, int *psz, 
    unsigned char **ntxt, int *nsz, unsigned char **ctxt, int *csz,
    char *der, int sz) {
    ASN1_TYPE ecdhe_asn1 = ASN1_TYPE_EMPTY;
    ASN1_TYPE pubkey_asn1 = ASN1_TYPE_EMPTY;
    char asnError[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
    int result;

    // read ASN1 syntax
    result = asn1_array2tree(ecdhe_asn1_tab, &ecdhe_asn1, asnError);

    if (result != 0) {
        asn1_perror (result);
        printf ("%s", asnError);
        return -1;
    }

    // create an empty ASN1 message structure
    result = asn1_create_element(ecdhe_asn1, "ExampleASN.ECDHEMessage",
        &pubkey_asn1);
    assert(result == 0);

    //printf("-----------------\n");
    //asn1_print_structure(stdout, pubkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // read DER into ASN1 structure
    result = asn1_der_decoding(&pubkey_asn1, der, sz, asnError);
    if (result != ASN1_SUCCESS) return -1;

    //printf("-----------------\n");
    //asn1_print_structure(stdout, pubkey_asn1, "", ASN1_PRINT_ALL);
    //printf("-----------------\n");

    // private key value
    result = myasn1_read_octet_string_to_bytes(pubkey_asn1, "pubkeybytes", ptxt, psz);
    if (result != 0) goto error_cleanup;

    // private key value
    result = myasn1_read_octet_string_to_bytes(pubkey_asn1, "nonce", ntxt, nsz);
    if (result != 0) goto error_cleanup;

    // private key value
    result = myasn1_read_octet_string_to_bytes(pubkey_asn1, "ciphertext", ctxt, csz);
    if (result != 0) goto error_cleanup;

error_cleanup:
    asn1_delete_structure(&pubkey_asn1);
    asn1_delete_structure(&ecdhe_asn1);
    return result;
}
