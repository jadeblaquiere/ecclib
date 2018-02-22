noinst_PROGRAMS = mul_bench gen_bench mul_bench_libsodium gen_bench_libsodium

mul_bench_SOURCES = mul_bench.c
mul_bench_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
mul_bench_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

gen_bench_SOURCES = gen_bench.c
gen_bench_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
gen_bench_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

mul_bench_libsodium_SOURCES = mul_bench_libsodium.c
mul_bench_libsodium_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
mul_bench_libsodium_LDADD = -L../src/.libs/ -lsodium $(LDFLAGS) $(CHECK_LIBS)

gen_bench_libsodium_SOURCES = gen_bench_libsodium.c
gen_bench_libsodium_CFLAGS = -Wall -I ../include $(CFLAGS) $(CHECK_CFLAGS)
gen_bench_libsodium_LDADD = -L../src/.libs/ -lsodium $(LDFLAGS) $(CHECK_LIBS)
