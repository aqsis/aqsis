########################################################
# AQSIS_WITH_FLTK

AC_DEFUN([AQSIS_WITH_FLTK], [
AC_SUBST(AQSIS_FLTK_CFLAGS, '-DAQSIS_NO_FLTK')
AC_SUBST(AQSIS_FLTK_LIBS)
AC_ARG_WITH(fltk, [  --with-fltk           build with FLTK based framebuffer  [[yes]]],[],[withval=yes]) 
if test x$withval != xno; then
	AC_MSG_CHECKING([for the fltk UI library])
	if fltk-config --version >/dev/null 2>/dev/null; then
		AQSIS_FLTK_VERSION=`fltk-config --version`
		AQSIS_FLTK_CFLAGS=`fltk-config --cflags`
		AQSIS_FLTK_LIBS=`fltk-config --ldflags`
		AC_MSG_RESULT([found version $AQSIS_FLTK_VERSION])
	else
		AC_MSG_ERROR([couldn't find fltk library ... if you don't require FLTK support, re-run configre with the --witout-fltk option])
	fi
fi
])

