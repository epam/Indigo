AC_PREREQ(2.59)
CAIRO_PARSE_VERSION
AC_INIT([cairo],
	[cairo_version_major.cairo_version_minor.cairo_version_micro],
	[http://bugs.freedesktop.org/enter_bug.cgi?product=cairo])
AC_CONFIG_SRCDIR(src/cairo.h)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_MACRO_DIR(build)
AM_INIT_AUTOMAKE([1.9.6 gnu -Wall no-define])
AC_LIBTOOL_WIN32_DLL dnl Must be called before AC_PROG_LIBTOOL
AC_PROG_LIBTOOL dnl ([1.4]) Don't remove!
DOLT dnl Make my libtool fast!
GTK_DOC_CHECK([1.6])

dnl ===========================================================================
dnl
dnl The order of the includes here is rather important
dnl
m4_include(build/configure.ac.version)	dnl macros setting up various version declares
m4_include(build/configure.ac.tools)	dnl checks for tools we use
m4_include(build/configure.ac.system)	dnl checks for system functions, headers, libs
m4_include(build/configure.ac.features)	dnl macros for backend/feature handling
m4_include(build/configure.ac.warnings)	dnl checks for compiler warning
m4_include(build/configure.ac.analysis)	dnl checks for analysis tools (lcov, etc)
AC_CACHE_SAVE

dnl ===========================================================================

AC_CHECK_LIB(z, compress,
	 [AC_CHECK_HEADER(zlib.h, [have_libz=yes],
	 [have_libz="no (requires zlib http://www.gzip.org/zlib/)"])],
	 [have_libz="no (requires zlib http://www.gzip.org/zlib/)"])

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(xlib, Xlib, auto, [
  xlib_REQUIRES="x11"
  PKG_CHECK_MODULES(xlib, $xlib_REQUIRES, ,
		    [AC_MSG_RESULT(no)
		     xlib_REQUIRES=""
		     AC_PATH_XTRA
		     if test "x$no_x" = xyes; then
		       use_xlib="no (requires X development libraries)"
		     else
		       xlib_NONPKGCONFIG_LIBS="$X_PRE_LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"
		       xlib_NONPKGCONFIG_CFLAGS=$X_CFLAGS
		     fi])
])

CAIRO_ENABLE_SURFACE_BACKEND(xlib_xrender, Xlib Xrender, auto, [
  if test "x$use_xlib" != "xyes"; then
    use_xlib_xrender="no (requires --enable-xlib)"
  else
    dnl Check for Xrender header files if the Xrender package is not installed:
    xlib_xrender_BASE=cairo-xlib
    xlib_xrender_REQUIRES="xrender >= 0.6"
    PKG_CHECK_MODULES(xlib_xrender, $xlib_xrender_REQUIRES, ,
		      [AC_MSG_RESULT(no)
		       xlib_xrender_REQUIRES=""
		       old_CPPFLAGS=$CPPFLAGS
		       CPPFLAGS="$CPPFLAGS $xlib_CFLAGS $xlib_NONPKGCONFIG_CFLAGS"
		       AC_CHECK_HEADER(X11/extensions/Xrender.h,
				       [xlib_xrender_NONPKGCONFIG_LIBS="-lXrender"],
				       [use_xlib_xrender="no (requires $xlib_xrender_REQUIRES http://freedesktop.org/Software/xlibs)"])
		       CPPFLAGS=$old_CPPFLAGS
		      ])
  fi
])

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(xcb, XCB, no, [
  xcb_REQUIRES="xcb >= 0.9.92 xcb-render >= 0.9.92 xcb-renderutil"
  PKG_CHECK_MODULES(xcb, $xcb_REQUIRES, , [AC_MSG_RESULT(no)
  use_xcb="no (requires $xcb_REQUIRES http://xcb.freedesktop.org)"])
])

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(quartz, Quartz, auto, [
  dnl There is no pkgconfig for quartz; lets do a header check
  AC_CHECK_HEADER(ApplicationServices/ApplicationServices.h, , [use_quartz="no (requires ApplicationServices framework)"])
  if test "x$use_quartz" != "xyes" ; then
    dnl check for CoreGraphics as a separate framework
    AC_CHECK_HEADER(CoreGraphics/CoreGraphics.h, , [use_quartz="no (requires CoreGraphics framework)"])
    quartz_LIBS="-Xlinker -framework -Xlinker CoreGraphics"
  else
    quartz_LIBS="-Xlinker -framework -Xlinker ApplicationServices"
  fi
])

CAIRO_ENABLE_FONT_BACKEND(quartz_font, Quartz, auto, [
  use_quartz_font=$use_quartz
])

CAIRO_ENABLE_SURFACE_BACKEND(quartz_image, Quartz Image, no, [
  use_quartz_image=$use_quartz
])

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(win32, Microsoft Windows, auto, [
  if test "x$have_windows" != xyes; then
    use_win32="no (requires a Win32 platform)"
  fi
  win32_LIBS="-lgdi32 -lmsimg32"
])

CAIRO_ENABLE_FONT_BACKEND(win32_font, Microsoft Windows, auto, [
  use_win32_font=$use_win32
])

test_win32_printing=no
if test "x$use_win32" = "xyes"; then
  AC_CHECK_PROG(GS, gs, gs)
  if test "$GS"; then
    AC_DEFINE([CAIRO_CAN_TEST_WIN32_PRINTING_SURFACE], 1, [Define to 1 if the Win32 Printing backend can be tested (needs ghostscript)])
    test_win32_printing="yes"
  else
    AC_MSG_WARN([Win32 Printing backend will not be tested since ghostscript is not available])
    test_win32_printing="no (requires ghostscript)"
  fi
fi

AM_CONDITIONAL(CAIRO_CAN_TEST_WIN32_PRINTING_SURFACE, test "x$test_win32_printing" = "xyes")

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(os2, OS/2, no, [
  case "$host" in
    *-*-os2*)
      :
      ;;
    *)
      use_os2="no (requires an OS/2 platform)"
      ;;
  esac
])

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(beos, BeOS/Zeta, no, [
  case "$host" in
    *-*-beos)
      beos_LIBS=""
      dnl Add libbe and libzeta if available
      AC_CHECK_LIB(be,main,beos_LIBS="$beos_LIBS -lbe")
      AC_CHECK_LIB(zeta,main,beos_LIBS="$beos_LIBS -lzeta")
      ;;
    *)
      use_beos="no (requires a BeOS platform)"
      ;;
  esac
])

