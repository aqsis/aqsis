########################################################
# AQSIS_CHECK_FLTK

AC_DEFUN([AQSIS_CHECK_FLTK], [
AC_SUBST(AQSIS_FLTK_CFLAGS)
AC_SUBST(AQSIS_FLTK_LIBS)
AC_MSG_CHECKING([for the fltk UI library])
if fltk-config --version >/dev/null 2>/dev/null; then
	AQSIS_FLTK_VERSION=`fltk-config --version`
	AQSIS_FLTK_CFLAGS=`fltk-config --cflags`
	AQSIS_FLTK_LIBS=`fltk-config --ldflags`
	AC_MSG_RESULT([found version $AQSIS_FLTK_VERSION])
else
	AC_MSG_ERROR([couldn't find fltk library])
fi
])

