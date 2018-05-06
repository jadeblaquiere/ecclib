noinst_PROGRAMS = ecdh_gen
CLEANFILES=ecdhe_asn1_tab.c
ecdhe_asn1_tab.c: ecdhe.asn
	asn1Parser ecdhe.asn

ecdh_gen_SOURCES = ecdh_gen.c ecdh_der.c b64file.c ecdhe_asn1_tab.c
ecdh_gen_CFLAGS = -Wall -I ../include $(CFLAGS)
ecdh_gen_LDADD = -L../src/.libs/ -lecc -lgmp -ltasn1 -lpopt -lb64 $(LDFLAGS)

