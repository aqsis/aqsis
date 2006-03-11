#!/usr/bin/env python
######################################################################
# Renderer tester v1.11
# Copyright 2003 Matthias Baas (baas@ira.uka.de)
######################################################################
# For a short usage description call the script with the option -h or
# --help. To get a description of the config files see the comments
# in renderers.cfg, jobs.cfg, tasks.cfg
#
# If you start the script without any arguments the default tests are
# executed (the output goes into the directory "html").
######################################################################

import sys, os, os.path, time, string, shutil, getopt, copy, fnmatch, gzip
import copy
import Image, ImageChops

#try:
#    from wxPython.wx import *
#    wxInitAllImageHandlers()
#    has_wx = 1
#except:
#    has_wx = 0
has_wx = 0

##### Global variables #####

# A list of all available jobs (RenderJob objects)
AllJobs = []
# A list of all the tasks to do (Task objects)
Tasks = []
# A list of all renderer definitions (Renderer objects)
Renderers = []
# If not None only jobs are accepted that match a RIB name from this list.
IncludeRIBs = None
# Exclude any RIBs from this list
ExcludeRIBs = []

# A dictionary with RenderResult objects (key: (renderer,job))
ResultCache = {}

# Directories (Stack) where config files are located
# The current file is at the end of the list
CurrentConfigDir = [""]

# TIFF conversion command (INPUT and OUTPUT will be replaced by the
# corresponding file names)
TIFFConvCmd = "tifftopnm INPUT|pnmtotiff -color -truecolor >OUTPUT"
TIFFConvCmd = ""

# AUTO keyword for the "ricalls" parameter in job descriptions
AUTO = "_auto"

######################################################################

# jobAllowed
def jobAllowed(job):
    """Return True if the job should be processed."""
    global IncludeRIBs, ExcludeRIBs

    # Was the job excluded?
    for pattern in ExcludeRIBs:
        if fnmatch.fnmatch(job.rib, pattern):
            return False

    # Include everything?
    if IncludeRIBs==None:
        return True

    # Was the job included?
    for pattern in IncludeRIBs:
        if fnmatch.fnmatch(job.rib, pattern):
            return True
        
    return False       
        

# loadImage
def loadImage(name):
    """Loads and returns a PIL image or None."""
    global TIFFConvCmd

    normname = os.path.normpath(name)
    # Check if the file exists
    if not os.path.exists(normname):
        print 'Image "%s" does not exist!'%name
        return None
    
    # Try loading with PIL...
    try:
        img = Image.open(normname)
        return img
    except:
        if TIFFConvCmd=="":
            print 'Could not load image "%s". No conversion command specified.'%os.path.basename(name)
        else:
            # Try to convert the image and then load again using PIL...
            path,name = os.path.split(normname)
            tmpname = os.path.join(path,"_tmp.tif")
            try:
                os.remove(tmpname)
            except:
                pass
            cmd = TIFFConvCmd.replace("INPUT", normname).replace("OUTPUT", tmpname)
            print cmd
            os.system(cmd)
            if not os.path.exists(tmpname):
                print 'Error while loading image "%s" via PIL. An attempt to filter the image using the netpbm tools failed.'%name
            else:
                try:
                    img = Image.open(tmpname)
                    try:
                        os.remove(tmpname)
                    except:
                        pass
                    return img
                except:
                    print 'Error while loading filtered image file "%s"'%name
            
        
        # Try loading with wxPython...
#        if has_wx:
#            try:
#                wximg = wxImage(normname)
#                data = wximg.GetData()
#                size = (wximg.GetWidth(), wximg.GetHeight())
#                img = Image.fromstring("RGB",size,data)
#                return img
#            except:
#                pass
            
    return None


# tiff_conv_cmd
def tiff_conv_cmd(cmd):
    """Set a new conversion command."""
    global TIFFConvCmd
    TIFFConvCmd = cmd

# include_cfg
def include_cfg(cfg, recursive=False):
    """Read one or more config files.

    The argument cfg points to a config file which is read. If recursive
    is False a warning is printed if the file doesn't exist. If recursive
    is True no warning is printed and the function tries to read
    config files with the same name as cfg in every subdirectory.

    The function sets the global variable CurrentConfigDir to the
    directory where the config file is located.
    """
    global CurrentConfigDir

    # Check if the given name refers to a directory
    if os.path.isdir(cfg):
        print 'Error: "%s" is a directory. You have to add a config file name.'%cfg
        return

    # Read config file...
    cfgpath, cfgname = os.path.split(cfg)
    if os.path.isfile(cfg):
        print 'Reading config file "%s"'%cfg
        CurrentConfigDir.append(os.path.abspath(cfgpath))
        execfile(cfg, globals())
        CurrentConfigDir.pop()
    elif not recursive:
        print 'Warning: Config file "%s" not found'%cfg

    # Recursively read config files in subdirectories...
    if recursive:
        if os.path.isdir(cfgpath):
            for dir in os.listdir(cfgpath):
                subdir = os.path.join(cfgpath, dir)
                include_cfg(os.path.join(subdir, cfgname), True)
                
######################################################################

# RenderJob
class RenderJob:
    """This class holds the input data required to render one RIB file."""
    
    def __init__(self, workingdir="", rib="", shaders=[], outimagename=None,
                 ricalls=[AUTO], description="", known_issues=None):
        global AllJobs, IncludeRIBs, ExcludeRIBs, CurrentConfigDir

        workingdir = os.path.join(CurrentConfigDir[-1], workingdir)

        # No image name given? Then use RIB name and replace suffix
        if outimagename==None:
            base,ext = os.path.splitext(rib)
            outimagename = [base+".tif"]
        # String? Then create a list with this name
        elif isinstance(outimagename,str) or isinstance(outimagename,unicode):
            outimagename = [outimagename]
        # Convert to list...
        else:
            outimagename = list(outimagename)

        self.workingdir = workingdir
        self.rib = rib
        self.shaders = shaders
        # outimagename: List of output image names
        self.outimagename = outimagename
        self.ricalls = ricalls
        self.description = description
        self.known_issues = known_issues

        self.noref = False

        # Add job to the global list if it's allowed..
#        if ((IncludeRIBs==None or rib in IncludeRIBs)
#            and (rib not in ExcludeRIBs)):
        if jobAllowed(self):
            AllJobs.append(self)

    def acquireRIB(self):
        """Prepare the output stream and return the effective RIB name.

        If the input file of the job is actually a RIB (*.rib or *.gz)
        then the method does nothing and just returns the rib name.
        Otherwise, the job is actually a program which is executed and
        piped into a temporary file. In this case the method returns
        the temporary file name.
        releaseRIB() has to be called when the RIB is no longer needed.

        Precondition: The current directory must be set to the
        working directory.
        """
        # Check if the input file is a RIB or a program....
        ribname = self.rib
        n,ext = os.path.splitext(ribname)
        ext = ext.lower()
        # No RIB? Then execute the file (which has to output a RIB stream
        # to stdout)...
        if ext not in [".rib", ".gz"]:
            ribname = "_tmp.rib"
            cmd = "%s >%s"%(self.rib, ribname)
            os.system(cmd)
        return ribname

    def releaseRIB(self):
        """Release a RIB file that was previously acquired.

        May only be called after acquireRIB() was called.
        """

        n,ext = os.path.splitext(self.rib)
        ext = ext.lower()
        # Remove temporary RIB again...
        if ext not in [".rib", ".gz"]:
            try:
                os.remove("_tmp.rib")
            except:
                pass

        

######################################################################

# RenderResult
class RenderResult:
    """This class contains the result of a rendering.
    """

    def __init__(self, rendererobject, job, 
                 renderer, r_version, shadercompiler, sc_version,
                 shaderinfo, date, rendertime, realoutimagename,
                 stdout_buf, stderr_buf):

        # Renderer object
        self.rendererobject = rendererobject
        # Job that was rendered
        self.job = job
        # Name of the renderer
        self.renderer = renderer
        # Version of the renderer
        self.r_version = r_version
        # Name of the shading language compiler
        self.shadercompiler = shadercompiler
        # Version of the shading language compiler
        self.sc_version = sc_version
        # A list of tuples (shadername, outbuf, errbuf)
        self.shaderinfo = shaderinfo
        # Start date (as time tuple, see module "time")
        self.date = date
        # Render time (in seconds) 
        self.rendertime = rendertime
        # Real output image names
        self.realoutimagename = realoutimagename
        # The output to stdout as a list of strings
        self.stdout_buf = stdout_buf
        # The output to stderr as a list of strings      
        self.stderr_buf = stderr_buf

    def __str__(self):
        rversion = string.join(self.r_version.strip().split("\n")," / ")
        scversion = string.join(self.sc_version.strip().split("\n")," / ")
        
        s = "<RenderResult>\n"
        s += "Renderer    : %s\n"%self.renderer
        s += "Version     : %s\n"%rversion
        s += "SL Compiler : %s\n"%self.shadercompiler
        s += "Version     : %s\n"%scversion
        s += "Date        : %s\n"%time.asctime(self.date)
        s += "Render time : %1.1fs\n"%self.rendertime
        s += "----------------------stdout-------------------------------\n"
        s += "%s\n"%string.join(self.stdout_buf,"").strip()
        s += "----------------------stderr-------------------------------\n"
        s += "%s\n"%string.join(self.stderr_buf,"").strip()
        s += "-----------------------------------------------------------\n"
        return s

    # loadOutputImage
    def loadOutputImage(self, idx=0):
        """Loads and returns a PIL image or None."""

