####################################################################
# BtoR - Renderman Exporter for blenderman
# ShaderEditorGUI.py module
# Author: Bobby Parker
# This module provides functionality to 
# iterate the shaders along a given shaderPath
# and displays editors for the parameters reported by the 
# selected shader.
# It tries to make intelligent guesses 
# about String parameters, by filtering for the 
# strings "map" and "tex" (texture files) and by looking for 
# color space, coordinate space, or color model name.
####################################################################

####################################################################
# Defintions: 
# ShaderPath: A place to look for shaders
# Material: A configured set of parameters and shader name that provide a look for the
# object it's assigned to.
# Shader: the shader selected.
####################################################################

import Blender
from Blender.BGL import *
from Blender.Draw import *
reload(Blender)
import cgkit
reload(cgkit)
import cgkit.slparams
reload(cgkit.slparams)
import string
import re
import os
import os.path
reload(os)


# shader editor variables	
MaterialDeleteButton = Create(1)
PathDeleteButton = Create(1)
BrowseButton = Create(1)
PreviewButton = Create(1)
RestoreButton = Create(1)
Materials = Create(1)
ShaderTypeMenu = Create(1)
Material = Create('SH:')
ShaderPath = Create('')
ShaderMenu = Create(1)
ShaderPathMenu = Create(1)
ShaderTypes = "Surface |Displacement |Light |Volume "
SpaceMenuStrings = "current|object|shader|world|camera|screen|raster|NDC"
SpaceList = ["current", "object", "shader", "world", "camera", "screen", "raster", "NDC"]
ColorSpaceMenuStrings = "rgb|hsv|hsl|YIQ|xyz|xyY"
ColorSpaceList = ["rgb", "hsv", "hsl", "YIQ", "xyz", "xyY"]
ProjectionMenuStrings = "st|planar|perspective|spherical|cylindrical"
ProjectionList = ["st", "planar", "perspective", "spherical", "cylindrical"]
ShaderNames = "Empty"
CurrentShaderList = []
ShaderPaths = "Add New "
ShaderPathList = []
MaterialsNames = "Add New "
CurrentMaterial = ""
MaterialList = []
CurrentShader = ""
ShadersSurface = []
ShadersVolume = []
ShadersImager = []
ShadersLight = []
ShadersDisplacement = []
haveValues = False
parameditors = []
paramvalues = []	
parameditortypes = []
currentitem = 0
highlighta = [0.36, 0.44, 0.50]
highlightb = [0.38, 0.46, 0.54]
color = []
filebuttons = []
overridebuttons = []
DestIndex = 0

# shader editor event constants
evt_material_delete = 1
evt_path_delete = 2
evt_browse_button = 3
evt_preview_button = 4
evt_restore_button = 5
evt_material_rename = 6
evt_path_rename = 7
evt_material_select = 8
evt_type_select = 9	
evt_shader_select = 10
evt_path_select = 11
evt_shader_parm_modify = 100
evt_file_browse = 600


