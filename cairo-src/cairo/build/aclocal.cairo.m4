dnl ==========================================================================
dnl
dnl Cairo-specific macros
dnl

dnl ==========================================================================

dnl Usage:
dnl   CAIRO_BIGENDIAN
dnl
AC_DEFUN([CAIRO_BIGENDIAN],
[dnl
	case $host_os in
		darwin*)
	AH_VERBATIM([X_BYTE_ORDER],
[
/* Deal with multiple architecture compiles on Mac OS X */
#ifdef __APPLE_CC__
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#define FLOAT_WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#undef FLOAT_WORDS_BIGENDIAN
#endif
#endif
])
		;;
		*) 
	AC_C_BIGENDIAN
	AX_C_FLOAT_WORDS_BIGENDIAN
		;;
	esac
])

dnl CAIRO_CHECK_FUNCS_WITH_FLAGS(FUNCTION..., CFLAGS, LIBS
dnl                              [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Like AC_CHECK_FUNCS but with additional CFLAGS and LIBS
dnl --------------------------------------------------------------------
AC_DEFUN([CAIRO_CHECK_FUNCS_WITH_FLAGS],
[dnl 
	_save_cflags="$CFLAGS"
	_save_libs="$LIBS"
	CFLAGS="$CFLAGS $2"
	LIBS="$LIBS $3"
	AC_CHECK_FUNCS($1, $4, $5)
	CFLAGS="$_save_cflags"
	LIBS="$_save_libs"
])

dnl CAIRO_CONFIG_COMMANDS is like AC_CONFIG_COMMANDS, except that:
dnl
dnl	1) It redirects the stdout of the command to the file.
dnl	2) It does not recreate the file if contents didn't change.
dnl
AC_DEFUN([CAIRO_CONFIG_COMMANDS],
[dnl
	AC_CONFIG_COMMANDS($1,
	[
		_config_file=$1
		_tmp_file=cairoconf.tmp
		AC_MSG_NOTICE([creating $_config_file])
		{
			$2
		} >> "$_tmp_file" ||
	        AC_MSG_ERROR([failed to write to $_tmp_file])

		if cmp -s "$_tmp_file" "$_config_file"; then
		  AC_MSG_NOTICE([$_config_file is unchanged])
		  rm -f "$_tmp_file"
		else
		  mv "$_tmp_file" "$_config_file" ||
	          AC_MSG_ERROR([failed to update $_config_file])
		fi
	], $3)
])

dnl check compiler flags
AC_DEFUN([CAIRO_CC_TRY_FLAG],
[dnl
	AC_MSG_CHECKING([whether $CC supports $1])

	_save_cflags="$CFLAGS"
	CFLAGS="$CFLAGS -Werror $1"
	AC_COMPILE_IFELSE([ ], [cairo_cc_flag=yes], [cairo_cc_flag=no])
	CFLAGS="$_save_cflags"

	if test "x$cairo_cc_flag" = "xyes"; then
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
	AC_MSG_RESULT([$cairo_cc_flag])
])

dnl check compiler/ld flags
AC_DEFUN([CAIRO_CC_TRY_LINK_FLAG],
[dnl
	AC_MSG_CHECKING([whether $CC supports $1])

	_save_cflags="$CFLAGS"
	CFLAGS="$CFLAGS -Werror $1"
	AC_LINK_IFELSE([int main(void){ return 0;} ],
                       [cairo_cc_flag=yes],
                       [cairo_cc_flag=no])
	CFLAGS="$_save_cflags"

	if test "x$cairo_cc_flag" = "xyes"; then
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
	AC_MSG_RESULT([$cairo_cc_flag])
])

dnl Usage:
dnl   CAIRO_CHECK_NATIVE_ATOMIC_PRIMITIVES
AC_DEFUN([CAIRO_CHECK_NATIVE_ATOMIC_PRIMITIVES],
[dnl
	AC_CACHE_CHECK([for native atomic primitives], cairo_cv_atomic_primitives,
	[
		cairo_cv_atomic_primitives="none"

		AC_TRY_LINK([
int atomic_add(int i) { return __sync_fetch_and_add (&i, 1); }
int atomic_cmpxchg(int i, int j, int k) { return __sync_val_compare_and_swap (&i, j, k); }
], [],
		  cairo_cv_atomic_primitives="Intel"
		  )
	])
	if test "x$cairo_cv_atomic_primitives" = xIntel; then
		AC_DEFINE(HAVE_INTEL_ATOMIC_PRIMITIVES, 1,
			  [Enable if your compiler supports the Intel __sync_* atomic primitives])
	fi
])

dnl Usage:
dnl   CAIRO_CHECK_ATOMIC_OP_NEEDS_MEMORY_BARRIER
AC_DEFUN([CAIRO_CHECK_ATOMIC_OP_NEEDS_MEMORY_BARRIER],
[dnl
	AC_CACHE_CHECK([whether atomic ops require a memory barrier], cairo_cv_atomic_op_needs_memory_barrier,
	[
		case $host_cpu in
		    i?86)	cairo_cv_atomic_op_needs_memory_barrier="no"  ;;
		    x86_64)	cairo_cv_atomic_op_needs_memory_barrier="no"  ;;
		    arm*)	cairo_cv_atomic_op_needs_memory_barrier="no"  ;;
		    *)		cairo_cv_atomic_op_needs_memory_barrier="yes" ;;
		esac
	])
	if test "x$cairo_cv_atomic_op_needs_memory_barrier" = "xyes"; then
	    AC_DEFINE_UNQUOTED(ATOMIC_OP_NEEDS_MEMORY_BARRIER, 1,
			       [whether memory barriers are needed around atomic operations])
	fi
])

AC_DEFUN([CAIRO_TEXT_WRAP], [m4_text_wrap([$1], [$2],, 78)])
