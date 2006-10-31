import os
import sys
import fnmatch
from string import lower, split
import SCons
from SCons.Script.SConscript import SConsEnvironment
import shutil
import os.path


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


def Glob(self, match):
    """Similar to glob.glob, except globs SCons nodes, and thus sees
    generated files and files from build directories.  Basically, it sees
    anything SCons knows about.  A key subtlety is that since this function
    operates on generated nodes as well as source nodes on the filesystem,
    it needs to be called after builders that generate files you want to
    include."""
    def fn_filter(node):
        fn = str(node)
        return fnmatch.fnmatch(os.path.basename(fn), match)

    here = self.Dir('.')

    children = here.all_children()
    nodes = map(self.File, filter(fn_filter, children))
    node_srcs = [n.srcnode() for n in nodes]

    src = here.srcnode()
    if src is not here:
        src_children = map(self.File, filter(fn_filter, src.all_children()))
        for s in src_children:
            if s not in node_srcs:
                nodes.append(self.File(os.path.basename(str(s))))

    return nodes

def print_config(msg, two_dee_iterable):
    # this function is handy and can be used for other configuration-printing tasks
    print
    print msg
    print
    for key, val in two_dee_iterable:
        print "    %-20s %s" % (key, val)
    print

def AddSysPath(new_path):
	import sys, os

	# standardise
	new_path = os.path.abspath(new_path)

	# MS-Windows does not respect case
	if sys.platform == 'win32':
		new_path = new_path.lower()

	# disallow bad paths
	do = -1
	if os.path.exists(new_path):
		do = 1
		
		# check against all paths currently available
		for x in sys.path:
			x = os.path.abspath(x)
			if sys.platform == 'win32':
				x = x.lower()
			if new_path in (x, x + os.sep):
				do = 0

		# add path if we don't already have it
		if do:
			sys.path.append(new_path)
			pass

	return do

##
## Zipper.py
##
import distutils.archive_util

def zipperFunction(target, source, env):
        """Function to use as an action which creates a ZIP file from the arguments"""
        targetName = str(target[0])
        sourceDir = str(source[0])
        distutils.archive_util.make_archive(targetName, 'zip', sourceDir)


def accumulatorFunction(target, source, env):
  """Function called when builder is called"""
  destDir = str(target[0])
  if not os.path.exists(destDir):
      os.makedirs(destDir)
  for s in source:
      s = str(s)
      if os.path.isdir(s):
          myShutil.copytree(s, destDir, symlinks = False)
      else:
          shutil.copy2(s, destDir)

def copytree(src, dest, symlinks=False):
    """My own copyTree which does not fail if the directory exists.
    
    Recursively copy a directory tree using copy2().

    If the optional symlinks flag is true, symbolic links in the
    source tree result in symbolic links in the destination tree; if
    it is false, the contents of the files pointed to by symbolic
    links are copied.
    
    Behavior is meant to be identical to GNU 'cp -R'.    
    """
    def copyItems(src, dest, symlinks=False):
        """Function that does all the work.
        
        It is necessary to handle the two 'cp' cases:
        - destination does exist
        - destination does not exist
        
        See 'cp -R' documentation for more details
        """
        for item in os.listdir(src):
           srcPath = os.path.join(src, item)
           if os.path.isdir(srcPath):
               srcBasename = os.path.basename(srcPath)
               destDirPath = os.path.join(dest, srcBasename)
               if not os.path.exists(destDirPath):
                   os.makedirs(destDirPath)
               copyItems(srcPath, destDirPath)
           elif os.path.islink(item) and symlinks:
               linkto = os.readlink(item)
               os.symlink(linkto, dest)
           else:
               shutil.copy2(srcPath, dest)

    # case 'cp -R src/ dest/' where dest/ already exists
    if os.path.exists(dest):
       destPath = os.path.join(dest, os.path.basename(src))
       if not os.path.exists(destPath):
           os.makedirs(destPath)
    # case 'cp -R src/ dest/' where dest/ does not exist
    else:
       os.makedirs(dest)
       destPath = dest
    # actually copy the files
    copyItems(src, destPath)