dnl ===========================================================================

CAIRO_ENABLE_FUNCTIONS(png, PNG, yes, [
  use_png=no
  AC_ARG_VAR([png_REQUIRES], [module name for libpng to search for using pkg-config])
  if test "x$png_REQUIRES" = x; then
    # libpng13 is GnuWin32's libpng-1.2.8 :-(
    for l in libpng12 libpng13 libpng10 libpng; do
      if $PKG_CONFIG --exists $l ; then
        png_REQUIRES=$l
        use_png=yes
        break
      fi
    done
  else
    use_png=yes
  fi

  if test "x$use_png" = "xyes" ; then 
    PKG_CHECK_MODULES(png, $png_REQUIRES, , AC_MSG_RESULT(no))
  else
    AC_MSG_WARN([Could not find libpng in the pkg-config search path])
  fi    
])

dnl ===========================================================================

GLITZ_MIN_VERSION=0.5.1
CAIRO_ENABLE_SURFACE_BACKEND(glitz, glitz, no, [
  glitz_REQUIRES="glitz >= $GLITZ_MIN_VERSION"
  PKG_CHECK_MODULES(glitz, $glitz_REQUIRES, , [AC_MSG_RESULT(no)
  use_glitz="no (requires $glitz_REQUIRES http://freedesktop.org/Software/glitz)"])
])

