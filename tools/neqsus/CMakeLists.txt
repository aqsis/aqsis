project(neqsus)

set(neqsus_install_dir ${NEQSUSDIR})

if(WIN32)
	install(DIRECTORY houdini
		DESTINATION ${neqsus_install_dir}
		COMPONENT plugins
		PATTERN "*.sh" EXCLUDE)
else()
	install(DIRECTORY houdini
		DESTINATION ${neqsus_install_dir}
		COMPONENT plugins
		PATTERN "*.bat" EXCLUDE
		PATTERN "*.sh"
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
				GROUP_EXECUTE GROUP_READ
				WORLD_EXECUTE WORLD_READ)
endif()

