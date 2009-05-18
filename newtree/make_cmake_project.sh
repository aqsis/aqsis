#!/bin/bash

function indent()
{
	sed -e 's/^/\t/'
}

src_dir=$1

(
#------------------

cat <<!
project(XXX)

set(XXX_srcs
$(find ${src_dir} -maxdepth 1 -name \*.cpp \! -name \*_test.cpp -printf '%f\n' | sort | indent)
)

!

# Header files
if ls ${src_dir}/*.h &> /dev/null ; then cat <<!
set(XXX_hdrs
$(find ${src_dir} -maxdepth 1 -name \*.h -printf '%f\n' | sort | indent)
)
source_group("Header Files" FILES \${XXX_hdrs})

# Optional subproject
add_subproject(some_subproj)
include_subproject(some_subproj)

!
fi

cat <<!

# Generate extra files here.  Extra stuff in this directory is...
$(find ${src_dir} -maxdepth 1 -mindepth 1 \! \( -name \*.h -o -name \*.cpp \) | sort | sed -e 's/^/#/')

# Add #include's 
include_directories()
# Optional macro definitions for non-SHARE macros.
add_definitions()

aqsis_add_library(target_? \${XXX_srcs} \${XXX_hdrs}
	COMPILE_DEFINITIONS ?_EXPORTS
	DEPENDS ?
	LINK_LIBRARIES ?
)

aqsis_install_targets(target_?)

!
# For executable...
#add_executable(target_? \${XXX_srcs} \${XXX_hdrs})
#target_link_libraries(target_? lib_?)

if ls ${src_dir}/*_test.cpp &> /dev/null ; then cat <<!
#--------------------------------------------------
# Testing
set(XXX_test_srcs
$(find ${src_dir} -maxdepth 1 -name \*_test.cpp -printf '%f\n' | sort | indent)
)

aqsis_add_tests(\${XXX_test_srcs} LINK_LIBRARIES target_?)
!
fi


#----------------
) | sed -e "s/XXX/$(basename $src_dir)/g"
