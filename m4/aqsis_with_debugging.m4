########################################################
# AQSIS_WITH_DEBUGGING

AC_DEFUN([AQSIS_WITH_DEBUGGING], [

AC_ARG_WITH(debug, [  --with-debugging        compile with debugging [[default=no]]], [debug=$withval], [debug=no])

if test x$debug = xyes; then
	AC_DEFINE(_DEBUG, [], "Enable additional debug code.")
	AQSIS_DEBUG_CFLAGS="-g -Wall" 
	AQSIS_OPT_CFLAGS="-O0"
	AC_MSG_NOTICE([Debugging enabled])
else
	AC_DEFINE(NDEBUG, [], "Disable additional debug code.")
	AC_MSG_NOTICE([Debugging disabled])
fi
  
])

