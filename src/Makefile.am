if COND_SAFECLEAN
  MAYBE_SAFECLEAN = -DSAFE_CLEAN
  BUILD_SAFECLEAN = safememory.c
endif

lib_LTLIBRARIES=libecc.la
libecc_la_SOURCES = field.c ecurve.c ecpoint.c mpzurandom.c $(BUILD_SAFECLEAN)
libecc_la_CFLAGS = -Wall $(MAYBE_SAFECLEAN) -I ../include
libecc_la_LDFLAGS = -version-info 1:1:0
