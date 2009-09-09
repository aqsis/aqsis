#######################################################################
# BtoRGUIClasses.py - GUI Class library for the BtoR GUI system.
# Version 0.1a
# Author: Bobby Parker                                                                         
# 
# This is the GUI library in its entirety. 
# Todos: 
# 	Documentation. Documentation. Documentation.
# 	Fix bugs
# 	Come up with a more formal todo-list
#######################################################################
import Blender
import sys
import string
import math
draw = Blender.Draw
gl = Blender.BGL
 
 
class UI:
	def __init__(self):	
		self.theme = Blender.Window.Theme.Get()[0] # the current theme
		
	def get_string_width(self, text, fontsize):
		# set the raster position somewhere way offscreen
		#Blender.BGL.glRasterPos2i(-100, -100)
		width = Blender.Draw.GetStringWidth(text, fontsize)    
		return width
	def buildFontTable(self, fontsize):
		font_size_table = []
		for char in range(16, 127):
			c = chr(char)
			print c
			font_size_table.append(Blender.Draw.GetStringWidth(c, fontsize))
		return font_size_table
		
	def uidrawbox(self, mode, minx, miny, maxx, maxy, rad, cornermask):
		vec = Blender.BGL.Buffer(Blender.BGL.GL_FLOAT, [7,2], [[0.195, 0.02], [0.383, 0.067], [0.55, 0.169], [0.707, 0.293],[0.831, 0.45], [0.924, 0.617], [0.98, 0.805]])
		for a in range(7):
			vec[a][0] = vec[a][0] * rad
			vec[a][1] = vec[a][1] * rad
			
		Blender.BGL.glBegin(mode)    
		
		# start with corner right-bottom 
		if cornermask & 4:
			Blender.BGL.glVertex2f( maxx-rad, miny);
			for a in range(7):
				Blender.BGL.glVertex2f( maxx-rad+vec[a][0], miny+vec[a][1])
			
			Blender.BGL.glVertex2f( maxx, miny+rad);
		
		else:
			Blender.BGL.glVertex2f( maxx, miny)
		
		# corner right-top 
		if cornermask & 2:
			Blender.BGL.glVertex2f( maxx, maxy-rad)
			for a in range(7):
				Blender.BGL.glVertex2f( maxx-vec[a][1], maxy-rad+vec[a][0])
			
			Blender.BGL.glVertex2f( maxx-rad, maxy)	
		else:
			Blender.BGL.glVertex2f( maxx, maxy)
		
		# corner left-top 
		if cornermask & 1:
			Blender.BGL.glVertex2f( minx+rad, maxy)
			for a in range(7):
				Blender.BGL.glVertex2f( minx+rad-vec[a][0], maxy-vec[a][1])
			
			Blender.BGL.glVertex2f( minx, maxy-rad)
		else:
			Blender.BGL.glVertex2f( minx, maxy)
		
		# corner left-bottom 
		if cornermask & 8:
			Blender.BGL.glVertex2f( minx, miny+rad)
			for a in range(7):
				Blender.BGL.glVertex2f( minx+vec[a][1], miny+rad-vec[a][0])
			
			Blender.BGL.glVertex2f( minx+rad, miny)
		else:
			Blender.BGL.glVertex2f( minx, miny)
		Blender.BGL.glEnd()
		
	def uiRoundBox(self, minx, miny, maxx, maxy, rad, cornermask):
		
		color = Blender.BGL.Buffer(gl.GL_FLOAT, [4])
		if cornermask & 16:            
			Blender.BGL.glGetFloatv(gl.GL_CURRENT_COLOR, color)
			color[3] = 0.5
			Blender.BGL.glColor4fv(color)
			Blender.BGL.glEnable( gl.GL_BLEND )
			Blender.BGL.glBlendFunc( gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA )
		
		#solid part 
		self.uidrawbox(gl.GL_POLYGON, minx, miny, maxx, maxy, rad, cornermask)
		
		Blender.BGL.glEnable( gl.GL_LINE_SMOOTH )
		Blender.BGL.glEnable( gl.GL_BLEND )
		Blender.BGL.glBlendFunc( gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA )
	
		self.uidrawbox(gl.GL_LINE_LOOP, minx, miny, maxx, maxy, rad, cornermask)
		
		Blender.BGL.glDisable( gl.GL_BLEND )
		Blender.BGL.glDisable( gl.GL_LINE_SMOOTH )
		# Blender.BGL.glColor4fv(oldColor)
	
	def uiOutline(self, minx, miny, maxx, maxy, rad, cornermask):
		Blender.BGL.glEnable( gl.GL_LINE_SMOOTH )
		Blender.BGL.glEnable( gl.GL_BLEND )
		Blender.BGL.glBlendFunc( gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA )
		self.uidrawbox(gl.GL_LINE_LOOP, minx, miny, maxx, maxy, rad, cornermask)
		Blender.BGL.glDisable( gl.GL_BLEND )
		Blender.BGL.glDisable( gl.GL_LINE_SMOOTH )
	
	def area_mouse_coords(self):
		a = self.get_space_size()
		b = Blender.Window.GetMouseCoords()
		return b[0]-a[0], b[1]-a[1]
	
	def get_correct_space_size(self):
		# don't know about this
		size = Blender.Window.GetAreaSize()
		return size
	
	def get_space_size(self):
		#size = Blender.Window.GetAreaSize()
		size=Blender.BGL.Buffer(Blender.BGL.GL_FLOAT, 4)
		Blender.BGL.glGetFloatv(Blender.BGL.GL_SCISSOR_BOX, size)
		size= size.list 
		return int(size[0]), int(size[1])
		
	def drawline(self, x, y, xx, yy):
		Blender.BGL.glBegin(Blender.BGL.GL_LINE_STRIP)
		Blender.BGL.glVertex2f(x, y)
		Blender.BGL.glVertex2f(xx, yy)
		Blender.BGL.glEnd()	
		
	def drawBox(self, x, y, xx, yy):
		Blender.BGL.glRecti(x, y, xx, yy)
		
	
	def ui_shadowbox(self, minx, miny, maxx, maxy, shadsize, alpha):
		Blender.BGL.glEnable(gl.GL_BLEND)
		Blender.BGL.glShadeModel(gl.GL_SMOOTH)
		
		# right quad 
		Blender.BGL.glBegin(gl.GL_POLYGON)
		Blender.BGL.glColor4ub(0, 0, 0, alpha)
		Blender.BGL.glVertex2f(maxx, miny)
		Blender.BGL.glVertex2f(maxx, maxy-shadsize)
		Blender.BGL.glColor4ub(0, 0, 0, 0)
		Blender.BGL.glVertex2f(maxx+shadsize, maxy-shadsize-shadsize)
		Blender.BGL.glVertex2f(maxx+shadsize, miny)
		Blender.BGL.glEnd()
		
		# corner shape 
		Blender.BGL.glBegin(GL_POLYGON)
		Blender.BGL.glColor4ub(0, 0, 0, alpha)
		Blender.BGL.glVertex2f(maxx, miny)
		Blender.BGL.glColor4ub(0, 0, 0, 0)
		Blender.BGL.glVertex2f(maxx+shadsize, miny)
		Blender.BGL.glVertex2f(maxx+0.7 * shadsize, miny-0.7 * shadsize)
		Blender.BGL.glVertex2f(maxx, miny-shadsize)
		Blender.BGL.glEnd()
		
		# bottom quad 
		Blender.BGL.glBegin(GL_POLYGON)
		Blender.BGL.glColor4ub(0, 0, 0, alpha)
		Blender.BGL.glVertex2f(minx+shadsize, miny)
		Blender.BGL.glVertex2f(maxx, miny)
		Blender.BGL.glColor4ub(0, 0, 0, 0)
		Blender.BGL.glVertex2f(maxx, miny-shadsize)
		Blender.BGL.glVertex2f(minx+shadsize+shadsize, miny-shadsize)
		Blender.BGL.glEnd()
		
		Blender.BGL.glDisable(gl.GL_BLEND)
		Blender.BGL.glShadeModel(gl.GL_FLAT)
	
	def uiDrawBoxShadow(self, alpha, minx, miny, maxx, maxy):
		# accumulated outline boxes to make shade not linear, is more pleasant 
		self_shadowbox(minx, miny, maxx, maxy, 6.0, (30*alpha)>>8)
		self_shadowbox(minx, miny, maxx, maxy, 4.0, (70*alpha)>>8)
		self_shadowbox(minx, miny, maxx, maxy, 2.0, (100*alpha)>>8)
		
	
	def uiDrawMenuBox(self, minx, miny, maxx, maxy, flag):
		
		BIF_GetThemeColor4ubv(TH_MENU_BACK, col)		
		if noshadow:
			# accumulated outline boxes to make shade not linear, is more pleasant 
			self_shadowbox(minx, miny, maxx, maxy, 6.0, (30*col[3])>>8)
			self_shadowbox(minx, miny, maxx, maxy, 4.0, (70*col[3])>>8)
			self_shadowbox(minx, miny, maxx, maxy, 2.0, (100*col[3])>>8)
			
			Blender.BGL.glEnable(gl.GL_BLEND)
			Blender.BGL.glColor4ubv(col)
			Blender.BGL.glRectf(minx-1, miny, minx, maxy)	# 1 pixel on left, to distinguish sublevel menus
		
		Blender.BGL.glEnable(gl.GL_BLEND)
		Blender.BGL.glColor4ubv(col)
		Blender.BGL.glRectf(minx, miny, maxx, maxy)
		Blender.BGL.glDisable(gl.GL_BLEND)

	def ui_draw_pulldown_item(self, type, colorid, asp, x1, y1, x2, y2, flag):
		col = Blender.BGL.Buffer(gl.GL_FLOAT, [3])
		
		BIF_GetThemeColor4ubv(TH_MENU_BACK, col)
		if(col[3]!=255):
			Blender.BGL.glEnable(gl.GL_BLEND)
		if(flag & UI_ACTIVE) and type!=LABEL:
			BIF_ThemeColor4(TH_MENU_HILITE)
			Blender.BGL.glRectf(x1, y1, x2, y2)  
		else:
			BIF_ThemeColor4(colorid)	# is set at TH_MENU_ITEM when pulldown opened.
			Blender.BGL.glRectf(x1, y1, x2, y2)  
			
		Blender.BGL.glDisable(Blender.BGL.GL_BLEND)

	def ui_draw_pulldown_round(self, type, colorid, asp, x1, y1, x2,  y2, flag):
		if(flag & UI_ACTIVE):
			BIF_ThemeColor(TH_MENU_HILITE);
	
			
			self.drawbox(gl.GL_POLYGON, x1, y1+3, x2, y2-3, 7.0, 15)
	
			Blender.BGL.glEnable( Blender.BGL.GL_LINE_SMOOTH )
			Blender.BGL.glEnable( Blender.BGL.GL_BLEND );
			Blender.BGL.drawbox(Blender.BGL.GL_LINE_LOOP, x1, y1+3, x2, y2-3, 7.0);
			Blender.BGL.glDisable( Blender.BGL.GL_LINE_SMOOTH );
			Blender.BGL.glDisable( Blender.BGL.GL_BLEND );            
		else:
			BIF_ThemeColor(colorid);	# is set at TH_MENU_ITEM when pulldown opened.
			Blender.BGL.glRectf(x1-1, y1+2, x2+1, y2-2);
		
		
	def setColor(self, color):
		# set the current working OpenGL color
		if len(color) < 4:
			color.append(255)
			
		conv = 0.00392156
		xcolor = [color[0] * conv, color[1] * conv, color[2] * conv, color[3] * conv]
		# xcolor = color
		Blender.BGL.glColor4f(xcolor[0], xcolor[1], xcolor[2], xcolor[3])
	
	def setColorFloat(self, color):
		if len(color) < 4:
			color.append(1)
		Blender.BGL.glColor4f(color[0], color[1], color[2], color[3]) 

	def setRasterPos(self, x, y):
		Blender.BGL.glRasterPos2i(x, y)
		
	def rgbToHsv(self, value):
		#print "Incoming value: ", value
		color  =  [float(value[0]) / float(255), float(value[1]) / float(255), float(value[2])  / float(255)]
		print "Color = ", color
		minVal = min(color)  	
		maxVal = max(color)  
		deltaMax = maxVal - minVal	
		
		V = maxVal
		if deltaMax == 0:                     #grayscale value
			H = 0                                
			S = 0
			
		else:                                    # pretty colors!
			S = float(deltaMax) / float(maxVal)
			deltaRed = ((float((maxVal - color[0])) / float(6)) + float(float((deltaMax / float(2)))) / deltaMax)
			deltaGreen = ((float((maxVal - color[1])) / float(6)) + float(float((deltaMax / float(2)))) / deltaMax)
			deltaBlue = ((float((maxVal - color[2])) / float(6)) + float(float((deltaMax / float(2)))) / deltaMax)
			#print "DeltaRed: ", deltaRed
			#print "DeltaGreen: ", deltaGreen
			#print "DeltaBlue:", deltaBlue
			#print "MaxVal = ", maxVal
			if color[0] == maxVal:
				H = deltaBlue - deltaGreen
			elif color[1] == maxVal: 
				H = ( float(1) / float(3))  + deltaRed - deltaBlue
			elif color[2] == maxVal: 
				H = ( float(2) / float(3))  + deltaGreen - deltaRed
			if H < 0:
				H += 1
			if H > 1:
				H -= 1
		hsv = [H, S, V]
		#print "Outgoing HSV: ", hsv
		return hsv
		
	def hsvToRgb(self, hsv):
		if hsv[1] == 0: 
		   rgb = [hsv[2] * 255, hsv[2] * 255, hsv[2] * 255]		
		else:
			h = float(hsv[0] * 6) # determine the hue
			
			if h == 6 :
				h = 0      # H must be < 1
			var_i = int(h)             # Or ... var_i = floor( var_h )
			var_1 = float(hsv[2] * float( 1 - hsv[1] ))
			var_2 = float(hsv[2] * float(( 1 - hsv[1] * (h - var_i))))
			var_3 = float(hsv[2] * float(( 1 - hsv[1] * (1 - (h - var_i)))))
		
			if var_i == 0:
				rgb_t = [hsv[2], var_3, var_1]
			elif var_i == 1: 
				rgb_t = [var_2, hsv[2], var_1]				
			elif var_i == 2: 
				rgb_t = [var_1, hsv[2], var_3]
			elif var_i == 3:  
				rgb_t = [var_1, var_2, hsv[2]]
			elif var_i == 4: 
				rgb_t = [var_3, var_1, hsv[2]]				
			else:
				rgb_t = [hsv[2], var_1, var_2]
				
			rgb = [rgb_t[0], rgb_t[1], rgb_t[2]]
			# print "HSV was ", hsv, " RGB is ", rgb
		return rgb

	
	def hslToRgb(self, hsl):
		if hsl[1] == 0:                  
			rgb = [hsl[2] * 255, hsl[2] * 255, hsl[2] * 255]
		else:
			if hsl[2] < 0.5:
				var_2 = hsl[2] * ( 1 + hsl[1] )	
			else:
				var_2 = ( hsl[2] + hsl[1] ) - ( hsl[1] * hsl[2] )
				
			var_1 = 2 * hsl[2] - var_2
			rgb = [255 * Hue_2_RGB( var_1, var_2, hsl[0] + ( 1 / 3 )), 255 * Hue_2_RGB( var_1, var_2, hsl[0] ), 255 * Hue_2_RGB( var_1, var_2, hsl[0] - ( 1 / 3 ) )]
		return rgb

	def Hue_2_RGB(self, v1, v2, vH ):            # Function Hue_2_RGB
		if vH < 0:
			vH += 1
		if vH > 1:
			vH -= 1
		if ( 6 * vH ) < 1: 
			return ( v1 + ( v2 - v1 ) * 6 * vH )
		if ( 2 * vH ) < 1:
			return ( v2 )
		if ( 3 * vH ) < 2:
			return ( v1 + ( v2 - v1 ) * ( ( 2 / 3 ) - vH ) * 6 )
		return v1 		

	def rgbToHsl(self, value):
		var_R = ( value[0] / 255 )                     
		var_G = ( value[1] / 255 )
		var_B = ( value[2] / 255 )
		
		var_Min = min( var_R, var_G, var_B )    # Min. value of RGB
		var_Max = max( var_R, var_G, var_B )    # Max. value of RGB
		del_Max = var_Max - var_Min             # Delta RGB value
		
		L = (var_Max + var_Min ) / 2
		
		if del_Max == 0:                    # This is a gray, no chroma...		
		   H = 0                            
		   S = 0
		else:                               #Chromatic data...
			if ( L < 0.5 ):
				S = del_Max / ( var_Max + var_Min )
			else:
				S = del_Max / ( 2 - var_Max - var_Min )
			
			del_R = ( ( ( var_Max - var_R ) / 6 ) + ( del_Max / 2 ) ) / del_Max
			del_G = ( ( ( var_Max - var_G ) / 6 ) + ( del_Max / 2 ) ) / del_Max
			del_B = ( ( ( var_Max - var_B ) / 6 ) + ( del_Max / 2 ) ) / del_Max
			
			if var_R == var_Max:
				H = del_B - del_G
			elif var_G == var_Max: 
				H = ( 1 / 3 ) + del_R - del_B
			elif var_B == var_Max:
				H = ( 2 / 3 ) + del_G - del_R
		
			if H < 0:
				H += 1
			if H > 1:
				H -= 1
		hsl = [H, S, L]
		return hsl
		
	def xyzToRgb(self, value):
			
		# Observer = 2°, Illuminant = D65
		var_X = value[0] / 100        # Where X = 0 /  95.047
		var_Y = value[1] / 100        # Where Y = 0 / 100.000
		var_Z = value[2] / 100        # Where Z = 0 / 108.883
		
		var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986
		var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415
		var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570
		
		if ( var_R > 0.0031308 ):
			var_R = 1.055 * ( var_R ^ ( 1 / 2.4 ) ) - 0.055
		else:
			var_R = 12.92 * var_R
		if var_G > 0.0031308: 
			var_G = 1.055 * ( var_G ^ ( 1 / 2.4 ) ) - 0.055
		else:
			var_G = 12.92 * var_G
		if var_B > 0.0031308:
			var_B = 1.055 * ( var_B ^ ( 1 / 2.4 ) ) - 0.055
		else:
			var_B = 12.92 * var_B
		
		R = var_R * 255
		G = var_G * 255
		B = var_B * 255   
		rgb = [R, G, B]
		return rgb
	
	def rgb2Float(self, color):		
		r_s = color[0]
		g_s = color[1]
		b_s = color[2]
		if float(r_s) > 1:			
			r = int(r_s) 
		else:
			r = float(r_s) * 255			
			
		if float(g_s) > 1:
			g = float(g_s) 
		else:
			g = float(g_s) * 255
						
		if float(b_s) > 1:
			b = float(b_s) 
		else:
			b = float(b_s) * 255			
			
		rgb = [r, g, b, 255]
		return rgb
	
	def rgbToXyz(self, value):
		var_R = ( value[0] / 255 )        # Where R = 0 / 255
		var_G = ( value[1] / 255 )        # Where G = 0 / 255
		var_B = ( value[2] / 255 )        # Where B = 0 / 255
		
		if var_R > 0.04045:
			var_R = ( ( var_R + 0.055 ) / 1.055 ) ^ 2.4
		else:
			var_R = var_R / 12.92
		if var_G > 0.04045: 
			var_G = ( ( var_G + 0.055 ) / 1.055 ) ^ 2.4
		else:
			var_G = var_G / 12.92
		if var_B > 0.04045:
			var_B = ( ( var_B + 0.055 ) / 1.055 ) ^ 2.4
		else:
			var_B = var_B / 12.92
		
		var_R = var_R * 100
		var_G = var_G * 100
		var_B = var_B * 100
		
		# Observer. = 2°, Illuminant = D65
		X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805
		Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722
		Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505
		xyz = [X, Y, Z]
		return xyz

	def drawScreenGrid(self, gsize_x, gsize_y, color):
		y = 0
		x = 0
		size = Blender.Window.GetAreaSize()
		Blender.BGL.glColor3f(color[0], color[1], color[2])
		while y < size[1]:
			Blender.BGL.glBegin(Blender.BGL.GL_LINES)
			Blender.BGL.glVertex2f(0, y)
			Blender.BGL.glVertex2f(size[0], y)
			Blender.BGL.glEnd()
			y = y + gsize_y
		
		
		while x < size[0]:
			Blender.BGL.glBegin(Blender.BGL.GL_LINES)
			Blender.BGL.glVertex2f(x, 0)
			Blender.BGL.glVertex2f(x, size[1])
			Blender.BGL.glEnd()
			x = x + gsize_x
			
	def callFactory(self, func, *args, **kws):
		def curried(*moreargs, **morekws):
			kws.update(morekws) # the new kws override any keywords in the original
			print kws, " ", args
			return func(*(args + moreargs), **kws)
		return curried


