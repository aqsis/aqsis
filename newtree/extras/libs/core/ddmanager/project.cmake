set(ddmanager_srcs
	ddmanager.cpp
	debugdd.cpp
)
make_absolute(ddmanager_srcs ${ddmanager_SOURCE_DIR})

set(ddmanager_hdrs
	ddmanager.h
	debugdd.h
	iddmanager.h
)
make_absolute(ddmanager_hdrs ${ddmanager_SOURCE_DIR})

include_directories(${ddmanager_SOURCE_DIR})

