########################################################
# AQSIS_WITH_PROFILING

AC_DEFUN([AQSIS_WITH_PROFILING], [

AC_ARG_WITH(profiling, [  --with-profiling        compile with profiling [[default=no]]], [profile=$withval], [profile=no])

dnl NOTE: Many systems do not have libc compiled with profiling.  We
dnl       should add a test for this library before including it in
dnl       the libs, otherwise configure fails with some strange
dnl       warning about the compiler not being able to produce
dnl       executables!!!
dnl Default to O2 optimisations for profiling.  It typically makes sense to
dnl profile optimised rather than non-optimised code, because the results
dnl can be different, and the end-user normally runs optimised code.

if test x$profile = xyes; then
	AQSIS_PROFILE_CFLAGS="-g -Wall -pg -ftest-coverage -fprofile-arcs -O2"
	AQSIS_PROFILE_LDFLAGS="-pg"
	AC_MSG_NOTICE([Profiling enabled])
else
	AC_MSG_NOTICE([Profiling disabled])
fi
  
dnl AM_CONDITIONAL(PROFILE, test x$profile = xyes)

])