# Shader Editor
def shader_draw():
	# button and menu global variables
	global Materials, ShaderTypeMenu, DeleteMaterial, Material, ShaderPath, DeleteShaderPath, BrowseButton, ShaderMenu, ShaderPathMenu
	global PreviewButton, RestoreButton, filebuttons, overridebuttons, ColorSpaceMenuStrings, ProjectionMenuStrings, SpaceMenuStrings
	
	# event global variables
	global evt_material_delete, evt_path_delete, evt_browse_button, evt_preview_button, evt_restore_button, evt_material_rename, evt_path_rename
	global evt_material_select, evt_type_select, evt_shader_select, evt_shader_parm_modify
	
	# state control and storage variables
	global haveValues, parameditors, paramvalues, parameditortypes, currentitem
	
	# other stuff
	global highlighta, highlightb, color, ColorSpaceList, ProjectList, SpaceList

	glClearColor(0.4,0.48,0.57, 0.0) 	# blue background
	glClear(GL_COLOR_BUFFER_BIT)
	glColor3f(0, 0, 0) 			# main black back
	glRectf(2, 2, 670, 600)
	glColor3f(0.4, 0.48, 0.57) 		# main blue back
	glRectf(4, 4, 668, 598)
	#glColor3f(0.27, 0.3, 0.35) 		# blue top
	#glRectf(4, 302, 628, 338)

	glColor3f(1,1,1)
	glColor3f(1.000, 1.000, 1.000)
	glRasterPos2i(8, 532)
	Text('Material:')
	glRasterPos2i(8, 500)
	Text('Shader Type:')
	glRasterPos2i(272, 532)
	Text('Shader Path:')
	glRasterPos2i(208, 500)
	Text('Shader:')

	MaterialDeleteButton = Button('X', evt_material_delete, 240, 520, 23, 23, 'Delete Material')
	PathDeleteButton = Button('X', evt_path_delete, 568, 520, 23, 23, 'Delete Shader Path')
	BrowseButton = Button('Browse', evt_browse_button, 592, 520, 71, 23, '')
	PreviewButton = Button('Preview Material', evt_preview_button, 432, 488, 111, 23, '')
	RestoreButton = Button('Restore Defaults', evt_restore_button, 552, 488, 111, 23, '')

	Material = String(CurrentMaterial, evt_material_rename, 80, 520, 159, 23, Material.val, 512, '')
	ShaderPath = String(CurrentShader, evt_path_rename, 368, 520, 199, 23, ShaderPath.val, 512, '')

	Materials = Menu(MaterialsNames, evt_material_select, 64, 520, 20, 23, Materials.val, 'Current Material')
	ShaderTypeMenu = Menu	(ShaderTypes, evt_type_select, 88, 488, 103, 23, ShaderTypeMenu.val, 'Shader Type')
	ShaderMenu = Menu(ShaderNames, evt_shader_select, 256, 488, 167, 23, ShaderMenu.val, 'Shader')
	ShaderPathMenu = Menu(ShaderPaths, evt_path_select, 344, 520, 23, 23, ShaderPathMenu.val, 'Shader Search Path')
	if Material.val:
		if (CurrentShaderList): 
			# ShaderMenu = Menu(ShaderNames, evt_sel_shader,  10,  230, 200,  20, ShaderMenu.val)
			# working dynamic area: upperBound = 225, lowerbound = 10, leftbound = 10, rightbound = 600
			myshader = CurrentShaderList[ShaderMenu.val - 1]				
			count = 0							
			currenty = 480
			filebuttons = [] # global var to hold button objects
			overridebuttons = []
			idx = 0
			#pdb.set_trace()
			# I need the dimensions data for this shader
			getShaderDimensions(myshader)
			for x in myshader[2]:
				# get the color here
				if count % 2:
					color = highlighta
				else:
					color = highlightb
				idx = count
				eventidx = evt_shader_parm_modify + idx
				if haveValues == False:
					parameditors.append("") 
					# assign a placeholder value to parameditors
					
					# time for Black Magick and v00d0u
					tex = re.compile(".*tex.*")
					mapx = re.compile(".*map.*")
					if x[2] == "string" and ("tex" in x[4] or "map" in x[4]): # if the value is a string AND meets one of these two items
						# texture or shadow map something, show a file selection button
						parameditortypes.append("file")
						thisVal = cgkit.slparams.convertdefault(x)
						paramvalues.append(thisVal)

					# ok, now more Black Magickal evilness 
					# look for projection types, colorspace types, and coordinate space types
					
					elif x[2] == "string": # logically, this elif should catch here only IF the above criteria doesn't match
						sval = cgkit.slparams.convertdefault(x)
						isMenu = False
						for a in ProjectionList:
							if a in sval:								
								parameditortypes.append("projection")
								isMenu = True
								
						for a in SpaceList:
							if a in sval:
								parameditortypes.append("space")
								isMenu = True
								
						for a in ColorSpaceList:
							if a in sval:
								parameditortypes.append("colorspace")
								isMenu = True
								
						# if it doesn't match any of the above, do this
						if not isMenu:
							parameditortypes.append("string")
							
						thisVal = cgkit.slparams.convertdefault(x)
						paramvalues.append(thisVal)
						
					elif x[2] == "float" and x[3] == None:
						thisVal = cgkit.slparams.convertdefault(x)
						parameditortypes.append(x[2])
						paramvalues.append(thisVal) # something
						
					elif x[2] == "float" and x[3] != None: # float array
						valDef = cgkit.slparams.convertdefault(x)
						sval = []
						for val in valDef:
							sval.append("%f" % val)
						valstring = string.join(sval, " ")
						thisVal = valstring # now it's a string
						paramvalues.append(thisVal)
						parameditortypes.append("array") # this is now an array
						# I can extend this to fully support the array by dynamically generating number buttons
						# to the tune of len(array)

					elif x[2] == "color" or x[2] == "point" or x[2] == "vector" or x[2] == "normal": # the usual
						thisVal = []
						# convert the default value for my use
						y = cgkit.slparams.convertdefault(x)
						thisVal.append(x[5])
						thisVal.append(y[0])
						thisVal.append(y[1])
						thisVal.append(y[2])
						paramvalues.append(thisVal)
						parameditortypes.append(x[2])

					elif x[2] == "matrix":
						thisVal = cgkit.slparams.convertdefault(x)
						paramvalues.append(thisVal)
						parameditortypes.append(x[2])
					else:
						parameditortypes.append("string")
						paramvalues.append(x[2])
					# there should be no case that makes it past this
					print "Pre-init thisVal for parameter ", x[4], ": ", thisVal
					print "Parameter Data: paramvalues = ", paramvalues[idx], "parameditortype = ", parameditortypes[idx]
				else:
					# we've got the values initialized, so thisVal should be the value from the paramvalues list
					thisVal = paramvalues[idx] # should be broken out already, so it's just a matter of yanking from the paramvalues list 
					print "Post-init thisVal for parameter ", x[4], ": ", thisVal
				
				e = parameditortypes[idx] # get the editor type
				if count >= currentitem and (currenty - scrolllayout[idx - 4]) > 25:
					if e == "float": # float value, x is not neccessary here
						if isinstance(thisVal, float):
							finit = thisVal
						else:
							print thisVal
							print x
							finit = thisVal.val
						currenty = currenty - 31
						glColor3f(color[0], color[1], color[2])
						# draw the colored rectangle here
						glRecti(4, currenty - 4, 668, currenty + 24)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 4)
						stext = "Parameter: " + x[4]
						Text(stext)
						parameditors[idx] = Number("", eventidx, 180, currenty, 100, 20, finit, -10, 10, x[4])

					if e == "string" or (e == "float" and x[3] != None): # string value
						if isinstance(thisVal, basestring):
							sinit = thisVal
						else:
							sinit = thisVal.val	
						if sinit == "none":
							sinit = ""
						currenty = currenty - 31
						glColor3f(color[0], color[1], color[2])
						# draw the colored rectangle here
						glRecti(4, currenty - 4, 668, currenty + 24)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 4)
						stext = "Parameter: " + x[4]
						Text(stext)
						parameditors[idx] = String("", eventidx, 180, currenty, 250, 20, sinit, 50, x[4])

					if e == "color": # color value
						currenty = currenty - 54
						# For a color editor, we don't want to use paramvalues directly
						# we want to setup 4 seperate items that are tracked (i.e. an list within the that element) 
						print "thisVal at colorEditor:", thisVal
						controllist = []
						controllist.append(" ")
						controllist.append(" ")
						controllist.append(" ")
						controllist.append(" ")
						if not isinstance(thisVal[1], float):
							red = thisVal[1]
							green = thisVal[2]
							blue = thisVal[3]
						else:
							red = thisVal[1]
							green = thisVal[2]
							blue = thisVal[3]
						ctype = [] # this list is the container for the color model names, e.g. RGB, HSV, HSL
						if isinstance(thisVal[0], basestring): # it's a string, react accordingly
							xa = thisVal[0]
							if xa == "rgb":
								xval = 1
							elif xa == "hxv":
								xval = 2
							elif xa == "hsl":
								xval = 3
						else: # I know the color model's been set								
							xval = thisVal[0] # should be a numeric value here
						if xval == 1:
							ctype = ["Red: ", "Green: ", "Blue: "]
						elif xval == 2:
							ctype = ["Hue: ", "Saturation: ", "Value: "]
						elif xval == 3:
							ctype = ["Hue: ", "Saturation: ", "Lightness: "]
						glColor3f(color[0], color[1], color[2])
						# draw the colored rectangle here
						glRecti(4, currenty - 4, 668, currenty + 49)
						glColor3f(1, 1, 1)		
						glRasterPos2i(13, currenty + 30)
						tval = "Parameter: " + x[4]
						Text(tval)
						glRasterPos2i(13, currenty+ 5)
						Text("Color Space:")
						controllist[0] = Menu("rgb |hsv |hsl ", eventidx, 100, currenty, 60, 25, xval, "Color Model")
						controllist[1] = Slider(ctype[0], eventidx, 160, currenty + 30 , 200, 15, red, 0, 1, 0, ctype[0])
						controllist[2] = Slider(ctype[1], eventidx, 160, currenty + 15 , 200, 15, green, 0, 1, 0, ctype[1])
						controllist[3] = Slider(ctype[2], eventidx, 160, currenty, 200, 15, blue, 0, 1, 0, ctype[2])
						# here I want to display an image that shows the current RGB color value. I'll have to figure out some way of converting it to HSV/HSL, etc. as I go
						glColor3f(controllist[1].val, controllist[2].val, controllist[3].val) 		
						glRectf(370, currenty, 440, currenty + 45) # worry about conversion from HSV/HSL later
						parameditors[idx] = controllist # now I know that parameditors[idx][0 - 4] is a color editor with associated values

					if e == "point" or e == "vector" or e == "normal":
						currenty = currenty - 54
						# points, vectors, or normals
						# same rule applies to these parameter types, need to track 4 seperate values
						controllist = []
						controllist.append(" ")
						controllist.append(" ")
						controllist.append(" ")
						controllist.append(" ") # don't laugh, this avoids a creepy little bug
						xval = 0
						if isinstance(thisVal[0], basestring):
							xa = thisVal[0] # determine the coordinate space, and assign 
							if xa == "current":
								xval = 1
							if xa == "object":
								xval = 2
							if xa == "shader":
								xval = 3
							if xa == "world":
								xval = 4
							if xa == "camera":
								xval = 5
							if xa == "screen":
								xval = 6
							if xa == "raster":
								xval = 7
							if xa == "NDC":
								xval = 8
							
							xinit = thisVal[1]
							yinit = thisVal[2]
							zinit = thisVal[3]
							
						else: # this test should no longer be neccessary, more accurately, should only affect xval
							xval = thisVal[0] 
							xinit = thisVal[1]
							yinit = thisVal[2]
							zinit = thisVal[3]

						glColor3f(color[0], color[1], color[2])
						# draw the colored rectangle here
						glRecti(4, currenty - 4, 668, currenty + 49)
						# how do I display the name?
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 30)
						tval = "Parameter: " + x[4]
						Text(tval)
						glRasterPos2i(13, currenty + 9)
						Text("Coordinate Space:")
						controllist[0] = Menu("current |object |shader |world |camera |screen |raster |NDC", eventidx, 120, currenty, 90, 25, xval, "Coordinate Space")
						controllist[1] = Number("X: ", eventidx, 215, currenty + 30, 90, 15, xinit, 0, 100, "X coordinate")
						controllist[2] = Number("Y: ", eventidx, 215, currenty + 15, 90, 15, yinit, 0, 100, "Y coordinate")
						controllist[3] = Number("Z: ", eventidx, 215, currenty, 90, 15, zinit, 0, 100, "Z coordinate")
						parameditors[idx] = controllist

					if e == "matrix":
						#print "Thisval, matrix = ", thisVal
						currenty = currenty - 62
						# create a 4x4 matrix of number buttons
						controlllist = thisVal # this should be x[3]
						if isinstance(controllist[0], float) or isinstance(controllist[0], int):
							aa = controllist[0]
							ab = controllist[1]
							ac = controllist[2]
							ad = controllist[3]
							ba = controllist[4]
							bb = controllist[5]
							bc = controllist[6]
							bd = controllist[7]
							ca = controllist[8]
							cb = controllist[9]
							cc = controllist[10]
							cd = controllist[11]
							da = controllist[12]
							db = controllist[13]
							dc = controllist[14]
							dd = controllist[15]
						else:
							aa = controllist[0].val
							ab = controllist[1].val
							ac = controllist[2].val
							ad = controllist[3].val
							ba = controllist[4].val
							bb = controllist[5].val
							bc = controllist[6].val
							bd = controllist[7].val
							ca = controllist[8].val
							cb = controllist[9].val
							cc = controllist[10].val
							cd = controllist[11].val
							da = controllist[12].val
							db = controllist[13].val
							dc = controllist[14].val
							dd = controllist[15].val

						glColor3f(color[0], color[1], color[2])
						# draw the colored rectangle here
						glRecti(4, currenty - 4, 668, currenty + 47)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 30)
						Text(x[4]) # draw the variable name
						controllist[0] = Number("0,0: ", eventidx, 70, currenty, 100, 20, -100, 100, aa, "")
						controllist[1] = Number("0,1: ", eventidx, 130, currenty, 100, 20, -100, 100, ab, "")
						controllist[2] = Number("0,2: ", eventidx, 190, currenty, 100, 20, -100, 100, ac, "")
						controllist[3] = Number("0,3: ", eventidx, 250, currenty, 100, 20, -100, 100, ad, "")
						controllist[4] = Number("1,0: ", eventidx, 70, currenty - 20, 100, 20, -100, 100, ba, "")
						controllist[5] = Number("1,1: ", eventidx, 130, currenty - 20, 100, 20, -100, 100, bb, "")
						controllist[6] = Number("1,2: ", eventidx, 190, currenty - 20, 100, 20, -100, 100, bc, "")
						controllist[7] = Number("1,3: ", eventidx, 250, currenty - 20, 100, 20, -100, 100, bd, "")
						controllist[8] = Number("2,0: ", eventidx, 70, currenty - 40, 100, 20, -100, 100, ca, "")
						controllist[9] = Number("2,1: ", eventidx, 130, currenty - 40, 100, 20, -100, 100, cb, "")
						controllist[10] = Number("2,2: ", eventidx, 190, currenty - 40, 100, 20, -100, 100, cc, "")
						controllist[11] = Number("2,3: ", eventidx, 250, currenty - 40, 100, 20, -100, 100, cd, "")
						controllist[12] = Number("3,0: ", eventidx, 70, currenty - 60, 100, 20, -100, 100, da, "")
						controllist[13] = Number("3,1: ", eventidx, 130, currenty - 60, 100, 20, -100, 100, db, "")
						controllist[14] = Number("3,2: ", eventidx, 190, currenty - 60, 100, 20, -100, 100, dc, "")
						controllist[15] = Number("3,3: ", eventidx, 250, currenty - 60, 100, 20, -100, 100, dd, "")
						parameditors[ix] = controllist	

						# custom "guessed" editors
					if e == "file":
						# display a nice file editor
						# for the moment, display a string
						currenty = currenty - 31
						glColor3f(color[0], color[1], color[2])
						glRecti(4, currenty - 4, 668, currenty + 24)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 4)
						stext = "Parameter: " + x[4]
						Text(stext)
						parameditors[idx] = String("", eventidx, 180, currenty, 250, 20, thisVal, 50, x[4])
						filebuttons.append(" ")
						fidx = len(filebuttons) - 1
						filebuttons[fidx] = Button('Browse', evt_file_browse + eventidx, 430, currenty, 85, 20, x[4])
						# parameter type override button goes here

					if e == "space":
						currenty = currenty - 31
						# display a nice menu
						# now I gotta sort on the default value
						# thisVal should be the default value
						if isinstance(thisVal, basestring):
							icount = 1
							for a in SpaceList:
								if thisVal == a:
									xval = icount
								icount = icount + 1
						else:
							xval = thisVal

						glColor3f(color[0], color[1], color[2]) # colored rectangle
						glRecti(4, currenty - 4, 668, currenty + 24)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 4)
						stext = "Parameter: " + x[4]
						Text(stext)
						parameditors[idx] = Menu(SpaceMenuStrings, eventidx, 180, currenty, 100, 25, xval, "Coordinate Space")
						# parameter type override button goes here
						
					if e == "colorspace":
						currenty = currenty - 31
						# display a nice menu
						# now I gotta sort on the default value
						# thisVal should be the default value
						if isinstance(thisVal, basestring):
							for a in ColorSpaceList:
								icount = 1
								if thisVal == a:
									xval = icount
								icount = icount + 1
						else:
							xval = thisVal

						glColor3f(color[0], color[1], color[2]) # colored rectangle
						glRecti(4, currenty - 4, 668, currenty + 24)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 4)
						stext = "Parameter: " + x[4]
						Text(stext)
						parameditors[idx] = Menu(ColorSpaceMenuStrings, eventidx, 180, currenty, 100, 25, xval, "Color Space")
						# parameter type override button goes here
						
					if e == "projection":
						currenty = currenty - 31
						# display a nice menu
						# now I gotta sort on the default value
						# thisVal should be the default value
						if isinstance(thisVal, basestring):
							for a in ProjectionList:
								icount = 1
								if thisVal == a:
									xval = icount
								icount = icount + 1
						else:
							xval = thisVal

						glColor3f(color[0], color[1], color[2]) # colored rectangle
						glRecti(4, currenty - 4, 668, currenty + 24)
						glColor3f(1, 1, 1)
						glRasterPos2i(13, currenty + 4)
						stext = "Parameter: " + x[4]
						Text(stext)
						parameditors[idx] = Menu(ProjectionMenuStrings, eventidx, 180, currenty, 100, 25, xval, "Projection Type")
						# parameter type override button goes here


				count = count +1	

			if haveValues == False:
				# reset the haveValues so I know to take my data from the paramvalues array
				haveValues = True
	return 1