class EventManager(UI):
	""" GUI Event Manager
	
	The EventManager object intercepts all events from Blender's normal events and routes them according to type. 
	The EventManager is also capable of displaying confirmation and input dialogs.
	
	evt_manager = BtoRGUIClasses.EventManager()
	evt_manager.register()
	"""
	def __init__(self, auto_register = False ):		
		""" __init__(boolean auto_register) - EventManager constructor """
		UI.__init__(self)
		self.elements = []
		self.z_stack = []
		self.draw_stack = []
		self.lastX = 0
		self.lastY = 0
		self.noComp = True
		self.backGround = [0.4, 0.4, 0.4]		
		# register myself as the point of control
		if auto_register:
			self.register()
			
		self.draw_functions = []
		self.redraw_functions = []
		self.screen = Blender.Window.GetAreaSize()
		
	def register(self):
		""" register() - Register this object as the listener for Blender's events. """
		Blender.Draw.Register(self.draw, self.event, self.bevent)
		
	def addElement(self, element):
		""" addElement(UIElement element) - Add the supplied UIElement to the element list."""
		if element not in self.elements:
			self.elements.append(element)
			self.z_stack.append(element)
			self.draw_stack.append(element)
		self.raiseElement(element) # of course this does some redundant crap, but ok
			
	def removeElement(self, element):		
		""" removeElement(UIElement element) - Removes the supplied UIElement from the element list."""
		# a third test for posterity
		if element in self.elements:
			self.elements.remove(element)
		if element in self.z_stack:			
			self.z_stack.remove(element)

		if element in self.draw_stack:			
			self.draw_stack.remove(element)

		del element
		self.invalid = True
		Blender.Draw.Redraw()
	
	def raiseElement(self, element):
		""" raiseElement(UIElement element) - Bring the supplied UIElement to the top of the element list."""
		self.z_stack.remove(element)
		self.z_stack.insert(0, element)
		self.draw_stack.remove(element)
		self.draw_stack.append(element)
		element.invalid = True
		#element.offset = 4
		#for x in self.elements:
		#	if x <> element:
		#		x.offset = 7
		
	def showSimpleDialog(self, title, prompt, value, function):		
		""" showSimpleDialog(String title, String prompt, String value, function_ref function) - show a simple input dialog with a title and a prompt.
		
		BtoRGUIClasses.showSimpleDialog(String title, String prompt, String value, function_ref function)
		This function displays a simple input dialog. 
		
		Caveats: Blender's python implementation currently doesn't allow threading support, so this dialog requires that you supply a callback function to which 
		control will pass. When the OK or Cancel button is clicked, the dialog will close itself, and will then call the supplied function, and passes itself as an argument 
		to this function. The supplied function can then test the dialog's state and values for further processing. 
		
		See the TextDialog object for more information.
		
		Arguments: 
		title - A string that contains the title of the dialog.
		prompt - A string that contains the prompt to be displayed.
		value - A string to initialize the dialog with.
		function - The callback function to return the output of the dialog to.
		
		Example:
		def showValue(value):
			print value
			
		evt_man = BtoRGUIClasses.EventManager()
		evt_man.register()
		evt_man.showSimpleDialog("Example Dialog", "Enter a value", "Some Value", showValue)
		
		"""
		dialog = TextDialog(0, 0, 400, 400, title, prompt, value, self.closeDialog)	
		screen = Blender.Window.GetAreaSize()
		dialog.x = (screen[0] / 2) - (dialog.width / 2)
		dialog.absX = dialog.x
		dialog.y = (screen[1] / 2) + (dialog.height / 2)
		dialog.absY = dialog.y	
		dialog.target_function = function
		dialog.invalid = True
		dialog.dialog = True
		self.addElement(dialog)
		self.raiseElement(dialog)

	def showErrorDialog(self, title, prompt):
		dialog = MsgBox(title, prompt, self.closeDialog)
		screen = Blender.Window.GetAreaSize()
		dialog.x = (screen[0] / 2) - (dialog.width / 2)
		dialog.absX = dialog.x
		dialog.y = (screen[1] / 2) + (dialog.height / 2)
		dialog.absY = dialog.y	
		dialog.dialog = True
		dialog.invalid = True
		dialog.target_function = None
		self.addElement(dialog)
		self.raiseElement(dialog)
		
	def setupFontList(self):
		bgl_font_large = []
		bgl_font_normal = []
		bgl_font_small = []
		bgl_font_tiny = []
		charString = """ ABCDEFGHIJKLKMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890`,./;'[]-=<>?:"{}|~!@#$%^&*()_+"""
		for char in charString:
			self.bgl_font_large.append(Blender.BGL.getStringWidth(char, 'large'))
			self.bgl_font_normal.append(Blender.BGL.getStringWidth(char, 'normal'))
			self.bgl_font_small.append(Blender.BGL.getStringWidth(char, 'small'))
			self.bgl_font_tiny.append(Blender.BGL.getStringWidth(char, 'tiny'))
			
		self.bgl_font_sizes = { "large" : bgl_font_large, "normal" : bgl_font_normal, "small" : bgl_font_small, "tiny" : bgl_font_tiny }
		
	def showConfirmDialog(self, title, prompt, function, discard):
		""" showSimpleDialog(String title, String prompt, function_ref function, boolean discard) - show a simple input dialog with a title and a prompt.
		
		BtoRGUIClasses.showSimpleDialog(String title, String prompt, function_ref function, boolean discard)
		This function displays a simple confirmation dialog. 
		
		Caveats: Blender's python implementation currently doesn't allow threading support, so this dialog requires that you supply a callback function to which 
		control will pass. When the OK or Cancel button is clicked, the dialog will close itself, and will then call the supplied function, and passes itself as an argument 
		to this function. The supplied function can then test the dialog's state and values for further processing. 
		
		Arguments: 
		title - A string that contains the title of the dialog.
		prompt - A string that contains the prompt to be displayed.
		function - The callback function to return the output of the dialog to.
		discard - A boolean value to determine whether or not a discard button is displayed.
		
		Example:
		def showValue(value):
			print value
			
		evt_man = BtoRGUIClasses.EventManager()
		evt_man.register()
		evt_man.showConfirmDialog("Example Dialog", "Enter a value", showValue, True)
		
		"""
		dialog = ConfirmDialog(title, prompt, self.closeDialog, discard)
		screen = Blender.Window.GetAreaSize()
		dialog.x = (screen[0] / 2) - (dialog.width / 2)
		dialog.absX = dialog.x
		dialog.y = (screen[1] / 2) + (dialog.height / 2)
		dialog.absY = dialog.y	
		dialog.target_function = function
		dialog.invalid = True
		dialog.dialog = True
		self.addElement(dialog)
		self.raiseElement(dialog)
		
	def closeDialog(self, dialog):
		
		if dialog.target_function != None:
			dialog.target_function(dialog) # fire the target function
		self.removeElement(dialog) 
		
	def registerCallback(self, event, callback):
		# blackmagic and v00d00
		self.__dict__[event + "_functions"].append(callback)
	
	def removeCallbacks(self, event):
		self.__dict__[event + "_functions"] = []
					
	def click_event(self):	
		""" click_event() - Dispatches all mouse clicks to registered objects."""
		# build up the draw/event stack on click events, since that directs where the user's interest is
		# firstly, figure out if I even need to change the current interest		
		if self.z_stack[0].hit_test() == False and self.z_stack[0].dialog == False: # something changed, the mouse is no longer in the top-level element, so			
			for element in self.elements:
				
				if element.hit_test(): # modify hit_test to get the set bounds of the object in question
					# iterate the element list and assign the one that was clicked to the z_stack first element
					# delete from the list
					self.z_stack.remove(element)
					# and add at the top
					self.z_stack.insert(0, element)
					self.noComp = False
					break # bail out
			# now that we know the stack has changed, modify the draw_stack
			self.draw_stack = []
			for x in self.z_stack:
				self.draw_stack.append(x)
				
			# reverse the list for draw() purpose
			self.draw_stack.reverse()
			
		# let's hit them all
		self.z_stack[0].click_event()
		
		#for element in self.z_stack: # hit the top level item first
		#	if element <> self.z_stack[0]:
		#		element.offset = 7
		#	else:
		#		element.offset = 7
		
	def right_click_event(self):
		if self.z_stack[0].hit_test():
			self.z_stack[0].right_click_event()
				
			
			
	def mouse_event(self):
		""" mouse_event() - Dispatches all mouse events (mouse movement) to registered objects."""		
		for element in self.z_stack:	
			element.mouse_event() 
			# mouse events apply across the board, since everything is interested to know
			# for cases of mouse_in/mouse_out
			
	def release_event(self):
		""" release_event() - Dispatches all mouse button release events to registered objects."""
		for element in self.z_stack:
			element.release_event() 
			# release events apply across the board, since everything is interested 
			#in those, no matter where the mouse is(with a few exceptions) 
			# I do, however, want to iterate in order of the z_stack
	
	def wheel_event(self, evt):
		""" wheel_event() - Dispatches all mouse wheel events to registered objects."""
		if self.z_stack[0].dialog == True:
			self.z_stack[0].wheel_event(evt)
		else: # anything goes.
			for element in self.z_stack: 
				element.wheel_event(evt)
	
	def key_event(self, evt, val): # key events probably need to be directed to the top object(if a text field was clicked, all that)
		""" key_event() - Dispatches all key presses to registered objects."""
		# of course this means I now have to change my textfield class, dammit, but maybe this is better
		self.z_stack[0].key_event(evt, val)
		
	def draw(self):	
		""" draw() - sets up the background and draws the elements in the element list."""		
		# on screen refresh events, make any "automatic" stuff update 
		# run this first, to ensure that whatever's registered happens correctly
		# note that draw functions will need to remove themselves correctly from the list after they're done.
		for func in self.draw_functions:
			func() # should be no issue. I can use my call factory to build anything I need.
			
		screen = Blender.Window.GetAreaSize()
		if self.screen[0] != screen[0] or self.screen[1] != screen[1]:
			for element in self.draw_stack:
				element.invalid = True
			self.screen = screen
		
		Blender.BGL.glClearColor(self.backGround[0], self.backGround[1], self.backGround[2], 1)
		Blender.BGL.glClear(Blender.BGL.GL_COLOR_BUFFER_BIT)
		self.drawScreenGrid(20, 20, [0.3, 0.3, 0.3])
		for element in self.draw_stack:
			element.draw()
		
	def event(self, evt, val):
		""" event(evt, val) - Processes incoming events and dispatches them accordingly"""
		#process exit key first
		#if evt == Blender.Draw.ESCKEY:
		#	Blender.Draw.Exit()
		try:
			if evt == Blender.Draw.WHEELDOWNMOUSE or evt == Blender.Draw.WHEELUPMOUSE:
				self.wheel_event(evt)
				Blender.Draw.Redraw()
				
			if evt == Blender.Draw.LEFTMOUSE and val == 1: # mouse down
				self.click_event()
				Blender.Draw.Redraw()
				
			if evt == Blender.Draw.LEFTMOUSE and val == 0: # mouse button released
				self.release_event()            
				Blender.Draw.Redraw()
			
			if evt == Blender.Draw.RIGHTMOUSE and val == 0: # button released
				self.right_click_event()
				Blender.Draw.Redraw()
				
			# modify the event loop so that mouse events are queued, so that mousex and mousey are both received
			# before the mouse_event() handlers are called
			if (evt == Blender.Draw.MOUSEX and val <> self.lastX) or (evt == Blender.Draw.MOUSEY and val <> self.lastY):
				if evt == Blender.Draw.MOUSEX:
					self.lastX = val
				else:
					self.lastY = val
				self.mouse_event()
						
				Blender.Draw.Redraw()
		
			if evt > 20:
				self.key_event(evt, val)
				Blender.Draw.Redraw()
		except:
			import traceback #, StringIO
			#st = StringIO.StringIO("")
			traceback.print_exc(file=sys.stdout) #st)
			#st.seek(0)
			#st = st.read()
			self.showErrorDialog("An error occured!", "An error has occured, see console for details");
		
	def bevent(self, evt):
		""" bevent(evt) - bevent processor, which doesn't really do anything since I'm not processing blender's standard buttons. """
		pass
		# hey presto, this doesn't do JACK because it's not needed...imagine that!
		
	