if test "x$use_glitz" = "xyes";then
  PKG_CHECK_MODULES(GLITZ_AGL, glitz-agl >= $GLITZ_MIN_VERSION,
   [have_glitz_agl=yes], [have_glitz_agl=no; AC_MSG_RESULT(no)])
  PKG_CHECK_MODULES(GLITZ_EGL, glitz-egl >= $GLITZ_MIN_VERSION,
   [have_glitz_egl=yes], [have_glitz_egl=no; AC_MSG_RESULT(no)])
  PKG_CHECK_MODULES(GLITZ_GLX, glitz-glx >= $GLITZ_MIN_VERSION,
   [have_glitz_glx=yes], [have_glitz_glx=no; AC_MSG_RESULT(no)])
  PKG_CHECK_MODULES(GLITZ_WGL, glitz-wgl >= $GLITZ_MIN_VERSION,
   [have_glitz_wgl=yes], [have_glitz_wgl=no; AC_MSG_RESULT(no)])

  if test "x$have_glitz_agl" = "xyes";then
    AC_DEFINE(CAIRO_CAN_TEST_GLITZ_AGL_SURFACE, 1, [define if glitz backend can be tested against agl])
  fi
  if test "x$have_glitz_egl" = "xyes";then
    AC_DEFINE(CAIRO_CAN_TEST_GLITZ_EGL_SURFACE, 1, [define if glitz backend can be tested against egl])
  fi
  if test "x$have_glitz_glx" = "xyes";then
    AC_DEFINE(CAIRO_CAN_TEST_GLITZ_GLX_SURFACE, 1, [define if glitz backend can be tested against glx])
  fi
  if test "x$have_glitz_wgl" = "xyes";then
    AC_DEFINE(CAIRO_CAN_TEST_GLITZ_WGL_SURFACE, 1, [define if glitz backend can be tested against wgl])
  fi
fi

AM_CONDITIONAL(CAIRO_CAN_TEST_GLITZ_AGL_SURFACE, test "x$have_glitz_agl" = "xyes")
AM_CONDITIONAL(CAIRO_CAN_TEST_GLITZ_EGL_SURFACE, test "x$have_glitz_egl" = "xyes")
AM_CONDITIONAL(CAIRO_CAN_TEST_GLITZ_GLX_SURFACE, test "x$have_glitz_glx" = "xyes")
AM_CONDITIONAL(CAIRO_CAN_TEST_GLITZ_WGL_SURFACE, test "x$have_glitz_wgl" = "xyes")

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(directfb, directfb, no, [
  directfb_REQUIRES=directfb
  PKG_CHECK_MODULES(directfb, $directfb_REQUIRES, , AC_MSG_RESULT(no)
		    [use_directfb="no (requires $directfb_REQUIRES http://www.directfb.org)"])
])

dnl ===========================================================================

# We use pkg-config to look for freetype2, but fall back to
# freetype-config if it fails.  We prefer pkg-config, since we can
# then just put freetype2 >= $FREETYPE_MIN_VERSION in
# Requires.private, but at least up to 2003-06-07, there was no
# freetype2.pc in the release.
#
# FreeType versions come in three forms:
#   release (such as 2.1.9)
#   libtool (such as 9.7.3) (returned by freetype-config and pkg-config)
#   platform-specific/soname (such as 6.3.4)
# and they recommend you never use the platform-specific version
# (see docs/VERSION.DLL in freetype2 sources)
#
# Set these as appropriate:

# release number - for information only
FREETYPE_MIN_RELEASE=2.1.9
# libtool-specific version - this is what is checked
FREETYPE_MIN_VERSION=9.7.3

