#!/usr/bin/env python
#
######################################################################
#
# An example for an Aqsis display driver written in Python.
# Written by Matthias Baas (baas@ira.uka.de)
#
######################################################################
#
# Usage
# -----
#
# This display driver actually implements 3 separate display drivers:
#
# 1) A file display driver (default)
# 2) A framebuffer display driver (option -f or --framebuffer)
# 3) An inspection display driver that just dumps the messages (option -i
#    or --inspect)
#
# To use these display drivers copy this file into the "displays"
# subdirectory and add the following lines to the file "ddmsock.ini":
# 
# framebufferpy		displaydriver.py --framebuffer
# filepy		displaydriver.py
# inspect		displaydriver.py --inspect
#
# Then you can use the tokens "framebufferpy", "filepy" or "inspect"
# for the type argument in the call to RiDisplay() to use the corresponding
# display driver. In "RGBA" mode the drivers can already overlay the
# rendered image with a background image. You can specify the background
# image as a parameter to the RiDisplay() call:
#
# "string bgimage" [<imagename>]
#
# Furthermore, you can choose to save just the rendered image (including
# an alpha channel) or the composited image (without alpha):
#
# "integer savecomp" [0|1]
#
# The savecomp parameter is actually a boolean if you pass true (1)
# then the composite image is saved. Default is 0.
#
# Note: To use the file and framebuffer display drivers you must
# install the Python Imaging Library (PIL) and the Numeric package.
# The frambuffer driver further requires wxPython and the threading
# module (the latter is usually part of the standard library, but
# it might have been disabled when Python was compiled).
#
#######################################################################
#
# Implementation notes:
# ---------------------
#
# The base class which manages the communication with Aqsis is
# the class DisplayDriver. Each separate display driver derives
# from this class and implements the onXxx() handler methods.
# These methods are called whenever the corresponding message
# is sent by Aqsis.
#
# The class InspectDisplayDriver just prints all the information it
# gets. Here you can see all the values that Aqsis sends.
#
# The class FileDisplayDriver saves the image to disk using Numeric 
# and the Python Imaging Library (PIL). You can save any formats that
# PIL supports (the format is determined by the file extension).
#
# The class FrameBufferDisplayDriver forwards each message to the GUI.
# This class runs as a separate thread so that the GUI isn't blocked.
# The frambuffer driver uses wxPython as GUI toolkit. The application
# window is implemented by the class MainWin.
#
######################################################################

import sys, os, getopt, socket, struct, array

origout = sys.stdout
origerr = sys.stderr

# Import non-standard packages which might be missing...
try:
    import Image
    has_PIL = 1
except:
    has_PIL = 0
try:
    import Numeric
    has_numeric = 1
except:
    has_numeric = 0
try:
    from threading import *
    has_threading = 1
except:
    has_threading = 0
try:
    from wxPython.wx import *
    has_wxpython = 1
except:
    has_wxpython = 0
try:
    import win32api, win32process
    has_win32 = 1
except:
    has_win32 = 0


# Message IDs
MSGID_STRING           = 0
MSGID_FORMATQUERY      = 1
MSGID_DATA             = 2
MSGID_OPEN             = 3
MSGID_CLOSE            = 4
MSGID_FILENAME         = 5
MSGID_NL               = 6
MSGID_NP               = 7
MSGID_DISPLAYTYPE      = 8
MSGID_ABANDON          = 9
MSGID_USERPARAM        = 10
MSGID_FORMATRESPONSE   = 0x8001
MSGID_CLOSEACKNOWLEDGE = 0x8002