class UIElement(UI):
	# normal 216, 214, 230
	normalColor = [85, 85, 85, 255] # the normal color of the element
	textColor = [255, 255, 255, 255] # basic text color, can be overriden for anything
	textHiliteColor = [255, 255, 255, 255] # hilite color
	borderColor = [43, 0, 0, 255] # basic border color
	hoverBorderColor = [45, 45, 45, 255]
	designBorderColor = [255, 0, 0, 255]
	hoverColor = [105, 103, 127, 255]
	outlineColor = [65, 65, 65, 128]
	font_sizes = { 'tiny' : 9, 'small' : 10, 'normal' : 12, 'large' : 14} # convenience item
	font_cell_sizes = { 'tiny' : [5,9], 'small' : [5, 10], 'normal' : [7, 12], 'large' : [6, 14] } # replaces the above
	ui = None # default UI
	radius = 10
	fontsize = 'normal'
	cornermask = 0
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", "designBorderColor", "hoverColor", "outlineColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	""" UIEement class - This defines the basic object upon which all other GUI objects are built.
	
	Constructor: element = UIElement(x, y, width, height, name, title, parent, auto_register)
	
	Arguments: 
	x - integer, x location
	y - integer, y location
	width - integer, width of the element
	height - integer, height of the element
	name - String, name of the element
	title - String, title of the element
	parent - Container object for this element. 	
	"""
	
	def __init__(self, x, y, width, height, name, title, parent, auto_register, fontsize = 'normal'): # default values are x, y, width, height, name, title		
		# basic constructor method for all UI objects
		self.fontsize = fontsize
		self.elements = [] 
		self.z_stack = []
		self.draw_stack = []
		self.noComp = True
		self.click_functions = [self.objDebug] # all functions contain self.objDebug for testing purposes
		self.release_functions = [self.objDebug]
		self.mouse_functions = [self.objDebug]
		self.key_functions = [self.objDebug]	
		self.wheel_functions = [self.objDebug]	
		self.validate_functions = [self.objDebug]
		self.right_click_functions = [self.objDebug]
		self.select_functions = [self.objDebug]
		self.update_functions = []
		self.parent = parent # the parent, used for correctly positioning the element on the screen
		self.x = x  # base horizontal value
		self.y = y # base vertical value
		self.image = None
		self.transparent = False
		# niggly GUI stuff
		if parent == None:
			self.absX = self.x
			self.absY = self.y
			# I have no parent, so don't bother with the parent's UI, this will be set later
		else:			
			self.absX = parent.absX + self.x # corrected for the location within a parent component
			self.absY = parent.absX + self.y # ditto
			self.ui = parent.ui # for the object stack
			
		self.width = width
		self.height = height
		self.bounds = [[self.absX, self.absY, self.absX + self.width, self.absY - self.height]]
		self.name = name
		self.title = title
		self.outlined = False
		self.parentLockWidth = False
		self.parentLockHeight = False
		self.gutterWidth = 5
		self.gutterHeight = 10
		self.debug = 0
		self.drawLast = None		
		self.invalid = True
		self.isVisible = True # visible by default
		self.showScroll = True # visible in a scrollpane by default
		# calculate locations
		self.validate()
		self.mouse_target = None # for steering mouse input
		#self.registerCallback("click", self.stat)
		self.dialog = False # dialog tracking function for "modal" dialogs
		if auto_register: # automatically append myself to the parent
			parent.addElement(self) 
			
		self.local_undo_stack = []
		self.draggable = False
		self.designMode = False
		self.designMove = False
		self.designModeSelect = False
	
	def getTitle(self):
		return self.title
	def setTitle(self, title):
		self.title = title
	def addElement(self, element):
		if element not in self.elements:
			self.elements.append(element) # this maintains the original order of the element list
			self.z_stack.append(element) # the control stack
			self.draw_stack = [] # the draw stack...runs in reverse order of the control stack
			for element in self.z_stack:
				self.draw_stack.append(element)
				
			self.draw_stack.reverse()
	
	def removeElement(self, element):		
		self.elements.remove(element)
		if element in self.z_stack:			
			self.z_stack.remove(element)			

		if element in self.draw_stack:			
			self.draw_stack.remove(element)
		
		if element in self.elements:
			self.elements.remove(element)
		del element
		
	def raiseElement(self, element):
		""" raiseElement(UIElement element) - Bring the supplied UIElement to the top of the element list."""
		self.z_stack.remove(element)
		self.z_stack.insert(0, element)
		self.draw_stack.remove(element)
		self.draw_stack.append(element)
		element.invalid = True
			
	def clearElements(self):
		self.elements = []
		self.draw_stack = []
		self.z_stack = []
		self.invalid = True
	def registerCallback(self, event, callback):
		# blackmagic and v00d00
		# At some point, rewrite this a bit to be more interface driven so the object in question has certain signals per interface
		self.__dict__[event + "_functions"].append(callback)
	
	def removeCallbacks(self, event):
		self.__dict__[event + "_functions"] = []
		
	def draw(self):		
		# draw myself, but since this is the base class, I probably 
		# don't want to actually draw anything but a basic box and let the 
		# contained components deal with drawing themselves
		if self.isVisible: # do the deed
			self.validate()	# validate myself, and make sure everything is correctly offset        			
			if not self.transparent:
				self.setColor(self.normalColor) # use the basic color and draw a nice box to define this component
				self.uiRoundBox(self.absX, self.absY - self.height, self.absX + self.width, self.absY, self.radius, self.cornermask)
			#if self.designMode:
			#	self.drawDesignGrid()
			for element in self.draw_stack:
				if element.isVisible:
					element.draw()
				
			if self.outlined: # use the outline color and draw an outline
				self.setColor(self.outlineColor)
				self.uiOutline(self.absX, self.absY - self.height, self.absX + self.width, self.absY, self.radius, self.cornermask)
				
			if self.image != None:
				self.image.draw()
			
			if self.designModeSelect:
				self.setColor(self.designBorderColor)
				self.uiOutline(self.absX, self.absY - self.height, self.absX + self.wdith, self.absY, self.radius, self.cornermask)

				
	def drawDesignGrid(self):
		self.setColor([64, 64, 64, 255])
		rows = self.height / 5
		columns = self.width / 5
	
		for row in range(rows):
			for column in range(columns):
				self.uiRoundBox(row, column, row+1, column + 1, 0, 0)
	
			
		
	def objDebug(self):  
		pass
		#print "ObjectDebug: ", self.name
		#print "Title: ", self.title
		#print "Coords: x = ", self.x, ", y = ", self.y, ", width = ", self.width, ", height = ", self.height
		#print "Registered callbacks:"
		#print " click: ", self.click_funcs 
		#print " release: ", self.release_funcs
		#print " key: ", self.key_funcs

	def validate(self): 
		# apply transforms to the x,y values of this object using the parent's absolute locations
		if self.invalid:
			# cascade invalidations down the chain of contained elements
			for element in self.elements:
			#	if element.isVisible: # this test probably isn't neccessary
				element.invalid = True
			
			if self.parent == None:
				self.absX = self.x
				self.absY = self.y
			else:
				self.absX = self.parent.absX + self.x 
				self.absY = self.parent.absY - self.y			
			
			if self.image != None:				
				self.image.invalid = True
				self.image.validate() 
			if self.parentLockWidth and self.parent != None:
				# hey dumbass, these have to be OFFSETS!
				# width - self.x should give me the remaining width
				self.width = self.parent.width - self.x - self.gutterWidth
			if self.parentLockHeight and self.parent !=  None:
				self.height = self.parent.height - self.height - self.gutterHeight
		self.invalid = False
		if self.isVisible: 
			for func in self.validate_functions:
				func()
	def hide(self):
		self.isVisible = False
	def show(self):
		self.isVisible = True
					
	def hit_test(self):		
		rVal = False                
		if len(self.z_stack) > 0: # has children, check those first
			for element in self.z_stack:
				if element.hit_test() and element.isVisible:
					rVal = True 
		# if I get a hit, even if the below doesn't hit, it will still return true
		coords = self.area_mouse_coords()		
		if self.absX <= coords[0] and self.absX + self.width >= coords[0]: 
			if self.absY >= coords[1] and self.absY - self.height <= coords[1]:
				rVal = True		# this is an either/or situation, if ANY component in the list returns true, then the test is successful
		return rVal  	
	
	def click_event(self):
		""" click_event() - Dispatches all mouse clicks to registered objects."""	
		# all UIElements will use this function, unless they override it. 
		# I can thus supply it as a pass through function that calls 
		# itself on every contained element of this object				
		# build up the draw/event stacks on click events, since that directs where the user's interest is			
		found = False	
		# print self.__class__	
		if len(self.z_stack) > 0: # have some stuff to work on						
			if self.z_stack[0].hit_test() == False or self.z_stack[0].isVisible == False: # something changed, the mouse is no longer in the top-level element, so
				self.z_stack[0].invalid = True # set the invalidate flag
				for element in self.z_stack: # the z stack will be dynamic, not all elements will be tested, this is to support things like scrollpanes
					if element.hit_test() and element.isVisible: 
						# iterate the element list and assign the one that was clicked to the z_stack first element
						# delete from the list
						self.z_stack.remove(element)
						# and add at the top
						self.z_stack.insert(0, element)	
						self.draw_stack.remove(element)
						self.draw_stack.append(element) # append it to the rear, it's the last thing drawn
						found = True
						break
				if found:
					self.noComp = False
				else:
					self.noComp = True
			else: # the top element's been hit
				# print "hit the top element!"
				self.noComp = False
		if self.noComp == False:
			for element in self.z_stack:
				if element.isVisible:
					element.click_event()
					break
				
			# print "Click Event received by Object: ", self.z_stack[0].name, " ## Child of ", self.name						
		else: # no selected components
			if not self.designMode:
				self.click_funcs() # fire click callbacks for this component 
			
			else:
				self.designMove = True
				if (coords[0] >= self.absX and coords[0] <= self.absX + self.width) and (coords[1] <= self.absY and coords[1] >= self.absY - self.headerHeight) and self.movable:
						self.move_offsetx = coords[0] - self.absX # this actually needs to update the x and y relative
						self.move_offsety = coords[1] - self.absY
						self.designMove = True    	
		
	def click_funcs(self):
		# print "iterating click functions!"		
		for func in self.click_functions:                 
			if func == self.objDebug:
				if self.debug:
					func() # don't pass self here
			else:
				func(self)
				
	def right_click_event(self):
		# all UIElements will use this function, unless they override it. 
		# I can thus supply it as a pass through function that calls 
		# itself on every contained element of this object				
		# build up the draw/event stacks on click events, since that directs where the user's interest is			
		found = False	
		# print self.__class__	
		if len(self.z_stack) > 0: # have some stuff to work on						
			if self.z_stack[0].hit_test() == False or self.z_stack[0].isVisible == False: # something changed, the mouse is no longer in the top-level element, so
				self.z_stack[0].invalid = True # set the invalidate flag
				for element in self.z_stack: # the z stack will be dynamic, not all elements will be tested, this is to support things like scrollpanes
					if element.hit_test() and element.isVisible: 
						# iterate the element list and assign the one that was clicked to the z_stack first element
						# delete from the list
						self.z_stack.remove(element)
						# and add at the top
						self.z_stack.insert(0, element)	
						self.draw_stack.remove(element)
						self.draw_stack.append(element) # append it to the rear, it's the last thing drawn
						found = True
						break
				if found:
					self.noComp = False
				else:
					self.noComp = True
			else: # the top element's been hit
				# print "hit the top element!"
				self.noComp = False
		if self.noComp == False:
			for element in self.z_stack:
				if element.isVisible:
					element.right_click_event()
					break
		else:
			self.right_click_funcs() # fire click callbacks for this component 
				
		
	def right_click_funcs(self):
		for func in self.right_click_functions:
			if func == self.objDebug:
				if self.debug:
					func()
			else:
				func(self)
				
			
	def key_event(self, key, val): 
		""" key_event() - Dispatches all key press events to registered objects."""	
		# default pass through event   
		if self.noComp: # no selected component on the last click, so key funcs go to me
			self.key_funcs(key, val) # target object is myself
		else: # send the key_event to the target object on the z_stack
			self.z_stack[0].key_event(key, val)
		
		# print "Key Event:", key, ", ", val, " received by ", self.z_stack[0].name, " child of ", self.name
		
		
	def key_funcs(self, key, val):
		# this will let me decouple key events from the actual event handler
		for func in self.key_functions:
			if func == self.objDebug:
				if self.debug:
					func()
			else:
				func(self, key, val)
			
	def mouse_event(self):
		""" mouse_event() - Dispatches all mouse events (mouse movement) to registered objects."""		
		if not self.designMode:
			self.mouse_funcs()		
			for element in self.z_stack:
				if element.hit_test() and element.isVisible:
					element.mouse_event()
		else: # I'm in design mode
			found = False
			for element in self.z_stack:
				if element.hit_test():
					element.mouse_event()
					found = True
			if not found:
				coords = self.area_mouse_coords() 
				
				tempx = coords[0] - self.move_offsetx
				if tempx < 0:
					self.x = 0
				
				tempy = coords[1] - self.move_offsety
				if tempy < 0:
					self.y = 0
				
				self.invalid = True # validate will continually set this false, so      
		

	def mouse_funcs(self):
		for func in self.mouse_functions: # runs on everything, in case something is being moved or other things I can't think of right now
			if func == self.objDebug:
				if self.debug:
					func() # self is passed automatically to self.objDebug
			else:
				func(self)   
	
	def wheel_event(self, evt):
		""" wheel_event() - Dispatches all mouse wheel events to registered objects."""
		if self.hit_test() and self.isVisible:
			self.wheel_funcs(evt) # again, decouple functionality from the event handler and make it a function-based thing
		for element in self.z_stack:
			if element.isVisible:
				element.wheel_event(evt)
			
	def wheel_funcs(self, evt):
		for func in self.wheel_functions:
			if func == self.objDebug:
				if self.debug:
					func()
			else:
				func(evt)
				
  

	def stat(self, obj):
		""" debug function """
		print "Stat Event called from Object: ", obj.name, ", with target object:", self.name
		print "Object bounds are:", self.bounds[0]	
		if len(self.bounds) > 1: # there's a child object here
			print "Child object is: ", self.z_stack[0].name, " with bounds: ", self.bounds[1]
			print self.elements
		
	def release_event(self):		
		if self.noComp: # no child component hit, so...
			if not self.designMode:
				self.release_funcs() 
			else:
				if self.designMove:
					self.designMove = False
				if self.hit_test():
					self.designModeSelect = True
				else:
					self.designModeSelect = False
				
		for element in self.z_stack:
			if element.isVisible:
				element.release_event()
		# release event goes to all components
		
			
	def release_funcs(self):        
		# release funcs hit no matter where the mouse is	
		for func in self.release_functions:
			if func == self.objDebug:
				if self.debug:
					func()
			else:
				func(self)
				
	def update_funcs(self):
		for func in self.update_functions:
			if func == self.objDebug:
				if self.debug:
					func()
			else:
				func(self)
				
	def select_funcs(self):
		for func in self.select_functions:
			if func == self.objDebug:
				if self.debug:
					func()
			else:
				func(self)
	
class Panel(UIElement):
	# 198, 197, 203
	# header, 110, 110, 128
	headerColor = [60, 60, 60, 255]
	shadowColor = [45, 45, 45, 150]
	normalColor = [85, 85, 85, 255]
	outlineColor = [58, 58, 58, 255]
	titleColor = [255, 255, 255, 255]
	headerHeight = 25
	cornermask = 15
	offset = 7 
	shadowed = True
	properties = ["normalColor", 
				"textColor", 
				"textHiliteColor", 
				"borderColor", 
				"hoverBorderColor", 
				"designBorderColor", 
				"hoverColor", 
				"outlineColor", 
				"titleColor",
				"shadowColor",
				"radius", 
				"fontsize", 
				"cornermask",
				"headerHeight",
				"offset",
				"shadowed", 
				"outlined",
				"hasHeader"]
	parameters = ["x", "y", "width", "height", "name", "title", "auto_register"]
	def __init__(self, x, y, width, height, name, title, parent, auto_register, fontsize = 'large'):
		UIElement.__init__(self, x, y, width, height, name, title, parent, auto_register, fontsize = fontsize) # initialize with the baseclass constructor
		self.move_offsetx = 0
		self.move_offsety = 0
		self.hasHeader = True
		self.hasFooter = False
		self.fontsize = fontsize
		self.move = False
		self.resize = False
		self.invalid = False # for items that want revalidation in case the panel was moved
		self.drawLast = None
		self.bounds = [0, 0, 0, 0]
		self.movable = True		
		self.outlined = True
		self.max_height = 600
		self.min_height = 100
		self.max_width = 600
		self.min_width = 200
		self.x_resize = False
		self.y_resize = False
		
	def draw(self):
		# note, internally, a panel's absX and absY variables are offset depending upon header usage
		# maybe I should override these of this so uiElement.draw() gets called for all componenents?
		self.validate()
		ui = self 		
		if self.shadowed:
			ui.setColor(self.shadowColor)
			# draw two boxes here, butted agains where the actual panel will be.
			#ui.uiRoundBox(self.absX + self.width, self.absY - 4, self.absX + self.width + 5, self.absY - (self.height + 5), 5, 28)
			#ui.uiRoundBox(self.absX + 5, self.absY - self.height, self.absX + self.width + 5, self.absY - (self.height + 5), 5, 22) # bottom
			ui.uiRoundBox(self.absX + self.offset, self.absY - (self.height + self.offset), self.absX + self.width + self.offset, self.absY - self.offset, 10, self.cornermask + 16) # shadow
		
		ui.setColor(self.normalColor)
		ui.uiRoundBox(self.absX, self.absY - self.height, self.absX + self.width, self.absY, 10, self.cornermask) # the main container
		if self.outlined:
			self.setColor(self.outlineColor)
			self.uiOutline(self.absX, self.absY - self.height, self.absX + self.width, self.absY, self.radius, self.cornermask)
			
		ui.setColor(self.headerColor)
		headerMask = 0
		if self.cornermask & 1:
			headerMask = headerMask + 1
		if self.cornermask & 2:
			headerMask = headerMask + 2
			
		if self.hasHeader:
			ui.uiRoundBox(self.absX, self.absY - self.headerHeight, self.absX + self.width, self.absY, 10, headerMask) # the panel header
		
		if self.parent == None:
			ui.setColor(self.titleColor)
			ui.setRasterPos(self.absX + 10, self.absY - 18)	
			Blender.Draw.Text(self.title, self.fontsize) 
			# now that the panel is drawn
		else:
			ui.setColor(self.titleColor)
			ui.setRasterPos(self.absX + 2, self.absY - (self.font_cell_sizes[self.fontsize][1] + 2))
			Blender.Draw.Text(self.title, self.fontsize)	
		
		for element in self.draw_stack:
			if element.isVisible:
				element.draw()
		
		# if the panel was moved, all listeners should be updated, so now
		self.invalid = False # do I do this here? Or on valid?
		# now that the panel is drawn, revalidate to ensure that any offsets are taken into account
		
	
	def mouse_event(self):			
		if self.move:			
			coords = self.area_mouse_coords()
			self.x = coords[0] - self.move_offsetx
			self.y = coords[1] - self.move_offsety  
			self.invalid = True # validate will continually set this false, so          
		if self.resize:
			
			coords = self.area_mouse_coords()
			if self.x_resize:
				offset_x = coords[0] - self.absX + self.resize_offset_x
				if offset_x < self.max_width and offset_x > self.min_width:
					self.width = offset_x
			if self.y_resize:
				offset_y = self.absY - coords[1] + self.resize_offset_y
				if offset_y < self.max_height and offset_y > self.min_height:
					self.height = offset_y
				
					
			self.invalid = True
		else:
			UIElement.mouse_event(self) # use the base class
						
	def click_event(self): 
		coords = self.area_mouse_coords()
		sized = False
		if self.hasHeader: # check for a panel header
			# now check for an element IN the header
			found = False
			for element in self.elements:
				if element.hit_test():
					found = True
			if found == False:
				if (coords[0] >= self.absX and coords[0] <= self.absX + self.width) and (coords[1] <= self.absY and coords[1] >= self.absY - self.headerHeight) and self.movable:
					# yes. The above is ugly. But it's neccessary. Don't ask. And don't fuck with it either.
					# get the offset from where the mouse is now
					self.move_offsetx = coords[0] - self.absX
					self.move_offsety = coords[1] - self.absY
					self.move = True    		
					sized = True
		if self.hasFooter: # check for resizable footer
			
			if (coords[0] >= (self.absX + self.width - 10)) and (coords[0] < (self.absX + self.width)) and (coords[1] >= self.absY - self.height) and (coords[1] < self.absY - (self.height - 10)):
				self.resize_offset_x = (self.absX + self.width) - coords[0]
				self.resize_offset_y = coords[1] - (self.absY - self.height)
				self.resize = True
				sized = True
		if not sized:
			UIElement.click_event(self)  # use the base class for everything else
		
	def release_event(self):        
		if self.move == True: # the panel was being moved                
			self.move = False	
			self.invalid = True	
		elif self.resize == True:
			self.resize = False
			self.invalid = True
		UIElement.release_event(self)
		# done!
			

