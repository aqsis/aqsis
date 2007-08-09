#!/usr/bin/env python
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

import sys, os, os.path, struct, getopt
from cgkit.cgtypes import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from cgkit.sl import *

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
    
    def __init__(self, filename="mpdump.mp", numbers=None):
        self.filename = filename
        # None or a list of micro polygon numbers which should be processed,
        # all other micro polygons are ignored
        self.numbers = numbers
        self.sizefloat = 0
        self.sizevec3 = 0
        self.xres = 0
        self.yres = 0
        # The number of the micro polygon (1-based)
        self.mpnumber = 0
        # The number of micro polygons
        self.mpcount = 0
        self.samplecount = 0
        self.orient_names = {0:"Convex (CW)",
                             1:"Concave (CW, B)",
                             2:"Concave (CW, C)",
                             3:"Twisted (3, ABxCD)",
                             4:"Concave (CW, D)",
                             5:"Impossible",
                             6:"Twisted (6, BCxDA)",
                             7:"Concave (CCW, A)",
                             8:"Concave (CW, A)",
                             9:"Twisted (9, BCxDA)",
                             10:"Impossible",
                             11:"Concave (CCW, D)",
                             12:"Twisted (c, ABxCD)",
                             13:"Concave (CCW, C)",
                             14:"Concave (CCW, B)",
                             15:"Convex (CCW)"}

    def read(self):
        self.sizefloat = 0
        self.sizevec3 = 0
        self.xres = 0
        self.yres = 0
        self.mpcount = 0
        self.mpnumber = 0
        self.samplecount = 0

        f = file(self.filename, "rb")
        # sizeof(TqFloat)
        s = f.read(4)
        self.sizefloat = struct.unpack("i", s)[0]
        self.sizevec3 = 3*self.sizefloat

        self.onBegin()
        sizevec3 = self.sizevec3
        while 1:
            sid = f.read(2)
            if sid=="":
                break
            id = struct.unpack("h", sid[0:2])[0]
            # Micro polygon?
            if id==1:
                s = f.read(6*sizevec3)
                self.mpnumber += 1
                if self.numbers==None or self.mpnumber in self.numbers:
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
                    self.mpcount += 1
                    self.onMP(mp)

                if self.mpnumber%5000==0:
                    print "\015%d"%self.mpnumber,
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
        f.close()
        self.onEnd()

    def computeOrientationMask(self, mp):
        # Create the "orientation mask"
        # If a bit is 1 then the corresponding vertex lies to the left
        # of the preceding edge
        # Note: On screen everything is displayed in opposite order as
        # Y is pointing *down*.

        a,b,c,d = mp.a,mp.b,mp.c,mp.d

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

        return mask
        

    def onMP(self, mp): pass
    def onSample(self, sample):  pass
    def onImageInfo(self, imginfo): pass
    def onBegin(self): pass
    def onEnd(self): pass

######################################################################

# MPInfo
class MPInfo(MPDumpReader):
    def __init__(self, filename="mpdump.mp", numbers=None):
        MPDumpReader.__init__(self, filename, numbers)
        self.minx = 99999
        self.maxx = -1
        self.miny = 99999
        self.maxy = -1
        self.minz = 99999
        self.maxz = -1
        self.degenerated = 0
        self.orientations = {0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0,
                             9:0, 10:0, 11:0, 12:0, 13:0, 14:0, 15:0}

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

        mask = self.computeOrientationMask(mp)
        self.orientations[mask]+=1

    def onEnd(self):
        print "Image resolution  : %dx%d"%(self.xres, self.yres)
        print "Micro polygons    :",self.mpcount
        print "Degenerate        :",self.degenerated
        for k in self.orientations:
            if self.orientations[k]>0:
                print "%-18s: %d"%(self.orient_names[k], self.orientations[k])
        print "X range: %f - %f"%(self.minx, self.maxx)
        print "Y range: %f - %f"%(self.miny, self.maxy)
        print "Z range: %f - %f"%(self.minz, self.maxz)
        print self.samplecount, "pixel samples"

######################################################################

