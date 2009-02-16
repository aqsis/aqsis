SET(RIB2RI_SRCS
	${RIB2RI_SOURCE_DIR}/ribrequesthandler.cpp
	)
SET(RIB2RI_HDRS
	${RIB2RI_SOURCE_DIR}/rib2ri_share.h
	${RIB2RI_SOURCE_DIR}/ribrequesthandler.h
	)

# Deprecated, remove when new RIB parser is fully integrated.
SET(RIB2RI_SRCS_OLD
	${RIB2RI_SOURCE_DIR}/librib2ri.cpp
	)
SET(RIB2RI_HDRS_OLD
	${RIB2RI_SOURCE_DIR}/librib2ri.h
	)

ADD_DEFINITIONS(-DRIB2RI_EXPORTS)
INCLUDE_DIRECTORIES(${AQSIS_ZLIB_INCLUDE_DIR})
