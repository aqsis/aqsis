########################################################
# AQSIS_WITH_FLTK

AC_DEFUN(AQSIS_WITH_FLTK, [
AC_SUBST(AQSIS_FLTK_CFLAGS)
AC_SUBST(AQSIS_FLTK_LIBS)
AC_ARG_WITH(fltk, [  --with-fltk        build components that require the fltk UI library [[yes]]],[],[withval=yes])
AM_CONDITIONAL(AQSIS_FLTK, test x$withval = xyes)
if test x$withval != xno; then
	AC_MSG_CHECKING([for fltk])
	if fltk-config --version >/dev/null 2>/dev/null; then
		AQSIS_FLTK_VERSION=`fltk-config --version`
		AQSIS_FLTK_CFLAGS=`fltk-config --cflags`
		AQSIS_FLTK_LIBS=`fltk-config --libs`
		AC_MSG_RESULT([found version $AQSIS_FLTK_VERSION])
	else
		AC_MSG_ERROR([couldn't find fltk library ... if you don't require fltk support, re-run configure using the --without-fltk option])
	fi
fi
])

