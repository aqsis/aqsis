project(teqser)

set(teqser_srcs
	teqser.cpp
)

aqsis_add_executable(teqser ${teqser_srcs}
    LINK_LIBRARIES aqsis_util aqsis_core)

aqsis_install_targets(teqser)
