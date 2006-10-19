Import('conf')

# Check for the availability of various important files.

# Check for the TIFF libraries and headers
print 'Checking for libtiff'
if not conf.CheckLibWithHeader(conf.env.subst('$tiff_lib'), 'tiffio.h', 'c++'):
	print 'Cannot find libtiff'
	Exit(1)

# Check for the JPEG libraries and headers
print 'Checking for libjpeg'
if not conf.CheckLibWithHeader(conf.env.subst('$jpeg_lib'), ['stdio.h', 'jpeglib.h'], 'c++'):
	print 'Cannot find libjpeg'
	Exit(1)

# Check for the zlib libraries and headers
print 'Checking for zlib'
if not conf.CheckLibWithHeader(conf.env.subst('$z_lib'), 'zlib.h', 'c++'):
	print 'Cannot find zlib'
	Exit(1)

# Check for the FLTK libraries and headers
if not conf.env['no_fltk']:
	print 'Checking for FLTK'
	if not conf.CheckLibWithHeader(conf.env.subst('$fltk_lib'), ['stdio.h', 'FL/Fl.H'], 'c++'):
		print 'Cannot find FLTK - proceeding with no FLTK support.'
		conf.env['no_fltk'] = True
		conf.env.Replace(display_cppdefines = ['AQSIS_NO_FLTK'])
		conf.env.Replace(fltk_lib = '')
else:
	conf.env.Replace(display_cppdefines = ['AQSIS_NO_FLTK'])
	conf.env.Replace(fltk_lib = '')

# Check for the OpenEXR libraries and headers
if not conf.env['no_exr']:
	print 'Checking for OpenEXR'
	if not conf.CheckLibWithHeader(conf.env.subst('$exr_lib'), ['stdio.h', 'half.h'], 'c++'):
		print 'Cannot find OpenEXR - proceeding with no OpenEXR support.'
		conf.env['no_exr'] = True
		conf.env.Replace(exr_lib = '')
else:
	conf.env.Replace(exr_lib = '')

