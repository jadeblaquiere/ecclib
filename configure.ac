AC_INIT([libecc], [0.1], [https://github.com/jadeblaquiere/ecclib/issues])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC
AM_PROG_AR
LT_INIT
AC_ARG_ENABLE([benchmarks],
[  --enable-benchmarks       build benchmarks @<:@default=no@:>@],
[case "${enableval}" in
  yes) benchmarks=true ;;
  no)  benchmarks=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-benchmarks]) ;;
esac],[debug=false])
AM_CONDITIONAL([COND_BENCHMARKS], [test "x$benchmarks" = xtrue])
AC_CHECK_LIB([gmp], [__gmpz_realloc])
AC_CHECK_LIB([sodium], [crypto_scalarmult_ed25519])
AM_CONDITIONAL([HAVE_LIBSODIUM], [test "x$ac_cv_lib_sodium_crypto_scalarmult_ed25519" = xyes])
AC_CHECK_LIB([relic], [ep_curve_set_plain])
AM_CONDITIONAL([HAVE_LIBRELIC], [test "x$ac_cv_lib_relic_ep_curve_set_plain" = xyes])
PKG_CHECK_MODULES([CHECK], [check])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_OUTPUT([Makefile src/Makefile tests/Makefile include/Makefile examples/Makefile benchmark/Makefile])
