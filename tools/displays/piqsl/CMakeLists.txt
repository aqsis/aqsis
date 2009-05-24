include_subproject(dspyutil)
include_subproject(tinyxml)

aqsis_add_display(piqsl_display piqsldisplay.cpp ${dspyutil_srcs}
	${tinyxml_srcs} ${tinyxml_hdrs}
	LINK_LIBRARIES aqsis_tex ${AQSIS_TINYXML_LIBRARY} ${CARBON_LIBRARY})
