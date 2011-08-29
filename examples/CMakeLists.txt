project(example_content)

#source_tree_relpath(examples_reldir)
#set(examples_install_dir ${share_install_dir}/${examples_reldir})
set(examples_install_dir ${EXAMPLESDIR})

set(dirs_to_install
	features procedurals scenes point_based_gi)

if(WIN32)
	install(DIRECTORY ${dirs_to_install}
		DESTINATION ${examples_install_dir}
		COMPONENT examples
		# TODO: Refactor the blobby content examples to allow distribution.  For
		# now we exclude them with a pattern.
		PATTERN "features/blobby" EXCLUDE
		PATTERN "features/pointcloud" EXCLUDE
		PATTERN "*.sh" EXCLUDE)
else()
	install(DIRECTORY ${dirs_to_install}
		DESTINATION ${examples_install_dir}
		COMPONENT examples
		# TODO: Refactor the blobby content examples to allow distribution.  For
		# now we exclude them with a pattern.
		PATTERN "features/blobby" EXCLUDE
		PATTERN "features/pointcloud" EXCLUDE
		PATTERN "*.bat" EXCLUDE
		PATTERN "*.sh"
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
				GROUP_EXECUTE GROUP_READ
				WORLD_EXECUTE WORLD_READ)
endif()

