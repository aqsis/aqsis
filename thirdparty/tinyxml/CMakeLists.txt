PROJECT(TINYXML)

# The tinyxml code may be used in one of two ways depending on the value of
# the variable AQSIS_USE_EXTERNAL_TINYXML:
# 1) The code in this directory is compiled and statically linked into the
#    parts of aqsis which use it.  This is the default option.
# 2) The build system searches for an externally installed version of tinyxml
#    (named something like libtinyxml.so) and uses that instead of the
#    version included here.  This feature was added at the request of the
#    Fedora linux package maintainer, Nicolas Chauvet (kwizart), and may be
#    useful for other linux distros which want to avoid duplicated libs.
