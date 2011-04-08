cmake_minimum_required(VERSION 2.6)
find_package(Qt4 COMPONENTS QtGui QtOpenGL REQUIRED)
find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
include_directories(${QT_INCLUDES} ${ILMBASE_INCLUDE_DIR})

set(CMAKE_BUILD_TYPE Release)

qt4_wrap_cpp(moc_srcs ptview.h)

add_executable(ptview ptview.cpp ${moc_srcs})
target_link_libraries(ptview ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY})