#        print "loadOutputImage(%d) -> %s"%(idx,self.realoutimagename[idx])
        return loadImage(self.realoutimagename[idx])

    # renderTimeStr
    def renderTimeStr(self):
        secs = self.rendertime%60
        min  = int(self.rendertime/60)
        h    = int(min/60)
        min  = min%60
        if h==0 and min==0:
            if self.rendertime<10:
                return "%1.1fs"%(self.rendertime)
            else:
                return "%1.0fs"%(round(self.rendertime))
        elif h==0:
            return "%dm %ds"%(min,secs)
        else:
            return "%dh %dm %ds"%(h,min,secs)
        


######################################################################

# Renderer
class Renderer:
    """Renderer class.

    This class represents a particular RenderMan renderer. It can compile
    shaders and start a render job.

    To do a rendering you call the method render() which returns
    a RenderResult object.
    
    """

    def __init__(self, shortname,
                 renderer, shadercompiler, rversionopt=None, scversionopt=None,
                 environ = []):
        """Constructor.

        shortname - Name of the renderer package (will be used as part of
                    filenames)
        renderer - Name of the renderer command
        shadercompiler - Name of the shading language compiler
        rversionopt - Option string that has to be specified to get the version
        scversionopt - Same as above but for the SL compiler
        """
        global Renderers

        Renderers.append(self)

        self.shortname = shortname

        self.renderer = renderer
        self.shadercompiler = shadercompiler

        self.rversionopt  = rversionopt
        self.scversionopt = scversionopt

        self.environ = environ

    # render
    def render(self, job):
        """Render a RIB file and return a RenderResult object."""

        # Change into working dir...
        olddir = os.getcwd()
        try:
            os.chdir(job.workingdir)
        except OSError,e:
            print e
            # Return an empty "dummy" result
            res = RenderResult(rendererobject = self,
                               job = job,
                               renderer = self.renderer,
                               r_version = self.rendererVersion(),
                               shadercompiler = self.shadercompiler,
                               sc_version = self.shaderCompilerVersion(),
                               shaderinfo = [],
                               date = time.localtime(),
                               rendertime = 0.0,
                               realoutimagename = job.outimagename,
                               stdout_buf = [],
                               stderr_buf = [])
            return res
            

        # Compile shaders
        shinfo = []
        for sh in job.shaders:
            out,err = self.compileShader(sh)
            shinfo.append((sh,out,err))

        start_date = time.localtime()
        print '%s: Rendering "%s" using %s...'%(time.asctime(start_date),job.rib, self.shortname)
        t_start = time.time()

        # Delete any output image that is still lying around...
        for oin in job.outimagename:
            if os.path.exists(oin):
                print 'Removing output image "%s" (before rendering)...'%oin
                os.remove(oin)

        # Get the effective RIB name...
        ribname = job.acquireRIB()

        # Set environment variables...
        for var,val in self.environ:
            print 'Setting environment variable "%s" to "%s"'%(var,val)
            os.environ[var]=val
        # Render...
        cmd = "%s %s"%(self.renderer, ribname)
        out,err = self._execute(cmd)

        # Remove temporary RIB again...
        job.releaseRIB()

        t_end = time.time()

        # Rename output file(s)
        realnames = []
        for oin in job.outimagename:
            if not os.path.exists(oin):
                print 'Image "%s" was not created (please check the renderer output)!'%oin
            src = os.path.abspath(oin)
            path,name = os.path.split(src)
            dst = os.path.join(path,self.shortname+"_"+name)
            realnames.append(dst)
            try:
                os.remove(dst)
            except:
                pass
            try:
                os.rename(src,dst)
            except:
                pass
#                print "Can't rename"

        res = RenderResult(rendererobject = self,
                           job = job,
                           renderer = self.renderer,
                           r_version = self.rendererVersion(),
                           shadercompiler = self.shadercompiler,
                           sc_version = self.shaderCompilerVersion(),
                           shaderinfo = shinfo,
                           date = start_date,
                           rendertime = t_end-t_start,
                           realoutimagename = realnames,
                           stdout_buf = out,
                           stderr_buf = err)

        os.chdir(olddir)
        return res
                           


    # compileShader
    def compileShader(self, shader):
        """Compile a shader and return the stdout/stderr buffers.
        """

        print '%s: Compiling "%s" using %s...'%(time.asctime(),shader,self.shortname)

        shaderdir,shname = os.path.split(shader)
        if shaderdir=="":
            shaderdir="."

        olddir = os.getcwd()
        try:
            os.chdir(shaderdir)
        except OSError, e:
            print e

        cmd = "%s %s"%(self.shadercompiler, shname)
        out,err = self._execute(cmd)

        os.chdir(olddir)        
        return out,err

    # shaderCompilerVersion
    def shaderCompilerVersion(self):
        """Return the version string of the shading language compiler."""
        
        if self.scversionopt==None:
            return "<unknown>"
        
        cmd = "%s %s"%(self.shadercompiler, self.scversionopt)
        out,err = self._execute(cmd)
        return string.join(out,"")

    # rendererVersion
    def rendererVersion(self):
        """Return the version string of the renderer."""

        if self.rversionopt==None:
            return "<unknown>"

        cmd = "%s %s"%(self.renderer, self.rversionopt)
        out,err = self._execute(cmd)
        return string.join(out,"")

    
    def _execute(self, cmd):
        """Execute a command and return the output to stdout and stderr.

        The output is returned as a list of strings (each list element is
        one line).
        """

        outbuf = []
        errbuf = []

        # Execute the command and redirect the output
        cmd += " >_stdout.tmp 2>_stderr.tmp"
#        print 'Executing "%s"...'%cmd
        os.system(cmd)

        stdout = file("_stdout.tmp")
        stderr = file("_stderr.tmp")

        # Start executing the command...
#        stdin, stdout, stderr = os.popen3(cmd)

        # Read stdout...
        while 1:
            s = stdout.readline()
            if s=="":
                break
            outbuf.append(s)
        
        # Read stderr...
        while 1:
            s = stderr.readline()
            if s=="":
                break
            errbuf.append(s)

        stdout.close()
        stderr.close()
        # Clean up the directory...
        # (try several times if removing didn't succeed at first as it
        # has already happened that the above close wasn't finished yet...?)
        trials_left = 3
        while trials_left>0:
            try:
                os.remove("_stdout.tmp")
                os.remove("_stderr.tmp")
                trials_left = 0
            except OSError, e:
                time.sleep(0.1)
                trials_left -= 1
                if trials_left==0:
                    print e

        return outbuf, errbuf

######################################################################

class HTMLPage:
    """A simple wrapper around HTML pages."""

    def __init__(self, filename, title):
        self.fhandle = file(filename, "w")
        self.fhandle.write("""
<html><head><title>%s</title></head>
<body>\n"""%(title))

    def __del__(self):
        if self.fhandle!=None:
            self.close()

    def close(self):
        self.fhandle.write("""</body></html>""")
        self.fhandle.close()
        self.fhandle = None

    def flush(self):
        self.fhandle.flush()

    def section(self, title):
        self.fhandle.write("<h1>%s</h1>\n"%title)

    def subsection(self, title):
        self.fhandle.write("<h2>%s</h2>\n"%title)

    def subsubsection(self, title):
        self.fhandle.write("<h3>%s</h3>\n"%title)

    def write(self, rawtxt):
        self.fhandle.write(rawtxt+"\n")
        

######################################################################
    
# Task
class Task:
    """Base class for all tasks.

    A task is something this program has to accomplish. It usually involves
    rendering jobs, do something with the result and output a HTML page
    that displays those results.

    Each derived class has to implement the run() method and can make use
    of some helper methods (see below).
    """

    def __init__(self):
        global Tasks
        Tasks.append(self)

    # run
    def run(self, htmldir):
        """Do whatever the task is supposed to do.

        Each task has to implement at least this method. The argument
        htmldir is the directory where the generated html report should
        be stored.
        The return value of the method is either the name of the generated
        html page (relative to htmldir) or a tuple with that name and
        a description text (as html) which will be put after the link
        to the page.
        """
        pass
 
    # render
    def render(self, renderer, job):
        """Render an image and return a RenderResult object.

        If the particular job was already rendered by the given renderer
        then a cached RenderResult object is returned.
        """
        global ResultCache

        # Was the job already completed before?
        if (renderer,job) in ResultCache:
