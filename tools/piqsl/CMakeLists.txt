project(piqsl)

# FIXME: Should do this in the main CmakeLists.txt
find_package(Qt4 COMPONENTS QtGui)

# Check for boost thread
if(NOT Boost_THREAD_FOUND)
    message(FATAL_ERROR "Piqsl requires boost thread to build")
endif()

include_subproject(tinyxml)

#set(piqsl_srcs
#    book.cpp
#    bookbrowser.cpp
#    centerscroll.cpp
#    displayserverimage.cpp
#    image.cpp
#    pane.cpp
#    piqsl.cpp
#    piqslmainwindow.cpp
#    zoomimage.cpp
#    ${tinyxml_srcs}
#)
#
#set(piqsl_hdrs
#    book.h
#    bookbrowser.h
#    displayserverimage.h
#    image.h
#    piqsl.h
#    piqslmainwindow.h
#    centerscroll.h
#    pane.h
#    zoomimage.h
#    ${tinyxml_hdrs}
#)
#source_group("Header Files" FILES ${piqsl_hdrs})

set(piqsl_hdrs
    piqsl2.h
    image.h
    imagelistmodel.h
    ${tinyxml_hdrs}
)

qt4_wrap_cpp(moc_srcs
    piqsl2.h
    imagelistmodel.h
)

set(piqsl_srcs
    piqsl2.cpp
    image.cpp
    imagelistmodel.cpp
    ${moc_srcs}
    ${tinyxml_srcs}
)


include_directories(${QT_INCLUDES})

aqsis_add_executable(piqsl ${piqsl_srcs} ${piqsl_hdrs} GUIAPP
    LINK_LIBRARIES aqsis_util aqsis_tex ${QT_QTGUI_LIBRARY}
        ${Boost_THREAD_LIBRARY} ${AQSIS_TINYXML_LIBRARY})

aqsis_install_targets(piqsl)
