AC_INIT()
dnl AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(aqsis, 0.9.2)
AM_CONFIG_HEADER(libaqsistypes/config.h)

AC_DEFINE(STRNAME,"Aqsis", [Application Name] )
AC_PREFIX_DEFAULT(/usr/local/aqsis)

AC_ARG_ENABLE(debug,
[  --enable-debug          compile with debugging [default=no]],
[case "${enableval}" in
  true) debug=true ;;
  yes) debug=true ;;
  false) debug=false ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],
[debug=false])
if test x$debug = xtrue ; then
  AC_DEFINE(_DEBUG,[],"Enable additional debug code.")
  DEBUG_FLAGS="-g -Wall" 
  OPT_FLAGS="-O0"
else
  AC_DEFINE(NDEBUG,[],"Disable C99 asserts.")
  DEBUG_FLAGS="" 
  OPT_FLAGS=""
fi
  
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

AC_ARG_ENABLE(profile,
[  --enable-profile          compile with profiling [default=no]],
[case "${enableval}" in
  true) profile=true ;;
  yes) profile=true ;;
  false) profile=false ;;
  no)  profile=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-profile) ;;
esac],
[profile=false])
if test x$profile = xtrue ; then
  DEBUG_FLAGS="-g -Wall -pg -ftest-coverage -fprofile-arcs" 
  OPT_FLAGS="-O0"
  LDFLAGS="-lc_p"
fi

AM_CONDITIONAL(PROFILE, test x$profile = xtrue)


AC_ARG_ENABLE(plugins,
[  --enable-plugins        compile with plugin support [default=yes, requires shared lib support]],
[case "${enableval}" in
  true) plugins=true ;;
  yes) plugins=true ;;
  false) plugins=false ;;
  no) plugins=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-plugins) ;;
esac],

[plugins=true])
if test x$plugins = xtrue ; then
  PLUGIN_FLAGS="-ldl"; AC_DEFINE(PLUGINS,,"Supprt dynamically loaded plugins") 
else
  PLUGIN_FLAGS=""
fi
AM_CONDITIONAL(PLUGINS, test x$plugins = xtrue)
AC_SUBST(PLUGIN_FLAGS)


AC_PROG_CXX
AM_ENABLE_SHARED
AC_DISABLE_STATIC
LT_AC_PROG_SED
AC_PROG_LIBTOOL
AM_PROG_LEX
AC_PROG_YACC
AC_EXEEXT

dnl Needed by librib2/bdec.cpp. Define WORDS_BIGENDIAN.
AC_C_BIGENDIAN

dnl Check for OpenGL and glut
MDL_HAVE_OPENGL
AM_CONDITIONAL(OPENGL, test x$have_glut = xyes)
if test x$have_glut = xno ; then
	AC_MSG_WARN([*** Could not find GLUT, glut based framebuffer devices will not be built ***]);
fi

dnl Socket libraries
NET_LIBS=""
AC_CHECK_LIB(socket, socket, NET_LIBS="-lsocket $NET_LIBS",)
AC_CHECK_LIB(nsl, gethostbyname, NET_LIBS="-lnsl $NET_LIBS",)
AC_SUBST(NET_LIBS)

dnl TIFF library
saveCFLAGS=$CFLAGS
saveLIBS=$LIBS
HAVE_LIBTIFF_ARGS
CFLAGS=$TIFF_INC
LIBS=$TIFF_FLAGS
AC_CHECK_HEADER(tiff.h)
AC_CHECK_LIB(m, pow)
AC_CHECK_LIB(tiff, TIFFOpen)
CFLAGS=$saveCFLAGS
LIBS=$saveLIBS
if test ! "$ac_cv_header_tiff_h" || test ! "ac_cv_lib_tiff_TIFFOpen" ; then
        AC_MSG_ERROR([*** Please ensure you have libtiff libraries and headers installed ***]);
fi

dnl ZLIB library
saveCFLAGS=$CFLAGS
saveLIBS=$LIBS
HAVE_LIBZ_ARGS
CFLAGS=$ZLIB_INC
LIBS=$ZLIB_FLAGS
AC_CHECK_HEADER(zlib.h)
AC_CHECK_LIB(z,gzopen)
CFLAGS=$saveCFLAGS
LIBS=$saveLIBS
if test ! "$ac_cv_header_zlib_h" || test ! "ac_cv_lib_z_gzopen" ; then
        AC_MSG_ERROR([*** Please ensure you have zlib libraries and headers  installed ***]);
fi

