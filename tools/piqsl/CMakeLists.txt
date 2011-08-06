project(piqsl)

# Check for boost thread
if(NOT Boost_THREAD_FOUND OR NOT QT_FOUND)
    message(FATAL_ERROR "Piqsl requires boost thread and Qt4 to build")
endif()

include_subproject(tinyxml)

set(piqsl_hdrs
    displayserverimage.h
    image.h
    imagelistmodel.h
    piqsl_ui.h
    ${tinyxml_hdrs}
)
source_group("Header Files" FILES ${piqsl_hdrs})

qt4_wrap_cpp(moc_srcs
    displayserverimage.h
    image.h
    imagelistmodel.h
    piqsl_ui.h
)

set(piqsl_srcs
    displayserverimage.cpp
    image.cpp
    imagelistmodel.cpp
    piqsl_ui.cpp
    piqsl.cpp
    ${moc_srcs}
    ${tinyxml_srcs}
)

include_directories(${QT_INCLUDES})

aqsis_add_executable(piqsl ${piqsl_srcs} ${piqsl_hdrs} GUIAPP
    LINK_LIBRARIES aqsis_util aqsis_tex ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY}
        ${Boost_THREAD_LIBRARY} ${AQSIS_TINYXML_LIBRARY})

aqsis_install_targets(piqsl)
