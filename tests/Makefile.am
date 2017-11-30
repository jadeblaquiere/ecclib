noinst_PROGRAMS = test_field test_ecurve
TESTS = test_field test_ecurve

test_field_SOURCES = test_field.c
test_field_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_field_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_ecurve_SOURCES = test_ecurve.c
test_ecurve_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_ecurve_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)