# DisplayDriver
class DisplayDriver:
    """Base class for a display driver."""

    def __init__(self):
        """Constructor."""

        self.port = int(os.environ["AQSIS_DD_PORT"])

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(("localhost", self.port))

        self.running_flag = 1

    def MessageLoop(self):
        """Message loop."""
        
        while self.running_flag:
            # Wait for the next message...
            try:
                msgid,msglen,msgbody = self.ReceiveRawMessage()
            except:
                print "Connection to the renderer lost unexpectedly"
                msgid=None
                
            if msgid==None:
                break

            # Convert the data and call the appropriate message handler...

            # FormatQuery
            if msgid==MSGID_FORMATQUERY:
                self.onFormatQuery()
                retmsg = struct.pack("iii",MSGID_FORMATRESPONSE,12,2)
                self.sock.send(retmsg)
            # Data
            elif msgid==MSGID_DATA:
                XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,DataLength = struct.unpack("iiiiii", msgbody[:24])
                Data = msgbody[24:]
                self.onData(XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data)
            # Open
            elif msgid==MSGID_OPEN:
                XRes,YRes,Channels,NotUsed,CropWindowXMin,CropWindowXMax,CropWindowYMin,CropWindowYMax = struct.unpack("iiiiiiii",msgbody)
                self.onOpen(XRes, YRes, Channels, CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax)
            # Close
            elif msgid==MSGID_CLOSE:
                self.onClose()
                retmsg = struct.pack("ii",MSGID_CLOSEACKNOWLEDGE,8)
                self.sock.send(retmsg)
            # Filename
            elif msgid==MSGID_FILENAME:
                strlen = struct.unpack("i",msgbody[:4])[0]
                fname = struct.unpack("%ds"%strlen,msgbody[4:4+strlen])[0]
                self.onFileName(fname)
            # NL
            elif msgid==MSGID_NL:
                m = struct.unpack("ffffffffffffffff",msgbody)
                self.onNL(m)
            # NP
            elif msgid==MSGID_NP:
                m = struct.unpack("ffffffffffffffff",msgbody)
                self.onNP(m)
            # DisplayType
            elif msgid==MSGID_DISPLAYTYPE:
                strlen = struct.unpack("i",msgbody[:4])[0]
                dtype = struct.unpack("%ds"%strlen,msgbody[4:4+strlen])[0]
                self.onDisplayType(dtype)
            # UserParam
            elif msgid==MSGID_USERPARAM:
                datatype, namelength, datalength, datacount = struct.unpack("iiii",msgbody[:16])
                # ndlen: +1 because of the trailing 0-byte in the name
                ndlen = namelength+datalength+1  
                name_and_data = struct.unpack("%ds"%(ndlen),msgbody[16:16+ndlen])[0]
                name = name_and_data[:namelength]
                rawdata = name_and_data[namelength+1:]
                data = self.decodeUserParamData(datatype, datacount, rawdata)
                self.onUserParam(name, data)
            # Abandon
            elif msgid==MSGID_ABANDON:
                self.onAbandon()
                break
            # Unknown message
            else:
                sys.stderr.write("UNKNOWN MESSAGE (ID=%d)\n"%msgid)

        self.sock.close()

    # decodeUserParamData
    def decodeUserParamData(self, datatype, datacount, data):

        if datatype==4:
            res = data.split(chr(0))
            res = res[:-1]
            if len(res)==1:
                res = res[0]
            return res

        if datatype==1:
            elemcount = 1
            fmt = "f"
        elif datatype==2:
            elemcount = 1
            fmt = "i"
        elif datatype==3 or datatype==5 or datatype==8 or datatype==9:
            elemcount = 3
            fmt = "f"
        elif datatype==7:
            elemcount = 4
            fmt = "f"
        elif datatype==11:
            elemcount = 16
            fmt = "f"
        else:
            sys.stderr.write("Unknown user param data\n")
            return None

        if len(data)!=elemcount*datacount*4:
            sys.stderr.write("User param data has invalid size\n")
            return None
        
        res = []
        for i in range(datacount):
            elem = []
            for j in range(elemcount):
                v = struct.unpack(fmt,data[:4])[0]
                elem.append(v)
                data = data[4:]
            if elemcount==1:
                elem = elem[0]
            res.append(elem)
        if datacount==1:
            res = res[0]
            
        return res

    # ReceiveSome
    def ReceiveSome(self, length):
        """Returns the next length bytes."""
        
        buffer = ""
        tot = 0
        need = length
        while need>0:
            data = self.sock.recv(need)
            n = len(data)
            if (n>0):
                buffer += data
                need -= n
                tot += n
            else:
                return None
        return buffer

    # ReceiveRawMessage
    def ReceiveRawMessage(self):
        """Returns the raw data of the next message."""
        
        head = self.ReceiveSome(8)
        if head==None:
            return None,None,None

        msgid, msglen = struct.unpack("ii",head)
        msgbody = self.ReceiveSome(msglen-8)

        return msgid, msglen, msgbody

    # Message handlers:

    def onFormatQuery(self): pass
    def onFileName(self, filename): pass
    def onDisplayType(self, dtype): pass
    def onNL(self, m): pass
    def onNP(self, m): pass
    def onOpen(self, XRes, YRes, Channels, CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax): pass
    def onClose(self): pass
    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data): pass
    def onAbandon(self): pass
    def onUserParam(self, name, data): pass

######################################################################

