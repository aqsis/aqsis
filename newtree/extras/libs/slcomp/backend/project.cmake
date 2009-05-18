set(backend_srcs
	codegengraphviz.cpp
	codegenvm.cpp
	parsetreeviz.cpp
	vmdatagather.cpp
	vmoutput.cpp
)
make_absolute(backend_srcs ${backend_SOURCE_DIR})

set(backend_hdrs
	parsetreeviz.h
	vmdatagather.h
	vmoutput.h
)
make_absolute(backend_hdrs ${backend_SOURCE_DIR})
