#!/bin/bash

function indent()
{
	sed -e 's/^/\t/'
}

src_dir=$1

(
#------------------

cat <<!
set(XXX_srcs
$(find ${src_dir} -maxdepth 1 -name \*.cpp \! -name \*_test.cpp -printf '%f\n' | sort | indent)
)
make_absolute(XXX_srcs \${XXX_SOURCE_DIR})

!

# Header files
if ls ${src_dir}/*.h &> /dev/null ; then cat <<!
set(XXX_hdrs
$(find ${src_dir} -maxdepth 1 -name \*.h -printf '%f\n' | sort | indent)
)
make_absolute(XXX_hdrs \${XXX_SOURCE_DIR})

include_directories(\${XXX_SOURCE_DIR})

!
fi


if ls ${src_dir}/*_test.cpp &> /dev/null ; then cat <<!
set(XXX_test_srcs
$(find ${src_dir} -maxdepth 1 -name \*_test.cpp -printf '%f\n' | sort | indent)
)
make_absolute(XXX_test_srcs \${XXX_SOURCE_DIR})
!
fi


#----------------
) | sed -e "s/XXX/$(basename $src_dir)/g"