# InspectDisplayDriver
class InspectDisplayDriver(DisplayDriver):

    def __init__(self):
        DisplayDriver.__init__(self)
        
        print "----Argument list:------------------"
        i=0
        for a in sys.argv:
            print '%d: "%s"'%(i,a)
            i+=1
        print "----Available modules:--------------"
        print "Imaging (PIL): %s"%self.avail(has_PIL)
        print "Numeric      : %s"%self.avail(has_numeric)
        print "Threading    : %s"%self.avail(has_threading)
        print "wxPython     : %s"%self.avail(has_wxpython)
        print "win32        : %s"%self.avail(has_win32)
        print "------------------------------------"

    def avail(self, flag):
        if flag:
            return "OK"
        else:
            return "not available"
       
    def onFormatQuery(self):
        print "FormatQuery"

    def onFileName(self, filename):
        print 'FileName "%s"'%filename

    def onDisplayType(self, dtype):
        print 'DisplayType "%s"'%dtype

    def onNL(self, m):
        print "NL",m

    def onNP(self, m):
        print "NP",m

    def onOpen(self, XRes, YRes, Channels, 
               CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax):
        print "Open - Res:%dx%d  Channels:%d  Crop:(%d,%d)-(%d,%d)"%(XRes,YRes,Channels,CropWindowXMin,CropWindowYMin,CropWindowXMax,CropWindowYMax)

    def onClose(self):
        print "Close"

    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data):
        print "Data - Rect:(%d,%d)-(%d,%d)  ElementSize:%d  DataLen:%d"%(XMin,YMin,XMaxPlus1,YMaxPlus1,ElementSize,len(Data))

    def onAbandon(self):
        print "Abandon"

    def onUserParam(self, name, data):
        print "UserParam - %s = %s"%(name,data)


# FileDisplayDriver
class FileDisplayDriver(DisplayDriver):

    def __init__(self):
        DisplayDriver.__init__(self)

        self.filename = "out.tif"
        self.img = None
        self.imagemanager = ImageManager()
        self.savecomp = 0

    def onFileName(self, filename):
        self.filename = filename

    def onOpen(self, XRes, YRes, Channels, 
               CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax):

        self.imagemanager.initialise(XRes, YRes, Channels)

#        if Channels==4:
#            mode = "RGBA"
#        else:
#            mode = "RGB"
#        self.img = Image.new(mode, (XRes, YRes))

    def onClose(self):
        nimg = self.imagemanager.getImage(comp=self.savecomp)
        img = self.imagemanager.convertToPIL(nimg)
        try:
            img.save(self.filename)
        except:
            sys.stderr.write("Couldn't save image file \"%s\"."%filename)
        
#        self.img.save(self.filename)

    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data):

        w = XMaxPlus1-XMin
        h = YMaxPlus1-YMin

        self.imagemanager.setBucket(XMin,YMin,w,h,Data)

#        if self.img.mode=="RGBA":
#            i=0
#            for y in range(YMin, YMaxPlus1):
#                for x in range(XMin, XMaxPlus1):
#                    r,g,b,a = struct.unpack("ffff", Data[i:i+16])
#                    self.img.putpixel((x,y),(int(r),int(g),int(b),int(a)))
#                    i+=16
#        else:
#            i=0
#            for y in range(YMin, YMaxPlus1):
#                for x in range(XMin, XMaxPlus1):
#                    r,g,b = struct.unpack("fff", Data[i:i+12])
#                    self.img.putpixel((x,y),(int(r),int(g),int(b)))
#                    i+=12

    def onUserParam(self, name, data):
        if name=="bgimage":
            self.imagemanager.setBGImageName(data)
        elif name=="savecomp":
            if isinstance(data, int):
                self.savecomp = data


######################################################################
######################################################################
######################################################################

# ImageManager
class ImageManager:
    """Handles the images and composites them."""

    def __init__(self):
        self.xres = 0
        self.yres = 0
        self.samples_per_element = 0
        self.has_alpha = 0

        self.bgimagename = None

        # Background image
        self.bgimage = None
        # Rendered image (RGB)
        self.image = None
        # Alpha channel
        self.alpha = None

        self.is_initialised = 0

    # initialise
    def initialise(self, xres, yres, samples_per_element):
        self.xres = xres
        self.yres = yres
        self.samples_per_element = samples_per_element
        self.has_alpha = 0
        if samples_per_element==4:
            self.has_alpha = 1

        # Create images (as Numeric array)
#        self.image = Numeric.zeros((yres,xres,3),Numeric.Int16)
        self.image = Numeric.array(xres*yres*3*[128], Numeric.Int16)
        self.image.shape = (yres,xres,3)
        self.image.savespace(1)