#            print "Using cached result"
            return ResultCache[(renderer,job)]

        # Render...
        res = renderer.render(job)
        ResultCache[(renderer,job)] = res

        return res

    # saveResAsWeb
    def saveResAsWeb(self, res, htmldir, idx=0, imgname=None):
        """Save the output image from a RenderResult as a web image.

        Saves the output image in the pics directory and returns a
        string that can be used in the html page ('<img ...>' or
        '[Image not available]'). 

        res: RenderResult object
        htmldir: Base directory where the output pages are stored
        idx:     Index of the output image to use
        imgname: Raw image name (without path), an extension is allowed
                 but will be replaced by ".png". If this parameter isn't
                 specified, then a default name will be used which is
                 the output image name with the renderer name as prefix.
        """

        # No name given then use default name (Renderer+output name)
        if imgname==None:
            imgname = "%s_%s"%(res.rendererobject.shortname, res.job.outimagename[idx])

        img = res.loadOutputImage(idx)
        return self.saveImgAsWeb(img,htmldir,imgname)

    # saveImgAsWeb
    def saveImgAsWeb(self, img, htmldir, imgname):
        """Save a PIL image as a web image.

        Saves the image in the pics directory and returns a
        string that can be used in the html page ('<img ...>' or
        '[Image not available]' if img was None). 

        img:     PIL image or None.
        htmldir: Base directory where the output pages are stored
        imgname: Raw image name (without path), an extension is allowed
                 but will be replaced by ".png".
        """

        # Replace suffix
        base,ext = os.path.splitext(imgname)
        imgname = base+".png"

        # Picture directory...
        picpath = os.path.join(htmldir, "pics")
        if not os.path.exists(picpath):
            os.mkdir(picpath)

        # Check if the image is already there
        pimgname = os.path.join(picpath, imgname)
        if not os.path.exists(pimgname):
            if img!=None:
                img.save(pimgname)
            else:
                return "[Image not available]"

        imgref = os.path.join("pics", imgname)
        imgref = imgref.replace("\\","/")
        return '<img src="%s">'%(imgref)

    # convertJobList
    def convertJobList(self, jobs):
        """Converts strings in a job list.

        The argument jobs must be a list of either RenderJob objects
        or strings specifying a RIB name.
        The return value is a list that only contains RenderJob objects.
        """
        global AllJobs
        
        res = []
        for j in jobs:
            # String? Then search for job with corresponding RIB name
            if type(j)==str:
                for job in AllJobs:
                    if job.rib==j:
                        j = job
                        break
                else:
                    raise ValueError, 'Job "%s" not found!'%j
            res.append(j)
            
        return res

######################################################################

# JobOverview
class JobOverview(Task):
    """Renders all jobs and creates an index/overview page."""

    def __init__(self, renderer, jobs=AllJobs):
        """Constructor.

        renderer - Renderer object that'll be used to do the overview.
        jobs - Jobs that'll be included in the overview.
        """
        Task.__init__(self)
        self.renderer = renderer
        self.jobs = self.convertJobList(jobs)

    def htmlBaseName(self):
        return "overview"

    def run(self, htmldir):

        htmlname = "overview.html"

        print "Creating overview using %s..."%(self.renderer.shortname)

        html = HTMLPage(os.path.join(htmldir, htmlname), "Job overview")
        html.section("Job overview")


        html.write("<table border=1 cellspacing=2 cellpadding=2>")
        html.write("<td><b>RIB</b></td>")
        html.write("<td><b>Thumbnail</b></td>")
        html.write("<td><b>Description</b></td>")
        html.write("<tr>")
        for job in self.jobs:
            res = self.render(self.renderer, job)
            img = res.loadOutputImage()
            if img!=None:
                img.thumbnail((160,120), resample=Image.ANTIALIAS)
                
            tnname = "%s_tn_%s"%(self.renderer.shortname, res.job.outimagename[0])
            imgstr = self.saveImgAsWeb(img, htmldir, tnname)
            desc = job.description
            if job.known_issues!=None:
                desc = "%s<p><center><table border=1 width=97%s cellpadding=4 cellspacing=0 bgcolor=#000000>"%(desc, "%")
                desc += "<td bgcolor=#ffd8d8><b>Known issues:</b><p>%s</td></table></center>"%(job.known_issues)
            html.write("<td>%s</td><td>%s</td><td>%s</td></tr>"%(res.job.rib,imgstr, desc))

        html.write("</table>")
        
        return htmlname

######################################################################

# JobDirectoryNode
class JobDirectoryNode:
    """This class represents a node in the job directory hierarchy.

    An object of this class can be used like a sequence of subdirs.
    """
    
    def __init__(self, name="/", parent=None, basename=""):
        # The name of this node (this is the directory name this
        # node represents)
        self.name = name
        # The base name of the web pages
        # The has to provided for the root (everyone else has a copy)
        self.basename = basename
        # Parent node or None
        self.parent = parent
        self.subdirs = []
        self.jobs = []
        if parent!=None:
            parent.subdirs.append(self)

    def __len__(self):
        return len(self.subdirs)
    
    def __iter__(self):
        return iter(self.subdirs)


    # jobList
    def jobList(self):
        """Return all jobs of this subtree."""
        res = copy.copy(self.jobs)
        for sd in self.subdirs:
            res += sd.jobList()
        return res

    # numJobs
    def numJobs(self):
        """Return the number of jobs in this subtree.

        May only be called after renderJobs() was called!
        """
        res = len(self.jobs)
        for sd in self.subdirs:
            res += sd.numJobs()
        return res

    # numNoRef
    def numNoRef(self):
        """Return the number of jobs with missing reference images.

        May only be called after renderJobs() was called!
        """
        res = 0
        for j in self.jobs:
            if j.noref:
                res += 1
        for sd in self.subdirs:
            res += sd.numNoRef()
        return res

    # numFailures
    def numFailures(self):
        """Return the number of jobs that failed.

        May only be called after renderJobs() was called!
        """
        res = 0
        res = 0
        for j in self.jobs:
            if j.failure:
                res += 1
        for sd in self.subdirs:
            res += sd.numFailures()
        return res

    # hasKnownIssues
    def hasKnownIssues(self):
        """Return whether one of the jobs has known issues.

        May only be called after renderJobs() was called!
        """
        for j in self.jobs:
            if j.known_issues!=None:
                return True
        for sd in self.subdirs:
            if sd.hasKnownIssues():
                return True
        return False

    # findSubDir
    def findSubDir(self, name):
        """Return the direct subdir node with the given name.

        If there is no node with the given name then None is returned.
        """
        for sd in self.subdirs:
            if sd.name==name:
                return sd
        return None

    # pageName
    def pageName(self):
        """Return the name of the correspondding web page.
        """
        return "%s%d.html"%(self.basename, id(self))

    # insertJob
    def insertJob(self, job):
        """Insert the given job into the tree.

        The position in the tree is determined by the job's working
        directory.
        For each directory a node is created if it doesn't already exist.

        This method should only be called on the root node!
        """
        dir = os.path.abspath(job.workingdir)
        dr, dir = os.path.splitdrive(dir)
        node = self
        for dirname in dir.split(os.sep)[1:]:
            # Search for the next subdir node...
            sd = node.findSubDir(dirname)
            # None? Then create a new node
            if sd==None:
                sd = JobDirectoryNode(name=dirname, parent=node, basename=self.basename)
            node = sd
        node.jobs.append(job)
        
    
# RegressionTest
class RegressionTest(Task):
    """Renders jobs and compares the output with reference images."""

    def __init__(self, renderer, reference, jobs=AllJobs, cmp_threshold=0,
                 create_references=False):
        """Constructor.
        """
        Task.__init__(self)
        self.renderer = renderer
        self.reference = reference
        self.jobs = self.convertJobList(jobs)
        self.cmp_threshold = cmp_threshold
        self.create_references = create_references

    # run
    def run(self, htmldir):

        if self.create_references:
            self.createReferences()
            return None

        print "Creating regression test for %s..."%(self.renderer.shortname)

        # Render all the jobs and create the reports for each job
        root = self.renderJobs(htmldir)

        htmlname = "regression_%s.html"%(self.renderer.shortname)