class ScrollBar(UIElement):
	properties = ["normalColor", 
			"borderColor", 
			"designBorderColor", 
			"outlineColor", 
			"shadowed", 
			"style",
			"bordered"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	def __init__(self, x, y, width, height, name, style, parent, auto_register):
		# style 1 is vertical, style 2 is horizontal
		UIElement.__init__(self, x, y, width, height, name, name, parent, auto_register)
		self.bordered = True
		# a scrollbar consists of 3 buttons, one of varying height/width, and two arrow buttons for up/down, left/right
		# thus
		if style == 1:
			self.button1 = Button(0, 0, self.width, 15, "", "", 'tiny', self, True)
			#self.button2 = Button(0, 15, self.width, 15, "", "", 'tiny', self, True) # at the top. I'll figure out sizing and crap later
			self.button3 = Button(0, self.height - 15, self.width, 15, "", "", 'tiny', self, True)
		else:
			self.button1 = Button(0, 0, 15, self.height, "", "", 'tiny', self, True)
			#self.button2 = Button(15, 0, 15, self.height, "", "", 'tiny', self, True)
			self.button3 = Button(0, self.width - 15, 15, self.height, "", "", 'tiny', self, True)
		self.style = style
		#self.rounded = False
		self.button1.cornermask = 0
		self.button1.shadowed = False
		
		#self.button2.cornermask = 0
		#self.button2.shadowed = False
		
		self.button3.cornermask = 0
		self.button3.shadowed = False
		
		# ok, so...
		self.button1.registerCallback("release", self.scrollUp)
		self.button3.registerCallback("release", self.scrollDown)
		self.scrolling = False
		
		self.last_y = 0
		
		
		
		
	def scrollUp(self, obj):
		self.parent.scrollUp()
	
	def scrollDown(self, obj):
		self.parent.scrollDown()
			
			
	#def mouse_event(self):
	#	jump_dist = 0
	#	inc = 1
	#	
	##		coords = self.area_mouse_coords()
		#	# I'm scrolling, so move button two up or down depending upon the offset at hand.
		#	# figure out a scroll percentage based on the number of elements in my parent object
		#	# how many elements do I have?
		#	count = len(self.parent.elements)
		#	if count > 0: # of course, this only works if there's more than ZERO elements!
		#		# so now hack the available space of this scrollbar into increments based on the # of elements that I have
		#		# obviously if I go below single pixel distances, I'll have to do something different
		#		inc = float(self.height - 30) / float(count) 
		#		if inc < 1:
		#			# less than single pixel values, start jumping
		#			# ok so I know that I have more items than pixels
		#			# so instead of dividing pixels by count, I go the other way 
		#			# and divide count by pixels
		#			# thus
		#			inc = 1
		#			jump = int(float(count) / float(self.height - 30)) # make it an int so it's happy
		#		else:
		#			inc = int(inc)
		#			jump = 0
		#		
		#		if self.style == 1:
		#			# vertical scrolling...I only care about the y value
		#			
		#			if (coords[1]  < self.button1.absY + 16) and (coords[1] > self.button3.absY): # checking for upper and lower bounds
		#				# for scrolling, we've first established an incremental value
		#				# now what we have to do is determine if the mouse has moved far enough of that incremental value to scroll or not.
		#				# we will *still* move the button, but we won't actually scroll unless the mouse movement is > inc
		#				# so:										
		#				# get the mouse position relative to self
		#				y_relative = self.absY - coords[1] 
		#				
		#				#move button 2 to it's new location
		#				self.button2.y = y_relative
		#				self.button2.invalid = True						
		#				# pick a scrolling direction
		#				if self.last_y > self.button2.y and (self.last_y - self.button2.y) > inc:
		#					if (jump > 0):								
		#						for x in range(jump):
		#							self.scrollDown(None)
		#					else:
		#						self.scrollDown(None)
		#				elif self.last_y < self.button2.y and (self.button2.y - self.last_y) > inc:
		#					if jump > 0:
		#						for x in range(jump):
		#							self.scrollUp(None)
		#					else:
		#						self.scrollUp(None)						
		#					
		#				self.last_y = y_relative
		#		else:
		#			if (coords[0] > self.button1.absX + 16) and (coords[0] < self.button3.absX + self.button3.width):
		#				x_relative = coords[0] - self.absX
		#				self.button2.x = x_relative
		#				self.button2.invalid = True	
		#				self.last_x = x_relative
			
			
	def click_event(self):
		#if self.button2.hit_test():
			# if button 2 is hit, then I know that I'm probably scrolling
			#coords = self.area_mouse_coords()
			#self.scrolling = True
			#self.scrollOffsetX = coords[0] - self.button2.absX
			#self.scrollOffsetY = coords[1] - self.button2.absY
		#else:
		self.button1.click_event()
		#self.button2.click_event()
		self.button3.click_event()
		self.scrolling = False
	
	def release_event(self):
		self.scrolling = False
		self.button1.release_event()
		#self.button2.release_event()
		self.button3.release_event()
		
	def draw(self):
		self.invalid = True
		self.validate()	
		UIElement.draw(self)
		self.button1.invalid = True
		self.button1.draw()
		#self.button2.invalid = True
		#self.button2.draw()
		self.button3.invalid = True
		self.button3.draw()
		
		if self.style == 1:
			if self.button1.hit_test():
				self.setColor(self.textHiliteColor)
			else:
				self.setColor(self.textColor)
			# top arrow
			Blender.BGL.glBegin(Blender.BGL.GL_POLYGON)
			Blender.BGL.glVertex2i(self.button1.absX + (self.button1.width - 2), self.button1.absY - ((self.button1.height / 2) + 3))
			Blender.BGL.glVertex2i(self.button1.absX + (self.button1.width - 14), self.button1.absY - ((self.button1.height / 2) + 3))
			Blender.BGL.glVertex2i(self.button1.absX + (self.button1.width - 8), self.button1.absY - ((self.button1.height / 2) - 3))
			Blender.BGL.glEnd()
			
			if self.button3.hit_test():
				self.setColor(self.textHiliteColor)
			else:
				self.setColor(self.textColor)
				
			# bottom arrow
			Blender.BGL.glBegin(Blender.BGL.GL_POLYGON)
			Blender.BGL.glVertex2i(self.button3.absX + (self.button3.width - 2), self.button3.absY - ((self.button3.height / 2) - 3))
			Blender.BGL.glVertex2i(self.button3.absX + (self.button3.width - 14), self.button3.absY - ((self.button3.height / 2) - 3))
			Blender.BGL.glVertex2i(self.button3.absX + (self.button3.width - 8), self.button3.absY - ((self.button3.height / 2) + 3))
			Blender.BGL.glEnd()				
		
class ScrollPane(UIElement):
	properties = ["normalColor", 
			"borderColor", 
			"designBorderColor", 
			"outlineColor", 
			"shadowed", 
			"style",
			"bordered"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	def __init__(self, x, y, width, height, name, title, parent, auto_register):
		UIElement.__init__(self, x, y, width, height, name, title, parent, auto_register)		
		self.currentItem = 1
		self.cornermask = 0 # setting the cornermask to 0 for scrollpanes
		self.registerCallback("wheel", self.scroll)
		self.registerCallback("validate", self.layout)
		self.lastItem = 0
		self.invalid = True
		self.scrollbar = ScrollBar(self.width - 15, 0, 15, self.height, "Scrollbar", 1, self, False)
		self.updated = False
		self.offset = 3
		self.hasHidden = False

	def clearElements(self):
		UIElement.clearElements(self)
		self.lastItem = 0
		self.updated = True
		self.invalid = True
	def draw(self):			
		self.validate() 		
		UIElement.draw(self)
		if len(self.z_stack) <> len(self.elements):
			self.scrollbar.draw()
		for element in self.draw_stack:
			element.draw() # this draws only the objects in the draw stack
			# the z_stack will handle click events
			# objects not in the z_stack won't receive events
		

	def scroll(self, evt):		
		if evt == Blender.Draw.WHEELUPMOUSE:
			if self.currentItem > 1:
				self.currentItem = self.currentItem - 1
		elif evt == Blender.Draw.WHEELDOWNMOUSE:
			if self.currentItem < len(self.elements):
				self.currentItem = self.currentItem + 1
		self.invalid = True
				
	def scrollUp(self):
		if self.currentItem > 1:
			self.currentItem = self.currentItem - 1
		self.invalid = True
		print "scrolling up!"
	
	def scrollDown(self):
		print "scrolling down!"
		if self.currentItem < len(self.elements):
			self.currentItem = self.currentItem + 1
		self.invalid = True
	
	def addElement(self, element):		
		UIElement.addElement(self, element)
		self.updated = True		
		self.layout()
		
	def removeElement(self, element):
		UIElement.removeElement(self, element)
		self.updated = True
		self.layout()

		
	def layout(self):		
		# now setup the z and draw stacks		
		if self.lastItem <> self.currentItem or self.lastItem == 0 or self.updated: # something changed, update the layout	
			idx = 1
			currenty = 0
			heightlimit = self.height
			# start out with a new z_stack - on a scroll event, the zorder is disrupted anyway, since everything is moving
			self.z_stack = []		
			for element in self.elements: # surf the entire element list and add when appropriate
				if idx >= self.currentItem and currenty + element.height < heightlimit:	
					if element.showScroll:
						element.y = currenty					
						element.isVisible = True
						currenty = currenty + (element.height + self.offset)
						self.z_stack.append(element) # add this element to the stack	
						element.invalid = True
				else:
					element.isVisible = False			
				idx = idx + 1
			self.draw_stack = []
			for element in self.z_stack:
				self.draw_stack.append(element)
			self.draw_stack.reverse() 
			self.lastItem = self.currentItem
			self.updated = False
	
	def mouse_event(self):
		self.scrollbar.mouse_event()
		UIElement.mouse_event(self)
	
	def click_event(self):
		self.scrollbar.click_event()
		UIElement.click_event(self)
	def release_event(self):
		self.scrollbar.release_event()
		UIElement.release_event(self)
	def showElement(self, elementName):
		for element in self.elements:
			if element.name == elementName:
				element.showScroll = True
		self.lastItem = 0
		self.updated = True			
		self.layout()
		self.invalid = True
		
		
	def hideElement(self, elementName):
		for element in self.elements:
			if element.name == elementName:
				element.showScroll = False
		self.lastItem = 0				
		self.updated = True				
		self.layout()
		self.invalid = True
class Button(UIElement):
	shadowColor = [45, 45, 45, 255]
	downColor = [45, 45, 45, 255]
	shadowed = True
	outlined = True
	shadowoffset = 4
	textHiliteColor = [255, 255, 255, 255]
	push_offset = 0
	textlocation = 0
	radius = 15    
	cornermask = 0
	transparent = False
	properties = ["normalColor", 
			"textColor", 
			"textHiliteColor", 
			"borderColor", 
			"hoverBorderColor", 
			"designBorderColor", 
			"hoverColor", 
			"outlineColor", 
			"titleColor",
			"shadowColor",
			"radius", 
			"cornermask",
			"headerHeight",
			"offset",
			"shadowed", 
			"transparent",
			"shadowoffset",
			"downColor"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "fontsize", "auto_register"]
	def __init__(self, x, y, width, height, name, title, fontsize, parent, auto_register):
		self.image = None
		UIElement.__init__(self, x, y, width, height, name, title, parent, auto_register, fontsize = fontsize) # initialize with the baseclass constructor
		self.fontsize = fontsize
		self.pushed = False        
		self.value = title
		self.registerCallback("click", self.push)
		self.outlined = True
		
		
		
	def draw(self):
		self.validate()
		coords = self.area_mouse_coords()
		if self.transparent:
			pass # nothing, only register events
		else:
			# draw a basic button
			if self.shadowed == True: # the shadow should always be drawn
				self.setColor(self.shadowColor) # draw a pretty shadow
				self.uiRoundBox(self.absX + self.shadowoffset, self.absY - self.height - self.shadowoffset, self.absX + self.width + self.shadowoffset, self.absY - self.shadowoffset, self.radius, self.cornermask + 16)
			if self.pushed == True: # this only happens on a click event so
				self.setColor(self.downColor)
				if self.debug:
					print "setting downColor", self.downColor
			elif self.hit_test() and self.pushed == False: # 
				self.setColor(self.hoverColor)  
				if self.debug:
					print "setting hoverColor:", self.hoverColor
			else:
				self.setColor(self.normalColor) 
				if self.debug:
					print "setting normalColor", self.normalColor
				
			if self.shadowed and self.pushed:				
				pass
			else:            
				self.uiRoundBox(self.absX, self.absY - self.height, self.absX + self.width, self.absY, self.radius, self.cornermask)  
				if self.outlined:
					# the basic button is drawn, draw the outline
					if self.hit_test() and self.pushed == False:
						self.setColor(self.hoverBorderColor)
					else:
						self.setColor(self.outlineColor)    
					#self.setColor(self.outlineColor)    
					self.uiOutline(self.absX, self. absY - self.height, self.absX + self.width, self.absY, self.radius, self.cornermask)
				
			drawWidth = self.get_string_width(self.title, self.fontsize)
			if self.hit_test():
				self.setColor(self.textHiliteColor)
				if self.debug:
					print "setting textHiliteColor color: ", self.textHiliteColor
			else:
				self.setColor(self.textColor)
				if self.debug:                
					print "setting textColor: ", self.textColor
				
			push_offset = 0
			if self.pushed == True:
				
				push_offset = self.push_offset
			
			if self.textlocation == 0: # centered text 
				# the button center is absX + ((self.width / 2) - (drawWidth / 2))
				t_x = self.absX + push_offset + ((self.width / 2) - (drawWidth / 2)) # locate at the center of the button
				t_y = self.absY - push_offset + 2 - ((self.height / 2) + (self.font_cell_sizes[self.fontsize][1] / 2))  # this should center it
				self.setRasterPos(t_x, t_y) # this is malfunctioning somehow...
			elif self.textlocation == 1: # upper left
				# I probably don't care about width on the left hand side, so
				self.setRasterPos(self.absX + push_offset + 2, self.absY - push_offset - (self.font_cell_sizes[self.fontsize][1] + 2))
			elif self.textlocation == 2: # lower left
				self.setRasterPos(self.absX + push_offset + 2, self.absY - push_offset - (self.height - 2))
			elif self.textlocation == 3: # upper right            
				self.setRasterPos(self.absX + push_offset + (self.width - drawWidth - 2) , self.absY - push_offset - (self.font_cell_sizes[self.fontsize] + 2))
			elif self.textlocation == 4: # lower right
				self.setRasterPos(self.absX + push_offset + (self.width - drawWidth - 2), self.absY - push_offset - (self.height - 2))
			
			Blender.Draw.Text(self.title, self.fontsize)
		
		for element in self.elements:
			element.draw()
			
		if self.image != None:
			self.image.validate()
			self.image.draw()
		
	def push(self, button):		
		self.pushed = True
			
	def release_event(self): # override the event to provide extra functionality
		if self.pushed and self.hit_test(): # I have to test both to determine if the mouse was released over this component AND it was pushed			
			UIElement.release_event(self) # use the baseclass to launch events for this component
		self.pushed = False
		
	def setValue(self, value):
		self.title = value
							
class ToggleButton(Button):    
	toggleColor = [65, 65, 65]
	normalColor = [85, 85, 85, 255]
	shadowColor = [45, 45, 45, 255]
	textColor = [0, 0, 0, 255]
	textUpColor = [255, 255, 255, 255]
	textDownColor = [255, 255, 255, 255]
	hoverColor = [128, 128, 128, 255]
	downColor = [150, 150, 150, 255]
	shadowed = True
	shadowoffset = 4
	textHiliteColor = [255, 255, 255, 255]	
	upColor = normalColor
	cornermask = 15 
	textlocation = 0
	radius = 1.5
	push_offset = 4
	properties = ["normalColor", 
		"textColor", 
		"textHiliteColor", 
		"borderColor", 
		"hoverBorderColor", 
		"designBorderColor", 
		"hoverColor", 
		"outlineColor", 
		"hoverColor",
		"textUpColor",
		"toggleColor",
		"titleColor",
		"shadowColor",
		"downColor",
		"radius", 
		"cornermask",
		"headerHeight",
		"offset",
		"shadowed", 
		"transparent",
		"shadowoffset",
		"push_offset"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "fontsize", "auto_register"]
	def __init__(self, x, y, width, height, name, title, fontsize, parent, auto_register):
		Button.__init__(self, x, y, width, height, name, title, fontsize, parent, auto_register)
		self.state = False 
		self.registerCallback("release", self.toggle)
		
	def toggle(self, button):		
		self.pushed = False
		if self.hit_test():
			if self.state == True:
				self.state = False
				self.normalColor = self.upColor
				self.textColor = self.textUpColor
			else:
				self.state = True
				self.normalColor = self.downColor
				self.textColor = self.textDownColor
				
	def set_state(self, state):	
		self.state = state
		if state == False:			
			self.normalColor = self.upColor
			self.textColor = self.textUpColor
		else:			
			self.normalColor = self.downColor
			self.textColor = self.textDownColor
			

	def setValue(self, value): # convenience function
		self.set_state(value)
		
	def getValue(self):
		return self.state
class CheckBox(UIElement):
	# 198, 197, 203
	normalColor = [85, 85, 85, 255]
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", "designBorderColor", "hoverColor", "outlineColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]	
	def __init__(self, x, y, name, title, value, parent, auto_register, fontsize = 'normal'):
		""" __init__ function """
		if title == "":
			width = self.get_string_width(title, fontsize) + 25
			title = " "
		else:
			width = 14
		
		UIElement.__init__(self, x, y, width, self.font_cell_sizes["normal"][1] + 4, name, title, parent, auto_register)
		self.addElement(Label(15, 0, name, title, self, auto_register))
		self.registerCallback("release", self.toggleValue)
		self.value = value
		self.x_offset = 0
		self.y_offset = 0
		
	def toggleValue(self, obj):
		if self.hit_test():
			if self.value:
				self.value = False
			else:
				self.value = True
			
	def draw(self):
		UIElement.draw(self)
		x_offset = self.x_offset
		y_offset = self.y_offset
		# draw the box
		y = self.absY - 2
		self.setColor([255, 255, 255, 255])
		self.drawBox(self.absX + x_offset, y - y_offset, self.absX + 10 + x_offset, y - 10 - y_offset) # the box
		self.setColor([0, 0, 0, 255])
		self.drawline(self.absX - 1 + x_offset, y - y_offset, self.absX + 10 + x_offset, y - y_offset) # top of box
		self.drawline(self.absX - 1 + x_offset, y - y_offset, self.absX - 1 + x_offset, y - 10 - y_offset) # left hand side of box
		self.drawline(self.absX - 1 + x_offset, y - 10 - y_offset, self.absX + 10 + x_offset, y - 10 - y_offset) # bottom of box
		self.drawline(self.absX + 10 + x_offset, y - y_offset, self.absX + 10 + x_offset, y - 10 - y_offset) # right hand side of box
		
		# now, determine the state of the thing
		if self.value:
			# draw a check!
			x = self.absX
			
			self.setColor([0, 0, 0, 255])
			# Blender.BGL.glEnable( gl.GL_BLEND )
			# Blender.BGL.glBlendFunc( gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA )
			Blender.BGL.glBegin(gl.GL_POLYGON)
			verts = [[x + x_offset, y - 5 - y_offset],[x + x_offset + 5, y - 10 - y_offset], [x + x_offset + 9, y - y_offset], [x + x_offset + 5, y - 9 - y_offset]] 
			for vert in verts:
				Blender.BGL.glVertex2f(vert[0],vert[1])
				
			Blender.BGL.glEnd()
			# Blender.BGL.glDisable( gl.GL_BLEND )
			
		
	def setValue(self, value):
		self.value = value
	def getValue(self):
		return self.value
	
class ToggleGroup:
	
	def __init__(self, items):
		self.elements = items # should be a list of toggle buttons or other item objects
		self.funcs = []
		self.value = ""        
		self.stickyToggle = False
		for item in items:
			item.registerCallback("release", self.toggleValue) # on click funcs for this object type
			
	def toggleValue(self, button): 
		# print "Yo, I toggled and shit!"
		# sticky toggle keeps the button set, no matter what 		
		if self.stickyToggle:
			for element in self.elements:
				if element <> button:
					element.set_state(False)
			# simply set the state of this button and move on
			button.set_state(True)
		else:
			# print button.state
			if button.state == False:
				self.value = None # no value, since nothing's on
				# do nothing, the button has been turned off
			else: # the button was turned on, turn everything else off
				for element in self.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.name 

		for func in self.funcs: # iterate any functions that have been registered
			func(self) # always return self
		

class Menu(UIElement):
	cornermask = 0
	# normalColor = [218, 218, 218, 255]
	normalColor = [85, 85, 85, 255]
	hoverColor = [127, 127, 127, 255]
	shadowColor = [208, 208, 208, 255]
	textColor = [255, 255, 255, 255]
	textHiliteColor = [255, 255, 255, 255]
	arrowShade = [200, 200, 200, 255]
	arrowHover = [140, 140, 140, 255]
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"hoverColor", "outlineColor", "arrowShade", "arrowHover", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "menu", "auto_register"]
	
	def __init__(self, x, y, width, height, name, menu, parent, auto_register, 
						enableArrowButton = True, 
						enableMenuHeader = True, 
						noSelect = False, 
						baseButtonTitle = None, outlined = True,
						shadowed = True, fontsize = 'normal', 
						custom = False, customPrompt = "None", customFunc = None,
						noButton = False):
		self.expanded = False  # this is first to deal with an issue in validate()
		UIElement.__init__(self, x, y, width, height, name, name, parent, auto_register, fontsize = fontsize)
		self.fontsize = fontsize
		self.close_functions = []
		self.selected = 0	
		self.noButton = noButton
		self.menu = menu		
		# container for the invididual buttons  
		self.panel = Panel(0, 0, 100, 100, name, name, self, False)
		if not enableMenuHeader:
			self.panel.hasHeader = False
			
		self.panel.cornermask = 0
		self.panel.debug = 0
		self.panel.fontsize = self.fontsize
		self.panel.movable = False
		#self.panel.panelColor = [185, 185, 185, 255]
		self.panel.normalColor = [145, 145, 145, 255]
		self.panel.registerCallback("release", self.release)
		self.enableArrowButton = enableArrowButton
		self.enableMenuHeader = enableMenuHeader
		
		butIdx = 0     
		self.title = menu[0] # take the first item if initialized
		self.radius = 1.5   
		# base button
		if enableArrowButton:
			self.baseButton = Button(x, y, width - 26, height, name, name, self.fontsize, parent, False) # the base button object
		else:
			self.baseButton = Button(x, y, width, height, name, name, self.fontsize, parent, False)
		self.baseButton.cornermask = 0
		self.baseButton.radius = 0
		self.baseButton.push_offset = 0
		self.baseButton.shadowed = shadowed
		self.baseButton.normalColor = [85, 85, 85, 255]
		self.baseButton.outlined = outlined
		
		self.baseButton.hoverColor = self.hoverColor
		self.baseButton.textColor = self.textColor
		self.baseButton.textHiliteColor = self.textHiliteColor
		if baseButtonTitle != None:
			self.baseButton.title = baseButtonTitle
		else:
			self.baseButton.title = menu[0]
		self.baseButton.textlocation = 1
		self.baseButton.registerCallback("release", self.release)
		self.outlined = outlined
		# arrow button        
		if enableArrowButton:
			self.arrowButton = Button(x + (width - 25), y, 25, height, 'ArrowButton', '', 'normal', parent, False)
			self.arrowButton.cornermask = 0
			self.arrowButton.radius = 0
			self.arrowButton.push_offset = 0
			self.arrowButton.normalColor = self.normalColor
			self.arrowButton.hoverColor = self.hoverColor
			self.arrowButton.registerCallback("release", self.release)
			if shadowed == False:
				self.arrowButton.shadowed = False
				
		for a in menu: 
			# create some button objects here
			self.panel.addElement(Button(0, 0, 100, 20, butIdx, a, self.fontsize, self.panel, False)) # this will self-validate, and I bypass the auto_register
			self.panel.elements[butIdx].registerCallback("release", self.setValueAndCollapse) # on a click & release
			self.panel.elements[butIdx].cornermask = 0
			self.panel.elements[butIdx].shadowed = False
			self.panel.elements[butIdx].textlocation = 1        
			self.panel.elements[butIdx].outlined = False
			# button colors are set to the colors of the menu
			self.panel.elements[butIdx].hoverColor = self.baseButton.hoverColor 
			self.panel.elements[butIdx].normalColor = self.baseButton.normalColor
			self.panel.elements[butIdx].textColor = self.baseButton.textColor
			self.panel.elements[butIdx].textHiliteColor = self.baseButton.textHiliteColor
			self.panel.elements[butIdx].menu_parent = self
			butIdx = butIdx + 1 # so we know what button was pushed
		if custom:
			self.panel.addElement(Button(0, 0, 100, 20, butIdx, customPrompt, self.fontsize, self.panel, False)) # this will self-validate, and I bypass the auto_register
			self.panel.elements[butIdx].registerCallback("release", self.setValueAndCollapse) # on a click & release
			self.panel.elements[butIdx].cornermask = 0
			self.panel.elements[butIdx].shadowed = False
			self.panel.elements[butIdx].textlocation = 1        
			self.panel.elements[butIdx].outlined = False
			# button colors are set to the colors of the menu
			self.panel.elements[butIdx].hoverColor = self.baseButton.hoverColor 
			self.panel.elements[butIdx].normalColor = self.baseButton.normalColor
			self.panel.elements[butIdx].textColor = self.baseButton.textColor
			self.panel.elements[butIdx].textHiliteColor = self.baseButton.textHiliteColor
			self.panel.elements[butIdx].menu_parent = self	
		
		if not noButton:
			self.customValue = TextField(0, 0, self.baseButton.width, self.baseButton.height, "Custom", " ", self, True)
			self.customValue.registerCallback("update", self.customUpdated)
			self.customValue.menu_parent = self
		
			self.customValue.isVisible = False
			
		self.registerCallback("validate", self.calculateMenuSizes)
		#self.registerCallback("release", self.release)
		self.calculateMenuSizes()
		self.value = self.panel.elements[0].name
		self.noSelect = noSelect
		self.custom = custom
		self.customPrompt = customPrompt
		self.valueIsCustom = False
		self.custom_func = customFunc
	def hasValue(self, value):
		if value in self.menu:
			return self.menu.index(value)
	def customUpdated(self, obj):
		if self.custom_func != None:
			self.custom_func(self.customValue)
		
	def key_event(self, key, val):
		if self.customValue.isVisible:
			self.customValue.key_event(key, val)
	def re_init(self, menu):
		# clear the existing element list
		for element in self.panel.elements:
			self.panel.removeElement(element)
		self.panel.elements = []
		self.panel.draw_stack = []
		self.panel.z_stack = []
			
		butIdx = 0
		for a in menu: 
			# create some button objects here
			self.panel.addElement(Button(0, 0, 100, 25, butIdx, a, 'normal', self.panel, False)) # this will self-validate
			self.panel.elements[butIdx].registerCallback("release", self.setValueAndCollapse) # on a click & release
			self.panel.elements[butIdx].cornermask = 0
			self.panel.elements[butIdx].shadowed = False
			self.panel.elements[butIdx].textlocation = 1           
			self.panel.elements[butIdx].outlined = self.outlined
			# button colors are set to the colors of the menu
			self.panel.elements[butIdx].hoverColor = self.baseButton.hoverColor 
			self.panel.elements[butIdx].normalColor = self.baseButton.normalColor
			self.panel.elements[butIdx].textColor = self.baseButton.textColor
			self.panel.elements[butIdx].textHiliteColor = self.baseButton.textHiliteColor
			butIdx = butIdx + 1 # so we know what button was pushed
		self.calculateMenuSizes()
		
	def renameElement(idx, name):
		self.elements[idx].title = name
		if self.getValue() == idx:
			self.baseButton.title = name
				
	def setValueAndCollapse(self, button):
		
		if self.hit_test():
			if not self.noSelect:
				self.value = button.name # the VALUE of the menu object is the selected button index
				self.baseButton.title = button.title # the display string is the button title that was clicked.
				if self.custom:
					if button.title == self.customPrompt:
						# display the text field
						self.baseButton.isVisible = False
						self.customValue.isVisible = True
						self.valueIsCustom = True
					else:
						if self.baseButton.isVisible == False:
							self.baseButton.isVisible = True
							self.customValue.isVisible = False
							self.valueIsCustom = False
		print "here!"
		self.expanded = False
		self.close_funcs()
		self.select_funcs()
			# print "Return menu value ", self.value, ", and button title ", button.title
	def setShadowed(self, value):
		self.shadowed = value
		self.baseButton.shadowed = value
		self.arrowButton.shadowed = value
		
	def close_funcs(self):
		for func in self.close_functions:
			func(self)
		
	def calculateMenuSizes(self):
		# get the screen height first
		#self.debug = True
		size = self.get_correct_space_size()			
		self.baseButton.invalid = True
		if self.enableArrowButton:
			self.arrowButton.invalid = True
		self.panel.invalid = True
		panel = self.panel
		butheight = panel.elements[0].height
		heightlimit = size[1] - (size[1] * .30) # get the screen height - 25% of the screen height
		if self.debug:
			print "Screen size is ", size
			print "Button height is ", butheight
			print "Panel Height limit is ", heightlimit
		# get the # of buttons and multiply by button height
		buts = len(panel.elements) # fix this by making the panel stand alone (or rely upon UIelement
		for a in range(1, 10):
			if (buts / a) * butheight < heightlimit:				
				columns = a				
				break
		
		# now that I have my column count
		width = self.find_widest_value()	
		# button widths will initally be width
		panelWidth = width * columns
		titleWidth = self.get_string_width(panel.title, panel.fontsize)
		if panelWidth < titleWidth:
			panelWidth = titleWidth + 10   
			width = panelWidth
		else:
			width = width + 20
			panelWidth = width * columns
		
		# so how many buttons per column?
		columnButs = buts / columns
		if self.debug:
			print "Calculated ", columns, " columns"
			print "Using ", width, " button width"
			print "Calculated ", columnButs, " buttons per column"
		if self.enableMenuHeader:
			panelHeight = (columnButs * butheight) + columnButs +  27 # add the number of buttons per column, the 1 pixel gap per button (columnButs) and the header height
		else:
			panelHeight = (columnButs * butheight) + columnButs 
			
		panel.height = panelHeight # the height of the panel
		idx = 1
		column = 0
		
		y_offset = butheight # offset for the panel title
		for x in panel.elements:			
			x.width = width # 20 pixels to the right of the text it will display
			x.height = 25 # 25 pixels, for a normal font
			# that's got the width and height, now to place the buttons
			 # find out which column I'm in
			x.x = column * x.width # this should place the button horizontally
			# the y value should start out being subtracted from the y value of the container panel
			# so
			x.y = y_offset  # use the panel's y value as a starting point 
			if (idx < columnButs):
				y_offset = y_offset + butheight + 1 # give a 1 pixel gap
			else:
				y_offset = butheight
				idx = 0
				# move to the next column
				column = column + 1
			idx = idx + 1
				
		# size the panel that contains the buttons
		panel.width = panelWidth
		# and done. Display positioning is taken care of in validate()
			
	def draw(self):		
		self.validate()	
		
		if self.expanded == True: 
			# test to see if the panel is off the screen. 
			self.panel.draw() # the buttons should actually arrange themselves			
		elif not self.noButton:
			if self.hit_test():						
				self.baseButton.normalColor = self.hoverColor
				self.baseButton.textColor = self.textHiliteColor				
			else:			
				self.baseButton.normalColor = self.normalColor
				self.baseButton.textColor = self.textColor
				
			if self.baseButton.isVisible:
				self.baseButton.draw()
			if self.custom:
				if self.customValue.isVisible:
					self.customValue.validate()
					self.customValue.draw()
			if self.enableArrowButton:
				self.arrowButton.draw()
			
			if (self.hit_test()):
				self.setColor(self.baseButton.textHiliteColor) # use the text color of course
			else:
				self.setColor(self.baseButton.textColor) 
			
			if self.enableArrowButton:
					
				Blender.BGL.glBegin(Blender.BGL.GL_POLYGON)
				Blender.BGL.glVertex2i(self.absX + (self.width - 10), self.absY - ((self.height / 2) - 2))
				Blender.BGL.glVertex2i(self.absX + (self.width - 16), self.absY - ((self.height / 2) - 2))
				Blender.BGL.glVertex2i(self.absX + (self.width - 13), self.absY - ((self.height / 2) -6))
				Blender.BGL.glEnd()
				
				Blender.BGL.glBegin(Blender.BGL.GL_POLYGON)
				Blender.BGL.glVertex2i(self.absX + (self.width - 10), self.absY - ((self.height / 2) + 2))
				Blender.BGL.glVertex2i(self.absX + (self.width - 16), self.absY - ((self.height / 2) + 2))
				Blender.BGL.glVertex2i(self.absX + (self.width - 13), self.absY - ((self.height / 2) + 6))
				Blender.BGL.glEnd()
						
	def find_widest_value(self):
		w = 0
		for x in self.panel.elements:
			y = self.get_string_width(x.title, 'normal') # menu objects use normal text
			if y > w:
				w = y		
		return w
	
	def showPanel(self):
		self.expanded = True
		self.elements = [self.panel]
		self.z_stack = [self.panel]
		self.draw_stack = [self.panel]
	def hidePanel(self):
		self.expanded = False
		self.elements = []
		self.z_stack = []
		self.draw_stack = []
		
	def release(self, obj):		
		# click magic	
		if self.hit_test() and self.expanded == False:
			#self.baseButton.pushed = False
			#self.arrowButton.pushed = False
			self.expanded = True # expand ye olde menu 			
			self.elements = [self.panel]
			self.z_stack = [self.panel]
			self.draw_stack = [self.panel]
		elif self.panel.hit_test() and self.expanded: # process internal elements
			for element in self.panel.elements:
				element.release_event()			
			self.expanded = False
			# print "Got a release event!"
			if self.enableArrowButton:
				self.elements = [self.baseButton, self.arrowButton]
				self.z_stack = [self.baseButton, self.arrowButton]
				self.draw_stack = [self.baseButton, self.arrowButton]			

			else:
				self.elements = [self.baseButton]
				self.z_stack = [self.baseButton]
				self.draw_stack = [self.baseButton]			
		elif not self.noButton:
			# close the menu			
			self.expanded = False
			if self.enableArrowButton:
				self.elements = [self.baseButton, self.arrowButton]
				self.z_stack = [self.baseButton, self.arrowButton]
				self.draw_stack = [self.baseButton, self.arrowButton]
			else:
				self.elements = [self.baseButton]
				self.z_stack = [self.baseButton]
				self.draw_stack = [self.baseButton]			
				

		
	def validate(self): # the base class validate get overriden here because the panel element of this control has to set the bounds here		
		if self.expanded: # I want to set a different set of bounds in the parent here	
			self.panel.invalid = False
			screen = Blender.Window.GetAreaSize()	
			
			if (self.panel.absX + self.panel.width) > screen[0]:				
				self.panel.absX = screen[0] - (self.panel.width + 15) # horizontal position 15 pixels from the right of the screen
			elif self.panel.absX < 0: # 15 pixels from the left
				self.panel.absX = 15

		UIElement.validate(self)


	def release_event(self):
		if self.expanded and self.hit_test():
			self.panel.release_event()
			self.release_funcs()
		elif self.expanded and self.hit_test() == False:
			self.expanded = False			
		elif not self.noButton:
			if self.baseButton.isVisible:
				self.baseButton.release_event()
			if self.custom:
				if self.customValue.isVisible:
					self.customValue.release_event()
			if self.enableArrowButton:
				self.arrowButton.release_event()

	def click_event(self):
		if self.hit_test() and self.expanded:			
			self.panel.click_event() # pass the event on to the panel so it can dispatch to its buttons
		elif self.hit_test() and self.expanded == False:
			if self.baseButton.isVisible:
				self.baseButton.click_event()
			if self.custom:
				self.customValue.click_event()
			if self.enableArrowButton:
				self.arrowButton.click_event()
		elif self.expanded:
			self.expanded = False
			
	def hit_test(self):
		rVal = False
		if self.expanded:
			#print "have a panel"
			rVal = self.panel.hit_test()			
		elif not self.noButton:
			if self.enableArrowButton:
				# print "Have arrow, will travel"
				if self.baseButton.hit_test() or self.arrowButton.hit_test():
					rVal = True				
			else:
				rVal = self.baseButton.hit_test()
		else:
			rVal = False
		return rVal
		

	def getSelectedIndex(self):					
		return self.value
		
	def removeMenuItem(self, index):
		button = self.panel.elements[index]
		self.panel.removeElement(button)
		
	def getValue(self):
		if self.valueIsCustom:
			return self.customValue.getValue()
		else:
			return self.baseButton.title
	
	def setValue(self, idx):
		if self.custom:
			self.customValue.isVisible = False
			self.baseButton.isVisible = True
			
		self.baseButton.title = self.panel.elements[idx].title
		self.value = idx
			
	def setValueString(self, value):
		found = False
		idx = 0
		for element in self.panel.elements:
			if element.title == value:
				self.value = idx
				found = True
		if found:
			self.baseButton.title = value
		else:
			self.customValue.isVisible = True
			self.customValue.setValue(value)
			self.baseButton.isVisible = False
		
	def registerCallbackForElementIndex(self, idx, signal, callback):
		self.panel.elements[idx].registerCallback(signal, callback)
	def getValueIndex(self, value):
		if value in self.menu:
			return self.menu.index(value)
		else:
			return -1
		