dnl See if we have xsltproc for rebuilding the ri.inl and ri_cache.inl
AC_CHECK_PROG(have_xsltproc, xsltproc, yes, no)
if test x$have_xsltproc = "xno" ; then
	AC_MSG_WARN([*** Could not find xsltproc, you will need it if you edit any of the xml or xsl files ***]);
fi

AC_SUBST(AQSIS_CPPFLAGS)
AC_SUBST(AQSIS_CXXFLAGS)
AC_SUBST(AQSIS_CFLAGS)
AQSIS_CPPFLAGS="-DBASE_PATH=\\\"\"\${pkgdatadir}\\\"\" -DCONFIG_PATH=\\\"\"$sysconfdir\\\"\""
AQSIS_CXXFLAGS="$DEBUG_FLAGS $OPT_FLAGS"
AQSIS_CFLAGS="$DEBUG_FLAGS $OPT_FLAGS"

AC_OUTPUT( \
all/Makefile \
all/AdditionalDeveloperLibs/Makefile \
aqsis/Makefile \
aqsl/Makefile \
aqsl/tests/Makefile \
boost/Makefile \
boost/boost/Makefile \
boost/boost/bind/Makefile \
boost/boost/config/Makefile \
boost/boost/config/compiler/Makefile \
boost/boost/config/platform/Makefile \
boost/boost/config/stdlib/Makefile \
boost/boost/detail/Makefile \
boost/boost/filesystem/Makefile \
boost/boost/mpl/Makefile \
boost/boost/mpl/aux_/Makefile \
boost/boost/mpl/aux_/config/Makefile \
boost/boost/mpl/aux_/preprocessed/Makefile \
boost/boost/mpl/aux_/preprocessed/bcc/Makefile \
boost/boost/mpl/aux_/preprocessed/bcc551/Makefile \
boost/boost/mpl/aux_/preprocessed/gcc/Makefile \
boost/boost/mpl/aux_/preprocessed/msvc60/Makefile \
boost/boost/mpl/aux_/preprocessed/msvc70/Makefile \
boost/boost/mpl/aux_/preprocessed/mwcw/Makefile \
boost/boost/mpl/aux_/preprocessed/no_ctps/Makefile \
boost/boost/mpl/aux_/preprocessed/no_ttp/Makefile \
boost/boost/mpl/aux_/preprocessed/plain/Makefile \
boost/boost/mpl/aux_/preprocessor/Makefile \
boost/boost/mpl/aux_/range_c/Makefile \
boost/boost/mpl/limits/Makefile \
boost/boost/mpl/list/Makefile \
boost/boost/mpl/list/aux_/Makefile \
boost/boost/mpl/list/aux_/preprocessed/Makefile \
boost/boost/mpl/list/aux_/preprocessed/plain/Makefile \
boost/boost/mpl/math/Makefile \
boost/boost/mpl/vector/Makefile \
boost/boost/mpl/vector/aux_/Makefile \
boost/boost/mpl/vector/aux_/preprocessed/Makefile \
boost/boost/mpl/vector/aux_/preprocessed/no_ctps/Makefile \
boost/boost/mpl/vector/aux_/preprocessed/plain/Makefile \
boost/boost/mpl/vector/aux_/preprocessed/typeof_based/Makefile \
boost/boost/multi_array/Makefile \
boost/boost/pending/Makefile \
boost/boost/pending/detail/Makefile \
boost/boost/pool/Makefile \
boost/boost/pool/detail/Makefile \
boost/boost/preprocessor/Makefile \
boost/boost/preprocessor/arithmetic/Makefile \
boost/boost/preprocessor/arithmetic/detail/Makefile \
boost/boost/preprocessor/array/Makefile \
boost/boost/preprocessor/comparison/Makefile \
boost/boost/preprocessor/config/Makefile \
boost/boost/preprocessor/control/Makefile \
boost/boost/preprocessor/control/detail/Makefile \
boost/boost/preprocessor/control/detail/edg/Makefile \
boost/boost/preprocessor/control/detail/msvc/Makefile \
boost/boost/preprocessor/debug/Makefile \
boost/boost/preprocessor/detail/Makefile \
boost/boost/preprocessor/facilities/Makefile \
boost/boost/preprocessor/iteration/Makefile \
boost/boost/preprocessor/iteration/detail/Makefile \
boost/boost/preprocessor/iteration/detail/bounds/Makefile \
boost/boost/preprocessor/iteration/detail/iter/Makefile \
boost/boost/preprocessor/list/Makefile \
boost/boost/preprocessor/list/detail/Makefile \
boost/boost/preprocessor/list/detail/edg/Makefile \
boost/boost/preprocessor/logical/Makefile \
boost/boost/preprocessor/punctuation/Makefile \
boost/boost/preprocessor/repetition/Makefile \
boost/boost/preprocessor/repetition/detail/Makefile \
boost/boost/preprocessor/repetition/detail/edg/Makefile \
boost/boost/preprocessor/repetition/detail/msvc/Makefile \
boost/boost/preprocessor/selection/Makefile \
boost/boost/preprocessor/seq/Makefile \
boost/boost/preprocessor/seq/detail/Makefile \
boost/boost/preprocessor/slot/Makefile \
boost/boost/preprocessor/slot/detail/Makefile \
boost/boost/preprocessor/tuple/Makefile \
boost/boost/random/Makefile \
boost/boost/random/detail/Makefile \
boost/boost/regex/Makefile \
boost/boost/regex/v3/Makefile \
boost/boost/spirit/Makefile \
boost/boost/spirit/attribute/Makefile \
boost/boost/spirit/core/Makefile \
boost/boost/spirit/core/composite/Makefile \
boost/boost/spirit/core/composite/impl/Makefile \
boost/boost/spirit/core/impl/Makefile \
boost/boost/spirit/core/meta/Makefile \
boost/boost/spirit/core/meta/impl/Makefile \
boost/boost/spirit/core/non_terminal/Makefile \
boost/boost/spirit/core/non_terminal/impl/Makefile \
boost/boost/spirit/core/primitives/Makefile \
boost/boost/spirit/core/primitives/impl/Makefile \
boost/boost/spirit/core/scanner/Makefile \
boost/boost/spirit/core/scanner/impl/Makefile \
boost/boost/spirit/debug/Makefile \
boost/boost/spirit/debug/impl/Makefile \
boost/boost/spirit/dyic/Makefile \
boost/boost/spirit/dyic/impl/Makefile \
boost/boost/spirit/dynamic/Makefile \
boost/boost/spirit/dynamic/impl/Makefile \
boost/boost/spirit/error_handling/Makefile \
boost/boost/spirit/error_handling/impl/Makefile \
boost/boost/spirit/iterator/Makefile \
boost/boost/spirit/iterator/impl/Makefile \
boost/boost/spirit/phoenix/Makefile \
boost/boost/spirit/symbols/Makefile \
boost/boost/spirit/symbols/impl/Makefile \
boost/boost/spirit/tree/Makefile \
boost/boost/spirit/tree/impl/Makefile \
boost/boost/spirit/utility/Makefile \
boost/boost/spirit/utility/impl/Makefile \
boost/boost/spirit/utility/impl/chset/Makefile \
boost/boost/tuple/Makefile \
boost/boost/tuple/detail/Makefile \
boost/boost/type_traits/Makefile \
boost/boost/type_traits/detail/Makefile \
boost/boost/utility/Makefile \
boost/libs/Makefile \
boost/libs/filesystem/Makefile \
boost/libs/filesystem/src/Makefile \
boost/libs/mpl/Makefile \
boost/libs/mpl/preprocessed/Makefile \
boost/libs/mpl/preprocessed/list/Makefile \
boost/libs/mpl/preprocessed/src/Makefile \
boost/libs/mpl/preprocessed/vector/Makefile \
boost/libs/regex/Makefile \
boost/libs/regex/src/Makefile \
filebuffer/Makefile \
framebuffer_glut/Makefile \
framebuffer_glut_z/Makefile \
images/Makefile \
libaqsistypes/Makefile \
libaqsistypes/posix/Makefile \
libaqsistypes/win32/Makefile \
libaqsistypes/win32/intel/Makefile \
libcodegenvm/Makefile \
libdd/Makefile \
libddmsimple/Makefile \
libddmsock/Makefile \
libraytrace/Makefile \
libri2rib/Makefile \
librib2/Makefile \
librib2/tests/Makefile \
librib2ri/Makefile \
librib2stream/Makefile \
libslparse/Makefile \
render/Makefile \
render/macosx/Makefile \
render/macosx/std/Makefile \
ribfiles/Makefile \
shaders/Makefile \
shadowmap/Makefile \
slpp/Makefile \
teqser/Makefile \
aqsltell/Makefile \
plugins/Makefile \
plugins/common/Makefile \
plugins/gif2tif/Makefile \
plugins/jpg2tif/Makefile \
plugins/ppm2tif/Makefile \
plugins/pcx2tif/Makefile \
plugins/tga2tif/Makefile \
plugins/bake2tif/Makefile \
libslxargs/Makefile \
libshadervm/Makefile \
libshaderexecenv/Makefile \
libargparse/Makefile \
dsotest/Makefile \
Makefile )