# MPDump
class MPDump(MPDumpReader):
    def __init__(self, filename="mpdump.mp", numbers=None):
        MPDumpReader.__init__(self, filename, numbers)
        self.bits = ["0000", "0001", "0010", "0011",
                     "0100", "0101", "0110", "0111",
                     "1000", "1001", "1010", "1011",
                     "1100", "1101", "1110", "1111"]

    def onBegin(self):
        pass

    def onImageInfo(self, imginfo):
        print "ImageInfo:",imginfo.xres, imginfo.yres

    def onSample(self, sample):
        print "sample:",sample.px, sample.py, sample.idx, sample.x, sample.y

    def onMP(self, mp):
        a,b,c,d = mp.a,mp.b,mp.c,mp.d
        mask = self.computeOrientationMask(mp)
        print 70*"-"
        print "micro poly %d: orientation mask: %d (%s, %s, %s)"%(self.mpnumber, mask, hex(mask), self.bits[mask], self.orient_names[mask])
        print "%11s A: %s"%("",a)
        print "%11s B: %s"%("",b)
        print "%11s C: %s"%("",c)
        print "%11s D: %s"%("",d)
        print "%11s Color   : %s"%("",mp.col)
        print "%11s Opacity : %s"%("",mp.opacity)

    def onEnd(self):
        print self.mpcount, "micro polygons"

######################################################################
        
