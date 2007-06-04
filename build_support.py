import os, re
import sys
import fnmatch
from string import lower, split
import SCons
from SCons.Script.SConscript import SConsEnvironment
import xml.dom.minidom
import glob
import StringIO

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

def embedManifest(env, targetenv, source, type):
	'''Embed manifests for a DLL or EXE if VC8 is used
	type is either DLL or EXE (as a string)
	'''
	suffix = '1' # EXE
	
	if (type == 'DLL'):
		suffix = '2' # DLL
		
	if ('win32' == env['PLATFORM']) and (env['MSVS_VERSION'] == '8.0'):
		targetenv.AddPostAction(source, 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;' + suffix)

def getSCMRevision():
	scm_command = 'svn info --xml ' + os.getcwd()

	if os.system(scm_command) <= 0:
		scm_resource = os.popen(scm_command).read()
		scm_resourcepp = xml.dom.minidom.parseString(scm_resource)
		scm_revision = scm_resourcepp.getElementsByTagName('entry')[0].attributes['revision']
		scm_revisionpp = scm_revision.value
		return "(revision " + scm_revisionpp + ")"
	else:
		return ""

def checkBoostLibraries(conf, lib, pathes):
	''' look for boost libraries '''
	conf.Message('Checking for boost library %s... ' % lib)
	for path in pathes:
		# direct form: e.g. libboost_iostreams.a
		if os.path.isfile(os.path.join(path, 'lib%s.a' % lib)):
			conf.Result('yes')
			return (path, lib)
		# check things like libboost_iostreams-gcc.a
		files = glob.glob(os.path.join(path, 'lib%s-*.a' % lib))
		# if there are more than one, choose the first one
		# FIXME: choose the best one.
		if len(files) >= 1:
			# get xxx-gcc from /usr/local/lib/libboost_xxx-gcc.a
			conf.Result('yes')
			return (path, files[0].split(os.sep)[-1][3:-2])
	conf.Result('n')
	return ('','')


class TerminalController:
    """
    A class that can be used to portably generate formatted output to
    a terminal.  
    
    `TerminalController` defines a set of instance variables whose
    values are initialized to the control sequence necessary to
    perform a given action.  These can be simply included in normal
    output to the terminal:

        >>> term = TerminalController()
        >>> print 'This is '+term.GREEN+'green'+term.NORMAL

    Alternatively, the `render()` method can used, which replaces
    '${action}' with the string required to perform 'action':

        >>> term = TerminalController()
        >>> print term.render('This is ${GREEN}green${NORMAL}')

    If the terminal doesn't support a given action, then the value of
    the corresponding instance variable will be set to ''.  As a
    result, the above code will still work on terminals that do not
    support color, except that their output will not be colored.
    Also, this means that you can test whether the terminal supports a
    given action by simply testing the truth value of the
    corresponding instance variable:

        >>> term = TerminalController()
        >>> if term.CLEAR_SCREEN:
        ...     print 'This terminal supports clearning the screen.'

    Finally, if the width and height of the terminal are known, then
    they will be stored in the `COLS` and `LINES` attributes.
    """
    # Cursor movement:
    BOL = ''             #: Move the cursor to the beginning of the line
    UP = ''              #: Move the cursor up one line
    DOWN = ''            #: Move the cursor down one line
    LEFT = ''            #: Move the cursor left one char
    RIGHT = ''           #: Move the cursor right one char

    # Deletion:
    CLEAR_SCREEN = ''    #: Clear the screen and move to home position
    CLEAR_EOL = ''       #: Clear to the end of the line.
    CLEAR_BOL = ''       #: Clear to the beginning of the line.
    CLEAR_EOS = ''       #: Clear to the end of the screen

    # Output modes:
    BOLD = ''            #: Turn on bold mode
    BLINK = ''           #: Turn on blink mode
    DIM = ''             #: Turn on half-bright mode
    REVERSE = ''         #: Turn on reverse-video mode
    NORMAL = ''          #: Turn off all modes

    # Cursor display:
    HIDE_CURSOR = ''     #: Make the cursor invisible
    SHOW_CURSOR = ''     #: Make the cursor visible

    # Terminal size:
    COLS = None          #: Width of the terminal (None for unknown)
    LINES = None         #: Height of the terminal (None for unknown)

    # Foreground colors:
    BLACK = BLUE = GREEN = CYAN = RED = MAGENTA = YELLOW = WHITE = ''
    
    # Background colors:
    BG_BLACK = BG_BLUE = BG_GREEN = BG_CYAN = ''
    BG_RED = BG_MAGENTA = BG_YELLOW = BG_WHITE = ''
    
    _STRING_CAPABILITIES = """
    BOL=cr UP=cuu1 DOWN=cud1 LEFT=cub1 RIGHT=cuf1
    CLEAR_SCREEN=clear CLEAR_EOL=el CLEAR_BOL=el1 CLEAR_EOS=ed BOLD=bold
    BLINK=blink DIM=dim REVERSE=rev UNDERLINE=smul NORMAL=sgr0
    HIDE_CURSOR=cinvis SHOW_CURSOR=cnorm""".split()
    _COLORS = """BLACK BLUE GREEN CYAN RED MAGENTA YELLOW WHITE""".split()
    _ANSICOLORS = "BLACK RED GREEN YELLOW BLUE MAGENTA CYAN WHITE".split()

    def __init__(self, term_stream=sys.stdout):
        """
        Create a `TerminalController` and initialize its attributes
        with appropriate values for the current terminal.
        `term_stream` is the stream that will be used for terminal
        output; if this stream is not a tty, then the terminal is
        assumed to be a dumb terminal (i.e., have no capabilities).
        """
        # Curses isn't available on all platforms
        try: import curses
        except: return

        # If the stream isn't a tty, then assume it has no capabilities.
        if not term_stream.isatty(): return

        # Check the terminal type.  If we fail, then assume that the
        # terminal has no capabilities.
        try: curses.setupterm()
        except: return

        # Look up numeric capabilities.
        self.COLS = curses.tigetnum('cols')
        self.LINES = curses.tigetnum('lines')
        
        # Look up string capabilities.
        for capability in self._STRING_CAPABILITIES:
            (attrib, cap_name) = capability.split('=')
            setattr(self, attrib, self._tigetstr(cap_name) or '')

        # Colors
        set_fg = self._tigetstr('setf')
        if set_fg:
            for i,color in zip(range(len(self._COLORS)), self._COLORS):
                setattr(self, color, curses.tparm(set_fg, i) or '')
        set_fg_ansi = self._tigetstr('setaf')
        if set_fg_ansi:
            for i,color in zip(range(len(self._ANSICOLORS)), self._ANSICOLORS):
                setattr(self, color, curses.tparm(set_fg_ansi, i) or '')
        set_bg = self._tigetstr('setb')
        if set_bg:
            for i,color in zip(range(len(self._COLORS)), self._COLORS):
                setattr(self, 'BG_'+color, curses.tparm(set_bg, i) or '')
        set_bg_ansi = self._tigetstr('setab')
        if set_bg_ansi:
            for i,color in zip(range(len(self._ANSICOLORS)), self._ANSICOLORS):
                setattr(self, 'BG_'+color, curses.tparm(set_bg_ansi, i) or '')

    def _tigetstr(self, cap_name):
        # String capabilities can include "delays" of the form "$<2>".
        # For any modern terminal, we should be able to just ignore
        # these, so strip them out.
        import curses
        cap = curses.tigetstr(cap_name) or ''
        return re.sub(r'\$<\d+>[/*]?', '', cap)

    def render(self, template):
        """
        Replace each $-substitutions in the given template string with
        the corresponding terminal control string (if it's defined) or
        '' (if it's not).
        """
        return re.sub(r'\$\$|\${\w+}', self._render_sub, template)

    def _render_sub(self, match):
        s = match.group()
        if s == '$$': return s
        else: return getattr(self, s[2:-1])


# need an Environment and a matching buffered_spawn API .. encapsulate
class idBuffering:
        def buffered_spawn( self, sh, escape, cmd, args, env ):
                stderr = StringIO.StringIO()
                stdout = StringIO.StringIO()
                command_string = ''
                for i in args:
                        if ( len( command_string ) ):
                                command_string += ' '
                        command_string += i
                try:
                        retval = self.env['PSPAWN']( sh, escape, cmd, args, env, stdout, stderr )
                except OSError, x:
                        if x.errno != 10:
                                raise x
                        print 'OSError ignored on command: %s' % command_string
                        retval = 0
                #print command_string
                #sys.stdout.write( stdout.getvalue() )
                #sys.stderr.write( stderr.getvalue() )
                return retval

# get a clean error output when running multiple jobs
def SetupBufferedOutput( env ):
        buf = idBuffering()
        buf.env = env
        env['SPAWN'] = buf.buffered_spawn

def print_cmd_line(s, target, src, env):
	#if env != None and env.has_key['CMDLINE_QUIET'] and env['CMDLINE_QUIET']:
	term = TerminalController()
	print term.render(" ${GREEN}%s...${NORMAL}"%(' and '.join([str(x) for x in target])))
	#sys.stdout.write(" %s...\n"%(' and '.join([str(x) for x in target])))
		# Save real cmd to log file
	open(env['CMD_LOGFILE'], 'a').write("%s\n"%s);
	#else:
	#	sys.stdout.write("%s\n"%s);

def SetupCleanPrinting( env ):
	env['PRINT_CMD_LINE_FUNC'] = print_cmd_line
	if not env.has_key('CMD_LOGFILE') or ['CMD_LOGFILE'] == '':
		env['CMD_LOGFILE'] = 'build-log.txt'

