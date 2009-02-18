PROJECT(WIN32Dist)
#
# Create resource files for use during linking
#
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/info.rc.in.cmake" "${CMAKE_BINARY_DIR}/info.rc")
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/icon.rc.in.cmake" "${CMAKE_BINARY_DIR}/icon.rc")
SET(INFORES_SRCS "${CMAKE_BINARY_DIR}/info.rc")
SET(ICONRES_SRCS "${CMAKE_BINARY_DIR}/icon.rc")