#        desc = self.createFlatPage(htmlname, htmldir, root)
        desc = self.createPages(htmlname, htmldir, root)

        return htmlname, desc

    # createPages
    def createPages(self, htmlname, htmldir, root):
        """Create the output pages."""
        
        title = "Regression test for %s"%self.renderer.shortname
        html = HTMLPage(os.path.join(htmldir, htmlname), title)
        html.section(title)

        # Save the status indicator images
        try:
            self.saveStatusImages(htmldir)
        except:
            pass

        html.write("""
        Below you see the directory hierarchy of the processed jobs.
        If you click a directory name you get a list of all jobs that
        are in this sub tree. So just click on the root directory if
        you want to see all jobs at once.
        <p>
        <img src="pics/green.png"> - Everything is ok<br>
        <img src="pics/green_issues.png"> - There are known issues with one or more tests, but there haven't been any new failures<br>
        <img src="pics/orange.png"> - Some reference images are still missing<br>
        <img src="pics/red.png"> - There have been failures
        <p>
        <h4>Job directories:</h4><p>
        """)

        html.write("<ul>")
        self.writeSubDirs(root, htmldir, html)
        html.write("</ul>")

        html.write("<hr>")
        html.write('<a href="index.html">Index page</a>')

        return self.createStatusString(root)

    # writeSubDirs
    def writeSubDirs(self, node, htmldir, html):
        """Helper method for createPages().
        """
        desc = self.createStatusString(node)
        if node.numFailures()>0:
            pic = "red.png"
        elif node.numNoRef()>0:
            pic = "orange.png"
        elif node.hasKnownIssues():
            pic = "green_issues.png"
        else:
            pic = "green.png"
        html.write('<li><img src="pics/%s" width=12 height=10 align=center> <a href="%s">%s</a> - %s'%(pic, node.pageName(), node.name, desc))
        self.createFlatPage(node.pageName(), htmldir, node)
        if len(node)==0:
            return 
        html.write("<ul>")
        for sd in node:
            self.writeSubDirs(sd,htmldir,  html)
        html.write("</ul>")
        
    
    # createFlatPage
    def createFlatPage(self, htmlname, htmldir, node):
        """Create the output page that actually shows the list of tests.

        All jobs in the entire hierarchy given by node are listed. 
        """
        numjobs = node.numJobs()
        numfailures = node.numFailures()
        numnoref = node.numNoRef()
        jobs = node.jobList()
        
        title = "Regression test for %s"%self.renderer.shortname
        html = HTMLPage(os.path.join(htmldir, htmlname), title)

        html.write("<font size=-1>")
        # Output a reference to the parent page
        if node.parent!=None:
            p = node.parent
            html.write('<b>Parent:</b> <a href="%s">%s</a> '%(p.pageName(), p.name))

        # Output references to the sub directories
        if len(node)>0:
            html.write("<b>Sub directories:</b>")
            lst = []
            for sd in node:
                lst.append('<a href="%s">%s</a>'%(sd.pageName(), sd.name))
            html.write(", ".join(lst))
        
        html.write("</font>")

        html.section(title)        

        # Create the description (desc) which appears on the index page
        # and of top of the regression test output
        desc = self.createStatusString(node)

        # Output the render results as html...
        html.write(desc+'<p>')
        if numfailures>0:
            html.write("<h4>Failures:</h4>")
            lst = []
            for job in jobs:
                if job.maxdiff!=None and max(job.maxdiff)>self.cmp_threshold:
                    lst.append('<a href="#%s">%s</a>'%(job.rib,job.rib))
            html.write(", ".join(lst))
            html.write("<p>")
                    
        if numnoref>0:
            html.write("<h4>Jobs without a reference image:</h4>")
            lst = []
            for job in jobs:
                if job.maxdiff==None:
                    lst.append('<a href="#%s">%s</a>'%(job.rib,job.rib))
            html.write(", ".join(lst))
            html.write("<p>")
        
        html.write("""<hr>
Click on the RIB name or thumbnail to display details.
<p>
<table border=0 cellspacing=2 cellpadding=2>
<td><font size=-1><b>RIB:</b></font></font></td>
<td><font size=-1>Name of the rendered RIB file.</font></td><tr>
<td><font size=-1><b>Thumbnail:</b></font></td>
<td><font size=-1>A thumbnail version of the rendering output.</font></td><tr>
<td><font size=-1><b>Renderer stdout:</b></font></td>
<td><font size=-1>Number of lines in stdout during rendering.</font></td><tr>
<td><font size=-1><b>Renderer stderr:</b></font></td>
<td><font size=-1>Number of lines in stderr during rendering.</font></td><tr>
<td><font size=-1><b>SL compiler stdout:</b></font></td>
<td><font size=-1>Number of lines in stdout during shader compiling (summed up over all shaders).</font></td><tr>
<td><font size=-1><b>SL compiler stderr:</b></font></td>
<td><font size=-1>Number of lines in stderr during shader compiling (summed up over all shaders).</font></td><tr>
<td><font size=-1><b>Max_diff:</b></font></td>
<td><font size=-1>Maximum pixel difference per color channel of the output image with the highest difference value.</font></td><tr>
</table>
<p>
""")
        html.write("<table border=1 cellspacing=2 cellpadding=2>")
        html.write('<td><b>RIB</b></td>')
        html.write('<td><b>Thumbnail<b></td>')
        html.write('<td><b>Renderer<br>stdout</b></td>')
        html.write('<td><b>Renderer<br>stderr</b></td>')
        html.write('<td><b>SL compiler<br>stdout</b></td>')
        html.write('<td><b>SL compiler<br>stderr</b></td>')
        html.write('<td><b><center>Max_diff</center></b></td>')
        html.write('</tr>')

        for job in jobs:
            res = job.res
            imgstr = job.imgstr
            reshtmlname = job.reshtmlname
            maxdiff = job.maxdiff
            
            html.write('<td><a href="%s">%s</a></td>'%(reshtmlname, res.job.rib))
            html.write('<td><a href="%s" name="%s">%s</a></td>'%(reshtmlname, res.job.rib, imgstr))
            html.write('<td align=center>%s</td>'%len(res.stdout_buf))
            html.write('<td align=center>%s</td>'%len(res.stderr_buf))
            nout, nerr = self.countSLCoutput(res)
            html.write('<td align=center>%s</td>'%nout)
            html.write('<td align=center>%s</td>'%nerr)
            if maxdiff==None:
                smaxdiff = "???"
            else:
                smaxdiff = str(maxdiff)
            if maxdiff!=None and max(maxdiff)<=self.cmp_threshold:
                col="#000000"
                sb=""
                eb=""
            else:
                col="#ff0000"
                sb="<b>"
                eb="</b>"
            html.write('<td align=center><font color=%s>%s%s%s</font>'%(col, sb, smaxdiff, eb))
            if job.known_issues!=None:
                html.write('<p><font size=-1>(there are known issues)</font>')
            html.write('</td><tr>')

        html.write("</table>")
        
        return desc

    # createStatusString
    def createStatusString(self, node):
        """Create the status string that contains infos about the tests.

        The method returns a string like:
        "17 jobs, 3 failures, 2 jobs without a reference image".
        """

        numjobs = node.numJobs()
        numfailures = node.numFailures()
        numnoref = node.numNoRef()

        desc = "%d jobs, "%(numjobs)
        if numfailures>0:
            desc += "<font color=#ff0000><b>%d</b></font> failures"%(numfailures)
        else:
            desc += "no failures"
        if numnoref>0:
            desc += ", <b>%d</b> jobs without a reference image"%(numnoref)

        return desc
        

    # renderJobs
    def renderJobs(self, htmldir):
        """Render all jobs.

        This method attaches the following new attributes to each job:

        - noref: True if there was no reference image (set in outputResult())
        - failure: True if the images were not identical
        - res: Result object
        - imgstr: HTML-Code for the image
        - reshtmlname: File name of the result page
        - maxdiff: Maximum pixel difference or None
        """

        basename = "regression_%s_"%(self.renderer.shortname)
        root = JobDirectoryNode(basename = basename)

        # Render all jobs...
        for job in self.jobs:
            # Insert the job into the directory hierarchy
            root.insertJob(job)
            
            # Render image
            res = self.render(self.renderer, job)
            
            # Create result page
            base,ext = os.path.splitext(res.job.rib)
            reshtmlname = "%s_%s.html"%(base,self.renderer.shortname)
            maxdiff = self.outputResult(res, htmldir, reshtmlname)

            # Load output image
            img = res.loadOutputImage()

            # Create thumbnail
            if img!=None:
                img.thumbnail((160,120), resample=Image.ANTIALIAS)
                
            tnname = "%s_tn_%s"%(self.renderer.shortname, res.job.outimagename[0])
            imgstr = self.saveImgAsWeb(img, htmldir, tnname)

            # Attach some variables to the job object so we can access
            # them later again...
#            job.noref = (maxdiff==None)
            job.failure = (maxdiff!=None and max(maxdiff)>self.cmp_threshold)
            if not job.noref and maxdiff==None:
                job.failure = True
                

            job.res = res
            job.imgstr = imgstr
            job.reshtmlname = reshtmlname
            job.maxdiff = maxdiff

        # Cut away unnecessary directories at the beginning of the path
        while len(root)==1:
            root = root.subdirs[0]
            root.parent = None
            
        return root

    # compareOutput
    def compareOutput(self, res, idx=0):
        """Load output image, reference image and create difference.

        Returns (img,refimg,diffimg,diff2img,diffalpha,diff2alpha,maxdiff).
        The images are PIL image objects and maxdiff a list of difference
        values.
        """
        
        img = None        # output image
        refimg = None     # reference image
        diffimg = None    # Difference image (RGB)
        diff2img = None   # "Binary" difference image (RGB)
        diffalpha = None  # Difference image (Alpha)
        diff2alpha = None # "Binary" difference image (Alpha)
        maxdiff = None

        # Load output image
        img = res.loadOutputImage(idx)

        # Load reference image
        refimgname = os.path.join(self.reference, res.job.outimagename[idx])
        refimg = loadImage(refimgname)

        if img!=None and refimg!=None and img.mode==refimg.mode:
            diffimg = ImageChops.difference(img, refimg)
            hist = diffimg.histogram()
            maxdiff = self.maxDiff(hist)
            a = self.cmp_threshold+1
            lut = a*[0]+(256-a)*[255]
            lut = len(img.mode)*lut
            diff2img = diffimg.point(lut)
            # RGBA? Then separate the alpha channel
            if img.mode=="RGBA":
                r,g,b,diffalpha = diffimg.split()
                diffimg = Image.merge("RGB", (r,g,b))
                r,g,b,diff2alpha = diff2img.split()
                diff2img = Image.merge("RGB", (r,g,b))

        return (img,refimg,diffimg,diff2img,diffalpha,diff2alpha,maxdiff)
  

    # countSLoutput
    def countSLCoutput(self, res):
        nout = 0
        nerr = 0
        for sh,out,err in res.shaderinfo:
            nout += len(out)
            nerr += len(err)
        return nout, nerr
    
    # createReferences
    def createReferences(self):
        """Create reference images."""
        
        print 'Creating reference images in "%s" using %s...'%(self.reference, self.renderer.shortname)
        preparePath(self.reference)
        for job in self.jobs:
            # Render image
            res = self.render(self.renderer, job)
            # Copy image to reference directory
            for refname,src in zip(res.job.outimagename, res.realoutimagename):
                dst = os.path.join(self.reference, refname)
                try:
                    shutil.copyfile(src, dst)
                except:
                    print 'Could not copy output file "%s".'%src
        

    # maxDiff
    def maxDiff(self, hist):
        """Return maximum difference.

        hist must be the histogram of a difference image.
        The method returns the maximum index (=difference) that is not zero.
        So the return value is the maximum pixel difference.
        """
        res = []
        
        n = len(hist)/256
        for i in range(n):
            h = hist[i*256:(i+1)*256]
            idx = 0
            for i in range(256):
                if h[i]>0:
                    idx = i
            res.append(idx)
                
        return res


    # outputResult
    def outputResult(self, res, htmldir, htmlbasename):
        """Creates result page and returns maxdiff."""

        res_maxdiff = [-1]

        title = "Render result: %s"%res.job.rib
        html = HTMLPage(os.path.join(htmldir, htmlbasename), title)
        html.section(title)

        desc = res.job.description
        if desc!="":
