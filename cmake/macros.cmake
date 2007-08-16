# Filter out entries from a list.
MACRO(FILTER_OUT FILTERS INPUTS OUTPUT)
   # Mimicks Gnu Make's $(filter-out) which removes elements 
   # from a list that match the pattern.
   # Arguments:
   #  FILTERS - list of patterns that need to be removed
   #  INPUTS  - list of inputs that will be worked on
   #  OUTPUT  - the filtered list to be returned
   # 
   # Example: 
   #  SET(MYLIST this that and the other)
   #  SET(FILTS this that)
   #
   #  FILTER_OUT("${FILTS}" "${MYLIST}" OUT)
   #  MESSAGE("OUTPUT = ${OUT}")
   #
   # The output - 
   #   OUTPUT = and;the;other
   #
   SET(FOUT "")
   FOREACH(INP ${INPUTS})
	   SET(FILTERED 0)
	   FOREACH(FILT ${FILTERS})
		   IF(${FILTERED} EQUAL 0)
			   IF("${FILT}" STREQUAL "${INP}")
				   SET(FILTERED 1)
			   ENDIF("${FILT}" STREQUAL "${INP}")
		   ENDIF(${FILTERED} EQUAL 0)
	   ENDFOREACH(FILT ${FILTERS})
	   IF(${FILTERED} EQUAL 0)
		   SET(FOUT ${FOUT} ${INP})
	   ENDIF(${FILTERED} EQUAL 0)
   ENDFOREACH(INP ${INPUTS})
   SET(${OUTPUT} ${FOUT})
ENDMACRO(FILTER_OUT FILTERS INPUTS OUTPUT)


