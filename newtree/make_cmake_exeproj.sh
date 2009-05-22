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

# Generate extra files here.  Extra stuff in this directory is...
$(find ${src_dir} -maxdepth 1 -mindepth 1 \! \( -name \*.h -o -name \*.cpp \) | sort | sed -e 's/^/#/')

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

!
fi

cat <<!
aqsis_add_executable(XXX \${XXX_srcs} \${XXX_hdrs}
    LINK_LIBRARIES )

aqsis_install_targets(XXX)
!

#----------------
) | sed -e "s/XXX/$(basename $src_dir)/g"
