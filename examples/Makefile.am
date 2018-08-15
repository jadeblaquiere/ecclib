noinst_PROGRAMS = ecdh_gen ecdh_pub ecdh_enc ecdh_dec ecdsa_sign ecdsa_verify ecelgamal_enc ecelgamal_dec
CLEANFILES=ecdhe_asn1_tab.c elgamal_asn1_tab.c
ecdhe_asn1_tab.c: ecdhe.asn
	asn1Parser ecdhe.asn
elgamal_asn1_tab.c: elgamal.asn
	asn1Parser elgamal.asn

ecdh_gen_SOURCES = ecdh_gen.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdh_gen_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdh_gen_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecdh_pub_SOURCES = ecdh_pub.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdh_pub_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdh_pub_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecdh_enc_SOURCES = ecdh_enc.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdh_enc_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdh_enc_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecdh_dec_SOURCES = ecdh_dec.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdh_dec_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdh_dec_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecdsa_sign_SOURCES = ecdsa_sign.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdsa_sign_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdsa_sign_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecdsa_verify_SOURCES = ecdsa_verify.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdsa_verify_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdsa_verify_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecelgamal_enc_SOURCES = ecelgamal_enc.c ecdh_der.c elgamal_der.c b64file.c ecdhe_asn1_tab.c elgamal_asn1_tab.c
ecelgamal_enc_CFLAGS = -Wall -I ../include $(CFLAGS)
ecelgamal_enc_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

ecelgamal_dec_SOURCES = ecelgamal_dec.c ecdh_der.c elgamal_der.c b64file.c ecdhe_asn1_tab.c elgamal_asn1_tab.c
ecelgamal_dec_CFLAGS = -Wall -I ../include $(CFLAGS)
ecelgamal_dec_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)