#        self.alpha = Numeric.zeros((yres,xres,3),Numeric.Int16)
        self.alpha = Numeric.array(xres*yres*3*[255], Numeric.Int16)
        self.alpha.shape = (yres,xres,3)
        self.alpha.savespace(1)
        self.bgimage = self.createBGImage()

        # Load background image
        if self.bgimagename!=None:
            self.loadBGImage(self.bgimagename)

        self.is_initialised = 1

    # isInitialised
    def isInitialised(self):
        """Return True, if the manager is already initialised."""
        return self.is_initialised

    # setBGImageName
    def setBGImageName(self, filename):
        """Set the name of the background image to load.

        The image will be loaded during initialisation.
        """
        self.bgimagename = filename

    # createBGImage
    def createBGImage(self, blocksize=8):
        """Create checkered background image.

        The resolution of the image is taken from self.xres/self.yres.
        The image is stored in self.bgimage.
        The return value is the created image.
        """
        
        bg = Numeric.zeros((self.yres,self.xres,3),Numeric.Int16)
        bg.savespace(1)
        for j in range(self.yres/blocksize):
            for i in range(self.xres/blocksize):
                x=i*blocksize
                y=j*blocksize
                if (i+j)%2==0:
                    c=140
                else:
                    c=255
                bg[y:y+blocksize,x:x+blocksize,:]=c

        self.bgimage = bg
        return self.bgimage

    # loadBGImage
    def loadBGImage(self, filename):
        try:
            pimg = Image.open(filename)
        except:
            sys.stderr.write("Couldn't read image file \"%s\".\n"%filename)
            return
            
        w,h = pimg.size
        imgdata = pimg.tostring()
        imgdata = array.array('B', imgdata)
        nimg = Numeric.array(imgdata, Numeric.Int16)
        nimg.shape = (h,w,3)
        img = Numeric.zeros((self.yres,self.xres,3), Numeric.Int16)
        img[:,:,:] = nimg[0:self.yres, 0:self.xres, :]
        self.bgimage = img

    # setBucket
    def setBucket(self, x, y, w, h, data):
        """Copy the image data of a bucket into the final image.
        
        x/y:  The pixel position of the bucket (upper left corner).
        w/h:  The size of the bucket.
        data: The binary data as it is passed from the renderer.
        """
        if w==0 or h==0:
            return

        # Convert the image data into a Numeric array...
        buck = array.array('f', data)
        buck = Numeric.array(buck, Numeric.Int16)
        buck.shape = (h,w,self.samples_per_element)

        if self.samples_per_element==4:
            abuck = buck[:,:,3:4]
            alphabuck = Numeric.zeros((h,w,3), Numeric.Int16)
            alphabuck[:,:,0:1] = abuck
            alphabuck[:,:,1:2] = abuck
            alphabuck[:,:,2:3] = abuck
            self.alpha[y:y+h:, x:x+w, 0:3] = alphabuck

        buck = buck[:,:,0:3]

        # Copy the bucket data into the final image
        self.image[y:y+h:, x:x+w, 0:3] = buck

    # getCombinedImage
    def getCombinedImage(self, x=0, y=0, w=-1, h=-1):

        if w==-1:
            w = self.xres
        if h==-1:
            h = self.yres
        
        img = self.image[y:y+h, x:x+w, :].astype(Numeric.Float)

        if self.has_alpha:
            bgimg = self.bgimage[y:y+h, x:x+w, :].astype(Numeric.Float)
            alpha = self.alpha[y:y+h, x:x+w, :].astype(Numeric.Float)

            alpha /= Numeric.array(255, Numeric.Float)

            maximg = Numeric.array(w*h*3*[255], Numeric.Float)
            maximg.shape = (h,w,3)
        
            img = (1.0-alpha)*bgimg + img
            img = Numeric.minimum(img,maximg)

        return img

    # getImage
    def getImage(self, comp=0):
        # Composite?
        if comp:
            return self.getCombinedImage()

        # No composition...
        
        if not self.has_alpha:
            return self.image

        res = Numeric.zeros((self.yres,self.xres,4),Numeric.Int8)
        res.savespace(1)

        img = self.image.astype(Numeric.Int8)
        alpha = self.alpha[:,:,0:1].astype(Numeric.Int8)

        res[:,:,0:3] = img
        res[:,:,3:4] = alpha
        return res

    # getAlpha
    def getAlpha(self, rgbmode=0):
        """Return the alpha channel.

        The returned image only has one channel unless rgbmode is 1.
        """
        if rgbmode:
            return self.alpha
        else:
            return self.alpha[:,:,0:1]

    # convertToPIL
    def convertToPIL(self, nimg):
        yres, xres, nsamples = nimg.shape
        nimg = nimg.astype(Numeric.Int8)
        rawdata = nimg.tostring()
        if nsamples==1:
            mode = "L"
        elif nsamples==3:
            mode = "RGB"
        else:
            mode = "RGBA"
        img = Image.fromstring(mode, (xres,yres), rawdata)
        return img
    

######################################################################
###################### Frambuffer ####################################
######################################################################

