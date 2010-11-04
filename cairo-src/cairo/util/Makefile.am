include $(top_srcdir)/build/Makefile.am.common

util: malloc-stats.so backtrace-symbols.so

.la.so:
	$(RM) $@
	$(LN_S) .libs/$*.so $@

CLEANFILES += *.so

# The -rpath is needed to build shared objects that are not installed,
# ie. with EXTRA_LTLIBRARIES
AM_LDFLAGS = -module -avoid-version -export-dynamic -rpath /dev/null

EXTRA_LTLIBRARIES += malloc-stats.la backtrace-symbols.la

backtrace_symbols_la_LIBADD  = -lbfd -liberty

malloc_stats_la_LIBADD  = $(backtrace_symbols_la_LIBADD) backtrace-symbols.lo

EXTRA_DIST += \
	xr2cairo \
	cairo-api-update
