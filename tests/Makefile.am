noinst_PROGRAMS = test_field test_curve
TESTS = test_field test_curve

test_field_SOURCES = test_field.c
test_field_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_field_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_curve_SOURCES = test_curve.c
test_curve_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_curve_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)