MENU_OPENBG = wxNewId()
MENU_SAVE   = wxNewId()
MENU_SAVEAS = wxNewId()
MENU_SAVECOMPOSITE   = wxNewId()
MENU_SAVECOMPOSITEAS = wxNewId()
MENU_SAVEALPHAAS = wxNewId()
MENU_EXIT = wxNewId()

MENU_TOCLIPBOARD = wxNewId()
MENU_COMPOSITETOCLIPBOARD = wxNewId()
MENU_ALPHATOCLIPBOARD = wxNewId()


EVT_DD_OPEN_ID      = wxNewId()
EVT_DD_CLOSE_ID     = wxNewId()
EVT_DD_FILENAME_ID  = wxNewId()
EVT_DD_DATA_ID      = wxNewId()
EVT_DD_USERPARAM_ID = wxNewId()
EVT_DD_ABANDON_ID   = wxNewId()

def EVT_DD_OPEN(win, func):
    win.Connect(-1, -1, EVT_DD_OPEN_ID, func)

def EVT_DD_CLOSE(win, func):
    win.Connect(-1, -1, EVT_DD_CLOSE_ID, func)

def EVT_DD_FILENAME(win, func):
    win.Connect(-1, -1, EVT_DD_FILENAME_ID, func)

def EVT_DD_DATA(win, func):
    win.Connect(-1, -1, EVT_DD_DATA_ID, func)

def EVT_DD_USERPARAM(win, func):
    win.Connect(-1, -1, EVT_DD_USERPARAM_ID, func)

def EVT_DD_ABANDON(win, func):
    win.Connect(-1, -1, EVT_DD_ABANDON_ID, func)

# Display device events
class DDOpenEvent(wxPyEvent):
    def __init__(self, XRes, YRes, Channels):
        wxPyEvent.__init__(self)
        self.SetEventType(EVT_DD_OPEN_ID)
        self.XRes = XRes
        self.YRes = YRes
        self.Channels = Channels

class DDCloseEvent(wxPyEvent):
    def __init__(self):
        wxPyEvent.__init__(self)
        self.SetEventType(EVT_DD_CLOSE_ID)

class DDFileNameEvent(wxPyEvent):
    def __init__(self, filename):
        wxPyEvent.__init__(self)
        self.SetEventType(EVT_DD_FILENAME_ID)
        self.filename = filename

class DDDataEvent(wxPyEvent):
    def __init__(self, XMin,XMaxPlus1,YMin,YMaxPlus1,Data):
        wxPyEvent.__init__(self)
        self.SetEventType(EVT_DD_DATA_ID)
        self.XMin = XMin
        self.YMin = YMin
        self.XMaxPlus1 = XMaxPlus1
        self.YMaxPlus1 = YMaxPlus1
        self.Data = Data

class DDUserParamEvent(wxPyEvent):
    def __init__(self, name, value):
        wxPyEvent.__init__(self)
        self.SetEventType(EVT_DD_USERPARAM_ID)
        self.name = name
        self.value = value

class DDAbandonEvent(wxPyEvent):
    def __init__(self):
        wxPyEvent.__init__(self)
        self.SetEventType(EVT_DD_ABANDON_ID)