class MenuBar(Panel):
	x_offset = 15 # 15 pixel offsetx
	y_offset = 5
	def __init__(self, parent, auto_register):
		# this is a menu bar, the top is dynamic, will be a standard height, and should always be drawable
		Panel.__init__(self, 0, 0, 0, 0, "none", "", parent, auto_register)
		self.cornermask = 0
		# that should be that.
		self.invalid = True
		self.shadowed = False
		self.hasHeader = False
	#	self.layout()
		self.validate()
		
	#def layout(self): # layout the contained elements
	#	x = self.x_offset
	#	y = self.y_offset
	#	for element in self.elements:
	#		element.x = x + element.width
	#		element.y = y
	#		element.normalColor = [198, 197, 203, 255]
	#		x = x + self.offset 
			
		# annnnnd done.
	# draw should always figure out the space size
	
	def validate(self): # I'm always invalid, so never set that flag.
		screen = Blender.Window.GetAreaSize()	
		self.absX = 0
		self.absY = screen[1] - 2
		self.height = 25
		
		if self.width != screen[0]: # but I can control this!			
	#		self.layout() # just in case.
			self.width = screen[0] - 1
			for element in self.elements:
				element.normalColor = self.normalColor
				element.validate()
			
		
		
		
	
class TextField(Button):   
	normalColor = [255, 255, 255, 255]
	hoverColor = [190, 110, 110, 255]
	textColor = [0, 0, 0, 255]
	textHiliteColor = [0, 0, 0, 255]
	borderColor = [128, 128, 128, 255]
	editingColor = [255, 255, 255, 255]
	editingBorderColor = [180, 128, 128, 255]
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"designBorderColor", "hoverColor", "outlineColor", 
				"editingColor", "editingBorderColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	def __init__(self, x, y, width, height, name, value, parent, auto_register, fontsize = 'normal', undo = False):
		UIElement.__init__(self, x, y, width, height, name, value, parent, auto_register, fontsize = fontsize)
		self.pre_update_functions = []
		self.undo = undo
		self.setupUndo()
		self.undo_history = [value]

		self.isUndo = False
		self.fontsize = fontsize
		self.cursorIndex = 0
		self.editing = False
		self.value = value
		if isinstance(value, float):
			self.strValue = "%.3f" % value
			self.type = "float"
		elif isinstance(value, int):
			self.type = "int"
			self.strValue = "%d" % value
		else:
			self.type = "string"
			self.strValue = value
		self.selection = [0,0]
		self.selectionColor = [200, 128, 128, 255]
		self.index = 0		
		self.isEditing = False
		self.shifted = False
		self.mouseDown = False  		
		self.strict = 1 # strict enforces the type that was assigned by the type of the intial value of the field
		# hence, floats will restrict to floats, same goes for ints and strings
		self.registerCallback("click", self.stat)		
		self.validate_functions = []
		#self.undoMenu = Menu(0, 0, 60, 0, "Local Undo", ["No undo history"], parent, False)
		self.registerCallback("release", self.hideUndo)
		
	def draw(self):

		self.validate()    
		# draw the background box
		if self.isEditing:
			self.setColor(self.editingColor)
		else:
			self.setColor(self.normalColor)
			
		self.drawBox(self.absX, self.absY, self.absX + self.width, self.absY - self.height)
		
		# draw the border
		if self.isEditing:
			self.setColor(self.editingBorderColor)
		else:
			self.setColor(self.borderColor)
						
		self.drawline(self.absX - 1, self.absY, self.absX + self.width + 1, self.absY) # top of box
		self.drawline(self.absX - 1, self.absY, self.absX - 1, self.absY - self.height) # left hand side of box
		self.drawline(self.absX - 1, self.absY - (self.height), self.absX + self.width + 1, self.absY - (self.height)) # bottom of box
		self.drawline(self.absX + self.width, self.absY, self.absX + self.width, self.absY - self.height) # right hand side of box
		
		# there might be a cursor or selection, draw that, then draw the text 
		# over it. each character gets drawn individually        
		# determine what text is to be drawn
		if self.strValue != "" or self.strValue != None:
			displayValue = self.getVisibleText()
			displayWidth = self.get_string_width(displayValue, 'normal') 
			
			if self.isEditing:               
				if self.selection[0] < self.selection[1]:
					# the selection box is drawn
					self.drawSelectionMask(displayValue, displayWidth)
				else:
					displayValue = self.getVisibleText()
					if self.cursorIndex > (len(displayValue) + self.index):
						# the cursor is currently not visible, move the text
						# subtract len(displayValue) from self.cursorIndex to get the right index
						# required
						self.index = self.cursorIndex - len(displayValue)
					elif self.cursorIndex < self.index:
						self.index = self.cursorIndex
						
					# the cursor box is drawn
					self.drawCursor()
					
			# and now draw the text
			if self.isEditing:
				self.setColor(self.textHiliteColor)
			else:
				self.setColor(self.textColor)
			# now where do I put the text? Decide that based on the value in question
			if self.type == "float" or self.type == "int": # numeric value, shift to the right
				# this should equal x + (width - stringwidth)
				self.setRasterPos(self.absX + (self.width - displayWidth), self.absY - (self.height - 4))
			else:
				self.setRasterPos(self.absX + 2, self.absY - (self.height - 4 ))
			Blender.Draw.Text(displayValue) 
			# draw anything else that needs drawing
		
			for element in self.elements:
				element.draw()
	
	def setupUndo(self, maxLevels = 6):
		self.undo_history = ["No undo history!"]
		self.undo_menu = Menu(0, 0, 0, 0, "Undo", self.undo_history, self, True, noButton = True) 
		# now I get to do something tricky to the menu component so that it's panel can be displayed without a button push
		if self.undo: # otherwise, do nothing
			self.undo_menu.registerCallback("select", self.updateWithUndo)
			self.undo_menu.registerCallback("release", self.showUndo)
			self.registerCallback("pre_update", self.addUndo)		
			self.registerCallback("right_click", self.showUndo)

		self.maxLevels = maxLevels
		# register a callback to keep track of undos
		self.undoUpdate = False
		
	def showUndo(self, obj):
		if self.undo_menu.expanded:
			self.undo_menu.hidePanel()
			self.isUndo = False
		else:
			self.isUndo = True
			self.undo_menu.showPanel()
	def hideUndo(self, obj):
		self.undo_menu.hidePanel()
		
	def updateWithUndo(self):		
		if self.isEditing:
			self.isEditing = False
		
		val = self.undo_stack.pop(self.undo_menu.getSelectedIndex())
		self.setValue(val)
		self.undo_menu.hidePanel()
		self.isUndo = False
		
	def addUndo(self, value):	
		
		if value.getValue() not in self.undo_history:			
			if self.undo_history[0] == "No undo history!":
				self.undo_history = []
			if len(self.undo_history) == self.maxLevels:
				self.undo_history.remove(self.maxLevels - 1)
			self.undo_history.insert(0, self.getValue())
			self.undo_menu.re_init(self.undo_history)	
	
	def drawCursor(self):
		# draw a cursor at the current cursorindex
		# iterate the text values to find out where it's at
		# to do this, I only need to get the text from the index to 
		# the cursor index   
		
		# first problem - which side am I justifying from?
		if self.type == "float" or self.type == "int":
			if len(self.strValue) < 2:
				s_width = self.get_string_width(self.strValue, 'normal')
			else:             
				s_width = self.get_string_width(self.strValue[self.cursorIndex:], 'normal')                            
			cursorPos = self.absX + (self.width - s_width) 
		else:
			if len(self.strValue) < 2:
				s_width = self.get_string_width(self.strValue, 'normal')
			else:
				s_width = self.get_string_width(self.strValue[self.index:self.cursorIndex], 'normal')        
			cursorPos = self.absX + 2 + s_width
				
		# now, draw a nice box in the cursor (selection) color at 
		# cursorPos, self.absY - 2, cursorPos + 2, self.absY - (self.height - 2)
		self.setColor(self.selectionColor)
		# basic OpenGL call
		Blender.BGL.glRecti(cursorPos, self.absY - 2, cursorPos + 4, self.absY - (self.height))
		# done
		
	
		

	def drawSelectionMask(self, displayValue, displayWidth):
		index = self.index
		self = self
		sel_start = 0
		sel_end = len(self.strValue) - 1
		# deal with selected text here. Problems with this include cases where 
		# the selection may not all be visible
		if self.selection[0] > (index + len(displayValue)) or self.selection[1] < self.index:
			# do nothing since the selection isn't visible. I should probably invalidate it here
			pass
		if self.selection[0] >= index and self.selection[0] <= (index + len(displayValue)):
			# snip off the text that fits between index and selection[0] and measure it
			sel_start = self.get_string_width(self.strValue[index:self.selection[0]], 'normal') 
		elif self.index > self.selection[0]: # selection begins outside of visible area                 
			sel_start = 0
		if self.selection[1] > index and self.selection < index + len(displayValue):
			# the selection end location calc depends upon the start
			if self.selection[0] >= index and self.selection[0] <= (index + len(displayValue)):
				# sel_start is visible, thus I measure from sel_start to the end
				sel_end = self.get_string_width(self.strValue[self.selection[0]:self.selection[1]], 'normal')
			else: 
				sel_end = self.get_string_width(self.strValue[index:self.selection[1]], 'normal')
		elif self.selection[1] > index + len(displayValue): # selection ends beyond visible area
			sel_end = displayWidth 
		self.setColor(self.selectionColor)
		
		# so all of that was to get this
		self.drawBox(self.absX + sel_start + 2, self.absY - (self.height), self.absX + sel_end, self.absY - (self.height - self.font_cell_sizes['normal'][1]))
		
	def getVisibleText(self):
		t_width = self.get_string_width(self.strValue, 'normal') # text strings are all normal
		if t_width > self.width: # the text width is larger than the width of the control
			# start at index and try to find a fit in the window
			range = self.strValue[self.index:] # slice off a piece to work with
			s_width = 0
			testValue = ''
			displayValue = ''
			for x in range: 
				testValue = testValue + x
				s_width = self.get_string_width(testValue, 'normal')
				if s_width > self.width:
					# stop here
					break
				else:
					displayValue = displayValue + x 
		else:
			displayValue = self.strValue
		return displayValue
		
	

	def getNearestCharIndex(self):
		sText = self.getVisibleText()
		# figure out the x/y coords for each character
		x = self.area_mouse_coords()[0]
		# adjust t_offset to be validated with the 
		# actual offset of the control itself
		# text starts 2 pixels in from self.absX, so
		if self.type == "float" or self.type == "int":
			t_offset = self.absX + (self.width - 2) # starting point 
			idx = len(sText) - 1
			while x < t_offset and idx >= 0:
				# for each character, get its x position on the screen by calling get_string_width()
				# and then subtracting that cumulatively from the offset until the offset is less than the
				# the mouse coords x value  
				t_offset = t_offset - self.get_string_width(sText[idx], 'normal')                
				idx = idx - 1 # decrement to get the next char
			rVal = idx + 1
		else:
			t_offset = self.absX + 2 # this is the starting point
			idx = 0
			while x > t_offset and idx < len(sText):            
				# for each character, get its x position on the screen by calling get_string_width()
				# and then adding that cumulatively to the offset until the offset exceeds the
				# the mouse coords x value            
				t_offset = t_offset + self.get_string_width(sText[idx], 'normal')
				idx = idx + 1
			rVal = idx - 1 
		
		return rVal 
	

	def getValue(self):
		return self.strValue

	def setValue(self, value):		
		# if the value is either a float or int, then I know I can format them immediately
		if isinstance(value, float):
			# int types should be promoted automatically like
			self.strValue = "%.3f" % value			
		elif isinstance(value, int):		
			# floats should automatically be stripped down to ints
			self.strValue = "%d" % value
		else:	
			# if the value is a string, but the validation type is int or float, then...
			if self.type == "float":
				# try to initialize the value. If this fails, pop-up an error message
				try:
					test = float(value)
					self.strValue = "%.3f" % value
				except ValueError:
					# something went terribly wrong.
					# pop-up a dialog or something
					for func in self.validate_functions:
						func(self)
					
			elif self.type == "int":				
				try:
					test = int(value)
					self.strValue = "%d" % value
				except ValueError:
					for func in self.validate_functions:
						func(self)						
			else:
				# normal strings initialized as such are ok
				self.strValue = value
								
			
	def delete_selection(self):
		deletedwidth = self.selection[1] - self.selection[0]
		endidx = len(self.strValue) - 1
		if self.selection[1] > 0:
			if self.selection[0] == 0 and self.selection[1] == endidx: # delete the whole string
				self.strValue = ""
			elif self.selection[0] == 0:
				self.strValue = self.strValue[self.selection[1]:]
			elif self.selection[1] == endidx:
				self.strValue = self.strValue[self.selection[0]:]
			else:
				self.strValue = self.strValue[:self.selection[0]] + self.strValue[self.selection[1]:]
			# that should be that. the draw method will handle things from here
			# the selection should be reset by the mouse event
			# recreate the display value somehow, or maybe intercept before I get this far?
			
		else: # delete an indvidual character
			if self.cursor == 0:
				self.strValue = self.strVvalue[1:]
			elif self.cursor == endidx:
				self.strValue = self.strValue[:endidx]
		
	
	def insert_char(self, char):
		if isinstance(char, float) or isinstance(char, int):
			char = str(char)
			
		endidx = len(self.strValue) - 1
		# if a selection exists, replace the text with this text
			
		if self.selection[1] > self.selection[0]:
			strValue = self.strValue[0:self.selection[0]] + char + self.strValue[self.selection[1]:endidx]
		else:
			# insert the character where the cursor is
			if self.cursorIndex == endidx: # cursor is at the end of the string
				strValue = self.strValue + char
			elif self.cursorIndex == 0: # cursor at the beginning of the string
				strValue = char + self.strValue
			else: # somewhere in the middle
				strValue = self.strValue[:self.cursorIndex] + char + self.strValue[self.cursorIndex:]
		
		#try to convert the string value back to a float or int value. If that fails, then I know that the value isn't correct.
		if self.type == "int":			
			try:
				test = int(strValue) 
				self.strValue = strValue
				self.cursorIndex = self.cursorIndex + 1
			except ValueError:
				for func in self.validate_functions:
					func(self)				
		elif self.type == "float":
			try:
				test = float(strValue)
				self.strValue = strValue
				self.cursorIndex = self.cursorIndex + 1
			except ValueError:
				for func in self.validate_functions:
					func(self)
		else:
			# just add the string in, no problem
			self.strValue = strValue			
			self.cursorIndex = self.cursorIndex + 1


		
	def delete_char(self):
		# delete the character at the current index
		self.strValue = self.strValue[:self.cursorIndex - 1] + self.strValue[self.cursorIndex:]
		self.cursorIndex = self.cursorIndex - 1
		self.value = self.strValue
		
	
	def validateFormat(self):
		pass

	def mouse_event(self): # drag magic
		
		if self.mouseDown and 4 == 5:
			idx = self.getNearestCharIndex() + self.index # the absolute index of the nearest character
			# where's the mouse?
			if self.selection[0] < self.selection[1]: 
				# there's a selection
				# .....index..s..idx...s
				# get the absolute index of the character
				# char is the index in the visible string
				# so to get that, I have to add index to char
				if idx < self.selection[1] and idx > self.selection[0]:
					self.selection[1] = idx
				elif idx > self.selection[1]:
					self.selection[1] = idx
				elif idx < self.selection[0]:
					self.selection[0] = idx
		elif 4 == 5:
			# locate the cursor
			self.cursorIndex = self.getNearestCharIndex()
			
		self.mouse_funcs() # on this control, there should be no mouse funcs except for debug
		
	def click_event(self): # 
		if self.isUndo and not self.undo_menu.expanded:
			self.isUndo = False
			
		if self.isUndo :	
			if self.undo_menu.expanded:
				UIElement.click_event(self)
	
		else:
			# on a click down, I should clear the selection and reset to wherever I'm at    		
			self.mouseDown = True
			# possible states - I'm editing already
			# clear the selection
			if self.isEditing:
				self.selection = [0, 0]
				# get the mouse position in relation to displayed text, and move the cursor to that location
				self.cursorIndex = self.getNearestCharIndex()
			else:
				for func in self.pre_update_functions:
					func(self)
				self.isEditing = True # invoke editing
				# select everything in the field	
				self.cursorIndex = self.getNearestCharIndex()
			self.click_funcs()
		

	
	def release_event(self): # override the default release event processing
		if self.hit_test() != True: # update with the latest value
			self.mouseDown = False
			if self.isEditing:
				self.isEditing = False			
				self.update_funcs()
		else:
			pass
		self.release_funcs()    
			
		
		
	def key_event(self, key, val):
		if self.isEditing:
			# this will be a huge event handler, since it has to handle every key possible.
			# perhaps a lookup table?
			# in fact, that's the best way
			lowercase = "abcdefghijklmnopqrstuvwxyz"
			uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			special = "!@#$%^&*()"
			# numbers = "0123456789" unneccessary
			# shift key
			
			if (key == Blender.Draw.RIGHTSHIFTKEY or key == Blender.Draw.LEFTSHIFTKEY) and val == 0: # released
				self.shifted = False
			elif (key == Blender.Draw.RIGHTSHIFTKEY or key == Blender.Draw.LEFTSHIFTKEY) and val == 1: # pressed
				self.shifted = True
				
			# special characters have to be handled a totally different way
			# test on alphas first			
				
			if key == Blender.Draw.LEFTARROWKEY and val == 1:
				if self.cursorIndex > 0:
					self.cursorIndex = self.cursorIndex - 1
					
			elif key == Blender.Draw.RIGHTARROWKEY and val == 1:
				if self.cursorIndex < len(self.strValue):
					self.cursorIndex = self.cursorIndex + 1
					
			elif key == Blender.Draw.BACKSPACEKEY and val == 1:
				if self.cursorIndex == 0:
					pass
				else:
					self.delete_char()
				
			elif key == Blender.Draw.DELKEY and val == 1:
				if self.cursorIndex < len(self.strValue):
					self.cursorIndex = self.cursorIndex + 1
					self.delete_char()
					
			elif key == Blender.Draw.RETKEY and val == 0:
				if self.isEditing == True:
					self.isEditing = False
					self.update_funcs()
			
			elif key == Blender.Draw.HOMEKEY and val == 0:
				self.cursorIndex = 0
				
			elif key == Blender.Draw.ENDKEY and val == 0:
				self.cursorIndex = len(self.strValue) - 1
			
			if val == 0:
				# from here, only 0 values get passed
				if (key > 96 and key < 123) and val == 0:
					# alpha character
					if self.shifted:
						char = uppercase[key - 97] 
					else:
						char = lowercase[key - 97]
					self.insert_char(char)
				elif key == Blender.Draw.BACKSLASHKEY and self.shifted == False:
					self.insert_char("\\")
				elif key == Blender.Draw.BACKSLASHKEY and self.shifted == True:
					self.insert_char("|")
				elif key == Blender.Draw.ACCENTGRAVEKEY and self.shifted == False:            
					self.insert_char("`")
				elif key == Blender.Draw.ACCENTGRAVEKEY and self.shifted == True:
					self.insert_char("~")
				elif key == Blender.Draw.COMMAKEY and self.shifted == False:
					self.insert_char(",")
				elif key == Blender.Draw.COMMAKEY and self.shifted == True:
					self.insert_char("<")
				elif key == Blender.Draw.EQUALKEY and self.shifted == False:            
					self.insert_char("=")
				elif (key == Blender.Draw.EQUALKEY and self.shifted == True) or key == Blender.Draw.PADPLUSKEY:
					self.insert_char("+")
				elif (key == Blender.Draw.MINUSKEY and self.shifted == False) or key == Blender.Draw.PADMINUS:
					self.insert_char("-")
				elif key == Blender.Draw.MINUSKEY and self.shifted == True:
					self.insert_char("_")
				elif (key == Blender.Draw.PERIODKEY and self.shifted == False) or key == Blender.Draw.PADPERIOD:
					self.insert_char(".")
				elif key == Blender.Draw.QUOTEKEY and self.shifted == False:
					self.insert_char("'")
				elif key == Blender.Draw.QUOTEKEY and self.shifted == True:
					self.insert_char('"')
				elif key == Blender.Draw.SLASHKEY and self.shifted == False:
					self.insert_char("/")
				elif key == Blender.Draw.SLASHKEY and self.shifted == True:
					self.insert_char("?")
				elif key == Blender.Draw.SEMICOLONKEY and self.shifted == False:
					self.insert_char(";")
				elif key == Blender.Draw.SEMICOLONKEY and self.shifted == True:
					self.insert_char(":")
				elif key == Blender.Draw.LEFTBRACKETKEY and val == 1:
					self.insert_char("[")
				elif key == Blender.Draw.RIGHTBRACKETKEY and val == 1:
					self.insert_char("]")
				elif (key > 47 and key < 58) and val == 0 and self.shifted == False:
					char = "%d" % (key - 48)
					self.insert_char(char)
				elif (key > 149 and key < 160) :
					char = "%d" % (key - 150)
					self.insert_char(char)
				elif (key > 47 and key < 58) and val == 0 and self.shifted == True:
					char = special[key - 49]
					self.insert_char(char)
				elif key == Blender.Draw.SPACEKEY and val == 0:
					self.insert_char(" ")
			self.key_funcs(key, val) # probably won't be any but hey, you never know
			# and wouldn't you imagine that, I was wrong, there IS a need for this!
				
				
