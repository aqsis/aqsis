project(example_content)

#source_tree_relpath(examples_reldir)
#set(examples_install_dir ${share_install_dir}/${examples_reldir})
set(examples_install_dir ${EXAMPLESDIR})

install(DIRECTORY features procedurals scenes
	DESTINATION ${examples_install_dir}
	# TODO: Refactor the blobby content examples to allow distribution.  For
	# now we exclude them with a pattern.
	PATTERN features/blobby EXCLUDE
)