# MainWin
class MainWin(wxFrame):

    def __init__(self, parent, id, title):
        """Constructor."""

        wxFrame.__init__(self, parent,id,title)

        self.CreateStatusBar(2)

        bmp = wxEmptyBitmap(320,240)
        self.bmplabel = wxStaticBitmap(self,-1,bmp)

        menubar = wxMenuBar()

        menu = wxMenu()
        menu.Append(MENU_OPENBG, "&Open background image...", "Open a background image.")
        menu.Append(MENU_SAVE, "&Save\tw", "Save the rendered image.")
        menu.Append(MENU_SAVEAS, "Save &as...", "Save the rendered image under a new name.")
        menu.Append(MENU_SAVECOMPOSITE, "Save composite", "Save the composite image.")
        menu.Append(MENU_SAVECOMPOSITEAS, "Save &composite as...", "Save the composite image under a new name.")
        menu.Append(MENU_SAVEALPHAAS, "Save alpha as...", "Save the alpha channel as a separate greyscale image.")
        menu.AppendSeparator()
        menu.Append(MENU_EXIT, "&Exit\tEsc", "Exit the program.")

        menubar.Append(menu, "&File")
        self.filemenu = menu

        menu = wxMenu()
        menu.Append(MENU_TOCLIPBOARD, "Copy rendered image to &clipboard\tc", "Copy the rendered image as RGB to the clipboard (no alpha).")
        menu.Append(MENU_COMPOSITETOCLIPBOARD, "Copy composite to clipboard", "Copy the composite image to the clipboard.")
        menu.Append(MENU_ALPHATOCLIPBOARD, "Copy alpha to clipboard", "Copy the alpha channel as RGB image to the clipboard.")
        
        menubar.Append(menu, "&Edit")
        self.editmenu = menu

        self.disableMenu()
        self.SetMenuBar(menubar)

        EVT_MENU(self, MENU_OPENBG, self.onMenuOpenBG)
        EVT_MENU(self, MENU_SAVE, self.onMenuSave)
        EVT_MENU(self, MENU_SAVEAS, self.onMenuSaveAs)
        EVT_MENU(self, MENU_SAVECOMPOSITE, self.onMenuSaveComposite)
        EVT_MENU(self, MENU_SAVECOMPOSITEAS, self.onMenuSaveCompositeAs)
        EVT_MENU(self, MENU_SAVEALPHAAS, self.onMenuSaveAlphaAs)
        EVT_MENU(self, MENU_SAVEALPHAAS, self.onMenuSaveAlphaAs)
        EVT_MENU(self, MENU_EXIT, self.onMenuExit)

        EVT_MENU(self, MENU_TOCLIPBOARD, self.onToClipboard)
        EVT_MENU(self, MENU_COMPOSITETOCLIPBOARD, self.onCompToClipboard)
        EVT_MENU(self, MENU_ALPHATOCLIPBOARD, self.onAlphaToClipboard)

        EVT_PAINT(self, self.onPaint)
        EVT_CHAR(self, self.onChar)

        EVT_DD_OPEN(self, self.onDDOpen)
        EVT_DD_CLOSE(self, self.onDDClose)
        EVT_DD_FILENAME(self, self.onDDFileName)
        EVT_DD_DATA(self, self.onDDData)
        EVT_DD_USERPARAM(self, self.onDDUserParam)
        EVT_DD_ABANDON(self, self.onDDAbandon)

        self.filename = "out.tif"
        self.savecomp = 0

        self.datacount = 0
        self.lastupdate = -1

        self.imagemanager = ImageManager()

    def onPaint(self, event):
        dc = wxPaintDC(self)

        if not self.imagemanager.isInitialised():
            return

        if self.lastupdate==self.datacount:
            return

        self.lastupdate = self.datacount

        xres = self.imagemanager.xres
        yres = self.imagemanager.yres

        img = self.imagemanager.getImage(comp=1)
        img = img[:,:,0:3].astype(Numeric.Int8)

        imgdata = img.tostring()
        img = wxEmptyImage(xres,yres)
        img.SetData(imgdata)
        bmp = wxBitmapFromImage(img)
        self.bmplabel.SetBitmap(bmp)


    def onChar(self, event):
        c = event.GetKeyCode()
        if c==27:
            self.onMenuExit(None)
        elif c==ord('w'):
            print 'Saving "%s"'%self.filename
            if self.savecomp:
                self.onMenuSaveComposite(None)
            else:
                self.onMenuSave(None)
        elif c==ord('c'):
            self.onToClipboard(None)

    def onDDOpen(self, event):

        xres = event.XRes
        yres = event.YRes

        self.SetTitle(self.filename)
        if event.Channels==3:
            mode = "RGB"
        elif event.Channels==4:
            mode = "RGBA"    
        self.SetStatusText('"%s", %dx%d, %s'%(self.filename,xres,yres,mode),1)

        # Resize window
        self.SetSize((xres,yres))
        w,h = self.bmplabel.GetSize()
        dw = max(0,xres-w)
        dh = max(0,yres-h)
        self.SetSize((xres+dw,yres+dh))

        self.imagemanager.initialise(event.XRes, event.YRes,
                                     event.Channels)

        # Test
        img = self.imagemanager.getImage(comp=0)
        img = img[:,:,0:3].astype(Numeric.Int8)

        imgdata = img.tostring()
        img = wxEmptyImage(xres,yres)
        img.SetData(imgdata)
        bmp = wxBitmapFromImage(img)
        self.bmplabel.SetBitmap(bmp)

    def onDDUserParam(self, event):
        name = event.name
        value = event.value
        
        if name=="bgimage":
            self.imagemanager.setBGImageName(value)
        elif name=="savecomp":
            if isinstance(value, int):
                self.savecomp = value
                if self.savecomp:
                    s = str(self.filemenu.GetLabel(MENU_SAVE))
                    self.filemenu.SetLabel(MENU_SAVE, s.replace("\tw",""))
                    s = str(self.filemenu.GetLabel(MENU_SAVECOMPOSITE))
                    self.filemenu.SetLabel(MENU_SAVECOMPOSITE, s+"\tw")

    def onDDClose(self, event):

        xres = self.imagemanager.xres
        yres = self.imagemanager.yres
        
        img = self.imagemanager.getCombinedImage()
        img = img.astype(Numeric.Int8)

        imgdata = img.tostring()
        img = wxEmptyImage(xres,yres)
        img.SetData(imgdata)
        bmp = wxBitmapFromImage(img)
        self.bmplabel.SetBitmap(bmp)

        self.enableMenu()

    def onDDFileName(self, event):
        self.filename = event.filename

    def onDDData(self, event):
        dc = wxClientDC(self.bmplabel)
        x = event.XMin
        y = event.YMin
        w = event.XMaxPlus1-x
        h = event.YMaxPlus1-y

        if w==0 or h==0:
            return 

        self.imagemanager.setBucket(x,y,w,h,event.Data)

        img = self.imagemanager.getCombinedImage(x,y,w,h)
        self.drawImg(dc,x,y,img)

        self.datacount+=1

    def onDDAbandon(self, event):
        # Close window if the display wasn't opened
        if not self.imagemanager.isInitialised():
            self.onMenuExit(None)

    def onMenuOpenBG(self, event):
        dlg = wxFileDialog(self, message="Open background image")
        if dlg.ShowModal()==wxID_OK:
            filename = str(dlg.GetPath())
            self.imagemanager.loadBGImage(filename)
            self.onDDClose(None)

    def onMenuSave(self, event):
        nimg = self.imagemanager.getImage(comp=0)
        self.saveImage(nimg, self.filename)

    def onMenuSaveAs(self, event):
        dlg = wxFileDialog(self, message="Save rendered image",
                           defaultFile=self.filename,
                           style=wxSAVE|wxOVERWRITE_PROMPT)
        if dlg.ShowModal()==wxID_OK:
            nimg = self.imagemanager.getImage(comp=0)
            self.saveImage(nimg, str(dlg.GetPath()))

    def onMenuSaveComposite(self, event):
        nimg = self.imagemanager.getImage(comp=1)
        self.saveImage(nimg, self.filename)

    def onMenuSaveCompositeAs(self, event):
        dlg = wxFileDialog(self, message="Save composite",
                           defaultFile=self.filename,
                           style=wxSAVE|wxOVERWRITE_PROMPT)
        if dlg.ShowModal()==wxID_OK:
            nimg = self.imagemanager.getImage(comp=1)
            self.saveImage(nimg, str(dlg.GetPath()))

    def onMenuSaveAlphaAs(self, event):
        dlg = wxFileDialog(self, message="Save alpha channel",
                           defaultFile=self.filename,
                           style=wxSAVE|wxOVERWRITE_PROMPT)
        if dlg.ShowModal()==wxID_OK:
            nimg = self.imagemanager.getAlpha()
            self.saveImage(nimg, str(dlg.GetPath()))

    def onMenuExit(self, event):
        self.Close()

    def onToClipboard(self, event):
        nimg = self.imagemanager.getImage(comp=0)
        self.copyToClipboard(nimg)

    def onCompToClipboard(self, event):
        nimg = self.imagemanager.getImage(comp=1)
        self.copyToClipboard(nimg)

    def onAlphaToClipboard(self, event):
        nimg = self.imagemanager.getAlpha(rgbmode=1)
        self.copyToClipboard(nimg)

    def copyToClipboard(self, nimg):
        """Copy a numeric image to the clipboard.

        nimg must contain at least 3 channels (only 3 are used).
        """
        cb = wxClipboard()
        if not cb.Open():
            sys.stderr.write("Can't open clipboard.\n")
            return

        nimg = nimg[:,:,0:3].astype(Numeric.Int8)
        imgdata = nimg.tostring()

        yres,xres,nsamps = nimg.shape
        img = wxEmptyImage(xres,yres)
        img.SetData(imgdata)
        bmp = wxBitmapFromImage(img)
        bmpdata = wxBitmapDataObject(bmp)
        cb.SetData(bmpdata)

        cb.Close()

    def saveImage(self, nimg, filename):
        """Save an image to a file.

        nimg: Image given as Numeric array.
        filename: Name of the output file.
        """
        img = self.imagemanager.convertToPIL(nimg)
        try:
            img.save(filename)
        except:
            sys.stderr.write("Couldn't save image file \"%s\"."%filename)

    def drawImg(self, dc, x, y, img):
        """Draw an image.

        dc:  Window DC
        x,y: Position where to draw the image
        img: Image as Numeric array
        """

        xres,yres,depth = img.shape

        if img.typecode()!=Numeric.Int8:
            img = img.astype(Numeric.Int8)

        imgdata = img.tostring()
        wximg = wxEmptyImage(xres,yres)
        wximg.SetData(imgdata)
        bmp = wxBitmapFromImage(wximg)
        dc.DrawBitmap(bmp,x,y)

    def disableMenu(self):
        self.filemenu.Enable(MENU_OPENBG, 0)
        self.filemenu.Enable(MENU_SAVE, 0)
        self.filemenu.Enable(MENU_SAVEAS, 0)
        self.filemenu.Enable(MENU_SAVECOMPOSITE, 0)
        self.filemenu.Enable(MENU_SAVECOMPOSITEAS, 0)
        self.filemenu.Enable(MENU_SAVEALPHAAS, 0)

        self.editmenu.Enable(MENU_TOCLIPBOARD, 0)
        self.editmenu.Enable(MENU_COMPOSITETOCLIPBOARD, 0)
        self.editmenu.Enable(MENU_ALPHATOCLIPBOARD, 0)

    def enableMenu(self):
        self.filemenu.Enable(MENU_SAVE, 1)
        self.filemenu.Enable(MENU_SAVEAS, 1)
        self.editmenu.Enable(MENU_TOCLIPBOARD, 1)
        
        if self.imagemanager.has_alpha:
            self.filemenu.Enable(MENU_OPENBG, 1)
            self.filemenu.Enable(MENU_SAVECOMPOSITE, 1)
            self.filemenu.Enable(MENU_SAVECOMPOSITEAS, 1)
            self.filemenu.Enable(MENU_SAVEALPHAAS, 1)

            self.editmenu.Enable(MENU_COMPOSITETOCLIPBOARD, 1)
            self.editmenu.Enable(MENU_ALPHATOCLIPBOARD, 1)

