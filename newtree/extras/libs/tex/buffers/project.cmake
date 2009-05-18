set(buffers_srcs
	imagechannel.cpp
	mixedimagebuffer.cpp
)
make_absolute(buffers_srcs ${buffers_SOURCE_DIR})

set(buffers_test_srcs
	channellist_test.cpp
	imagechannel_test.cpp
	mixedimagebuffer_test.cpp
)
make_absolute(buffers_test_srcs ${buffers_SOURCE_DIR})
