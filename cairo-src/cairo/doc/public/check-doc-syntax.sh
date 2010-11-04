#!/bin/sh

LANG=C

test -z "$srcdir" && srcdir=.
test -z "$top_srcdir" && top_srcdir=$srcdir/../..

SGML_DOCS=true
FILES=`echo $srcdir/tmpl/*.sgml`

. "$top_srcdir/src/check-doc-syntax.sh"
