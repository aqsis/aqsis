Import('env')

# Check for the availability of various important files.
conf = Configure(env)

# Check for the TIFF libraries and headers
if not conf.CheckLibWithHeader('tiff', 'tiffio.h', 'c++'):
	print 'Cannot find libtiff'
	Exit(1)

# Check for the JPEG libraries and headers
if not conf.CheckLibWithHeader('jpeg', ['stdio.h', 'jpeglib.h'], 'c++'):
	print 'Cannot find libjpeg'
	Exit(1)

# Check for the zlib libraries and headers
if not conf.CheckLibWithHeader('z', 'zlib.h', 'c++'):
	print 'Cannot find zlib'
	Exit(1)

env = conf.Finish()
