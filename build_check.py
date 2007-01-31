Import('conf')

# Store the current LIBS setting, as the tests will modify it, but we don't want
# every module to have the LiBS for all libraries.

libs_store = []
cpppath_store = []
if conf.env.has_key('LiBS'):
	libs_store = conf.env['LIBS']
if conf.env.has_key('CPPPATH'):
	cpppath_store = conf.env['CPPPATH']

# Check for the availability of various important files.

# Check for the TIFF libraries and headers
conf.env.AppendUnique(CPPPATH = ['$tiff_include_path'])
print 'Checking for libtiff'
if not conf.CheckLibWithHeader(conf.env.subst('$tiff_lib'), 'tiffio.h', 'c++'):
	print 'Cannot find libtiff'
	Exit(1)
conf.env.Replace(CPPPATH = cpppath_store)

# Check for the JPEG libraries and headers
conf.env.AppendUnique(CPPPATH = ['$jpeg_include_path'])
print 'Checking for libjpeg'
if not conf.CheckLibWithHeader(conf.env.subst('$jpeg_lib'), ['stdio.h', 'jpeglib.h'], 'c++'):
	print 'Cannot find libjpeg'
	Exit(1)
conf.env.Replace(CPPPATH = cpppath_store)

# Check for the zlib libraries and headers
conf.env.AppendUnique(CPPPATH = ['$zlib_include_path'])
print 'Checking for zlib'
if not conf.CheckLibWithHeader(conf.env.subst('$z_lib'), 'zlib.h', 'c++'):
	print 'Cannot find zlib'
	Exit(1)
conf.env.Replace(CPPPATH = cpppath_store)

# Check for the FLTK libraries and headers
conf.env.AppendUnique(CPPPATH = ['$fltk_include_path'])
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
conf.env.Replace(CPPPATH = cpppath_store)

# Check for the OpenEXR libraries and headers
conf.env.AppendUnique(CPPPATH = ['$exr_include_path'])
if not conf.env['no_exr']:
	print 'Checking for OpenEXR'
	if not conf.CheckLibWithHeader(conf.env.subst('$exr_lib'), ['stdio.h', 'half.h'], 'c++'):
		print 'Cannot find OpenEXR - proceeding with no OpenEXR support.'
		conf.env['no_exr'] = True
		conf.env.Replace(exr_lib = '')
else:
	conf.env.Replace(exr_lib = '')

conf.env.Replace(CPPPATH = cpppath_store)
conf.env.Replace(LIBS = libs_store)
