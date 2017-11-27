noinst_PROGRAMS = test_field
TESTS = test_field

test_field_SOURCES = test_field.c
test_field_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
test_field_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)