def shader_event(evt, val): # shader screen mouse/keyboard events	
	global currentitem, scrolllayout
	if evt in [WHEELUPMOUSE, UPARROWKEY]: # scroll down
		if not currentitem == 0:						
			currentitem = currentitem -1								
		Draw()
	if evt in [WHEELDOWNMOUSE, DOWNARROWKEY]: # scroll up
		if not currentitem == (len(scrolllayout) - 1):
			# find the last item
			currentitem = currentitem +1
		Draw()			
	# Emergency bail out mode
	if (evt== ESCKEY and not val): 
		Exit() 
		# Display ye olde dialog here.

def shader_bevent(evt): # shader screen button events
	# event constant globals
	global evt_material_delete, evt_path_delete, evt_browse_button, evt_preview_button, evt_restore_button, evt_material_rename, evt_path_rename
	global evt_material_select, evt_type_select, evt_shader_select, evt_path_select, evt_shader_parm_modify

	# Control globals
	global Materials, ShaderTypeMenu, Material, ShaderPath, ShaderPathMenu, ShaderTypes, ShaderNames, CurrentShaderList, ShaderPaths
	global ShaderPathList, MaterialsNames, CurrentMaterial, MaterialList, CurrentShader, haveValues, parameditors
	global ShadersSurface, ShadersDisplacement, ShadersLight, ShadersVolume, paramvalues, parameditortypes, currentitem
	
	if evt == evt_material_select: #Materials		
		# to avoid confusion:
		# Materials = the menu object (Blender.Draw.Menu)
		# Material = the String object (Blender.Draw.String)
		# MaterialNames = Array that tracks the actual names of the materials - Maybe this isn't neccessary		
		namelist = string.split(MaterialsNames, "|") # split up the menu string to get the selected item
		Material.val = namelist[Materials.val - 1] # change the label of the shader in the shadereditor to the one selected
		listlenght = len(namelist) # length of the list
		increment = 0
		if (Materials.val == listlenght): # if the length of the list of shader names is the same as the surfaceshader menu then add another item to the menu
			increment = increment +1
			name = 0
			numbername = "%03d" % (int(increment))
			while (name == 0):
				name = 1
				for x in namelist[:]:
					if ('Surface.%s'%numbername == x):
						increment = increment +1
						name = 0
						numbername = "%03d" % (int(increment))
			namelist.insert(listlenght - 1 , 'Surface.%s'%numbername) # insert the new slot into the material list
			MaterialNames = string.join(namelist, "|") # recreate the menu string that contains the material names and "Add New"
			Material.val = namelist[Materials.val - 1] # set the value of the STRING object to the current selected shader object
			Materials.val = 1
		
	elif evt == evt_type_select: #ShaderTypeMenu
		if ShaderTypeMenu.val == 1:
			CurrentShaderList = ShadersSurface
		if ShaderTypeMenu.val == 2:
			CurrentShaderList = ShadersDisplacement
		if ShaderTypeMenu.val == 3:
			CurrentShaderList = ShadersLight
		if ShaderTypeMenu.val == 4:
			CurrentShaderList = ShadersVolume
		setShaderList()		
		haveValues = False
		parameditors = []
		paramvalues = []
		parameditortypes = []
		currentitem = 0

	elif evt == evt_material_delete: #DeleteMaterial
		# Delete the material from the list here
		# determine the selected material
		idx = Materials.val - 1 # lists are 0 bounded, blender menu values are NOT. annoying
		del MaterialList[idx]
		# reconstruct the menu string
		matlist = string.split(MaterialsNames, "|")
		MaterialsNames = string.join(matlist, "|")
		# the material should be deleted now.
		
	elif evt == evt_path_delete: #DeleteShaderPath
		idx = ShaderPathMenu.val - 1
		del ShaderPaths[idx]
		pathlist = string.split(ShaderPaths, "|")
		ShaderPaths = string.join(pathlist, "|")
		
	elif evt == evt_browse_button: #BrowseButton
		Blender.Window.FileSelector(dirselect, 'Choose any file')

	elif evt == evt_shader_select: #ShaderMenu
		# shader selection...I should have to do nothing here
		# except fire Redraw()
		# this should actually be handled by the code that draws the GUI
		# hey idiot, you need to reset the values of that stuff
		haveValues = False
		parameditors = []
		paramvalues = []
		parameditortypes = []
		currentitem = 0
		Redraw()

	elif evt == evt_path_select: #ShaderPathMenu
		# process here to see if I need to add a new path
		# so for tracking...we have to have 
		# ShaderPathMenu and ShaderPaths
		# ShaderPaths is the menu string, and contains "Add New"
		# I must test for "Add new" selection
		idx = ShaderPathMenu.val		
		pathlist = string.split(ShaderPaths, "|")
		length = len(pathlist)
		if (ShaderPathMenu.val == length):
			# I know here that I'm adding a value to the list, 			
			pathlist.insert(length - 1, " ") # insert an empty value
			ShaderPaths = string.join(pathlist, "|")
			ShaderPath.val = "Enter New Path"
			ShaderPathList.append("")
		else:
			ShaderPath.val = ShaderPathList[ShaderPathMenu.val - 1]
			# I've chosen a path from the list
			# so, I need to test the path here, and display an error message if it's not right
			if (os.path.exists(ShaderPath.val)):
				iterateShaders()
			else:
				error_state = 1
				error_message = "Invalid search path!"
				print "Something wicked happened!"
	elif evt == evt_path_rename:
		# I renamed the path here 
		ShaderPathList[ShaderPathMenu.val - 1] = ShaderPath.val 
		pathlist = string.split(ShaderPaths, "|")
		pathlist[ShaderPathMenu.val - 1] = ShaderPath.val
		ShaderPaths = string.join(pathlist, "|")
		# test for a valid path, and bail if not valid
		if os.path.exists(ShaderPath.val):
			iterateShaders()
		else:
			# display error value here
			error_state = 1
			error_message = "Invalid search path!"
			print "Something wicked happened!"
			
	elif evt == evt_preview_button: #PreviewButton
		InsertCodeHere = 1
	elif evt == evt_restore_button: #RestoreButton
		ResetParams()		
	elif evt >= evt_shader_parm_modify and evt < evt_file_browse:
		# make sure the data value for the parameter is stored in the paramvalues array
		# determine the affected control
		idx = evt - 100
		# the test: Assign the value of the editor (from parameditors) to paramvalues
		ptype = parameditortypes[idx]
		if ptype in ("float", "string", "array", "file"):
			paramvalues[idx] = parameditors[idx].val # array is actually a string
		if ptype == "color":		
			paramlist = []
			paramlist.append(parameditors[idx][0].val)
			paramlist.append(parameditors[idx][1].val)
			paramlist.append(parameditors[idx][2].val)
			paramlist.append(parameditors[idx][3].val)
			paramvalues[idx] = paramlist
		if ptype == "vector" or ptype == "point" or ptype == "normal":
			paramlist = []
			paramlist.append(parameditors[idx][0].val)
			paramlist.append(parameditors[idx][1].val)
			paramlist.append(parameditors[idx][2].val)
			paramlist.append(parameditors[idx][3].val)
			paramvalues[idx] = paramlist
		if ptype == "matrix":
			paramlist.append(parameditors[idx][0].val)
			paramlist.append(parameditors[idx][1].val)
			paramlist.append(parameditors[idx][2].val)
			paramlist.append(parameditors[idx][3].val)
			paramlist.append(parameditors[idx][4].val)
			paramlist.append(parameditors[idx][5].val)
			paramlist.append(parameditors[idx][6].val)
			paramlist.append(parameditors[idx][7].val)
			paramlist.append(parameditors[idx][8].val)
			paramlist.append(parameditors[idx][9].val)
			paramlist.append(parameditors[idx][10].val)
			paramlist.append(parameditors[idx][11].val)
			paramlist.append(parameditors[idx][12].val)
			paramlist.append(parameditors[idx][13].val)
			paramlist.append(parameditors[idx][14].val)
			paramlist.append(parameditors[idx][15].val)
			paramvalues[idx] = paramlist
		if ptype in ("projection", "space", "colorspace"):
			paramvalues[idx] = parameditors[idx].val		

	if evt > evt_file_browse:
		# must store data here
		# actual target index is
		DestIndex = evt - evt_file_browse - evt_shader_parm_modify
		# now that I have a destination index
		Blender.Window.FileSelector(fileselect, 'Choose any file')
	Blender.Redraw()