#            desc = desc.replace("<", "&lt;")
#            desc = desc.replace(">", "&gt;")
#            desc = desc.replace("\n\n", "<p>")
            desc = desc + "<p>"

        # Add the known_issues string...
        if res.job.known_issues!=None:
            desc = "%s<center><table border=1 width=97%s cellpadding=4 cellspacing=0 bgcolor=#000000>"%(desc, "%")
            desc += "<td bgcolor=#ffd8d8><b>Known issues:</b><p>%s</td></table></center><p>"%(res.job.known_issues)

        if len(res.job.outimagename)>1:
            str_image = "images"
        else:
            str_image = "image"

        riburl = os.path.abspath(os.path.join(res.job.workingdir, res.job.rib))
        riburl = "file://"+riburl.replace("\\","/")

        html.write("""%s
<table border=1 cellspacing=2 cellpadding=4>
<td><b>Package:</b></td> <td>%s</td><tr>
<td><b>Renderer:</b></td> <td><tt>%s</tt></td><tr>
<td><b>Version:</b></td> <td>%s</td><tr>
<td><b>SL compiler:</b></td> <td><tt>%s</tt></td><tr>
<td><b>Version:</b></td> <td>%s</td><tr>
<td><b>Platform:</b></td> <td>%s</td><tr>
<td><b>Date:</b></td> <td>%s</td><tr>
<td><b>RIB:</b></td> <td><a href="%s">%s</a></td><tr>
<td><b>Render time:</b></td> <td>%1.1fs</td><tr>
<td><b>Output %s:</b></td> <td>%s</td><tr>
</table>
"""%(desc, res.rendererobject.shortname,
     res.renderer, res.r_version, res.shadercompiler, res.sc_version,
     sys.platform,
     time.asctime(res.date), riburl, res.job.rib, res.rendertime,
     str_image, string.join(res.job.outimagename,", ")))

        # Output image...
        html.write("<h2>Output %s</h2>"%str_image)

        for i in range(len(res.job.outimagename)):
            img,refimg,diffimg,diff2img,diffalpha,diff2alpha,maxdiff = self.compareOutput(res, i)
            if refimg==None:
                res.job.noref = True
                            
            if maxdiff==None:
                res_maxdiff = None
            elif res_maxdiff!=None:
                if max(maxdiff)>max(res_maxdiff):
                    res_maxdiff = maxdiff
            
            html.write('Output / Reference / RGB diff / RGB binary diff')
            if diffalpha!=None:
                html.write(' / Alpha diff / Binary alpha diff')
            html.write('<br>\n')

            imgstr = self.saveResAsWeb(res, htmldir, i)
            html.write(imgstr)

            outname = res.job.outimagename[i]
            refname = "%s_ref_%s"%(res.rendererobject.shortname, outname)
            imgstr = self.saveImgAsWeb(refimg, htmldir, refname)
            html.write(imgstr)
    
            diffname = "%s_diff_%s"%(res.rendererobject.shortname, outname)
            imgstr = self.saveImgAsWeb(diffimg, htmldir, diffname)
            html.write(imgstr)

            diff2name = "%s_diff2_%s"%(res.rendererobject.shortname,outname)
            imgstr = self.saveImgAsWeb(diff2img, htmldir, diff2name)
            html.write(imgstr)

            if diffalpha!=None:
                alphaname = "%s_diffa_%s"%(res.rendererobject.shortname,
                                           outname)
                imgstr = self.saveImgAsWeb(diffalpha, htmldir, alphaname)
                html.write(imgstr)

                alpha2name = "%s_diff2a_%s"%(res.rendererobject.shortname,
                                             outname)
                imgstr = self.saveImgAsWeb(diff2alpha, htmldir, alpha2name)
                html.write(imgstr)

            if maxdiff==None:
                s = "???"
            else:
                s = str(maxdiff)
            html.write("<p>Maximum difference: <b>%s</b>"%s)
            html.write("<p>")

        # RIB header...
        html.write("<h2>RIB header</h2>")
        header = self.readRIBHeader(res.job)
        if header==None:
            html.write("<b>Error:</b> Could not read RIB file.")
        elif header==[]:
            html.write("No header present.")
        else:
            html.write("<pre>")
            for s in header:
                html.write(s.strip())
            html.write("</pre>")

        # Rendering output...
        html.write("<h2>Rendering</h2>\n")

        html.write("""
<em>stdout:</em><br>
<pre>%s</pre>
<em>stderr:</em><br>
<pre>%s</pre>
"""%(string.join(res.stdout_buf,"").strip(), string.join(res.stderr_buf,"").strip()))

        # Shader Compiler output...
        html.write("<h2>Shader compiling</h2>\n")

        if len(res.shaderinfo)==0:
            html.write("No extra shaders used\n")

        for sh,out,err in res.shaderinfo:
            html.write("""
<hr>
Shader: <tt><b>%s</b></tt><p>
<em>stdout:</em><br>
<pre>%s</pre>
<em>stderr:</em><br>
<pre>%s</pre>
"""%(sh,string.join(out,"").strip(),string.join(err,"").strip()))

        html.write("<hr>\n")
        html.close()
        return res_maxdiff

    # readRIBHeader
    def readRIBHeader(self, job):
        """Return the first comment block from a RIB file.

        The return value is a list of strings (=lines). If an error
        occurs, None is returned.
        """

        # Change to working dir...
        olddir = os.getcwd()
        try:
            os.chdir(job.workingdir)
        except OSError,e:
            return None

        # Open RIB file...
        try:
            f = file(job.rib)
        except:
            os.chdir(olddir)
            return None

        # Read header...
        res = []
        while 1:
            s = f.readline()
            if s=="":
                break
            if s[0]!="#":
                break
            res.append(s)

        # Close file and change directory back to what it was at the beginning
        f.close()
        os.chdir(olddir)
        return res

    # saveStatusImages
    def saveStatusImages(self, htmldir):
        """Save the green, orange and red status indicators."""
        
        name = os.path.join(htmldir, "pics", "green.png")
        f = file(name, "wb")
        f.write("\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x0c\x00\x00\x00\n\x08\x06\x00\x00\x00\x80,\xbf\xfa\x00\x00\x00\x04gAMA\x00\x00\xb1\x8e|\xfbQ\x93\x00\x00\x00 cHRM\x00\x00z%\x00\x00\x80\x83\x00\x00\xf9\xff\x00\x00\x80\xe8\x00\x00u0\x00\x00\xea`\x00\x00:\x97\x00\x00\x17o\x97\xa9\x99\xd4\x00\x00\x01\x1bIDATx\x9cb\xfc\xff\xff?\x03)\x00 \x80\x98HR\r\x04\x00\x01\xc4\x82.P\xf1\xb4\xe2\xff\x8f\x7f?\x18>\xfe\xfd\xc80_a>#\xba<@\x001\xc2\x9cT\xfa\xa4\xf4?3#3\xc3\xdf\xff\x7f\x19\xbe\xfe\xfb\xca\xf0\xee\xcf;\x86\xf7\x7f\xdf\x835\x9e\xd08\x01\xd7\x08\x10@p'\x81L\xfd\xfa\xf7+\xc3\xa7\xbf\x9f\xc0\x8a\xdf\xfcy\xc3\xf0\xfa\xcfk\x86W\xbf_1(\x9cQ\x80{\x14 \x80\xc0N\x8a\xba\x17\xf5\x9f\x91\x91\x91\xe1\xf7\xff\xdf\x0c\xdf\xff}gx\xff\xe7=\xc3\xab?\xaf\x18~\xfd\xf8\xc5\xf0\xff\xe7\x7f0\x86\x01\x80\x00\x02kx\xf9\xe7%\xc3\xaf\xff\xbf\x18\xbe\xfd\xfb\xc6\xf0\xf9\xefg\xb0\xc2\x7f\xdf\xfe1\xfc\xff\x0eT\xfc\x1b\xa8\xf8/\xc2\x0f\x00\x01\x04\xd6\xf0\xec\xf73\x86\x1f\x7f\x7f\x80M\xfa\xf7\x1d\xa8\xf0\xdb\x7f\x88\x86_@\xc5\xff@>Eh\x00\x08 \xb8\xa7\xe5\x8f\xca\xff\x07+\x06\x99\n\xc4\xff~\x00U\xfe\x86(z\x9e\xfc\x1c\xae\x05 \x80\x18\x91#Nf\xb3\xcc\x7f\xb0\xe9?\xa0\xa6\x03\xd1\x8b\xb4\x17(A\x0b\x10`\x00\x81\xda\xa2\xe6\xeaA\xf7\xf2\x00\x00\x00\x00IEND\xaeB`\x82")

        name = os.path.join(htmldir, "pics", "green_issues.png")
        f = file(name, "wb")
        f.write("\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x0c\x00\x00\x00\n\x08\x06\x00\x00\x00\x80,\xbf\xfa\x00\x00\x00\x04gAMA\x00\x00\xb1\x8e|\xfbQ\x93\x00\x00\x00 cHRM\x00\x00z%\x00\x00\x80\x83\x00\x00\xf9\xff\x00\x00\x80\xe8\x00\x00u0\x00\x00\xea`\x00\x00:\x97\x00\x00\x17o\x97\xa9\x99\xd4\x00\x00\x01\x90IDATx\x9cb\xfc\xff\xff?\x03)\x00 \x80\x18\xff\t\x0b#\xf3\xf9\x81X\x01J\x7f\x04\xe2\x07P\x1a\xa2\xf8\xcd\x1b\x06\x80\x00bAS\xa8\x97s$|\xd1\x8f\x7f?\x18>\xfe\xfd\xc8\xb0\xda\xf1@\x1cP\xec\x12\xb2F\x80\x00\x82\xd9\xa0\x9fy8\xe4\x023#3\xc3\xdf\xff\x7f\x19\xbe\xfe\xfb\xca\xf0\xee\xcf;\x86\xf7\x7f\xdf38\xee}\xc4\x90\xb6\xfc{\x81\xdc#\xc6\x05@\x1b>\x02\x04\x10\x13\xd4t=\x90\xa9_\xff~e\x88|\xaf\xc2\xc0\xf0\xeb;\xc3\x9b?o\x18\xfe\xbd{\xc1\xc0\xf0\xe3\x1b\xc3\x0e\xeb\xff\x13\x8eY\xfd\xed\x00\x99\x0c\x10@ \r\n~{-\x17\xbd\xfb\xfb\x8e!\x85\xd9\x8a\xe1,\xdf\x0b\x86\xeb\x7f\xef3<\xfa\xf5\x08\xe8\xe6\x8f\x0cF\xf7\xfe3(\xbe\xf9\xcf`u\x8cy9H\x03@\x00\x81\xfc\xc0\xff\xf2\xcfK\x86_\xff\x7f1$\xfd\xe8a\xf8\xfc\xf73\xc3\xaf\x1f\xbf\x18\xfe}\xfb\xc7\xe0p\xf1?\x03\xcfOF\x86\x7f\x8c\x0c\x0c\x17\xf5\xff9\x1900\x1c\x02\x08 \x90\x86\x8f\xcf~?c\xf8\xf1\xf7\x07\xc3\xff\x9f\xff\x19\xfe}\xff\xc7\xa0\xfe\xf8?C\xdaYF\x06\xf1o@\xc5@\x05\xcc\xc0\x90\x17|\xcfx\x02d\x03@\x00\x814<x\xec\xf3\xc3/\xa8\xe1\xdf&\x9e/\xff\x18\xde\x00E\x92/12\xf0\xfca`\xf8\x0et0\x1bP\x81\xc0'\xc6-@O\x1f\x07i\x00\x08 X(\xf1\xbf\x12\xfb\x1f\xb2\xcc\xf9\xef\x1c\xad7\x8c\x0c\xdfX\xfe3p\xffa\x04\x87\xb7\xe5q\xa6H\x9e/\x8c\xdbA.\x01\xc5\x03@\x80\x01\x00\xbd[\xa4R \x1f\xa47\x00\x00\x00\x00IEND\xaeB`\x82")


        name = os.path.join(htmldir, "pics", "orange.png")
        f = file(name, "wb")
        f.write("\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x0c\x00\x00\x00\n\x08\x06\x00\x00\x00\x80,\xbf\xfa\x00\x00\x00\x04gAMA\x00\x00\xb1\x8e|\xfbQ\x93\x00\x00\x00 cHRM\x00\x00z%\x00\x00\x80\x83\x00\x00\xf9\xff\x00\x00\x80\xe8\x00\x00u0\x00\x00\xea`\x00\x00:\x97\x00\x00\x17o\x97\xa9\x99\xd4\x00\x00\x01\x11IDATx\x9cb\xfc\xff\xff?\x03)\x00 \x80\x98HR\r\x04\x00\x01\xc4\x82.\xf0\xe1f\xde\xff\xff\xff\xbe3\xfc\xfb\xfd\x81AXo5#\xba<@\x001\xc2\x9c\xf4\xe1F\xd6\x7f\x06F\xa0\xfe\xff\x7f\x18\xfe\xfd\xfd\x02\xd4\xf0\x16\x88\xdf\x01\xb9\xef\x19\xc4-o\xc05\x02\x04\x10\xdcI\xff\xff\xfd`\xf8\x0fR\xf8\xe7#D\xf1\xaf\xd7@\xfc\x8a\xe1\xef\xcf\x97\x0c\xf7v\xf2\xc3=\n\x10@`'\xbd\xbd\xe0\x0b4\x9d\x11\xa8\xe97\x10\x7f\x03\x9b\xfc\xef\xd7K\x86\x1f?~1\xfc\xf8\xf9\x9f\xe1\xc7/D\xc0\x00\x04\x10X\xc3\xdf_/\x80\n\x7f\x02m\xf8\x06\xc4\x9f\xc0\n\xbf|\xfb\xcf\xf0\x1d\xa8\xf8\xd7o\xa0\xfc_\x84\x1f\x00\x02\x08\xa2\xe1\xc7c\xa0\xbb\x7f\x80M\xfa\xf6\x9d\x81\xe1\xebw\x10\xfd\x9f\xe1'P\xf1\xbf\x7f\x0c \xcb\xe1\x00 \x80\xe0\x9e\xbe\xbe\x99\xff\xff\xb7\x1f@\x85?\x18\x18@\xf4w \xfe\xf3\x07\xa2(\xa4\xf0\x0b\\\x0b@\x00\xc1=\xad\xe9\xfb\x91\x11\xa4\x18n\xfa/\x06\x86?\x7fQ\x15\x83\x00@\x80\x01\x00\xf4\xdd\xa0Ds\xc9#J\x00\x00\x00\x00IEND\xaeB`\x82")

        name = os.path.join(htmldir, "pics", "red.png")
        f = file(name, "wb")
        f.write('\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x0c\x00\x00\x00\n\x08\x06\x00\x00\x00\x80,\xbf\xfa\x00\x00\x00\x04gAMA\x00\x00\xb1\x8e|\xfbQ\x93\x00\x00\x00 cHRM\x00\x00z%\x00\x00\x80\x83\x00\x00\xf9\xff\x00\x00\x80\xe8\x00\x00u0\x00\x00\xea`\x00\x00:\x97\x00\x00\x17o\x97\xa9\x99\xd4\x00\x00\x00\xfaIDATx\x9cb\xfc\xff\xff?\x03)\x00 \x80\x98HR\r\x04\x00\x01\xc4\x82.\xf0/5\xf5?\xc3\xb7o\x0c\x0c\xef\xdf30m\xdb\xc6\x88.\x0f\x10@\x8c0\'\xfdKL\xfc\xcf\xc0\x02\xd4\xff\xe7\x0f\x03\xc3\xe7\xcf\x0c\x0co\xde00\xbc}\xcb\xf0\xff\xdd;\x06\xe6\xa7O\xe1\x1a\x01\x02\x08a\xc3\xf7\xef\x10\xfa\xe7O\xb0\xe9\xff_\xbe\x84k\xfa\xc2\xc8\xf8\x9f\xe7\xff\x7f\xb0&\x80\x00\x02k\xf8\xe7\xe4\xf4\x9f\x81\x11\xc8\xff\xfd\x9b\x81\xe1\xebW\x86\xff@E\x0c\xcf\x9f3\xfc\xf8\xf5\x8b\x01d\x0c\x08\xf3@\xcd\x05\x08 \xb0\x86\xff\xcf\x9eAL\x06*f\xf8\xf8\x11\xac\x10\xe8(\x86o \x0b\x81\xf8/\x10\xcbB5\x00\x04\x10\xc4I\x8f\x1e1\xfc\x07:\td\x12P\x0b\xc3\x17(\rS\x8c\xecs\x80\x00\x82{\xfa\r\xd0\x9d_\xa1\n\xbfA\xf1o\xa8"g\xa8\xfbA\x00 \x80\xe0\xf1 \x02\x14\x84i\x00\xe1\x1f@\xfc\x07M1\x08\x00\x04\x18\x00|NlZ[\xe7=\xdf\x00\x00\x00\x00IEND\xaeB`\x82')

        


