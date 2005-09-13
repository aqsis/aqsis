Import('env')

# Check for the availability of various important files.
conf = Configure(env)
if not conf.CheckLibWithHeader('tiff', 'tiffio.h', 'c++'):
	print 'Cannot find libtiff'
	Exit(1)
env = conf.Finish()