CAIRO_ENABLE_FONT_BACKEND(ft, FreeType, auto, [
  ft_REQUIRES="fontconfig"
  PKG_CHECK_MODULES(FONTCONFIG, $ft_REQUIRES,,
		    [AC_MSG_RESULT(no); use_ft="no (requires $ft_REQUIRES)"])

  if test "x$use_ft" = "xyes"; then

    CAIRO_CHECK_FUNCS_WITH_FLAGS(FcFini, [$FONTCONFIG_CFLAGS], [$FONTCONFIG_LIBS])

    PKG_CHECK_MODULES(FREETYPE, freetype2 >= $FREETYPE_MIN_VERSION,
                      [freetype_pkgconfig=yes],
		      [AC_MSG_RESULT(no)
		       freetype_pkgconfig=no])
  
    if test "x$freetype_pkgconfig" = "xyes"; then
      ft_REQUIRES="freetype2 >= $FREETYPE_MIN_VERSION $ft_REQUIRES"
    else
  
      if test -z "$FREETYPE_CONFIG"; then
        AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
      fi
      if test "x$FREETYPE_CONFIG" = "xno" ; then
        use_ft='no (freetype-config not found in path or $FREETYPE_CONFIG)'
      else
        AC_MSG_CHECKING(freetype2 libtool version)

        FREETYPE_VERSION=`$FREETYPE_CONFIG --version`
	AX_COMPARE_VERSION([$FREETYPE_VERSION], [gt], [$FREETYPE_MIN_VERSION],
			   [AC_MSG_RESULT($FREETYPE_VERSION - OK)
			   ft_NONPKGCONFIG_CFLAGS=`$FREETYPE_CONFIG --cflags`
			   ft_NONPKGCONFIG_LIBS=`$FREETYPE_CONFIG --libs`],
			   [AC_MSG_RESULT($FREETYPE_VERSION - Too old)
			   use_ft="no ($FREETYPE_VERSION found; version $FREETYPE_MIN_VERSION from release $FREETYPE_MIN_RELEASE required)"])
      fi
    fi
  fi
  ft_CFLAGS="$FREETYPE_CFLAGS $FONTCONFIG_CFLAGS"
  ft_LIBS="$FREETYPE_LIBS $FONTCONFIG_LIBS"
])

