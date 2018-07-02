if COND_SAFECLEAN
  MAYBE_SAFECLEAN = -DSAFE_CLEAN
endif

VALGRIND_TEST_CASES = mtest_field mtest_ecurve mtest_ecpoint
noinst_PROGRAMS = test_field test_ecurve test_ecpoint test_mpzurandom $(VALGRIND_TEST_CASES)
TESTS = test_field test_ecurve test_ecpoint test_mpzurandom

test_field_SOURCES = test_field.c
test_field_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_field_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_ecurve_SOURCES = test_ecurve.c
test_ecurve_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_ecurve_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_ecpoint_SOURCES = test_ecpoint.c
test_ecpoint_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_ecpoint_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

test_mpzurandom_SOURCES = test_mpzurandom.c
test_mpzurandom_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_mpzurandom_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

mtest_field_SOURCES = mtest_field.c
mtest_field_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
mtest_field_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS)

mtest_ecurve_SOURCES = mtest_ecurve.c
mtest_ecurve_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
mtest_ecurve_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS)

mtest_ecpoint_SOURCES = mtest_ecpoint.c
mtest_ecpoint_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include $(CFLAGS) $(CHECK_CFLAGS)
mtest_ecpoint_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS)