def dirselect(pathname):
	global ShaderPaths, ShaderPathMenu, ShaderPath
	# use the registry here to generalize this function
	# file selection callback here.						
	idx = ShaderPathMenu.val # get the selected path
	pathname = os.path.dirname(pathname)		
	namelist = string.split(ShaderPaths, "|")
	namelist[idx - 1] = pathname
	ShaderPaths = string.join(namelist, "|")
	# update the string value of the string control
	ShaderPath.val = pathname
	ShaderPathList[idx - 1] = pathname
	iterateShaders()
	#Register(master_gui, master_event, master_bevent)
	
def fileselect(pathname):
	global DestIndex, paramvalues
	# everything should be find here, simply bring back the data
	# so now get the filename
	# I probably want to use the absolute returned path
	# thus 
	paramvalues[DestIndex] = pathname
	

def addShaderPath(pathname):
	global ShaderPaths, ShaderPath				
	idx = ShaderPathMenu.val # get the selected path
	pathname = os.path.dirname(pathname)	
	namelist = string.split(ShaderPaths, "|")
	namelist[idx - 1] = pathname
	ShaderPaths = string.join(namelist, "|")
	# update the string value of the string control
	ShaderPathMenu.val = pathname
	iterateShaders()
	
def ResetParams():
	# reset the shader parameters for the shader in question
	print "Weird! WTF is this here for?"
	