class TextBox(UIElement):
	normalColor = [255, 255, 255, 255]
	hoverColor = [190, 110, 110, 255]
	textColor = [0, 0, 0, 255]
	textHiliteColor = [0, 0, 0, 255]
	borderColor = [128, 128, 128, 255]
	editingColor = [255, 255, 255, 255]
	editingBorderColor = [180, 128, 128, 255]
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"designBorderColor", "hoverColor", "outlineColor", 
				"editingColor", "editingBorderColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	def __init__(self, x, y, width, height, name, value, parent, auto_register, fontsize = 'normal'):
		self.fontTable = self.buildFontTable(fontsize)		
		UIElement.__init__(self, x, y, width, height, name, value, parent, auto_register, fontsize = fontsize)
		Undoable.__init__(self, self)
		self.fontsize = fontsize
		
		self.cursorIndex = 0
		self.editing = False
		self.value = value
		if isinstance(value, float):
			self.strValue = "%.3f" % value
			self.type = "float"
		elif isinstance(value, int):
			self.type = "int"
			self.strValue = "%d" % value
		else:
			self.type = "string"
			self.strValue = value
		self.value = self.value.replace("\t", "    ")
		self.selection = [0,0]
		self.selectionColor = [200, 128, 128, 255]
		self.index = 0		
		self.isEditing = False
		self.shifted = False
		self.mouseDown = False  		
		self.strict = 1 # strict enforces the type that was assigned by the type of the intial value of the field
		# hence, floats will restrict to floats, same goes for ints and strings
		self.registerCallback("click", self.stat)		
		self.validate_functions = []
		#self.undoMenu = Menu(0, 0, 60, 0, "Local Undo", ["No undo history"], parent, False)
		self.registerCallback("right_click", self.showUndo)
		# Ok this works on the concept of lines
		self.Enabled = True
		self.lineIndex = 0
		self.mouse_x = 0
		self.mouse_y = 0
		self.clicked = False
		self.lines = 0
		

		
	def draw(self):		
		# figure out lines
		self.validate()
		
		# component background
		self.setColor(self.normalColor)
		self.drawBox(self.absX, self.absY, self.absX + self.width, self.absY - self.height)
			
		# component border
		self.setColor(self.borderColor)
		self.drawline(self.absX - 1, self.absY, self.absX + self.width + 1, self.absY) # top of box
		self.drawline(self.absX - 1, self.absY, self.absX - 1, self.absY - self.height) # left hand side of box
		self.drawline(self.absX - 1, self.absY - (self.height), self.absX + self.width + 1, self.absY - (self.height)) # bottom of box
		self.drawline(self.absX + self.width, self.absY, self.absX + self.width, self.absY - self.height) # right hand side of box
		
		self.drawText()

			
	def drawText(self):
		# text height/width
		lineHeight = int(math.ceil(self.font_cell_sizes[self.fontsize][1] + 4))
		maxLines = self.height / lineHeight  		
		
		# draw each character
		
		val = self.value
		idx = 0
		sIdx = 0
		y_offset = self.absY - 2	
		x_offset = self.absX + 2
		charIndex = 0		
		lineIndex = 0		
		self.lines = 1
		for char in val:
			self.setRasterPos(x_offset, y_offset - lineHeight + 2)
			if char == "\n":
				x_offset = self.absX + 2 # line down
				y_offset = y_offset - lineHeight
				lineIndex = lineIndex + 1
				self.lines = self.lines + 1
			else:
				if not self.clicked and self.cursorIndex == charIndex:
					self.drawCursor(x_offset, y_offset)
					
				if lineIndex >= self.lineIndex and lineIndex < maxLines:					
					self.setColor(self.textColor)
					width = Blender.Draw.Text(char, self.fontsize) # increment across the control drawing each character, this is openGL shoudl be fast
				else:
					width = Blender.Draw.GetStringWidth(char, self.fontsize)				
				
				# mouse position
				if self.clicked: # the cursor moved, do something about it.
					if self.mouse_y < y_offset and self.mouse_y > y_offset - (lineHeight + 2): # cursor is on this line
						if self.mouse_x >= x_offset and self.mouse_x < x_offset + width:
							# stop checking, I found my location...it's an optmization at least.
							self.clicked = False
							self.cursorIndex = charIndex # damn, that was easy
							
				x_offset = x_offset + width
				
				if x_offset > (self.absX + self.width) - width: # I need some arbitrary width here!! What will it be? Maybe I'll add + self.font_size_table[ord(char)]
					y_offset = y_offset - lineHeight
					x_offset = self.absX + 2
					lineIndex = lineIndex + 1
					
			charIndex = charIndex + 1	

		
	def drawCursor(self, cursorX, y_offset):
		self.setColor([128, 45, 45, 128])
		Blender.BGL.glEnable(gl.GL_BLEND)
		Blender.BGL.glShadeModel(gl.GL_SMOOTH)
		# basic OpenGL call 
		# enable blending here
		Blender.BGL.glRecti(cursorX, y_offset + 2, cursorX + 4, y_offset - (self.font_cell_sizes[self.fontsize][1] + 2))
		# done
		Blender.BGL.glDisable(gl.GL_BLEND)
		Blender.BGL.glShadeModel(gl.GL_FLAT)
		
	def drawSelection(self, start, end, y_offset):
		self.setColor(self.selectionColor)
		Blender.BGL.glRecti(start, y_offset + 2, end, y_offset - self.font_cell_sizes[self.fontsize] + 2)			
	
	def getValue(self):
		return self.value

	def setValue(self, value):		
		self.value = value.replace("\t", "    ")		
		self.invalid = True
			
	def delete_selection(self):
		deletedwidth = self.selection[1] - self.selection[0]
		endidx = len(self.value) - 1
		if self.selection[1] > 0:
			if self.selection[0] == 0 and self.selection[1] == endidx: # delete the whole string
				self.value = ""
			elif self.selection[0] == 0:
				self.value = self.value[self.selection[1]:]
			elif self.selection[1] == endidx:
				self.value = self.value[self.selection[0]:]
			else:
				self.value = self.value[:self.selection[0]] + self.value[self.selection[1]:]
			# that should be that. the draw method will handle things from here
			# the selection should be reset by the mouse event
			# recreate the display value somehow, or maybe intercept before I get this far?
			
		else: # delete an indvidual character
			if self.cursorIndex == 0:
				self.value = self.value[1:]
			elif self.cursorIndex == endidx:
				self.value = self.value[:endidx]
	
	def insert_char(self, char):
		endidx = len(self.value) - 1
		# if a selection exists, replace the text with this text
		if self.selection[1] > self.selection[0]:
			self.value = self.value[0:self.selection[0]] + char + self.value[self.selection[1]:endidx]
		else:
			# insert the character where the cursor is
			if self.cursorIndex == endidx: # cursor is at the end of the string
				self.value = self.value + char
			elif self.cursorIndex == 0: # cursor at the beginning of the string
				self.value = char + self.value
			else: # somewhere in the middle
				self.value = self.value[:self.cursorIndex] + char + self.value[self.cursorIndex:]		
		self.cursorIndex = self.cursorIndex + 1

	def delete_char(self):
		# delete the character at the current index
		self.value = self.value[:self.cursorIndex - 1] + self.value[self.cursorIndex:]
		self.cursorIndex = self.cursorIndex - 1
		# self.value = self.value
		
	
	def mouse_event(self): # drag magic		
		self.mouse_funcs() # on this control, there should be no mouse funcs except for debug
		
	def release_event(self):
		screen = self.area_mouse_coords()
		self.mouse_x = screen[0]
		self.mouse_y = screen[1]
		self.clicked = True

	def key_event(self, key, val):
		if self.hit_test():
			# this will be a huge event handler, since it has to handle every key possible.
			# perhaps a lookup table?
			# in fact, that's the best way
			lowercase = "abcdefghijklmnopqrstuvwxyz"
			uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			special = "!@#$%^&*()"
			# numbers = "0123456789" unneccessary
			# shift key

			if (key == Blender.Draw.RIGHTSHIFTKEY or key == Blender.Draw.LEFTSHIFTKEY) and val == 0: # released
				self.shifted = False
			elif (key == Blender.Draw.RIGHTSHIFTKEY or key == Blender.Draw.LEFTSHIFTKEY) and val == 1: # pressed
				self.shifted = True
				
			# special characters have to be handled a totally different way
			# test on alphas first			
				
			if key == Blender.Draw.LEFTARROWKEY and val == 1:
				if self.cursorIndex > 0:
					self.cursorIndex = self.cursorIndex - 1
					
			elif key == Blender.Draw.RIGHTARROWKEY and val == 1:				
				if self.cursorIndex < len(self.value):
					self.cursorIndex = self.cursorIndex + 1 # I will track cursorIndex per line, and self.strValue will be the current selected line in multiLine mode
			elif key == Blender.Draw.UPARROWKEY and val == 1:
				if self.lineIndex > 0:
					self.lineIndex = self.lineIndex - 1
			elif key == Blender.Draw.DOWNARROWKEY and val == 1:
				if self.lineIndex < self.lines - 1:
					self.lineIndex = self.lineIndex + 1
					
			elif key == Blender.Draw.BACKSPACEKEY and val == 1:
				if self.cursorIndex == 0:
					pass
				else:
					self.delete_char()
				
			elif key == Blender.Draw.DELKEY and val == 1:
				if self.cursorIndex < len(self.value):
					self.cursorIndex = self.cursorIndex + 1
					self.delete_char()
					
			elif key == Blender.Draw.RETKEY and val == 0:
				self.insert_char("\n")
			
			elif key == Blender.Draw.HOMEKEY and val == 0:
				self.cursorIndex = 0
				
			elif key == Blender.Draw.ENDKEY and val == 0:
				self.cursorIndex = len(self.value) - 1
			
			if val == 0:
				# from here, only 0 values get passed
				if (key > 96 and key < 123) and val == 0:
					# alpha character
					if self.shifted:
						char = uppercase[key - 97] 
					else:
						char = lowercase[key - 97]
					self.insert_char(char)
				elif key == Blender.Draw.BACKSLASHKEY and self.shifted == False:
					self.insert_char("\\")
				elif key == Blender.Draw.BACKSLASHKEY and self.shifted == True:
					self.insert_char("|")
				elif key == Blender.Draw.ACCENTGRAVEKEY and self.shifted == False:            
					self.insert_char("`")
				elif key == Blender.Draw.ACCENTGRAVEKEY and self.shifted == True:
					self.insert_char("~")
				elif key == Blender.Draw.COMMAKEY and self.shifted == False:
					self.insert_char(",")
				elif key == Blender.Draw.COMMAKEY and self.shifted == True:
					self.insert_char("<")
				elif key == Blender.Draw.EQUALKEY and self.shifted == False:            
					self.insert_char("=")
				elif (key == Blender.Draw.EQUALKEY and self.shifted == True) or key == Blender.Draw.PADPLUSKEY:
					self.insert_char("+")
				elif (key == Blender.Draw.MINUSKEY and self.shifted == False) or key == Blender.Draw.PADMINUS:
					self.insert_char("-")
				elif key == Blender.Draw.MINUSKEY and self.shifted == True:
					self.insert_char("_")
				elif (key == Blender.Draw.PERIODKEY and self.shifted == False) or key == Blender.Draw.PADPERIOD:
					self.insert_char(".")
				elif key == Blender.Draw.QUOTEKEY and self.shifted == False:
					self.insert_char("'")
				elif key == Blender.Draw.QUOTEKEY and self.shifted == True:
					self.insert_char('"')
				elif key == Blender.Draw.SLASHKEY and self.shifted == False:
					self.insert_char("/")
				elif key == Blender.Draw.SLASHKEY and self.shifted == True:
					self.insert_char("?")
				elif key == Blender.Draw.SEMICOLONKEY and self.shifted == False:
					self.insert_char(";")
				elif key == Blender.Draw.SEMICOLONKEY and self.shifted == True:
					self.insert_char(":")
				elif key ==Blender.Draw.LEFTBRACKETKEY and self.shifted == False:
					self.insert_char("[")
				elif key ==Blender.Draw.LEFTBRACKETKEY and self.shifted == True:
					self.insert_char("{")
				elif key == Blender.Draw.RIGHTBRACKETKEY and self.shifted == False:
					self.insert_char("]")
				elif key == Blender.Draw.RIGHTBRACKETKEY and self.shifted == True:
					self.insert_char("}")
				#elif key == Blender.Draw.TABKEY:
				#	self.insert_char("    ")
				elif (key > 47 and key < 58) and val == 0 and self.shifted == False:
					char = "%d" % (key - 48)
					self.insert_char(char)
				elif (key > 149 and key < 160) :
					char = "%d" % (key - 150)
					self.insert_char(char)
				elif (key > 47 and key < 58) and val == 0 and self.shifted == True:
					char = special[key - 49]
					self.insert_char(char)
				elif key == Blender.Draw.SPACEKEY and val == 0:
					self.insert_char(" ")
			self.key_funcs(key, val) # probably won't be any but hey, you never know
			# and wouldn't you imagine that, I was wrong, there IS a need for this!
			
