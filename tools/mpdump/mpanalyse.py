######################################################################
# Post process an Aqsis micro polygon dump file.
#
# Requirements:
#
# - Python 2.2 or higher
# - cgkit
# - PyOpenGL
#
# See mpanalyse.py -h for usage information. 
#
# Author: Matthias Baas (baas@ira.uka.de)
######################################################################

import sys, os, struct, getopt
from cgtypes import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from sl import *

# MicroPolygon
class MicroPolygon:
    """Stores a micro polygon data set."""
    def __init__(self, a, b, c, d, col, opacity, bin):
        self.a = a
        self.b = b
        self.c = c
        self.d = d
        self.col = col
        self.opacity = opacity
        self.bin = bin

# Sample
class Sample:
    """Stores a pixel sample data set."""
    def __init__(self, px, py, idx, x, y, bin):
        self.px = px
        self.py = py
        self.idx = idx
        self.x = x
        self.y = y
        self.bin = bin

# ImageInfo
class ImageInfo:
    """Stores global image infos."""
    def __init__(self, xres, yres, bin):
        self.xres = xres
        self.yres = yres
        self.bin = bin

# MPDumpReader
class MPDumpReader:
    """Base class for reading a dump file."""
    
    def __init__(self, filename="mpdump.mp"):
        self.f = file(filename, "rb")
        # sizeof(TqFloat)
        s = self.f.read(4)
        self.sizefloat = struct.unpack("i", s)[0]
        self.sizevec3 = 3*self.sizefloat
        self.xres = 0
        self.yres = 0
        self.mpcount = 0
        self.samplecount = 0

    def read(self):
        self.onBegin()
        sizevec3 = self.sizevec3
        f = self.f
        while 1:
            sid = f.read(2)
            if sid=="":
                break
            id = struct.unpack("h", sid[0:2])[0]
            # Micro polygon?
            if id==1:
                s = f.read(6*sizevec3)
                bin = sid+s
                A = struct.unpack("fff", s[0:sizevec3])
                B = struct.unpack("fff", s[sizevec3:2*sizevec3])
                C = struct.unpack("fff", s[2*sizevec3:3*sizevec3])
                D = struct.unpack("fff", s[3*sizevec3:4*sizevec3])
                col = struct.unpack("fff", s[4*sizevec3:5*sizevec3])
                opacity = struct.unpack("fff", s[5*sizevec3:])
                A = vec3(A)
                B = vec3(B)
                C = vec3(C)
                D = vec3(D)
                col = vec3(col)
                opacity = vec3(opacity)
                mp = MicroPolygon(A,B,C,D,col,opacity,bin)
                self.onMP(mp)

                self.mpcount += 1
                if self.mpcount%5000==0:
                    print "\015%d"%self.mpcount,
            # Sample position
            elif id==2:
                s = f.read(3*4+2*self.sizefloat)
                bin = sid+s
                px,py,idx,x,y = struct.unpack("iiiff", s)
                self.samplecount += 1
                sample = Sample(px,py,idx,x,y,bin)
                self.onSample(sample)
            # Image info?
            elif id==3:
                s = f.read(2*4)
                bin = sid+s
                xres, yres = struct.unpack("ii", s)
                self.xres = xres
                self.yres = yres
                imginfo = ImageInfo(xres, yres, bin)
                self.onImageInfo(imginfo)
            else:
                raise RuntimeError("ERROR: Unknown data id (%d)"%id)

        print "\015                \015",
        self.onEnd()

    def onMP(self, mp): pass
    def onSample(self, sample):  pass
    def onImageInfo(self, imginfo): pass
    def onBegin(self): pass
    def onFinished(self): pass

######################################################################