######################################################################

class CompareRenderers(Task):
    """Renders jobs using several renderers and displays each output."""

    def __init__(self, renderers, jobs=AllJobs):
        """Constructor.
        """
        
        Task.__init__(self)
        self.renderers = renderers
        self.jobs = self.convertJobList(jobs)

    def run(self, htmldir):

        print "Creating renderer comparison..."

        htmlname = "comparison.html"

        title = "Renderer comparison"
        html = HTMLPage(os.path.join(htmldir, htmlname), title)
        html.section(title)

        for job in self.jobs:
            
            html.subsection(job.rib)
            html.write('<table border=0 cellspacing=0 cellpadding=2>')

            results = []
            for renderer in self.renderers:
                # Render image
                res = self.render(renderer, job)
                results.append(res)
                imgstr = self.saveResAsWeb(res, htmldir)
                html.write('<td>%s</td>'%imgstr)
            html.write('<tr>')
                
            for res in results:
                html.write('<td align=center><b>%s</b><p>'%(res.rendererobject.shortname))
                html.write(res.renderTimeStr())
                html.write('</td>')

            html.write('<tr>')
                
                
            html.write('</table>')

        html.close()
        return htmlname

######################################################################

class SpeedComparison(Task):
    """Renders jobs using several renderers and compares their rendering
    times."""

    def __init__(self, renderers, jobs=AllJobs):
        """Constructor.
        """
        
        Task.__init__(self)
        self.renderers = renderers
        self.jobs = self.convertJobList(jobs)

    def run(self, htmldir):

        if self.renderers==[]:
            return
        
        print "Creating speed comparison..."

        htmlname = "speed_comparison.html"

        title = "Speed comparison"
        html = HTMLPage(os.path.join(htmldir, htmlname), title)
        html.section(title)

        colors = [(200,0,0), (0,200,0), (0,0,200),
                  (200,200,0), (200,0,200), (0,200,200)]
        # Make sure there are enough colors (by duplicating them)
        while len(colors)<len(self.renderers):
            colors*=2
        # Create the images for the color bars...
        barimgnames = []     
        for i in range(len(self.renderers)):
            r,g,b = colors[i]
            barimg = Image.fromstring("RGB", (16,8), 16*8*(chr(r)+chr(g)+chr(b)))
            barimgname = "barimg%s"%(i+1)
            self.saveImgAsWeb(barimg, htmldir, barimgname)
            barimgnames.append(barimgname)

        # Render the jobs...
        for job in self.jobs:
            
            html.subsection(job.rib)
            html.write('<table border=1 cellspacing=0 cellpadding=2>')

            # results: Takes the RenderResult objects
            results = []
            # Maximum rendering time
            maxrendertime = 0
            # Render...
            for renderer in self.renderers:
                # Render image
                res = self.render(renderer, job)
                results.append(res)
                if res.rendertime>maxrendertime:
                    maxrendertime = res.rendertime

            # Load first output image (it's assumed that the rendered
            # images look roughly the same anyway, so one representative
            # is enough)
            res = results[0]
            img = res.loadOutputImage()
            # Create a thumbnail
            if img!=None:
                img.thumbnail((160,120), resample=Image.ANTIALIAS)
                
            tnname = "%s_tn_%s"%(res.rendererobject.shortname, res.job.outimagename[0])
            imgstr = self.saveImgAsWeb(img, htmldir, tnname)
            html.write('<td rowspan=%d>%s</td>'%(len(self.renderers),imgstr))

            # Calculat the scale factor for the render time values...
            # (up to a maximum bar width 1 pixel corresponds to 1 second,
            # above the maximum bar width all time values are scaled down
            # such that the maximum bar width is maintained)
            maxbarwidth = 300
	    if maxrendertime>0:
	        timescale = float(maxbarwidth)/maxrendertime
	    else:
	        timescale = 0
                
            # Create the result table...
            for res,barimgname in zip(results,barimgnames):
                barlength = int(res.rendertime*timescale)
                html.write('<td valign=top>%s</td>'%(res.rendererobject.shortname))
                html.write('<td valign=top><img src="pics/%s.png" width=%s height=16></td>'%(barimgname, barlength))
                html.write('<td valign=top>%s</td><tr>'%res.renderTimeStr())

            html.write('</table>')

        html.close()
        return htmlname