# Application
class Application(wxApp):
    """Represents the entire application.

    It just activates the main window. All functionality is
    found there.
    """

    def __init__(self):
        "Constructor."
        wxApp.__init__(self)

    # OnInit
    def OnInit(self):
        self.mainwin = MainWin(NULL, -1, "Framebuffer")
        self.mainwin.SetSize((30,30))
        self.mainwin.Show(true)
        self.SetTopWindow(self.mainwin)
        return true


# FrameBufferDisplayDriver
class FrameBufferDisplayDriver(DisplayDriver):
    """This display driver class forwards all messages to the GUI."""

    def __init__(self, win):
        """Constructor.

        win: This window will receive the message notifications.
        """
        DisplayDriver.__init__(self)
        self.win = win

    def onFileName(self, filename):
        wxPostEvent(self.win, DDFileNameEvent(filename))

    def onOpen(self, XRes, YRes, Channels, 
               CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax):
        wxPostEvent(self.win, DDOpenEvent(XRes, YRes, Channels))

    def onClose(self):
        wxPostEvent(self.win, DDCloseEvent())

    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data):
        wxPostEvent(self.win, DDDataEvent(XMin,XMaxPlus1,YMin,YMaxPlus1,Data))

    def onUserParam(self, name, value):
        wxPostEvent(self.win, DDUserParamEvent(name,value))

    def onAbandon(self):
        wxPostEvent(self.win, DDAbandonEvent())