# MPCutArea
class MPCutArea(MPDumpReader):
    def __init__(self, x1, y1, x2, y2, filename="mpdump.mp", outname="mpdump_cut.mp", numbers=None):
        MPDumpReader.__init__(self, filename, numbers)
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
    def __init__(self, filename="mpdump.mp", sx=-1, sy=-1, zoom=1.0, random_colors = False, orientation_marking=False, numbers=None):
        MPDumpReader.__init__(self, filename, numbers)
        
        # 0: Not intialized - 1:Initialized - 2:Main loop entered
        self.glut_state = 0
        
        self.minz = 99999
        self.maxz = -1
        self.sx   = sx
        self.sy   = sy
        self.zoom = zoom
        self.random_colors = random_colors
        self.polygon_mode = GL_FILL
        self.grid_flag = True
        self.samples_flag = True
        self.mark_all_but_convex_ccw = orientation_marking
                
        # Current mouse position (in Aqsis pixel coordinates)
        self.currentx = None
        self.currenty = None
        # Currently selected area
        self.rectX1 = None
        self.rectY1 = None
        self.rectY2 = None
        self.rectY2 = None

    def onBegin(self):
        self.initGlut()
	glNewList(1, GL_COMPILE)
        self.samples = []
        self.strings = []

    def onImageInfo(self, imginfo):
        if self.sx==-1:
            self.sx = imginfo.xres/2
        if self.sy==-1:
            self.sy = imginfo.yres/2
        if self.glut_state==1:
            glutReshapeWindow(self.xres, self.yres)

    def onSample(self, sample):
        x,y = sample.x, sample.y
        self.samples.append((x,y))

    def onMP(self, mp):

        mask = self.computeOrientationMask(mp)

        a,b,c,d,col = mp.a,mp.b,mp.c,mp.d,mp.col

        self.minz = min(self.minz, a.z, b.z, c.z, d.z)
        self.maxz = max(self.maxz, a.z, b.z, c.z, d.z)

        if self.mark_all_but_convex_ccw and mask!=15:
            col = vec3(1,0,0)
        elif self.random_colors:
            col = color_noise(18.43*(a+b+c+d))
        glColor3d(col.x, col.y, col.z)

        # Twisted, AB intersects CD
        if mask==3 or mask==12:
            u = b-a
            v = d-c
            v2 = vec3(v.y, -v.x,0)
            t = (c*v2-a*v2)/(u*v2)
            s = a+t*u
            glBegin(GL_TRIANGLES)
            glVertex3d(s.x, s.y, s.z)
            glVertex3d(d.x, d.y, d.z)
            glVertex3d(a.x, a.y, a.z)
            glVertex3d(s.x, s.y, s.z)
            glVertex3d(b.x, b.y, b.z)
            glVertex3d(c.x, c.y, c.z)
            glEnd()
        # Twisted, BC intersects DA
        elif mask==6 or mask==9:
            u = c-b
            v = a-d
            v2 = vec3(v.y, -v.x,0)
            t = (d*v2-b*v2)/(u*v2)
            s = b+t*u
            glBegin(GL_TRIANGLES)
            glVertex3d(a.x, a.y, a.z)
            glVertex3d(b.x, b.y, b.z)
            glVertex3d(s.x, s.y, s.z)
            glVertex3d(s.x, s.y, s.z)
            glVertex3d(c.x, c.y, c.z)
            glVertex3d(d.x, d.y, d.z)
            glEnd()
        # A concave
        elif mask==7 or mask==8:
            glBegin(GL_TRIANGLE_FAN)
            glVertex3d(a.x, a.y, a.z)
            glVertex3d(b.x, b.y, b.z)
            glVertex3d(c.x, c.y, c.z)
            glVertex3d(d.x, d.y, d.z)
            glEnd()
        # B concave
        elif mask==1 or mask==14:
            glBegin(GL_TRIANGLE_FAN)
            glVertex3d(b.x, b.y, b.z)
            glVertex3d(c.x, c.y, c.z)
            glVertex3d(d.x, d.y, d.z)
            glVertex3d(a.x, a.y, a.z)
            glEnd()
        # C concave
        elif mask==2 or mask==13:
            glBegin(GL_TRIANGLE_FAN)
            glVertex3d(c.x, c.y, c.z)
            glVertex3d(d.x, d.y, d.z)
            glVertex3d(a.x, a.y, a.z)
            glVertex3d(b.x, b.y, b.z)
            glEnd()
        # D concave
        elif mask==4 or mask==11:
            glBegin(GL_TRIANGLE_FAN)
            glVertex3d(d.x, d.y, d.z)
            glVertex3d(a.x, a.y, a.z)
            glVertex3d(b.x, b.y, b.z)
            glVertex3d(c.x, c.y, c.z)
            glEnd()
        # Convex (CCW or CW)
        else:
            glBegin(GL_QUADS)
            glVertex3d(a.x, a.y, a.z)
            glVertex3d(b.x, b.y, b.z)
            glVertex3d(c.x, c.y, c.z)
            glVertex3d(d.x, d.y, d.z)
            glEnd()

        # Display the vertex order if individual micro polygons have
        # been selected
        if self.numbers!=None:
            self.strings.append((a.x,a.y,"A"))
            self.strings.append((b.x,b.y,"B"))
            self.strings.append((c.x,c.y,"C"))
            self.strings.append((d.x,d.y,"D"))

    def onEnd(self):
	glEndList()

	glNewList(2, GL_COMPILE)
        self.drawSamples()
	glEndList()
        
        print "Center at %f, %f - zoom: %f"%(self.sx, self.sy, self.zoom)
        if self.glut_state==1:
            self.glut_state = 2
            glutMainLoop()

    # initGlut
    def initGlut(self):
        """Initialize the GLUT window used to display the micro polygons."""
        
        if self.glut_state==0:
            glutInit(sys.argv)
            glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
            glutCreateWindow('mp dump')

            # Create popup menu...
            polymode = glutCreateMenu(self.onMenuPolygonMode)
            glutAddMenuEntry("Fill", 1)
            glutAddMenuEntry("Line", 2)
            glutAddMenuEntry("Point", 3)

            glutCreateMenu(self.onMenu)
            glutAddMenuEntry("Zoom in (+)", 2)
            glutAddMenuEntry("Zoom out (-)", 3)
            glutAddMenuEntry("Center this pixel", 1)
            glutAddMenuEntry("Center area", 5)
            glutAddMenuEntry("Cut area", 4)
            glutAddMenuEntry("Toggle sample points", 7)
            glutAddMenuEntry("Toggle pixel grid", 8)
            glutAddMenuEntry("Toggle random colors", 6)
            glutAddMenuEntry("Toggle orientation marking", 9)
            glutAddSubMenu("Polygon mode", polymode)
            glutAddMenuEntry("Help (to stdout)", 10)
            glutAttachMenu(GLUT_RIGHT_BUTTON)

            glutDisplayFunc(self.display)
            glutKeyboardFunc(self.onKey)
            glutMouseFunc(self.onMouseButton)
            glutMotionFunc(self.onMouseMotion)
            glutPassiveMotionFunc(self.onPassiveMouseMotion)

            self.glut_state = 1        

    # display
    def display(self):
        """GLUT Display callback."""
        
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
        glPolygonMode(GL_FRONT_AND_BACK, self.polygon_mode)
	glCallList(1)

        glTranslate(0,0,0.5*(self.minz+self.maxz))
        if self.zoom>3:
            # Draw the pixel grid
            glColor3d(0.4,0.4,0.6)
            glDisable(GL_DEPTH_TEST)
            if self.grid_flag:
                self.drawPixelGrid()

            # Draw the samples
            if self.samples_flag:
                glColor3d(1,1,0.3)
                glCallList(2)

            # Draw strings
            glColor3d(1,1,1)
            self.drawStrings()

        # Draw the selection rectangle
        if self.rectX1!=None:
            glDisable(GL_DEPTH_TEST)
            glDisable(GL_LIGHTING)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
            glColor3d(1,1,0)
            glBegin(GL_QUADS)
            glVertex2d(self.rectX1, self.rectY1)
            glVertex2d(self.rectX2, self.rectY1)
            glVertex2d(self.rectX2, self.rectY2)
            glVertex2d(self.rectX1, self.rectY2)
            glEnd()
        
	glutSwapBuffers()

    # drawPixelGrid
    def drawPixelGrid(self):
        """Draw the pixel grid lines."""
        glBegin(GL_LINES)
        for x in range(self.xres):
            glVertex3d(x,0,0)
            glVertex3d(x,self.yres,0)
        for y in range(self.yres):
            glVertex3d(0,y,0)
            glVertex3d(self.xres,y,0)
        glEnd()

        if self.zoom>10:
            glColor3d(0,0,0.4)
            px = int(self.sx)
            py = int(self.sy)
            glRasterPos3d(px, py+0.99, 0)
            self.drawText("%d/%d"%(px,py))

    # drawText
    def drawText(self, txt, font=GLUT_BITMAP_9_BY_15):
        """Helper method to draw text using GLUT."""
        for c in txt:
            glutBitmapCharacter(font, ord(c))

    # drawSamples
    def drawSamples(self):
        """Draw the pixel samples previously stored in the object."""
        glBegin(GL_LINES)
        for x,y in self.samples:
            r = 0.07
            glVertex3d(x-r, y-r, 0)
            glVertex3d(x+r, y+r, 0)
            glVertex3d(x-r, y+r, 0)
            glVertex3d(x+r, y-r, 0)
        glEnd()

    # drawStrings
    def drawStrings(self):
        """Draw strings."""
        for x,y,txt in self.strings:
            r = 0.01
            glRasterPos2d(x+r, y-r)
            self.drawText(txt)
            glBegin(GL_LINES)
            glVertex3d(x-r, y, 0)
            glVertex3d(x+r, y, 0)
            glVertex3d(x, y+r, 0)
            glVertex3d(x, y-r, 0)
            glEnd()
        

    # zoomIn
    def zoomIn(self):
        self.zoom *= 1.1
        print "Zoom in: %1.2f"%self.zoom
        glutPostRedisplay()

    # zoomOut
    def zoomOut(self):
        if self.zoom>1.0:
            self.zoom /= 1.1
            if self.zoom<1.0:
                self.zoom = 1.0
            print "Zoom out: %1.2f"%self.zoom
            glutPostRedisplay()


    # onMenu
    def onMenu(self, idx):
        """Main menu callback."""
        
        # Center current pixel
        if idx==1:
            self.sx = int(self.currentx)+0.5
            self.sy = int(self.currenty)+0.5
            glutPostRedisplay()
        # Zoom In
        elif idx==2:
            self.zoomIn()
        # Zoom Out
        elif idx==3:
            self.zoomOut()
        # Cut
        elif idx==4:
            if self.rectX1==None:
                print "No area selected"
            else:
                name, ext = os.path.splitext(self.filename)
                cutter = MPCutArea(self.rectX1, self.rectY1,
                                   self.rectX2, self.rectY2,
                                   self.filename, name+"_cut"+ext)
                cutter.read()
                self.rectX1 = None
                self.rectY1 = None
                self.rectX2 = None
                self.rectY2 = None
                glutPostRedisplay()
        # Center area
        elif idx==5:
            if self.rectX1==None:
                print "No area selected"
            else:
                self.sx = 0.5*(self.rectX1+self.rectX2)
                self.sy = 0.5*(self.rectY1+self.rectY2)
                self.zoom = self.xres/(self.rectX2-self.rectX1)
                self.rectX1 = None
                self.rectY1 = None
                self.rectX2 = None
                self.rectY2 = None
                glutPostRedisplay()
        # Toggle random colors
        elif idx==6:
            print "Rereading input file..."
            self.random_colors = not self.random_colors
            self.read()
            glutPostRedisplay()
        # Toggle sample points
        elif idx==7:
            self.samples_flag = not self.samples_flag
            glutPostRedisplay()
        # Toggle pixel grid
        elif idx==8:
            self.grid_flag = not self.grid_flag
            glutPostRedisplay()
        # Toggle orientation marking
        elif idx==9:
            print "Rereading input file..."
            self.mark_all_but_convex_ccw = not self.mark_all_but_convex_ccw
            self.read()
            glutPostRedisplay()
        # Toggle orientation marking
        elif idx==10:
            print """Keys:

  2,4,6,8 - Pan (use the numpad)
  +, -    - Zoom in/out

Menu:

  Zoom in (+) - Same as + key
  Zoom out (-) - Same as - key
  Center this pixel - Center the pixel where you right clicked
  Center area - Center the selected area
  Cut area - Create a new file that only contains the selected area
  Toggle sample points - Hides or shows the sample points (only when zoomed in)
  Toggle pixel grid - Hides or shows the pixel grid (only when zoomed in)
  Toggle random colors - Toggles using pseudo random colors for the micro polys
  Toggle orientation marking - Display all micro polys in red which are not
                               convex and counterclockwise
  Polygon mode - Select between Fill, Line and Point drawing mode

An area can be selected using the left mouse button.
Note: If you toggle random colors or orientation markings, the input file
has to be reread. So be patient if the file is large.
            """

    # onMenuPolygonMode
    def onMenuPolygonMode(self, idx):
        """Sub menu "Polygon mode" callback."""
        if idx==1:
            self.polygon_mode = GL_FILL
        elif idx==2:
            self.polygon_mode = GL_LINE
        else:
            self.polygon_mode = GL_POINT
        glutPostRedisplay()

    # onKey
    def onKey(self, key, x, y):
        """Key callback."""
        
        width = glutGet(GLUT_WINDOW_WIDTH)
        height = glutGet(GLUT_WINDOW_HEIGHT)
        zx = self.zoom * float(width)/self.xres
        zy = self.zoom * float(height)/self.yres

        if key=="+":
            self.zoomIn()
        elif key=="-":
            self.zoomOut()
        elif key=="4":
            self.sx -= width/zx/20
            glutPostRedisplay()
        elif key=="6":
            self.sx += width/zx/20
            glutPostRedisplay()
        elif key=="8":
            self.sy -= height/zy/20
            glutPostRedisplay()
        elif key=="2":
            self.sy += height/zy/20
            glutPostRedisplay()

    # onMouseButton
    def onMouseButton(self, button, state, x, y):
        """Mouse button callback."""
        
        if button==GLUT_LEFT_BUTTON:
            if state==GLUT_DOWN:
                self.rectX1, self.rectY1 = self.mouse2pixel(x,y)
                self.rectX2 = self.rectX1
                self.rectY2 = self.rectY1
            else:
                self.rectX2, self.rectY2 = self.mouse2pixel(x,y)
                x1 = min(self.rectX1, self.rectX2)
                x2 = max(self.rectX1, self.rectX2)
                y1 = min(self.rectY1, self.rectY2)
                y2 = max(self.rectY1, self.rectY2)
                if x1==x2 or y1==y2:
                    x1 = x2 = y1 = y2 = None
                self.rectX1 = x1
                self.rectY1 = y1
                self.rectX2 = x2
                self.rectY2 = y2
                if x1!=None:
                    print "Area: %1.2f/%1.2f - %1.2f/%1.2f"%(self.rectX1, self.rectY1,self.rectX2, self.rectY2)
                glutPostRedisplay()

    # onPassiveMouseMotion
    def onPassiveMouseMotion(self, x, y):
        """Passive mouse motion callback."""
        px,py = self.mouse2pixel(x,y)
        self.currentx = px
        self.currenty = py
        glutSetWindowTitle("mp dump - Pixel: %d/%d - Pos: %1.3f/%1.3f"%(px,py,px,py))

    # onMouseMotion
    def onMouseMotion(self, x, y):
        """Mouse motion callback."""
        self.onPassiveMouseMotion(x,y)
        self.display()

    # mouse2pixel
    def mouse2pixel(self, x, y):
        """Convert a window pixel position to image pixel position."""
        width = glutGet(GLUT_WINDOW_WIDTH)
        height = glutGet(GLUT_WINDOW_HEIGHT)

        zx = self.zoom * float(width)/self.xres
        zy = self.zoom * float(height)/self.yres
        px = (x-width/2)/zx+self.sx
        py = (y-height/2)/zy+self.sy
        return px,py



