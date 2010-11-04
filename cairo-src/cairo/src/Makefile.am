# Note: All source files are listed in Makefile.sources.

include $(top_srcdir)/build/Makefile.am.common
include $(srcdir)/Makefile.am.features

EXTRA_DIST += Makefile.win32      Makefile.win32.features
MAINTAINERCLEANFILES += $(srcdir)/Makefile.win32.features

AM_CPPFLAGS = -I$(srcdir) $(CAIRO_CFLAGS)

if OS_WIN32
export_symbols = -export-symbols cairo.def
cairo_def_dependency = cairo.def
endif

EXTRA_DIST           += cairo-supported-features.h
MAINTAINERCLEANFILES += cairo-supported-features.h

$(top_builddir)/config.h: $(top_srcdir)/config.h.in
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) config.h

cairoincludedir = $(includedir)/cairo
cairoinclude_HEADERS = $(enabled_cairo_headers)

lib_LTLIBRARIES = libcairo.la

libcairo_la_SOURCES = \
	$(enabled_cairo_headers) \
	$(enabled_cairo_private) \
	$(enabled_cairo_sources) \
	$(NULL)
libcairo_la_LDFLAGS = -version-info $(CAIRO_LIBTOOL_VERSION_INFO) -no-undefined $(export_symbols)
libcairo_la_LIBADD = $(CAIRO_LIBS)
libcairo_la_DEPENDENCIES = $(cairo_def_dependency)

# Special headers
cairoinclude_HEADERS += $(top_srcdir)/cairo-version.h
nodist_cairoinclude_HEADERS = cairo-features.h
libcairo_la_SOURCES += cairo-version.h cairo-features.h
BUILT_SOURCES  += cairo-features.h cairo-supported-features.h
cairo-features.h cairo-supported-features.h:
	cd $(top_builddir) && ./config.status src/$@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(enabled_cairo_pkgconf)

CLEANFILES += cairo.def
cairo.def: cairo-features.h $(enabled_cairo_headers)
	@echo Generating $@
	@(echo EXPORTS; \
	(cd $(srcdir); cat $(enabled_cairo_headers) || echo 'cairo_ERROR ()' ) | \
	grep -v -E '^# *include' | \
	( cat cairo-features.h - | $(CPP) - || echo 'cairo_ERROR ()' ) | \
	grep -E '^cairo_.* \(' | \
	sed -e 's/[ 	].*//' | \
	sort; \
	echo LIBRARY libcairo-$(CAIRO_VERSION_SONUM).dll; \
	) >$@
	@ ! grep -q cairo_ERROR $@ || ($(RM) $@; false)

TESTS_ENVIRONMENT = \
	srcdir="$(srcdir)" \
	MAKE="$(MAKE) $(AM_MAKEFLAGS)" \
	all_cairo_files="$(all_cairo_files)" \
	all_cairo_headers="$(all_cairo_headers)" \
	all_cairo_private="$(all_cairo_private)" \
	all_cairo_sources="$(all_cairo_sources)" \
	enabled_cairo_files="$(enabled_cairo_files)" \
	enabled_cairo_headers="$(enabled_cairo_headers)" \
	enabled_cairo_private="$(enabled_cairo_private)" \
	enabled_cairo_sources="$(enabled_cairo_sources)" \
	$(NULL)
TESTS_SH = \
	check-def.sh \
	check-doc-syntax.sh\
	check-headers.sh \
	check-plt.sh \
	check-preprocessor-syntax.sh \
	$(NULL)
TESTS += $(TESTS_SH)
if CROSS_COMPILING
else
TESTS += check-link$(EXEEXT)
endif

EXTRA_DIST += $(TESTS_SH) check-has-hidden-symbols.c
check_PROGRAMS += check-link
check_link_LDADD = libcairo.la

check: headers-standalone

PREPROCESS_ARGS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)
COMPILE_ARGS = $(PREPROCESS_ARGS) $(AM_CFLAGS) $(CFLAGS)

# The pre-processed result is used by check-{def,plt}.sh to determine whether
# cairo has been compiled with symbol hiding.
.c.i: $(cairoinclude_HEADERS) $(nodist_cairoinclude_HEADERS) cairoint.h $(top_builddir)/config.h
	$(CPP) $(PREPROCESS_ARGS) $< -o $@

include $(srcdir)/Makefile.am.analysis
