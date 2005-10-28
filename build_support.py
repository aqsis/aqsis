import os
import sys
from string import lower, split
import SCons
from SCons.Script.SConscript import SConsEnvironment

def SelectBuildDir(build_dir, platform=None):
	# if no platform is specified, then default to sys.platform
	if not(platform):
		platform = sys.platform

	print "Looking for build directory for platform '%s'" % platform

	# setup where we start looking at first
	test_dir = build_dir + os.sep + platform
	default_dir = build_dir + os.sep + 'default'


	# we look for a directory named exactly after the
	# platform so that very specific builds can be done
	if os.path.exists(test_dir):
		# make sure it is a directory
		target_dir = test_dir
	else:
		print "Exact match not found, finding closest guess"

		# looks like there isn't an exact match
		# find the closest matching directory
		dirs = os.listdir(build_dir)
		found_match = 0
		for dir in dirs:
			if platform.find(dir) != -1:
				# found a match (hopefully the right one)
				target_dir = build_dir + os.sep + dir
				found_match = 1
				break
		if not(found_match):
			print "No match found, looking for 'default' directory"
			# looks like this platform isn't available
			# try the default target
			if os.path.exists(default_dir):
				target_dir = default_dir
			else:
				# bad, nothing is available, tell the user
				print "No build directories found for your platform '%s'" % platform
				return None

	print "Found directory %s, will build there" % target_dir
	return target_dir


def UseTargetOptions(self, target_name):
	for value in split("""
		LIBS 
		CPPFLAGS 
		CPPDEFINES
		CPPPATH
		CCFLAGS
		SHCCFLAGS
		SHCXXFLAGS
		CXXFLAGS
		LIBPATH
		LINKFLAGS
		SHLINKFLAGS"""):
		if self.has_key(target_name + '_' + lower(value)):
			self.AppendUnique(**{value: self['' + target_name + '_' + lower(value)]})

SConsEnvironment.UseTargetOptions = UseTargetOptions