# CommunicationThread
class CommunicationThread(Thread):
    """This thread handles the socket communication with the renderer.
    """
    
    def __init__(self, win):
        """Constructor.

        win: This window will receive the message notifications.
        """
        Thread.__init__(self)
        self.win = win

    def run(self):
        dd = FrameBufferDisplayDriver(self.win)
        dd.MessageLoop()
        

######################################################################

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "fi", ["framebuffer", "inspect"])
    except getopt.GetoptError, e:
        sys.stderr.write("Display device: %s\n"%e)
        sys.exit()

    output = None
    for o, a in opts:
        # Inspect display device
        if o in ("-i", "--inspect"):
            dd = InspectDisplayDriver()
            dd.MessageLoop()
            sys.exit()
        # Framebuffer display device
        if o in ("-f", "--framebuffer"):
            if not (has_PIL and has_numeric):
                print "You must install PIL and Numeric to use this display driver"
                sys.exit()
            if not has_threading:
                print "Python must be compiled with support for threads"
                sys.exit()
            if not has_wxpython:
                print "You must install wxPython to be able to run the framebuffer driver"
                sys.exit()
            if not has_win32 and sys.platform=="win32":
                print "Warning: Can't increase display device priority (win32all not available)"

            app = Application()
            sys.stdout = origout
            sys.stderr = origerr

            ct = CommunicationThread(app.GetTopWindow())
            ct.setDaemon(1)
            ct.start()

            if has_win32:
                pid = win32api.GetCurrentProcess()
                win32process.SetPriorityClass(pid, win32process.HIGH_PRIORITY_CLASS)

            app.MainLoop()
            sys.exit()


    # File display device
    if not (has_PIL and has_numeric):
        print "You must install PIL and Numeric to use this display driver"
        sys.exit()
    
    dd = FileDisplayDriver()
    dd.MessageLoop()

######################################################################

try:
    main()
except KeyboardInterrupt:
    print "User abort"
