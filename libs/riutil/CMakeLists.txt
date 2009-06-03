project(riutil)

set(riutil_srcs
	primvartoken.cpp
	riparamlist.cpp
	tokendictionary.cpp
)

aqsis_add_library(aqsis_riutil ${riutil_srcs} ${riutil_hdrs}
	COMPILE_DEFINITIONS AQSIS_RIUTIL_EXPORTS
	LINK_LIBRARIES aqsis_util
)

aqsis_install_targets(aqsis_riutil)


#--------------------------------------------------
# Testing
set(riutil_test_srcs
	primvartoken_test.cpp
)
aqsis_add_tests(${riutil_test_srcs} LINK_LIBRARIES aqsis_riutil)