# MPInfo
class MPInfo(MPDumpReader):
    def __init__(self, filename="mpdump.mp"):
        MPDumpReader.__init__(self, filename)
        self.minx = 99999
        self.maxx = -1
        self.miny = 99999
        self.maxy = -1
        self.minz = 99999
        self.maxz = -1
        self.degenerated = 0
        self.orientations = {0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0,
                             9:0, 10:0, 11:0, 12:0, 13:0, 14:0, 15:0}
        self.orient_names = {0:"Convex (CW)",
                             1:"Concave (CW, B)",
                             2:"Concave (CW, C)",
                             3:"Twisted",
                             4:"Concave (CW, D)",
                             5:"Impossible",
                             6:"Twisted",
                             7:"Concave (CCW, A)",
                             8:"Concave (CW, A)",
                             9:"Twisted",
                             10:"Impossible",
                             11:"Concave (CCW, D)",
                             12:"Twisted",
                             13:"Concave (CCW, C)",
                             14:"Concave (CCW, B)",
                             15:"Convex (CCW)"}

    def onBegin(self):
        print "sizeof(TqFloat) = %d"%self.sizefloat
        setEpsilon(0)

    def onMP(self, mp):
        a,b,c,d = mp.a,mp.b,mp.c,mp.d
        self.minx = min(self.minx, a.x, b.x, c.x, d.x)
        self.maxx = max(self.maxx, a.x, b.x, c.x, d.x)
        self.miny = min(self.miny, a.y, b.y, c.y, d.y)
        self.maxy = max(self.maxy, a.y, b.y, c.y, d.y)
        self.minz = min(self.minz, a.z, b.z, c.z, d.z)
        self.maxz = max(self.maxz, a.z, b.z, c.z, d.z)

        if a==b or a==c or a==d or b==c or b==d or c==d:
            self.degenerated += 1

        # Create the "orientation mask"
        # If a bit is 1 then the corresponding vertex lies to the left
        # of the preceding edge

        # Check C against AB
        n = b-a
        n = vec3(-n.y, n.x)
        dist = (c-a)*n
        if dist>=0:
            mask = 0x01
        else:
            mask = 0x00

        # Check D against BC
        n = c-b
        n = vec3(-n.y, n.x)
        dist = (d-b)*n
        if dist>=0:
            mask |= 0x02

        # Check A against CD
        n = d-c
        n = vec3(-n.y, n.x)
        dist = (a-c)*n
        if dist>=0:
            mask |= 0x04

        # Check B against DA
        n = a-d
        n = vec3(-n.y, n.x)
        dist = (b-d)*n
        if dist>=0:
            mask |= 0x08

        self.orientations[mask]+=1

    def onEnd(self):
        print "Image resolution : %dx%d"%(self.xres, self.yres)
        print "Mico polygons    :",self.mpcount
        print "Degenerate       :",self.degenerated
        for k in self.orientations:
            if self.orientations[k]>0:
                print "%-17s: %d"%(self.orient_names[k], self.orientations[k])
        print "X range: %f - %f"%(self.minx, self.maxx)
        print "Y range: %f - %f"%(self.miny, self.maxy)
        print "Z range: %f - %f"%(self.minz, self.maxz)
        print self.samplecount, "pixel samples"

######################################################################
        
# MPCutArea
class MPCutArea(MPDumpReader):
    def __init__(self, x1, y1, x2, y2, filename="mpdump.mp", outname="mpdump_cut.mp"):
        MPDumpReader.__init__(self, filename)
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2
        self.outname = outname
        self.out = None
        self.count = 0
        self.scount = 0

    def onBegin(self):
        self.out = file(self.outname, "wb")
        self.out.write(struct.pack("i", self.sizefloat))
        print "Cut area: (%f,%f) - (%f, %f)"%(self.x1,self.y1,self.x2, self.y2)
        print "Output file:",self.outname

    def onImageInfo(self, imginfo):
        self.out.write(imginfo.bin)        

    def onSample(self, sample):
        x,y = sample.x, sample.y

        x1, y1 = self.x1, self.y1
        x2, y2 = self.x2, self.y2

        if x<x1 or x>x2 or y<y1 or y>y2:
            return

        self.out.write(sample.bin)
        self.scount += 1

    def onMP(self, mp):
        a,b,c,d = mp.a,mp.b,mp.c,mp.d
        x1, y1 = self.x1, self.y1
        x2, y2 = self.x2, self.y2

        if a.x<x1 and b.x<x1 and c.x<x1 and d.x<x1:
            return
        if a.x>x2 and b.x>x2 and c.x>x2 and d.x>x2:
            return
        if a.y<y1 and b.y<y1 and c.y<y1 and d.y<y1:
            return
        if a.y>y2 and b.y>y2 and c.y>y2 and d.y>y2:
            return

        self.out.write(mp.bin)
        self.count += 1
        

    def onEnd(self):
        print "%d/%d micro polygons written"%(self.count, self.mpcount)
        print "%d/%d samples written"%(self.scount, self.samplecount)
        self.out.close()
      
######################################################################

