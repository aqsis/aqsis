Import('conf')

# Check for the availability of various important files.

# Check for the TIFF libraries and headers
if not conf.CheckLibWithHeader(conf.env.subst('$tiff_lib'), 'tiffio.h', 'c++'):
	print 'Cannot find libtiff'
	Exit(1)

# Check for the JPEG libraries and headers
if not conf.CheckLibWithHeader(conf.env.subst('$jpeg_lib'), ['stdio.h', 'jpeglib.h'], 'c++'):
	print 'Cannot find libjpeg'
	Exit(1)

# Check for the zlib libraries and headers
if not conf.CheckLibWithHeader(conf.env.subst('$z_lib'), 'zlib.h', 'c++'):
	print 'Cannot find zlib'
	Exit(1)

# Check for the FLTK libraries and headers
if not conf.CheckLibWithHeader(conf.env.subst('$fltk_lib'), ['stdio.h', 'FL/Fl.h'], 'c++'):
	print 'Cannot find FLTK'
	conf.env.Replace(display_cppdefines = ['AQSIS_NO_FLTK'])
	conf.env.Replace(fltk_lib = '')

# Check for the OpenEXR libraries and headers
if not conf.CheckLibWithHeader(conf.env.subst('$exr_lib'), ['stdio.h', 'OpenEXR/half.h'], 'c++'):
	print 'Cannot find OpenEXR'
	conf.env['no_exr'] = True
	conf.env.Replace(exr_lib = '')