class TextItem(UIElement):
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"designBorderColor", "hoverColor", "outlineColor", 
				"editingColor", "editingBorderColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	def __init__(self, x, y, name, value, parent, auto_register, fontsize = 'normal', multiLine = False, ignoreWhitespace = False):
		self.multiLine = multiLine
		self.ignoreWhitespace = ignoreWhitespace
		self.value = value
		self.hasBorder = False
		UIElement.__init__(self, x, y, 0, 0, name, value, parent, auto_register, fontsize = fontsize)        

		if not multiLine:
			valueString = str(value)
			self.fontsize = fontsize
			self.width = self.get_string_width(valueString, self.fontsize)  # do different calcs here, since extra stuff is required
			self.height = self.font_cell_sizes[self.fontsize][1]         
			self.value = value        
		else:
			self.width = parent.width
			self.height = parent.height
			self.lineIndex = 0 # starting out at zero line index\
			self.registerCallback("wheel", self.scroll)
		
		self.valueLines = []			
		

	def draw(self):
		self.validate()
		if self.hasBorder:
			self.setColor(self.borderColor)
			self.uiOutline(self.absX, self.absY, self.absX + self.width, self.absY - self.height, 1, 0)

		if not self.multiLine:
			self.setColor(self.textColor)
			self.setRasterPos(self.absX, self.absY - (self.font_cell_sizes[self.fontsize][1] + 2))
			dispValue = str(self.value)
			Blender.Draw.Text(dispValue, self.fontsize) # sometimes this will be a float or something
		else:
			offset = self.absY - (self.font_cell_sizes[self.fontsize][1] + 2)
			self.setColor(self.textColor)
			for line in self.displayLines:
				self.setRasterPos(self.absX, offset)
				offset = offset - self.font_cell_sizes[self.fontsize][1] - 6
				Blender.Draw.Text(line, self.fontsize)
				
		
	def validate(self):
		UIElement.validate(self)
		self.lineHeight = self.parent.height / (self.font_cell_sizes[self.fontsize][1] + 6) # add a 4 pixel gutter
		pWidth = self.parent.width
		if self.multiLine:
			lines = self.value.split("\n") # splits the lines on newline characters, so I have an arbitrary method of creating lines
			self.valueLines = []
			for line in lines:
				# measure each line to see if it fits into the size of the *parent*.
				# this is important because the text item itself normally draws itself without regard to any width concerns.
				# if not, split the line at the boundary of a given word and create a new line
				
				cWidth = pWidth / self.font_cell_sizes[self.fontsize][0] # this is number of characters that fit on the screen
				if len(line) < cWidth:
					self.valueLines.append(line)
				else:
					if self.ignoreWhitespace:
						# the line is too long to display as is, so split at the word closest to the parent width
						# working with pWidth, I can now grab a substring
						# the width of the parent in *characters* is					
						myLine = line
						while len(myLine) > cWidth:
							subStr = myLine[:cWidth]
							self.valueLines.append(subStr)
							myLine = myLine[cWidth:]
						if len(myLine) > 0:
							self.valueLines.append(myLine)	

					else:
						myLine = line
						while len(myLine) > cWidth:
							subStr = myLine[:cWidth]
							# find the last whitespace
							lIdx = rfind(subStr, " ")
							self.valueLines.append(subStr[:lIdx])
							myLine = myLine[lIdx:]
						if len(myLine) > 0: # only if the line wasn't nibbled up by the above
							self.valueLines.append(myLine)
			
			# now figure out exactly which lines I'm actually going to display			
			if len(self.valueLines) > self.lineHeight:			
				if len(self.valueLines[self.lineIndex:]) > self.lineHeight:
					self.displayLines = self.valueLines[self.lineIndex:self.lineHeight]
				else:
					self.displayLines = self.valueLines[self.lineIndex:]
			else:
				self.displayLines = self.valueLines
				
	def scroll(self, evt):				
		if evt == Blender.Draw.WHEELUPMOUSE:
			if self.lineIndex > 1:
				self.lineIndex = self.lineIndex - 1
		elif evt == Blender.Draw.WHEELDOWNMOUSE:
			if self.lineIndex < len(self.valueLines):
				self.lineIndex = self.lineIndex + 1
		self.invalid = True
				
					
				
	def appendLine(self, value):
		self.value = self.value + "\n" + value
		
		self.validate()
		
	def setValue(self, value):
		self.value = value
		
	def insertLine(self, value):
		self.value = value + "\n" + self.value
		self.validate()
			
				
class Label(UIElement):
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"designBorderColor", "hoverColor", "outlineColor", 
				"radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "name", "title"]
	def __init__(self, x, y, name, value, parent, auto_register, fontsize = 'normal'):
		UIElement.__init__(self, x, y, 0, 0, name, value, parent, auto_register, fontsize = fontsize)		
		valueString = str(value)
		self.fontsize = fontsize
		self.width = self.get_string_width(valueString, self.fontsize) + 2 
		self.height = self.font_cell_sizes[self.fontsize][1] + 2 # voila!
		self.value = value 
		if isinstance(value, float) or isinstance(value, int):
			xT = parent.width - (self.width + 2)
		else:
			xT = 2
			
		self.addElement(TextItem(xT, 0, name, value, self, False, fontsize = fontsize)) # this item only has one element, but can be extended any time		
		self.elements[0].width = self.width
		self.elements[0].height = self.height
		self.elements[0].hasBorder = False
		self.elements[0].normalColor = parent.normalColor
		self.elements[0].textColor = parent.textColor
		self.normalColor = parent.normalColor # labels should use the same background as the parent unless overridden    
		self.textColor = parent.textColor
		
	def draw(self):
		self.normalColor = self.parent.normalColor
		UIElement.draw(self) # use the baseclass and do what's needed
			
	def setText(self, value):
		self.elements[0].value = value
	#def draw(self):        
	#	self.validate()        
	#	UIElement.draw(self)
	def setValue(self, value):
		self.elements[0].value = value
		self.value = value

	def getValue(self):
		return self.value
		
		
class Table(UIElement):
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"designBorderColor", "hoverColor", "outlineColor", 
				"editingColor", "editingBorderColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
	def __init__(self, x, y, width, height, name, value, parent, auto_register):
		UIElement.__init__(self, x, y, width, height, name, name, parent, auto_register)
		self.rows = 4
		self.columns = 4
		columnIdx = 0
		columnX = 0
		columnWidth = self.width / len(value) # width divided by columns = column width
		for column in value:
			self.addElement(TableColumn(columnX, 0, columnWidth, self.height, columnIdx, columnIdx, column, self, False))
			columnX = columnX + columnWidth
			columnIdx = columnIdx + 1
			
	def getValueAt(self, row, column):
		return self.elements[column].elements[row].value
		
	def setValueAt(self, row, column, value):
		self.elements[column].elements[row].setValue(value)
		
class TableColumn(UIElement):
	def __init__(self, x, y, width, height, name, index, cells, parent, auto_register):
		UIElement.__init__(self, x, y, width, height, name, index, parent, auto_register)
		self.value = 0 # only added to support value export    		
		self.ColumnHeader = Label(0, 0, name, name, self, True)
		# initialize each cell
		# cell width is tableWidth / columns
		self.drawLast = None
		cellHeight = self.height / len(cells) 
		cellY = 17
		cellIdx = 0
		for x in cells:
			self.addElement(TableCell(0, cellY, self.width, cellHeight, cellIdx, x, self, False))
			cellY = cellY + cellHeight
			cellIdx = cellIdx + 1
			
	
class TableCell(UIElement):
	normalColor = [255,255,255, 255]
	
	def __init__(self, x, y, width, height, name, value, parent, auto_register):
		UIElement.__init__(self, x, y, width, height, name, value, parent, auto_register)
		self.value = value
		self.editing = False        
		# setup the two components used, a label and an editor 
		self.label = Label(0, 0, name, str(value), self, False)
		self.label.width = width
		self.label.height = height
		self.label.elements[0].hasBorder = True
		self.label.elements[0].width = width
		self.label.elements[0].height = height
		self.label.textColor = [0, 0, 0, 255]
		self.label.elements[0].textColor = [0, 0, 0, 255]
		self.editor = TextField(0, 0, width, height, name, value, self, False)
		self.textColor = [0, 0, 0, 255]
		self.editor.textColor = [0, 0, 0, 255]
		self.addElement(self.label)
		self.editor.invalid = True
		self.editor.validate()
		self.registerCallback("click", self.stat)

	def release_event(self):
		if self.hit_test(): 
			self.editing = True
			self.editor.isEditing = True
			self.elements = []
			self.z_stack = []
			self.draw_stack = []
			self.addElement(self.editor)
			self.editor.invalid = True 
			self.editor.validate()
		else:
			if self.editing:
				for x in self.label.elements:
					x.value = self.editor.strValue
				self.editing = False
				self.elements = []
				self.z_stack = []
				self.draw_stack = []			
				self.addElement(self.label)
		UIElement.release_event(self)	
	
	def setValue(self, value):
		self.value = value
		self.editor.setValue(value)
					
class ColorPicker(Panel):
	# color picker note...color pickers are drawn in the middle of the screen!!!
	normalColor = UIElement.normalColor
	radius = 1.5
	cornermask = 0
	colors = [[0, 0, 0, 255], 
		[35, 35, 35, 255], 
		[71, 71, 71, 255], 
		[107, 107, 107, 255], 
		[142, 142, 142, 255], 
		[178, 178, 178, 255], 
		[216, 216, 216, 255], 
		[255, 255, 255, 255],
		[102, 30, 45, 255],
		[117, 53, 130, 255],
		[84, 73, 140, 255],
		[94, 142, 155, 255],
		[130, 192, 163, 255],
		[175, 206, 145, 255],
		[224, 226, 186, 255],
		[237, 211, 206, 255]]
	
	properties = ["normalColor", "textColor", "textHiliteColor", "borderColor", "hoverBorderColor", 
				"designBorderColor", "hoverColor", "outlineColor", 
				"editingColor", "editingBorderColor", "radius", "fontsize", "cornermask"]
	parameters = ["x", "y", "width", "height", "name", "title", "parent", "auto_register"]
		
	def __init__(self, x, y, name, title, parent, auto_register):
		
		Panel.__init__(self, x, y, 350, 340, name, title, parent, auto_register)# initialize a basic panel
		# initialize hsv @ red
		self.hsv = [1.0, 1.0, 1.0]
		self.value = [255, 255, 255, 255]
		self.hasHeader = False # disable the header	
		self.ok_functions = []
		self.cancel_functions = []			
		# add in the color select buttons
		self.addElement(Button(270, 5, 15, 25, 'Black', '', 'normal', self, False))
		self.addElement(Button(270, 30, 15, 25, 'DarkGrey', '', 'normal', self, False))
		self.addElement(Button(270, 55, 15, 25, 'NotSoDarkGrey', '', 'normal', self, False))
		self.addElement(Button(270, 80, 15, 25, 'SemiDarkGrey', '', 'normal', self, False))
		self.addElement(Button(270, 105, 15, 25, 'MediumGrey', '', 'normal', self, False))
		self.addElement(Button(270, 130, 15, 25, 'MidLightGrey', '', 'normal', self, False))
		self.addElement(Button(270, 155, 15, 25, 'LightGrey', '', 'normal', self, False))
		self.addElement(Button(270, 180, 15, 25, 'White', '', 'normal', self, False))
		self.addElement(Button(285, 5, 15, 25, 'Burgundy', '', 'normal', self, False))
		self.addElement(Button(285, 30, 15, 25, 'Violet', '', 'normal', self, False))
		self.addElement(Button(285, 55, 15, 25, 'Purple', '', 'normal', self, False))
		self.addElement(Button(285, 80, 15, 25, 'Teal', '', 'normal', self, False))
		self.addElement(Button(285, 105, 15, 25, 'SeaGreen', '', 'normal', self, False))
		self.addElement(Button(285, 130, 15, 25, 'ForestGreen', '', 'normal', self, False))
		self.addElement(Button(285, 155, 15, 25, 'Khaki', '', 'normal', self, False))
		self.addElement(Button(285, 180, 15, 25, 'Peach', '', 'normal', self, False))
		
		idx = 0
		for x in self.elements:
			x.normalColor = self.colors[idx]
			x.hoverColor = self.colors[idx]	
			x.downColor = self.colors[idx]
			x.outlined = True
			x.shadowed = False
			x.registerCallback("click", self.selectPreset)
			idx = idx + 1		
				
		# setup the big button
		self.addElement(Button(5, 5, 256, 256, 'Picker', '', 'normal', self, False))
		self.elements[16].transparent = True # bypassing the button's draw code
		self.elements[16].registerCallback("click", self.getColor)
		self.elements[16].outlined = True
		# self.elements[16].draw = self.drawHSV # override the draw method of the button object to use my custom one
		
		self.addElement(Button(5, 260, 256, 30, 'Hue', '', 'normal', self, False))
		self.elements[17].transparent = True 
		self.elements[17].outlined = True
		self.elements[17].registerCallback("click", self.getHue)
		# self.elements[17].draw = self.drawHue # override the draw method of the button object to use my custom drawHue method
		self.circleCoords = [0, 0] 
		
		self.addElement(Button(270, 215, 30, 45, 'Current', '', 'normal', self, False))
		self.elements[18].outlined = True
		self.elements[18].shadowed = False
		self.elements[18].normalColor = self.value
		self.elements[18].hoverColor = self.value
		self.elements[18].downColor = self.value
		
		self.addElement(Button(270, 265, 30, 30, 'Old', '', 'normal', self, False))
		self.elements[19].outlined = True
		self.elements[19].shadowed = False
		self.elements[19].normalColor = self.value
		self.elements[19].hoverColor = self.value
		self.elements[19].downColor = self.value
		self.elements[19].registerCallback("click", self.selectPreset) # the old button is ALSO a preset...
		
		self.addElement(Button(40, 300, 50, 25, 'CancelButton', 'Cancel', 'normal', self, False))
		self.addElement(Button(240, 300, 50, 25, 'OkButton', 'Ok', 'normal', self, False)) # these are probably going away
		self.elements[21].registerCallback("release", self.ok_funcs)
		self.elements[20].registerCallback("release", self.cancel_funcs)
		# to do - Add RGB/HSV fields, maybe make a slider
		self.offset = 16 # bounce my offset high so it look like I'm floating out there
		
	def getOld(self, button):				
		self.elements[18].normalColor = button.value
		self.elements[18].hoverColor = button.value
		self.elements[18].downColor = button.value
		self.value = button.value
		
	def getColor(self, button):
		# get the color at the current raster position
		# which is...
		coords = Blender.Window.GetMouseCoords()
		pixels = Blender.BGL.Buffer(Blender.BGL.GL_FLOAT, [4])
		gl.glReadPixels(coords[0], coords[1], 1, 1, gl.GL_RGBA, gl.GL_FLOAT, pixels)
		self.value = [int(pixels[0] * 255), int(pixels[1] * 255), int(pixels[2] * 255), int(pixels[3] * 255)]
		self.elements[18].normalColor = self.value
		self.elements[18].hoverColor = self.value
		self.elements[18].downColor = self.value
	
	def getValue(self):
		return self.value
		
	def getHue(self, button):		
		coords = Blender.Window.GetMouseCoords()		
		pixels = gl.Buffer(Blender.BGL.GL_FLOAT, [4])
		gl.glReadPixels(coords[0], coords[1], 1, 1, gl.GL_RGBA, gl.GL_FLOAT, pixels)
		hueR = [int(pixels[0] * 255), int(pixels[1] * 255), int(pixels[2] * 255), 255] 
		hsv = self.rgbToHsv(self.value) # current value
		hue = self.rgbToHsv(hueR) # incoming hue value
		#print "Pixels = ", pixels
		#print " HueR = ", hueR
		#print " Hue = ", hue
		#print  " hsv = ", hsv
		hsv[0] = hue[0]
		self.hsv = hsv
		
	def selectPreset(self, button):
		# just get the button's normal color and I'm done		
		self.value = button.normalColor		
		self.hsv = self.rgbToHsv(button.normalColor)
		self.elements[18].normalColor = self.value
		self.elements[18].hoverColor = self.value
		self.elements[18].downColor = self.value
		
	def ok_funcs(self, button):
		for func in self.ok_functions:
			func(self)
			
	def cancel_funcs(self, button):
		for func in self.cancel_functions:
			func(self)
		
	def draw(self):
		self.validate()		
		Panel.draw(self)		
		self.drawHSV()
		self.drawHue()
		
	def drawHSV(self):		
		self.validate()				
		myX = self.absX + 5
		myY = self.absY - 5
		gl = Blender.BGL
		self.setColor([10, 10, 10, 255])		
		self.uiRoundBox(myX - 1, myY - 256, myX + 256, myY + 1, 2.5, 15) # base background
		h = self.hsv[0]		
		toRgb = self.hsvToRgb	
			
		gl.glShadeModel(gl.GL_SMOOTH)
		gl.glEnable(gl.GL_BLEND)
		gl.glBegin(gl.GL_QUADS)
		# how to develop an array of values using an algorithmic method?					
		deltas = [[0, 0, float(0.0), float(0.0)], # upper left
				[16, 0, float(0.0), float(.065)], # upper right
				[16, 16, float(.065), float(.065)], # lower right
				[0, 16, float(.065), float(0.0)]] # lower left
		sat = float(1.0) # saturation starts at 1.0
		val = float(0.0)
		x = myX
		y = myY 
		for i in range(16):			
			for j in range(16):
				for delta in deltas:
					s = float(sat) - float(delta[2])
					v = float(val) + float(delta[3])
					rgb = toRgb([h, s, v])
					self.setColorFloat(rgb)
					gl.glVertex2i(x + delta[0], y - delta[1])
					
				x = x + 16 # move over one cell
				val = float(val) + float(0.065) # increment value by 1/4 
				
			x = myX # reset x
			y = y - 16 # set new Y
			sat = float(sat) - float(0.065) # decrement saturation by 1/4	
			val = 0 # reset the value to 0
			
		gl.glEnd()

	
	def drawHue(self):	
			
		hueValues = [[[255, 0, 0, 255], [255, 255, 0, 255]], # red, green ramp up
					[[255, 255, 0, 255],[0, 255, 0, 255]], # green, red ramp down
					[[0, 255, 0, 255], [0, 255, 255, 255]], # green, blue ramp up
					[[0, 255, 255, 255], [0, 0, 255, 255]], # blue, green ramp down
					[[0, 0, 255, 255], [255, 0, 255, 255]], # blue, red ramp up
					[[255, 0, 255, 255], [255, 0, 0, 255]]] # red, blue ramp down
		# 43 pixels e, each
		sx = self.absX + 5
		dx = self.absX + 48
		sy = self.absY - 265
		dy = self.absY - 295
		self.setColor([0, 0, 0, 1])
		self.uiRoundBox(sx - 1, dy - 1 , sx + 259, sy + 1, 2.5, 15)
		gl.glEnable(gl.GL_BLEND)
		for x in hueValues:		

			gl.glBegin(gl.GL_QUADS)
			self.setColor(x[0])
			gl.glVertex2i(sx, sy)
			self.setColor(x[1])
			gl.glVertex2i(dx, sy)
			self.setColor(x[1])
			gl.glVertex2i(dx, dy)
			self.setColor(x[0])
			gl.glVertex2i(sx, dy)
			gl.glEnd()
			sx = sx + 43
			dx = dx + 43
			
		gl.glDisable(gl.GL_BLEND)
		gl.glShadeModel(gl.GL_FLAT)				

		self.drawCoordCicle()
		
	def drawCoordCicle(self):
		pass # do nothing for now
				
	def validate(self):
		screen = Blender.Window.GetAreaSize()
		self.absX = (screen[0] / 2) - (self.width / 2)
		self.absY = (screen[1] / 2) + (self.height / 2)
		

		
				
