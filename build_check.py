Import('env conf')

# Check for the availability of various important files.

# Check for the TIFF libraries and headers
if not conf.CheckLibWithHeader(env.subst('$tiff_lib'), 'tiffio.h', 'c++'):
	print 'Cannot find libtiff'
	Exit(1)

# Check for the JPEG libraries and headers
if not conf.CheckLibWithHeader(env.subst('$jpeg_lib'), ['stdio.h', 'jpeglib.h'], 'c++'):
	print 'Cannot find libjpeg'
	Exit(1)

# Check for the zlib libraries and headers
if not conf.CheckLibWithHeader(env.subst('$z_lib'), 'zlib.h', 'c++'):
	print 'Cannot find zlib'
	Exit(1)

