def addDynamicLinkerPath(env, path):
	env.PrependENVPath('LD_LIBRARY_PATH', path)
