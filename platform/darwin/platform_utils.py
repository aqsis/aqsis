def addDynamicLinkerPath(env, path):
	env.PrependENVPath('DYLD_LIBRARY_PATH', path)
