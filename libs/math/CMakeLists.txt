project(math)

set(math_srcs
	cellnoise.cpp
	color.cpp
	lowdiscrep.cpp
	matrix.cpp
	noise.cpp
	noise1234.cpp
	random.cpp
	spline.cpp
)

set(math_test_srcs
	cellnoise_test.cpp
	color_test.cpp
	math_test.cpp
	matrix2d_test.cpp
	matrix_test.cpp
	noise1234_test.cpp
	noise_test.cpp
	spline_test.cpp
	vector2d_test.cpp
	vector3d_test.cpp
	vector4d_test.cpp
)

aqsis_add_library(aqsis_math ${math_srcs} ${math_hdrs}
	TEST_SOURCES ${math_test_srcs}
	COMPILE_DEFINITIONS AQSIS_MATH_EXPORTS
)

aqsis_install_targets(aqsis_math)
