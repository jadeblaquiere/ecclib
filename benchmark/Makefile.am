if HAVE_LIBSODIUM
  MAYBE_SODIUM_BENCH = mul_bench_libsodium gen_bench_libsodium
endif
if HAVE_LIBRELIC
  MAYBE_RELIC_BENCH = mul_bench_relic
endif
noinst_PROGRAMS = mul_bench gen_bench $(MAYBE_SODIUM_BENCH) $(MAYBE_RELIC_BENCH)

mul_bench_SOURCES = mul_bench.c
mul_bench_CFLAGS = -Wall -I../include $(CFLAGS) $(CHECK_CFLAGS)
mul_bench_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

gen_bench_SOURCES = gen_bench.c
gen_bench_CFLAGS = -Wall -I../include $(CFLAGS) $(CHECK_CFLAGS)
gen_bench_LDADD = -L../src/.libs/ -lecc -lgmp $(LDFLAGS) $(CHECK_LIBS)

mul_bench_libsodium_SOURCES = mul_bench_libsodium.c
mul_bench_libsodium_CFLAGS = -Wall -I../include $(CFLAGS) $(CHECK_CFLAGS)
mul_bench_libsodium_LDADD = -L../src/.libs/ -lsodium $(LDFLAGS) $(CHECK_LIBS)

gen_bench_libsodium_SOURCES = gen_bench_libsodium.c
gen_bench_libsodium_CFLAGS = -Wall -I../include $(CFLAGS) $(CHECK_CFLAGS)
gen_bench_libsodium_LDADD = -L../src/.libs/ -lsodium $(LDFLAGS) $(CHECK_LIBS)

mul_bench_relic_SOURCES = mul_bench_relic.c
mul_bench_relic_CFLAGS = -Wall -I../include -I/usr/local/include/relic $(CFLAGS) $(CHECK_CFLAGS)
mul_bench_relic_LDADD = -L../src/.libs/ -L/usr/local/lib -lrelic -lgmp $(LDFLAGS) $(CHECK_LIBS)
