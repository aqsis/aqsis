#!/usr/bin/env python
#
# An example for an Aqsis display driver written in Python.
# Written by Matthias Baas (baas@ira.uka.de)

# Below are 2 example display drivers which are derived from the
# DisplayDriver base class. In the derived classes only the
# onXxx() handler methods have to be implemented. Each method
# is called whenever the corresponding message was sent.
#
# The class InspectDisplayDriver just prints all the information it
# gets. Here you can see all the values that Aqsis sends.
#
# The class FileDisplayDriver saves the image to disk using the
# Python Imaging Library (PIL). You can save any formats that PIL
# supports (the format is determined by the file extension).

import sys, os, socket, struct, Image

MSGID_STRING           = 0
MSGID_FORMATQUERY      = 1
MSGID_DATA             = 2
MSGID_OPEN             = 3
MSGID_CLOSE            = 4
MSGID_FILENAME         = 5
MSGID_NL               = 6
MSGID_NP               = 7
MSGID_DISPLAYTYPE      = 8
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
            msgid,msglen,msgbody = self.ReceiveRawMessage()
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
                XRes,YRes,SamplesPerElement,BitsPerSample,CropWindowXMin,CropWindowXMax,CropWindowYMin,CropWindowYMax = struct.unpack("iiiiiiii",msgbody)
                self.onOpen(XRes, YRes, SamplesPerElement, BitsPerSample, CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax)
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
            # Unknown message
            else:
                print "UNKNOWN MESSAGE"

        self.sock.close()

    # ExitLoop
    def ExitLoop(self):
        self.running_flag = 0

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
    def onOpen(self, XRes, YRes, SamplesPerElement, BitsPerSample, CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax): pass
    def onClose(self): pass
    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data): pass

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
        print "------------------------------------"
       
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

    def onOpen(self, XRes, YRes, SamplesPerElement, BitsPerSample,
               CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax):
        print "Open - Res:%dx%d  SamplesPerElement:%d  BitsPerSample:%d  Crop:(%d,%d)-(%d,%d)"%(XRes,YRes,SamplesPerElement,BitsPerSample,CropWindowXMin,CropWindowYMin,CropWindowXMax,CropWindowYMax)

    def onClose(self):
        print "Close"

    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data):
        print "Data - Rect:(%d,%d)-(%d,%d)  ElementSize:%d  DataLen:%d"%(XMin,YMin,XMaxPlus1,YMaxPlus1,ElementSize,len(Data))


# FileDisplayDriver
class FileDisplayDriver(DisplayDriver):

    def __init__(self):
        DisplayDriver.__init__(self)

        self.filename = "out.tif"
        self.img = None

    def onFileName(self, filename):
        self.filename = filename

    def onOpen(self, XRes, YRes, SamplesPerElement, BitsPerSample,
               CropWindowXMin, CropWindowXMax, CropWindowYMin, CropWindowYMax):
        if SamplesPerElement==4:
            mode = "RGBA"
        else:
            mode = "RGB"
        self.img = Image.new(mode, (XRes, YRes))

    def onClose(self):
        self.img.save(self.filename)

    def onData(self, XMin,XMaxPlus1,YMin,YMaxPlus1,ElementSize,Data):
        if self.img.mode=="RGBA":
            i=0
            for y in range(YMin, YMaxPlus1):
                for x in range(XMin, XMaxPlus1):
                    r,g,b,a = struct.unpack("ffff", Data[i:i+16])
                    self.img.putpixel((x,y),(int(r),int(g),int(b),int(a)))
                    i+=16
        else:
            i=0
            for y in range(YMin, YMaxPlus1):
                for x in range(XMin, XMaxPlus1):
                    r,g,b = struct.unpack("fff", Data[i:i+12])
                    self.img.putpixel((x,y),(int(r),int(g),int(b)))
                    i+=12


######################################################################

dd = FileDisplayDriver()
#dd = InspectDisplayDriver()
dd.MessageLoop()