######################################################################

# RiCalls
class RiCalls(Task):
    """Creates a list of Ri calls and the jobs that test each call."""

    def __init__(self, jobs=AllJobs):
        """Constructor.

        jobs - Jobs that'll be included in the overview.
        """
        Task.__init__(self)
        self.jobs = self.convertJobList(jobs)

        self.ricalls = {"AreaLightSource" : 1,
                        "Atmosphere" : 1, "Attribute" : 1,
                        "AttributeBegin" : 1, "AttributeEnd" : 1,
                        "Basis" : 1, "Blobby" : 1, "Bound" : 1, "Clipping" : 1,
                        "ClippingPlane" : 1, "Color" : 1, "ColorSamples" : 1,
                        "ConcatTransform" : 1, "Cone" : 1,
                        "CoordSysTransform" : 1, "CoordinateSystem" : 1,
                        "CropWindow" : 1, "Curves" : 1, "Cylinder" : 1,
                        "Declare" : 1, "DepthOfField" : 1, "Detail" : 1,
                        "DetailRange" : 1, "Disk" : 1, "Displacement" : 1,
                        "Display" : 1, "ErrorHandler" : 1, "Exposure" : 1,
                        "Exterior" : 1, "Format" : 1, "FrameAspectRatio" : 1,
                        "FrameBegin" : 1, "FrameEnd" : 1, "GeneralPolygon" : 1,
                        "GeometricApproximation" : 1, "Geometry" : 1,
                        "Hider" : 1, "Hyperboloid" : 1, "Identity" : 1,
                        "Illuminate" : 1, "Imager" : 1, "Interior" : 1,
                        "LightSource" : 1, "MakeCubeFaceEnvironment" : 1,
                        "MakeLatLongEnvironment" : 1, "MakeShadow" : 1,
                        "MakeTexture" : 1, "Matte" : 1, "MotionBegin" : 1,
                        "MotionEnd" : 1, "NuPatch" : 1, "ObjectBegin" : 1,
                        "ObjectEnd" : 1, "ObjectInstance" : 1, "Opacity" : 1,
                        "Option" : 1, "Orientation" : 1, "Paraboloid" : 1,
                        "Patch" : 1, "PatchMesh" : 1, "Perspective" : 1,
                        "PixelFilter" : 1, "PixelSamples" : 1,
                        "PixelVariance" : 1, "Points" : 1,
                        "PointsGeneralPolygons" : 1, "PointsPolygons" : 1,
                        "Polygon" : 1, "Procedural" : 1,
                        "Projection" : 1, "Quantize" : 1, "ReadArchive" : 1,
                        "RelativeDetail" : 1, "ReverseOrientation" : 1,
                        "Rotate" : 1, "Scale" : 1, "ScreenWindow" : 1,
                        "ShadingInterpolation" : 1, "ShadingRate" : 1,
                        "Shutter" : 1, "Sides" : 1, "Skew" : 1,
                        "SolidBegin" : 1, "SolidEnd" : 1, "Sphere" : 1,
                        "SubdivisionMesh" : 1, "Surface" : 1,
                        "TextureCoordinates" : 1, "Torus" : 1, "Transform" : 1,
                        "TransformBegin" : 1, "TransformEnd" : 1,
                        "Translate" : 1,
                        "TrimCurve" : 1, "WorldBegin" : 1, "WorldEnd" : 1}

    def run(self, htmldir):

        htmlname = "ri_calls.html"

        print "Creating Ri list..."

        html = HTMLPage(os.path.join(htmldir, htmlname), "Ri calls")
        html.section("Ri calls")

        html.write(""" This page lists all Ri commands that can appear
        in a RIB stream together with the jobs that have this command
        somewhere in their stream.<p> <b>Note:</b> Keep in mind that
        this list is generated automatically by scanning the RIB
        streams, so the presence of a particular job name under a Ri
        call doesn't necessarily mean that this job is actually
        testing this particular call. Rather, you can say that a
        particular Ri call is tested by <em>no more</em> than the
        number of jobs listed.<hr> """)

        # jobs: A dictionary with Ri calls as keys and a list of job objects
        # as value. The list contains only jobs that have they key Ri call
        # in their stream
        jobs = {}
        # Initialize the jobs dict with all Ri calls and an empty list
        # for each. (this is so make *all* calls appear in the output)
        for k in self.ricalls:
            jobs[k] = []

        # Scan the jobs for Ri calls and update the jobs dict...
        for job in self.jobs:
            # Only scan when the AUTO keyword is present...
            if AUTO in job.ricalls:
                calls = self.scan(job)
                for k in calls:
                    jobs[k].append(job)

            # Check the explicit ricalls and add them to the jobs dict
            # if they were not added during automatic scanning
            for ricall in job.ricalls:
                if ricall!=AUTO:
                    if ricall in jobs:
                        if job not in jobs[ricall]:
                            jobs[ricall].append(job)
                    else:
                        jobs[ricall] = [job]

        html.write("Number of calls: %d<br>"%len(jobs))
        html.write("Total number of jobs: %d<br>\n"%len(self.jobs))

        # ricalls: A sorted list of all Ri calls in dict
        ricalls = jobs.keys()
        ricalls.sort()
        # Output the Ri calls list...
        for ri in ricalls:
            html.write("<h3>%s</h3>"%ri)
            if len(jobs[ri])>0:
                html.write("%d jobs<p>"%len(jobs[ri]))
                jobnames = []
                for j in jobs[ri]:
                    if ri in j.ricalls:
                        # Append the name in bold face
                        jobnames.append("<b>%s</b>"%j.rib)
                    else:
                        # Append the name using the normal font
                        jobnames.append(j.rib)
                html.write(" / ".join(jobnames))
            else:
                html.write("<font color=#ff0000>&lt;no tests available&gt;</font>")
        
        return htmlname

    def scan(self, job):
        """Scan the RIB for Ri calls.

        Returns a dictionary where the keys are the Ri calls that are
        present in the RIB file (the value is just 1).
        """

        print 'Scanning "%s"...'%job.rib
        # Change into working dir...
        olddir = os.getcwd()
        try:
            os.chdir(job.workingdir)
        except OSError,e:
            print e

        # Get the name of the effective RIB file...
        ribname = job.acquireRIB()
        n,ext = os.path.splitext(ribname)
        ext = ext.lower()

        res = {}
        # Open the file...
        if ext==".gz":
            f = gzip.open(ribname)
        else:
            f = file(ribname)
        # ...and read the content