def listCurrentShaders():
	print "\nSurface Shaders\n"
	for x in shaderssurface:
		print x[0]
	print "\nDisplacement Shaders\n"
	for x in shadersdisp:
		print x[0]
	print "\nVolume Shaders\n"
	for x in shadersvolume:
		print x[0]
	print "\nLight Shaders\n"
	for x in shaderslight:
		print x[0]
	print "\nImager Shaders\n"
	for x in shadersimager:
		print x[0]

def setShaderList():
	global CurrentShaderList, ShaderNames	
	slist = []
	for x in CurrentShaderList:
		print x[1]
		# here, roll the shader names out of the shader list 
		# and process into the shader menu 
		 # in CGKit, the shader name is index 1 in the list
		slist.append(x[1])
	ShaderNames = string.join(slist, "|")

def iterateShaders():
	global ShaderPath, CurrentShader, CurrentShaderList, ShaderSurface, ShadersLight, ShadersVolume, ShadersDisplacement, ShadersImager
	# reset the global shaders lists
	ShadersSurface = []
	ShadersLight = []
	ShadersVolume = []
	ShadersDisplacement = []
	ShadersImager = []		
	path = ShaderPath.val.strip()
	files = os.listdir(path)
	# determine the renderer	
	for x in files:
		if (x.endswith(".sl")):					
			# outx = os.path.splitext(x)			
			parms = getShaderParams(x)			
			if parms == "none":
				print "Shader error for shader ", x
			else:
				print "Got parms for shader ", x
				print parms
				# let's do the sorting here			
				if (parms[0][0] == "surface"):
					ShadersSurface.append(parms[0])
				elif (parms[0][0] == "imager"):
					ShadersImager.append(parms[0])
				elif (parms[0][0] == "displacement"):
					ShadersDisplacement.append(parms[0])
				elif (parms[0][0] == "volume"):
					ShadersVolume.append(parms[0])
				elif (parms[0][0] == "light"):
					ShadersLight.append(parms[0])	
	CurrentShaderList = ShadersSurface
	setShaderList()

def getShaderDimensions(thisshader):
	global scrolllayout
	# print "Shader data is", thisshader
	scrolllayout = []
	# list items 0 and 1 are shader name and type
	idx = 0		
	for dat in thisshader[2]: 
		if not idx == 0 or not idx == 1:
			if dat[2] == "float":
				scrolllayout.append(20)
			if dat[2] == "color" or dat[2] == "vector" or dat[2] == "point" or dat[2] == "normal":
				scrolllayout.append(45)
			if dat[2] == "string":
				scrolllayout.append(20)
			if dat[2] == "matrix":
				scrolllayout.append(60)
		idx = idx + 1

def getShaderParams(selshader):
	global ShaderPath
	maxpath = os.path.join(ShaderPath.val.strip(), selshader)
	print "Current Shader: ", selshader
	try:
		parms = cgkit.slparams.slparams(maxpath) 
	except cgkit.slparams.SyntaxError:
		# the shader's source-code was incomplete, don't include it.
		parms = "none"
	return parms
		
	
def saveShader():
	# this is a stub function to save the shader	
	# as called by the save/discard/cancel dialog
	# will be updated shortly once I figured
	# out where I'm saving configured materials
	print "Hello world!"