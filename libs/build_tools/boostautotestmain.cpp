#include <aqsis/aqsis.h>

// for boost.test version 1.33
#define BOOST_AUTO_TEST_MAIN

// for boost.test version 1.34
#define BOOST_TEST_MAIN

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>