#        for s in f:
        while 1:
            s = f.readline()
            if s=="":
                break
            s = s.strip()
            # Ignore empty or comment lines...
            if s=="" or s[0]=="#":
                continue
            call = s.split()[0]
            if call in self.ricalls:
                if call not in res:
                    res[call]=1

        if res=={}:
            print "--->WARNING: No RIB calls extracted from '%s'!"%job.rib
                    
        f.close()
        job.releaseRIB()
        os.chdir(olddir)
        return res
        

######################################################################

# preparePath
def preparePath(path):
    """Prepare a path.

    Checks if the path exists and creates it if it does not exist.
    """
    if not os.path.exists(path):
        parent = os.path.dirname(path)
        if parent!="":
            preparePath(parent)
        os.mkdir(path)

def rmfiles(arg, dirname, names):
    """Callback for walk() to remove files from a directory tree."""
    
    for n in names:
        pn = os.path.join(dirname,n)
        if os.path.isfile(pn):
            os.remove(pn)

def run(htmldir, tasklist):
    """Completes all tasks."""
    
    # If the output directory already exists then empty it otherwise
    # create it
    if os.path.exists(htmldir):
        # Remove all files in the output directory
        print 'Removing files in "%s"...'%htmldir
        os.path.walk(htmldir, rmfiles, None)
    else:
        print 'Creating "%s"...'%htmldir
        preparePath(htmldir)

    f_index = HTMLPage(os.path.join(htmldir,"index.html"), "Index")
    f_index.section("Index")

    f_index.write('Total number of jobs: %d<p>'%len(AllJobs))

    f_index.write("<ul>")

    for t in tasklist:
        desc = ""
        res = t.run(htmldir)
        if type(res)==tuple:
            htmlname, desc = res
        else:
            htmlname = res
            
        if htmlname!=None:
            name,ext = os.path.splitext(htmlname)
            f_index.write('<li><a href="%s">%s</a>'%(htmlname,name))
            if desc!="":
                f_index.write(' - %s'%desc)
            f_index.flush()

    f_index.write("</ul>")

    f_index.write('<hr>')
    f_index.write('%s, platform: %s'%(time.asctime(), sys.platform))
    
    f_index.close()

######################################################################

def usage():
    path, name = os.path.split(sys.argv[0])
    print "Usage: %s [Options] [targets ...]"%name
    print ""
    print "Renders all tasks specified by the targets or all available tasks if no"
    print "particular target is given."
    print ""
    print "Options:"
    print ""
    print "-h/--help                 Show this help"
    print "-j/--jobs <filename>      Jobs config file (default: jobs.cfg)"
    print "-r/--renderers <filename> Renderer config file (default: renderers.cfg)"
    print "-t/--tasks <filename>     Task config file (default: tasks.cfg)"
    print "-i/--include <RIB,RIB,..> Only allow the specified RIBs (wildcards allowed)"
    print "-e/--exclude <RIB,RIB,..> Do not allow the specified RIBs (wildcards allowed)"
    print "-o/--output <htmldir>     Set output directory (default: html)."
    print ""
    print 'The config files are executed as Python code in the following order:'
    print '1) renderers 2) jobs 3) tasks. They all use the same namespace so'
    print 'you can use variables from a previously read config file.'
    print ''
    print 'You can specify an empty string as config file name to disable reading'
    print 'a particular file.'
    print ''
    print 'WARNING: At the start of the program the output directory is cleared!'

######################################################################


def main():
    global Tasks
    global AllJobs, IncludeRIBs, ExcludeRIBs
    
    renderer_cfg = "renderers.cfg"
    jobs_cfg = "jobs.cfg"
    tasks_cfg = "tasks.cfg"
    htmldir = "html"

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   "hj:r:t:i:e:o:",
                                   ["help","jobs","renderers","tasks",
                                    "include","exclude","output"])
    except getopt.GetoptError:
        usage()
        sys.exit(1)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        if o in ("-j", "--jobs"):
            jobs_cfg = a
        if o in ("-r", "--renderers"):
            renderer_cfg = a
        if o in ("-t", "--tasks"):
            tasks_cfg = a
        if o in ("-i", "--include"):
            inc = a.split(",")
            for i in range(len(inc)):
                inc[i] = inc[i].strip()
            if IncludeRIBs==None:
                IncludeRIBs = inc
            else:
                IncludeRIBs += inc
        if o in ("-e", "--exclude"):
            ex = a.split(",")
            for i in range(len(ex)):
                ex[i] = ex[i].strip()
            ExcludeRIBs += ex
        if o in ("-o", "--output"):
            htmldir = a
  
    # Loading renderer...
    if renderer_cfg!="":
        if os.path.isdir(renderer_cfg):
            include_cfg(os.path.join(renderer_cfg,"renderers.cfg"), recursive=True)
        else:
            include_cfg(renderer_cfg)
#        print 'Loading renderers "%s"...'%renderer_cfg
#        execfile(renderer_cfg)

    # Loading jobs...
    if jobs_cfg!="":
        if os.path.isdir(jobs_cfg):
            include_cfg(os.path.join(jobs_cfg,"jobs.cfg"), recursive=True)
        else:
            include_cfg(jobs_cfg)
#        print 'Loading jobs "%s"...'%jobs_cfg
#        execfile(jobs_cfg)

    # Loading tasks...
    if tasks_cfg!="":
        if os.path.isdir(tasks_cfg):
            include_cfg(os.path.join(tasks_cfg,"tasks.cfg"), recursive=True)
        else:
            include_cfg(tasks_cfg)
#        print 'Loading tasks "%s"...'%tasks_cfg
#        execfile(tasks_cfg)

    print "%d renderer definitions"%len(Renderers)
    print "%d jobs"%len(AllJobs)
    print "%d tasks"%len(Tasks)

    # Check if there's a duplicate output name
    existing_names = {}
    for job in AllJobs:
        for n in job.outimagename:
            # make the comparison case-insensitive
            n = n.lower()
            if n in existing_names:
                s = os.path.join(job.workingdir, job.rib)
                print 'Error: Duplicate output name "%s" in:'%(n)
                print '  %s'%os.path.join(job.workingdir, job.rib)
                j = existing_names[n]
                print '  %s'%(os.path.join(j.workingdir, j.rib))
                sys.exit(1)
            existing_names[n] = job


    if len(args)==0:
        _tasklist = Tasks
    else:
        _tasklist = []

    # Check for targets and add them to the task list...
    for arg in args:
        try:
            exec "_tl = %s"%(arg)
        except:
            print 'Error: target "%s" not found'%(arg)
            sys.exit(1)
            
        if isinstance(_tl, Task):
            _tasklist.append(_tl)
        elif type(_tl)==list:
            for _t in _tl:
                if not isinstance(_t, Task):
                    print 'Error: target "%s" contains undefined tasks'%arg
                    sys.exit(1)
            _tasklist += _tl
        else:
            print 'Error: target "%s" does not specify a task'%arg
            sys.exit(1)

    # Complete the tasks...
    run(htmldir=htmldir, tasklist=_tasklist)

######################################################################

# Permit tracebacks
main()
sys.exit()

# Don't permit tracebacks to be displayed
try:
    main()
except SystemExit:
    pass
except:
    type, value, traceback = sys.exc_info()
    print "Exception:",value