if test "x$use_ft" = "xyes"; then
  _save_libs="$LIBS"
  _save_cflags="$CFLAGS"
  LIBS="$LIBS $ft_LIBS"
  CFLAGS="$CFLAGS $ft_CFLAGS"
  AC_CHECK_MEMBER(FT_Bitmap_Size.y_ppem,
                  HAVE_FT_BITMAP_SIZE_Y_PPEM=1,
                  HAVE_FT_BITMAP_SIZE_Y_PPEM=0,
                  [#include <ft2build.h>
                   #include FT_FREETYPE_H])
  AC_DEFINE_UNQUOTED(HAVE_FT_BITMAP_SIZE_Y_PPEM,$HAVE_FT_BITMAP_SIZE_Y_PPEM,
                     [FT_Bitmap_Size structure includes y_ppem field])

  AC_CHECK_FUNCS(FT_GlyphSlot_Embolden FT_Load_Sfnt_Table FT_Library_SetLcdFilter)

  LIBS="$_save_libs"
  CFLAGS="$_save_cflags"
fi

dnl ===========================================================================

AC_ARG_ENABLE(pthread,
  AS_HELP_STRING([--disable-pthread],
		 [Do not use pthread]),
  [use_pthread=$enableval], [use_pthread=auto])

have_pthread=no
if test "x$use_pthread" != "xno"; then
  AC_CHECK_HEADERS([pthread.h], have_pthread=yes, have_pthread=no)
fi
AM_CONDITIONAL(HAVE_PTHREAD, test "x$have_pthread" = "xyes")
if test "x$have_pthread" = xno -a "x$use_pthread" = xyes; then
  AC_MSG_ERROR([pthread requested but not found])
fi

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(ps, PostScript, yes, [
    # The ps backend requires zlib.
    use_ps=$have_libz
    ps_NONPKGCONFIG_LIBS=-lz
])

dnl ===========================================================================

SPECTRE_VERSION_REQUIRED=0.2.0
test_ps=no
any2ppm_ps=no
if test "x$use_ps" = "xyes"; then
  AC_CHECK_PROG(GS, gs, gs)
  if test "$GS"; then
    AC_DEFINE([CAIRO_CAN_TEST_PS_SURFACE], 1, [Define to 1 if the PS backend can be tested (needs ghostscript)])
    test_ps="yes"
  else
    AC_MSG_WARN([PS backend will not be tested since ghostscript is not available])
    test_ps="no (requires ghostscript)"
  fi

  libspectre_DEPENDENCY="libspectre >= $SPECTRE_VERSION_REQUIRED"
  PKG_CHECK_MODULES(LIBSPECTRE, $libspectre_DEPENDENCY,
		    [any2ppm_ps=yes],
		    [AC_MSG_RESULT(no)])
fi

AM_CONDITIONAL(CAIRO_CAN_TEST_PS_SURFACE, test "x$test_ps" = "xyes")
AM_CONDITIONAL(CAIRO_HAS_SPECTRE, test "x$any2ppm_ps" = "xyes")
if test "x$any2ppm_ps" = "xyes"; then
    AC_DEFINE([CAIRO_HAS_SPECTRE], 1, [Define to 1 if libspectre is available])
fi
AC_SUBST(LIBSPECTRE_CFLAGS)
AC_SUBST(LIBSPECTRE_LIBS)

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(pdf, PDF, yes, [
    # The pdf backend requires zlib.
    use_pdf=$have_libz
    pdf_NONPKGCONFIG_LIBS=-lz
])

dnl ===========================================================================

# poppler-0.9.2 has an important bug fixes for user-font and an
# enhancement not to gobble nearly 1GiB of memory during test/large-font.
POPPLER_VERSION_REQUIRED=0.9.2
test_pdf=no
any2ppm_pdf=no
if test "x$use_pdf" = "xyes"; then
  poppler_DEPENDENCY="poppler-glib >= $POPPLER_VERSION_REQUIRED"
  PKG_CHECK_MODULES(POPPLER, $poppler_DEPENDENCY pango gtk+-2.0,
		    [CAIRO_CHECK_FUNCS_WITH_FLAGS(poppler_page_render, [$POPPLER_CFLAGS], [$POPPLER_LIBS],
                    [test_pdf=yes; any2ppm_pdf=yes],
		    [AC_MSG_RESULT(no); test_pdf="no (requires $poppler_DEPENDENCY)"])],
		    [AC_MSG_RESULT(no); test_pdf="no (requires $poppler_DEPENDENCY)"])
  if test "x$test_pdf" = "xyes"; then
    AC_DEFINE([CAIRO_CAN_TEST_PDF_SURFACE], 1, [Define to 1 if the PDF backend can be tested (need poppler and other dependencies for pdf2png)])
  else
    AC_MSG_WARN([PDF backend will not be tested since poppler >= $POPPLER_VERSION_REQUIRED is not available])
  fi
fi

AM_CONDITIONAL(CAIRO_CAN_TEST_PDF_SURFACE, test "x$test_pdf" = "xyes")
AC_SUBST(POPPLER_CFLAGS)
AC_SUBST(POPPLER_LIBS)

AM_CONDITIONAL(CAIRO_HAS_MULTI_PAGE_SURFACES, test "x$use_ps" = "xyes" -o "x$use_pdf" = "xyes")

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(svg, SVG, yes, [
  if test "x$use_png" != "xyes"; then
    use_svg="no (requires --enable-png)"
  fi
])

LIBRSVG_VERSION_REQUIRED=2.15.0
test_svg=no
any2ppm_svg=no
if test "x$use_svg" = "xyes"; then
  librsvg_DEPENDENCY="librsvg-2.0 >= $LIBRSVG_VERSION_REQUIRED"
  PKG_CHECK_MODULES(LIBRSVG, $librsvg_DEPENDENCY gdk-2.0,
		    [CAIRO_CHECK_FUNCS_WITH_FLAGS(rsvg_pixbuf_from_file, [$LIBRSVG_CFLAGS], [$LIBRSVG_LIBS],
                    [test_svg=yes; any2ppm_svg=yes],
		    [AC_MSG_RESULT(no); test_svg="no (requires $librsvg_DEPENDENCY)"])],
		    [AC_MSG_RESULT(no); test_svg="no (requires $librsvg_DEPENDENCY)"])
  if test "x$test_svg" = "xyes"; then
    AC_DEFINE([CAIRO_CAN_TEST_SVG_SURFACE], 1, [Define to 1 if the SVG backend can be tested])
  else
    AC_MSG_WARN([SVG backend will not be tested since librsvg >= $LIBRSVG_VERSION_REQUIRED is not available])
  fi
fi

AM_CONDITIONAL(CAIRO_CAN_TEST_SVG_SURFACE, test "x$test_svg" = "xyes")
AC_SUBST(LIBRSVG_CFLAGS)
AC_SUBST(LIBRSVG_LIBS)

dnl ===========================================================================

dnl XXX make this a private feature?
CAIRO_ENABLE(test_surfaces, test surfaces, no)

dnl ===========================================================================

CAIRO_ENABLE_SURFACE_BACKEND(image, image, always, [
  pixman_REQUIRES="pixman-1 >= 0.12.0"
  PKG_CHECK_MODULES(pixman, $pixman_REQUIRES, , [AC_MSG_RESULT(no)
  use_image="no (requires $pixman_REQUIRES http://cairographics.org/releases/)"])
  image_REQUIRES=$pixman_REQUIRES
  image_CFLAGS=$pixman_CFLAGS
  image_LIBS=$pixman_LIBS
])

dnl ===========================================================================

CAIRO_ENABLE_FONT_BACKEND(user, user, always)

dnl ===========================================================================
dnl Build the external converter if we have any of the test backends
AM_CONDITIONAL(BUILD_ANY2PPM,
	       test "x$any2ppm_svg" = "xyes" \
	         -o "x$any2ppm_pdf" = "xyes" \
		 -o "x$any2ppm_ps"  = "xyes")

dnl ===========================================================================

AC_ARG_ENABLE(some-floating-point,
  AS_HELP_STRING([--disable-some-floating-point],
                 [Disable certain code paths that rely heavily on double precision
                  floating-point calculation. This option can improve
                  performance on systems without a double precision floating-point
                  unit, but might degrade performance on those that do.]), [
if test "x$enableval" = "xno"; then
  # A value of 'no' for $enableval means that they want to disable, which
  # means 'yes' for $disable_some_floating_point.
  disable_some_floating_point=yes
fi
], [disable_some_floating_point=no])

AM_CONDITIONAL(DISABLE_SOME_FLOATING_POINT,
               test "x$disable_some_floating_point" = "xyes")
if test "x$disable_some_floating_point" = "xyes"; then
  AC_DEFINE(DISABLE_SOME_FLOATING_POINT, 1,
            [Define to 1 to disable certain code paths that rely heavily on
             double precision floating-point calculation])
fi

dnl ===========================================================================

# We use GTK+ for some utility/debugging tools
PKG_CHECK_MODULES(gtk, "gtk+-2.0",, AC_MSG_RESULT(no))

AC_CONFIG_FILES([
Makefile
boilerplate/Makefile
src/Makefile
test/Makefile
test/pdiff/Makefile
perf/Makefile
util/Makefile
doc/Makefile
doc/public/Makefile
])

AC_OUTPUT
CAIRO_REPORT