# MPRender
class MPRender(MPDumpReader):
    def __init__(self, filename="mpdump.mp", sx=-1, sy=-1, zoom=1.0, random_colors = False):
        MPDumpReader.__init__(self, filename)
        self.minz = 99999
        self.maxz = -1
        self.sx   = sx
        self.sy   = sy
        self.zoom = zoom
        self.random_colors = random_colors

    def display(self):
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        glDisable(GL_LIGHTING)
        glDisable(GL_COLOR_MATERIAL)
	glEnable(GL_DEPTH_TEST)
        glDisable(GL_CULL_FACE)

	glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
	glOrtho(0.0, self.xres, 0.0, self.yres, self.minz-1, self.maxz+1)
        
	glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        glTranslated(0,self.yres,0)
        glScaled(1,-1,-1)

        sx = self.sx
        sy = self.sy
        zoom = self.zoom
        glTranslated(self.xres/2-sx, self.yres/2-sy, 0)
        glTranslated(sx,sy,0)
        glScale(zoom,zoom,1)
        glTranslated(-sx,-sy,0)
        
        glColor3d(1,1,1)

        # Draw the micro polygons
	glCallList(1)

        if self.zoom>3:
            # Draw the pixel grid
            glColor3d(0.4,0.4,0.6)
            glDisable(GL_DEPTH_TEST)
            glTranslate(0,0,0.5*(self.minz+self.maxz))
            self.drawPixelGrid()

            # Draw the samples
            glColor3d(1,1,0.3)
            glCallList(2)
        
	glutSwapBuffers()

    def drawPixelGrid(self):
        glBegin(GL_LINES)
        for x in range(self.xres):
            glVertex3d(x,0,0)
            glVertex3d(x,self.yres,0)
        for y in range(self.yres):
            glVertex3d(0,y,0)
            glVertex3d(self.xres,y,0)
        glEnd()

        if self.zoom>10:
            glColor3d(0,0,0)
            px = int(self.sx)
            py = int(self.sy)
            glRasterPos3d(px, py+0.99, 0)
            self.drawText("%d/%d"%(px,py))

    def drawText(self, txt, font=GLUT_BITMAP_9_BY_15):
        for c in txt:
            glutBitmapCharacter(font, ord(c))

    def drawSamples(self):
        glBegin(GL_LINES)
        for x,y in self.samples:
            r = 0.07
            glVertex3d(x-r, y-r, 0)
            glVertex3d(x+r, y+r, 0)
            glVertex3d(x-r, y+r, 0)
            glVertex3d(x+r, y-r, 0)
        glEnd()

    def onBegin(self):
	glutInit(sys.argv)
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
	glutCreateWindow('mp dump')
	glutDisplayFunc(self.display)
	glNewList(1, GL_COMPILE)
        self.samples = []

    def onImageInfo(self, imginfo):
        if self.sx==-1:
            self.sx = imginfo.xres/2
        if self.sy==-1:
            self.sy = imginfo.yres/2
        glutReshapeWindow(self.xres, self.yres)

    def onSample(self, sample):
        x,y = sample.x, sample.y
        self.samples.append((x,y))
#        glPushMatrix()
#        glTranslated(sample.x, sample.y, 0)
#        glutSolidSphere(0.1, 8, 8)
#        glPopMatrix()

    def onMP(self, mp):
        a,b,c,d,col = mp.a,mp.b,mp.c,mp.d,mp.col

        self.minz = min(self.minz, a.z, b.z, c.z, d.z)
        self.maxz = max(self.maxz, a.z, b.z, c.z, d.z)

        glBegin(GL_QUADS)
        if self.random_colors:
            col = color_noise(18.43*(a+b+c+d))
        glColor3d(col.x, col.y, col.z)
        glVertex3d(a.x, a.y, a.z)
        glVertex3d(b.x, b.y, b.z)
        glVertex3d(c.x, c.y, c.z)
        glVertex3d(d.x, d.y, d.z)
        glEnd()

    def onEnd(self):
	glEndList()

	glNewList(2, GL_COMPILE)
        self.drawSamples()
	glEndList()
        
        print "Center at %f, %f - zoom: %f"%(self.sx, self.sy, self.zoom)
	glutMainLoop()

######################################################################

def usage():
    print """Usage: mpanalyse.py [Options] [Mode]

This tool post processes an Aqsis micro polygon dump file. The Mode parameter
can be one of:

info   - Computes micro polygon statistics (this is the default mode).
render - Displays the micro polygons.
cut    - Dump all information relevant for one pixel into a new file.

Options:

-h / --help    Show this help text
-i / --input   Set the input file name (default: "mpdump.mp")
-o / --output  Set the output file name (default: "mpdump_cut.mp")
-p / --pixel   Specify a pixel position (default: center of the image)
-z / --zoom    Specify a zoom value (default: 1.0)
-c / --colored Display the micro polygon using random colors

Examples:

Dump a new file that contains all micro polygons that are relevant for
pixel 50/30:

   mpanalyse.py -p50,30 cut
   
Display the micro polygons with a view centered at pixel 50/30 and a zoom
value of 100:

   mpanalyse.py -p50.5,30.5 -z100 render
"""

######################################################################

inputfilename = "mpdump.mp"
outputfilename = "mpdump_cut.mp"
px = -1
py = -1
zoom = 1.0
random_colors = False
mode = "info"

try:
    opts, args = getopt.getopt(sys.argv[1:], "hp:i:o:z:c", ["help", "pixel=", "input=", "output=", "zoom=", "colored"])
except getopt.GetoptError:
    sys.exit(1)

for o, a in opts:
    if o in ("-h", "--help"):
        usage()
        sys.exit()
    if o in ("-p", "--pixel"):
        px,py = a.split(",")
        px = float(px)
        py = float(py)
    if o in ("-i", "--input"):
        inputfilename = a
    if o in ("-o", "--output"):
        outputfilename = a
    if o in ("-z", "--zoom"):
        zoom = float(a)
    if o in ("-c", "--colored"):
        random_colors = True

if len(args)==1:
    mode = args[0]
elif len(args)>1:
    usage()
    sys.exit()

print 'Reading file "%s"...'%inputfilename
if mode=="info":
    rd = MPInfo(inputfilename)
elif mode=="render":
    rd = MPRender(inputfilename, px, py, zoom, random_colors)
elif mode=="cut":
    rd = MPCutArea(px, py, px+1, py+1, inputfilename, outputfilename)
rd.read()