class ColorButton(UIElement):
	
	def __init__(self, x, y, width, height, name, value, parent, auto_register):
		# maybe I should make this standalone?
		# color buttons have no title
		UIElement.__init__(self, x, y, width, height, name, '', parent, auto_register)
		self.normalColor = value		
		self.picking = False
		self.picker = ColorPicker(0, 0, 'colorPicker', "ColorPicker", parent, False) 
		#self.registerCallback("mouse", self.setAndHide) # close and hide on mouse-out 
		self.registerCallback("click", self.clicked)
		self.value = value
		self.picker.ok_functions.append(self.setAndHide)
		self.picker.cancel_functions.append(self.hide)
		
	
	def draw(self):
		self.validate() # locate myself		
		if self.picking:
			self.invalid = False
			#self.picker.validate() #
			# position the color picker
			screen = Blender.Window.GetAreaSize()
			# self.picker.x = self.absX
			
			# determine the center of the screen			
			x = (screen[0] / 2) - (self.picker.width / 2)
			y = (screen[1] / 2) + (self.picker.height / 2)
			# find the offset from my button to the x/y locations of the picker
			if self.absX > x:
				# the distance is negative so I use a negative value
				self.picker.x = 0 - self.absX - x
			else:
				self.picker.x = x - self.absX
				
			if self.absY > y:
				self.picker.y = self.absY - y
			else:
				self.picker.y = 0 - y - self.absY
				
			self.picker.invalid = True
			
			#if self.picker.absY > screen[1]: # the size of the picker is off the screen vertically				
			#	self.picker.y = self.absY - self.height # draw at the bottom of the button 
			#	self.picker.invalid = True
			#elif self.picker.absY - self.picker.height < 35: # if negative, part of it's off the screen 
			#	self.picker.y = self.height - (self.absY + 45)
			#	self.picker.invalid = True
			#else:
			#	self.picker.y = 0
			#	self.picker.invalid = True
			self.picker.draw() # draw the panel
		else:
			UIElement.draw(self)

	def setAndHide(self, button):		
		self.value = self.picker.value		
		self.picking = False			
		self.normalColor = self.value	
		print self.value
		
	def hide(self, button):
		self.picking = False
				
	def clicked(self, obj):
		if self.picking:			
			self.picker.click_event()
		elif self.hit_test():			
			self.picking = True
			self.picker.elements[19].normalColor = self.value
			self.picker.elements[19].hoverColor = self.value
			self.picker.elements[19].downColor = self.value 	
			
	def getValue(self):
		return self.value
							
	def hit_test(self):
		if self.picking:
			rVal = self.picker.hit_test()
		else:
			rVal = UIElement.hit_test(self)
		return rVal

	def setValue(self, value):
		# color buttons need to have their colors updated across the board in one shot, so
		self.value = value
		self.hoverColor = value
		self.normalColor = value
		self.downColor = value
		
	def release_event(self):
		if self.picking:
			self.picker.release_event()
		else:
			UIElement.release_event(self)

class TextDialog(Panel):
	OK = 1
	CANCEL = 0
	def __init__(self, x, y, width, height, name, prompt, value, function):
		Panel.__init__(self, x, y, width, height, name, name, None, False)	
		self.button_functions = []
		# determine how many lines I need, and if neccessary, split the lines and
		# resize the panel
		t_width = self.get_string_width(prompt, 'normal')
		if t_width > width: # the string width is greater than the width of the panel
			if t_width - width < 100: # if less than 100 pixels of difference, simply resize the panel
				self.width = t_width + 10
				self.addElement(Label(5, 30, prompt, prompt, self, False)) # simply add the label
				field_y = 30 + self.font_cell_sizes[self.fontsize][1] + 8
				field_width = t_width
			else:
				# bit more complicated here. Create multiple labels...figure out how to split on whitespace
				# break the string up into words
				words = prompt.split()
				lines = []
				line = ""				
				currentWidth = 0
				for word in words:					
					word = word + " " # add a space on each end
					wordWidth = self.get_string_width(word, 'normal')
					if currentWidth + wordWidth < self.width - 10: # 5 pixel buffer zone on either side
						line = line + word
						currentWidth = currentWidth + wordWidth
					else:
						lines.append(line)
						line = ""
						currentWidth = 0
				label_y = 30
				for line in lines:					
					self.addElement(Label(5, label_y, line, line, self, False))
					label_y = label_y + self.font_cell_sizes[self.fontsize][1] + 4 # 4 pixel gap here
					
				field_y = label_y
		else:
			self.addElement(Label(5, 30, prompt, prompt, self, False))
			field_y = 30 + self.font_cell_sizes[self.fontsize][1] + 4
		
		self.data = TextField(5, field_y, self.width - 15, 20, "DialogData", value, self, True)
		# self.addElement(self.data) - this is now handled by auto_register
		# get the center of the dialog, then split out
		
		ok_offset = self.width - 110		
		
		but_y = field_y + 30
		self.ok_button = Button(ok_offset, but_y, 100, 25, "OKButton", "OK", 'normal', self, True)
		self.ok_button.registerCallback("release", self.button_funcs)
		self.cancel_button = Button(10, but_y, 100, 25, "OKButton", "Cancel", 'normal', self, True)
		self.cancel_button.registerCallback("release", self.button_funcs)		
		self.button_functions.append(function)
		#self.addElement(self.ok_button)
		#self.addElement(self.cancel_button)
		self.state = -1
		self.initValue = value
		self.height = but_y + 40
		
		
		
	def button_funcs(self, button):
		if button == self.ok_button:
			self.state = self.OK
		else:
			self.state = self.CANCEL
		for func in self.button_functions:
			func(self)	
		
	def getValue(self):
		if self.state == self.OK:
			value = self.data.getValue()
		else:
			value = self.initValue
		return value
	
class ConfirmDialog(Panel):
	OK = 1
	CANCEL = 0
	DISCARD = 2
	def __init__(self, name, prompt, function, discard):
		x = 50 
		y = 50
		width = 350
		height = 100
		Panel.__init__(self, x, y, width, height, name, name, None, False)	
		self.button_functions = []
		# determine how many lines I need, and if neccessary, split the lines and
		# resize the panel
		t_width = self.get_string_width(prompt, 'normal')
		if t_width > width: # the string width is greater than the width of the panel
			if t_width - width < 100: # if less than 100 pixels of difference, simply resize the panel
				self.width = t_width + 10
				self.addElement(Label(5, 30, prompt, prompt, self, False)) # simply add the label
				buts_y = 30 + self.font_cell_sizes[self.fontsize][1] + 8				
			else:
				# bit more complicated here. Create multiple labels...figure out how to split on whitespace
				# break the string up into words
				words = prompt.split()
				lines = []
				line = ""				
				currentWidth = 0
				for word in words:					
					word = word + " " # add a space on each end
					wordWidth = self.get_string_width(word, 'normal')
					if currentWidth + wordWidth < self.width - 10: # 5 pixel buffer zone on either side
						line = line + word
						currentWidth = currentWidth + wordWidth
					else:
						lines.append(line)
						line = ""
						currentWidth = 0
				label_y = 30
				for line in lines:					
					self.addElement(Label(5, label_y, line, line, self, False))
					label_y = label_y + self.font_cell_sizes[self.fontsize][1] + 4 # 4 pixel gap here
					
				buts_y = label_y
		else:
			self.addElement(Label(5, 30, prompt, prompt, self, False))
			buts_y = 35 + self.font_cell_sizes[self.fontsize][1] + 4
			
		discard_x = (self.width / 2) - 30
		ok_offset = self.width - 75		
		self.ok_button = Button(ok_offset, buts_y, 60, 25, "OKButton", "OK", 'normal', self, True)
		self.ok_button.registerCallback("release", self.button_funcs)
		self.cancel_button = Button(10, buts_y, 60, 25, "OKButton", "Cancel", 'normal', self, True)	
		self.cancel_button.registerCallback("release", self.button_funcs)
		
		if function != None:
			self.button_functions.append(function)
		#self.addElement(self.ok_button)
		#self.addElement(self.cancel_button)
		if discard:		
			self.discard = True
			self.discard_button = Button(discard_x, buts_y, 60, 25, "Discard", "Discard", 'normal', self, True)
			self.discard_button.registerCallback("release", self.button_funcs)
			# self.addElement(self.discard_button)
		else:
			self.discard = False
			
		self.height = buts_y + 40
		
	def button_funcs(self, button):
		if button == self.ok_button:
			self.state = self.OK
		elif self.discard:
			if button == self.discard_button:
				self.state = self.DISCARD
		else:
			self.state = self.CANCEL
		for func in self.button_functions:
			func(self)		
		
		
	def getValue(self):
		return self.state # returns the state of the dialog
class MsgBox(Panel):	
	def __init__(self, name, prompt, function):
		x = 50 
		y = 50
		width = 350
		height = 100
		Panel.__init__(self, x, y, width, height, name, name, None, False)	
		self.button_functions = []
		# determine how many lines I need, and if neccessary, split the lines and
		# resize the panel
		t_width = self.get_string_width(prompt, 'normal')
		if t_width > width: # the string width is greater than the width of the panel
			if t_width - width < 100: # if less than 100 pixels of difference, simply resize the panel
				self.width = t_width + 10
				self.addElement(Label(5, 30, prompt, prompt, self, False)) # simply add the label
				buts_y = 30 + self.font_cell_sizes[self.fontsize][1] + 8				
			else:
				# bit more complicated here. Create multiple labels...figure out how to split on whitespace
				# break the string up into words
				words = prompt.split()
				lines = []
				line = ""				
				currentWidth = 0
				for word in words:					
					word = word + " " # add a space on each end
					wordWidth = self.get_string_width(word, 'normal')
					if currentWidth + wordWidth < self.width - 10: # 5 pixel buffer zone on either side
						line = line + word
						currentWidth = currentWidth + wordWidth
					else:
						lines.append(line)
						line = ""
						currentWidth = 0
				label_y = 30
				for line in lines:					
					self.addElement(Label(5, label_y, line, line, self, False))
					label_y = label_y + self.font_cell_sizes[self.fontsize][1] + 4 # 4 pixel gap here
					
				buts_y = label_y
		else:
			self.addElement(Label(5, 30, prompt, prompt, self, False))
			buts_y = 35 + self.font_cell_sizes[self.fontsize][1] + 4
			
		discard_x = (self.width / 2) - 30
		ok_offset = self.width / 2 - 30
		self.ok_button = Button(ok_offset, buts_y, 60, 25, "OKButton", "OK", 'normal', self, True)
		self.ok_button.registerCallback("release", self.button_funcs)
		
		if function != None:
			self.button_functions.append(function)
		self.height = buts_y + 40
		
	def button_funcs(self, button):
		for func in self.button_functions:
			func(self)		
		

class Image(UIElement):
	useOpenGLTextures = False
	def __init__(self, x, y, width, height, image, parent, auto_register):
		UIElement.__init__(self, x, y, width, height, "Image",  "image", parent, auto_register)
		if image != None:
			self.image = image		
			self.bindCode = self.image.getBindCode()
			self.unbound = False
		else:
			self.image = None
			self.bindCode = 0
			self.unbound = True
		
	def bindImage(self):
		self.bindCode = self.image.getBindCode()
		

		
	
	def draw(self):
		self.validate()
		if self.useOpenGLTextures:
			self.validate()
			# setup the texture drawing stuff
			if self.unbound:
				self.bindCode = self.image.getBindCode()
				if self.bindCode == 0:
					self.unbound = True
			if not self.unbound:
				gl.glBindTexture(gl.GL_TEXTURE_2D, self.bindCode)
				
				gl.glTexEnvf(gl.GL_TEXTURE_ENV, gl.GL_TEXTURE_ENV_MODE, gl.GL_DECAL)
				
				gl.glEnable(gl.GL_TEXTURE_2D)
				gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP)
				gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP)
				gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
				gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
				
				gl.glBegin(gl.GL_QUADS)
				gl.glTexCoord2f(1.0, 0.0)
				gl.glVertex2f(self.absX + self.width, self.absY - self.height)
				gl.glTexCoord2f(1.0, 1.0)
				gl.glVertex2f(self.absX + self.width, self.absY)
				gl.glTexCoord2f(0.0, 1.0)
				gl.glVertex2f(self.absX, self.absY)
				gl.glTexCoord2f(0.0, 0.0)
				gl.glVertex2f(self.absX, self.absY - self.height)
				gl.glEnd()		
				
				gl.glDisable(gl.GL_TEXTURE_2D)			
		else:		
			size = self.image.getSize()
			if size[0] <> self.width or size[1] <> self.height:
				# I need to zoom the image
				# figure out the correct zoom ratio
				x_scale = float(self.width) / float(size[0])
				y_scale = float(self.height) / float(size[1])
				if x_scale > y_scale:
					zoom = x_scale
				else:
					zoom = y_scale
				Blender.Draw.Image(self.image, self.absX, self.absY, zoom, zoom)
			else:
				Blender.Draw.Image(self.image, self.absX, self.absY)
				# that should be that. Images should be sized to whatever they need to be on creation.
			
	def validate(self): 
		
		# apply transforms to the x,y values of this object using the parent's absolute locations
		if self.invalid:
			# cascade invalidations down the chain of contained elements
			for element in self.elements:
				element.invalid = True
			# image elements are different. images draw UP rather than down, so y indicates the bottom edge of the image.
			# thus, to achieve the desired behaviour, I must adjust the my Y value to actually be Y - self.height in 
			# my abs calculation UNLESS I'm using OpenGL textures
			# 
			if self.useOpenGLTextures:
				if self.parent == None:
					self.absX = self.x
					self.absY = self.y
				else:				
					self.absX = self.parent.absX + self.x 
					self.absY = self.parent.absY - self.y 
			else:
				if self.parent == None:
					self.absX = self.x
					self.absY = self.y - self.height
				else:				
					self.absX = self.parent.absX + self.x 
					self.absY = self.parent.absY - (self.y + self.height)
				
		self.invalid = False
		for func in self.validate_functions:
			func()
			
		
class VisibilityGroup: # maintains a list of objects and turns them on or off 
	def __init__(self, objList):
		self.objList = objList
	def hide(self):
		for obj in self.objList:
			obj.hide()
	def show(self):
		for obj in self.objList:
			obj.show()
			
			
			

class CurveEditor(Panel):
	# ok this is a curve editor. I should be able to specify a starting frame, an ending frame, and 
	# have a curve that I can add quite a few nodes to. This thing should probably be fairly large, and be activated by a 
	# launch icon that I click.
	def __init__(self, x, y, width, height, name, title, parent, auto_register = True, fontsize = 'normal'):
		Panel.__init__(self, x, y, width, height, name, title, parent)
		# a curve editor should have a back panel display
		# the curve itself
		# and maybe I use the curve as a boundary or something and use a floodfill?
		
			