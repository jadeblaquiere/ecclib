noinst_PROGRAMS = test_field test_ecurve test_ecpoint test_mpzurandom
TESTS = test_field test_ecurve test_ecpoint test_mpzurandom

test_field_SOURCES = test_field.c
test_field_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_field_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_ecurve_SOURCES = test_ecurve.c
test_ecurve_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_ecurve_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_ecpoint_SOURCES = test_ecpoint.c
test_ecpoint_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_ecpoint_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_mpzurandom_SOURCES = test_mpzurandom.c
test_mpzurandom_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_mpzurandom_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)