######################################################################

def usage():
    print """Usage: mpanalyse.py [Options] [Mode]

This tool post processes an Aqsis micro polygon dump file. The Mode parameter
can be one of:

info   - Computes micro polygon statistics (this is the default mode).
render - Displays the micro polygons.
cut    - Dump all information relevant for one pixel into a new file.
dump   - Dump the information in the binary file as readable text.

Options:

-h / --help    Show this help text
-i / --input   Set the input file name (default: "mpdump.mp")
-o / --output  Set the output file name (default: "mpdump_cut.mp")
-p / --pixel   Specify a pixel position (default: center of the image)
-z / --zoom    Specify a zoom value (default: 1.0)
-c / --colored Display the micro polygon using random colors
-m / --marking Display micro polygons that are not convex and CCW in red
-n / --numbers Only process the micro polygons with the specified numbers

Examples:

Dump a new file that contains all micro polygons that are relevant for
pixel 50/30:

   mpanalyse.py -p50,30 cut
   
Display the micro polygons with a view centered at pixel 50/30 and a zoom
value of 100:

   mpanalyse.py -p50.5,30.5 -z100 render

Only display micro polygon 11-13:

  mpanalyse.py -n11,12,13 render
"""

######################################################################

inputfilename = "mpdump.mp"
outputfilename = "mpdump_cut.mp"
px = -1
py = -1
zoom = 1.0
random_colors = False
orientation_marking = False
numbers = None
mode = "info"

try:
    opts, args = getopt.getopt(sys.argv[1:], "hp:i:o:z:cmn:", ["help", "pixel=", "input=", "output=", "zoom=", "colored", "marking", "numbers"])
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
    if o in ("-m", "--marking"):
        orientation_marking = True
    if o in ("-n", "--numbers"):
        exec "numbers=[%s]"%a

if len(args)==1:
    mode = args[0]
elif len(args)>1:
    usage()
    sys.exit()

print 'Reading file "%s"...'%inputfilename
rd = None
if mode=="info":
    rd = MPInfo(inputfilename, numbers)
elif mode=="render":
    rd = MPRender(inputfilename, px, py, zoom, random_colors, orientation_marking, numbers)
elif mode=="cut":
    rd = MPCutArea(px, py, px+1, py+1, inputfilename, outputfilename, numbers)
elif mode=="dump":
    rd = MPDump(inputfilename, numbers)
    
if rd!=None:
    rd.read()
