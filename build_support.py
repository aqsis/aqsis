import os
import sys
import fnmatch
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

	print "Found configuration directory %s, will use that" % target_dir
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


def Glob(match):
    """Similar to glob.glob, except globs SCons nodes, and thus sees
    generated files and files from build directories.  Basically, it sees
    anything SCons knows about.  A key subtlety is that since this function
    operates on generated nodes as well as source nodes on the filesystem,
    it needs to be called after builders that generate files you want to
    include."""
    def fn_filter(node):
        fn = str(node)
        return fnmatch.fnmatch(os.path.basename(fn), match)

    here = Dir('.')

    children = here.all_children()
    nodes = map(File, filter(fn_filter, children))
    node_srcs = [n.srcnode() for n in nodes]

    src = here.srcnode()
    if src is not here:
        src_children = map(File, filter(fn_filter, src.all_children()))
        for s in src_children:
            if s not in node_srcs:
                nodes.append(File(os.path.basename(str(s))))

    return nodes

def print_config(msg, two_dee_iterable):
    # this function is handy and can be used for other configuration-printing tasks
    print
    print msg
    print
    for key, val in two_dee_iterable:
        print "    %-20s %s" % (key, val)
    print

Export('Glob SelectBuildDir print_config')
