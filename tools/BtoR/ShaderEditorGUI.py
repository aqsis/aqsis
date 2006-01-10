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
# strings "map" and "tex" 
####################################################################

import Blender
from Blender.BGL import *
from Blender.Draw import *
import cgkit.slparams

# shader editor variables	
Materials = Create(1)
ShaderTypeMenu = Create(1)
Material = Create('SH:')
ShaderPath = Create('Path:')
ShaderMenu = Create(1)
ShaderPathMenu = Create(1)
ShaderTypes = "Surface |Displacement |Light |Volume "
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
controlllist = []

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
evt_shader_parm_modify = 15

# Shader Editor
def shader_draw():
	global Materials, ShaderTypeMenu, DeleteMaterial, Material, ShaderPath, DeleteShaderPath, BrowseButton, ShaderMenu, ShaderPathMenu, PreviewButton, RestoreButton

	glClearColor(0.753, 0.753, 0.753, 0.0)
	glClear(GL_COLOR_BUFFER_BIT)
	glColor3f(0.337, 0.486, 0.639)
	glRecti(8, 440, 663, 471)
	glColor3f(0.400, 0.541, 0.682)
	glRecti(8, 408, 663, 439)

	glColor3f(1.000, 1.000, 1.000)
	glRasterPos2i(8, 532)
	Text('Material:')
	glRasterPos2i(8, 500)
	Text('Shader Type:')
	glRasterPos2i(272, 532)
	Text('Shader Path:')
	glRasterPos2i(208, 500)
	Text('Shader:')

	Button('X', evt_material_delete, 240, 520, 23, 23, 'Delete Material')
	Button('X', evt_path_delete, 568, 520, 23, 23, 'Delete Shader Path')
	Button('Browse', evt_browse_button, 592, 520, 71, 23, '')
	Button('Preview Material', evt_preview_button, 432, 488, 111, 23, '')
	Button('Restore Defaults', evt_restore_button, 552, 488, 111, 23, '')

	Material = String(CurrentMaterial, evt_material_rename, 80, 520, 159, 23, Material.val, 512, '')
	ShaderPath = String(CurrentShader, evt_path_rename, 368, 520, 199, 23, ShaderPath.val, 512, '')

	Materials = Menu(MaterialsNames, evt_material_select, 64, 520, 20, 23, Materials.val, 'Current Material')
	ShaderTypeMenu = Menu(ShaderTypes, evt_type_select, 88, 488, 103, 23, ShaderTypeMenu.val, 'Shader Type')
	ShaderMenu = Menu(ShaderNames, evt_shader_select, 256, 488, 167, 23, ShaderMenu.val, 'Shader')
	ShaderPathMenu = Menu(ShaderPaths, evt_path_select, 344, 520, 23, 23, ShaderPathMenu.val, 'Shader Search Path')
	if Material.val:

		if (CurrentShaderList): 
			ShaderMenu = Menu(ShaderNames, evt_sel_shader,  10,  230, 200,  20, ShaderMenu.val)				
			# working dynamic area: upperBound = 225, lowerbound = 10, leftbound = 10, rightbound = 600					
			myshader = CurrentShaderList[ShaderMenu.val - 1]				
			count = 0							
			currenty = 220
			idx = 0
			# I need the dimensions data for this shader			
			#pdb.set_trace()
			getShaderDimensions(myshader)
			for x in myshader:															
				if count > 1:	# if not name, or shader type...					
					idx = count - 2					
					eventidx = evt_shader_data_modify + idx					
					if haveValues == False:					
						# haveValues controls value initialization of the parameter stack.												
						parameditors.append("") # assign the default value of the shader to the parameter editor stack						
						paramvalues.append(x[6]) # I don't know if this is neccessary now						
						thisVal = x[6] # yank the default value into thisVal for use below 
						parameditortypes.append(x[2])
						if x[2] == "color" or x[2] == "point" or x[2] == "vector" or x[2] == "normal":
							thisVal.insert([5]) 
					else:
						# we've got the values initialized, so thisVal should be the value from the paramvalues list
						thisVal = paramvalues[idx] # should be broken out already, so it's just a matter of yanking from the paramvalues list															
						parameditors[idx] = thisVal # I'm not sure I need this
					if count >= currentitem and (currenty - scrolllayout[idx - 2]) > 10:													
						# so now if this item is displayed, 								
						#pdb.set_trace()												
						if x[2] == "float": 								
							if isinstance(thisVal, float):
								finit = thisVal
							else:
								finit = thisVal.val
							currenty = currenty - 22							
							parameditors[idx] = Number(x[4], eventidx, 10, currenty, 200, 20, finit, -10, 10, x[4])														

						if x[2] == "string":
							if isinstance(thisVal, basestring):
								sinit = thisVal								
							else:							
								sinit = thisVal.val	
							if sinit == "none":
								sinit = ""
							currenty = currenty - 22
							parameditors[idx] = String(x[4] + ": ", eventidx, 10, currenty, 250, 20, sinit, 50, x[4])

						if x[2] == "color":
							currenty = currenty - 47
							# For a color editor, we don't want to use paramvalues directly
							# we want to setup 4 seperate items that are tracked (i.e. an list within the that element) 																				
							controllist = thisVal # this should be x[3]		
							if not isinstance(controllist[1], float):
								red = controllist[1].val
								green = controllist[2].val
								blue = controllist[3].val
							else:
								red = controllist[1]
								green = controllist[2]
								blue = controllist[3]								
							ctype = [] # this list is the container for the color model names, e.g. RGB, HSV, HSL
							if isinstance(controllist[0], basestring): # it's a string, react accordingly								
								xa = controllist[0]
								if xa == "rgb":
									xval = 1											
								elif xa == "hxv":
									xval = 2
								elif xa == "hsl":
									xval = 3								
							else: # I know the color model's been set							
								xval = controllist[0].val																
							if xval == 1:
								ctype = ["Red: ", "Green: ", "Blue: "]
							elif xval == 2:
								ctype = ["Hue: ", "Saturation: ", "Value: "]
							elif xval == 3:
								ctype = ["Hue: ", "Saturation: ", "Lightness: "]

							glColor3f(1, 1, 1)		
							glRasterPos2i(10, currenty + 30)
							Text(x[0])									
							controllist[0] = Menu("rgb |hsv |hsl ", eventidx, 10, currenty, 150, 25, xval, "Color Model")							
							controllist[1] = Slider(ctype[0], eventidx, 160, currenty + 30 , 200, 15, red, 0, 1, 0, ctype[0])							
							controllist[2] = Slider(ctype[1], eventidx, 160, currenty + 15 , 200, 15, green, 0, 1, 0, ctype[1])
							controllist[3] = Slider(ctype[2], eventidx, 160, currenty, 200, 15, blue, 0, 1, 0, ctype[2])
							# here I want to display an image that shows the current RGB color value. I'll have to figure out some way of converting it to HSV/HSL, etc. as I go														
							glColor3f(controllist[1].val, controllist[2].val, controllist[3].val) 		
							glRectf(370, currenty, 420, currenty + 45) # worry about conversion from HSV/HSL later																				
							parameditors[idx] = controllist # now I know that parameditors[idx][0 - 4] is a color editor with associated values

						if x[2] == "point" or x[2] == "vector" or x[2] == "normal":
							currenty = currenty - 47
							# points, vectors, or normals
							# same rule applies to these parameter types, need to track 4 seperate values

							controllist = thisVal # this should be x[3]													
							if isinstance(controllist[0], basestring):								
								xa = controllist[0] # determine the coordinate space, and assign 
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
								
								xinit = controllist[1]
								yinit = controllist[2]
								zinit = controllist[3]
							else:
								xval = controllist[0].val
								xinit = controllist[1].val
								yinit = controllist[2].val
								zinit = controllist[3].val

							# how do I display the name?
							glColor3f(1, 1, 1)
							glRasterPos2i(10, currenty + 30)
							Text(x[0])
							controllist[0] = Menu("current |object |shader |world |camera |screen |raster |NDC", eventidx, 10, currenty, 150, 25, xval, "Coordinate Space")
							controllist[1] = Number("X: ", eventidx, 160, currenty + 30, 200, 15, xinit, 0, 100, "X coordinate")
							controllist[2] = Number("Y: ", eventidx, 160, currenty + 15, 200, 15, yinit, 0, 100, "Y coordinate")
							controllist[3] = Number("Z: ", eventidx, 160, currenty, 200, 15, zinit, 0, 100, "Z coordinate")									
							parameditors[idx] = controllist

						if x[2] == "matrix":
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

							controllist[0] = Number("0,0: ", eventidx, 10, currenty, 100, 20, -100, 100, aa, "")									
							controllist[1] = Number("0,1: ", eventidx, 70, currenty, 100, 20, -100, 100, ab, "")
							controllist[2] = Number("0,2: ", eventidx, 130, currenty, 100, 20, -100, 100, ac, "")
							controllist[3] = Number("0,3: ", eventidx, 190, currenty, 100, 20, -100, 100, ad, "")
							controllist[4] = Number("1,0: ", eventidx, 10, currenty - 20, 100, 20, -100, 100, ba, "")
							controllist[5] = Number("1,1: ", eventidx, 70, currenty - 20, 100, 20, -100, 100, bb, "")
							controllist[6] = Number("1,2: ", eventidx, 130, currenty - 20, 100, 20, -100, 100, bc, "")
							controllist[7] = Number("1,3: ", eventidx, 190, currenty - 20, 100, 20, -100, 100, bd, "")
							controllist[8] = Number("2,0: ", eventidx, 10, currenty - 40, 100, 20, -100, 100, ca, "")
							controllist[9] = Number("2,1: ", eventidx, 70, currenty - 40, 100, 20, -100, 100, cb, "")
							controllist[10] = Number("2,2: ", eventidx, 130, currenty - 40, 100, 20, -100, 100, cc, "")
							controllist[11] = Number("2,3: ", eventidx, 190, currenty - 40, 100, 20, -100, 100, cd, "")
							controllist[12] = Number("3,0: ", eventidx, 10, currenty - 60, 100, 20, -100, 100, da, "")
							controllist[13] = Number("3,1: ", eventidx, 70, currenty - 60, 100, 20, -100, 100, db, "")
							controllist[14] = Number("3,2: ", eventidx, 130, currenty - 60, 100, 20, -100, 100, dc, "")
							controllist[15] = Number("3,3: ", eventidx, 190, currenty - 60, 100, 20, -100, 100, dd, "")							
							parameditors[ix] = controllist	
							
				count = count +1	

			if haveValues == False:
				# reset the haveValues so I know to take my data from the paramvalues array
				haveValues = True
	return 1

def shader_event(evt, val): # shader screen mouse/keyboard events	
	global currentitem, scrolllayout
	if evt in [WHEELUPMOUSE, UPARROWKEY]: # scroll down
		if not currentitem == 1:						
			currentitem = currentitem -1								
		Draw()
	if evt in [WHEELDOWNMOUSE, DOWNARROWKEY]: # scroll up
		if not currentitem == len(scrolllayout):
			# find the last item
			currentitem = currentitem +1				
		Draw()			
	# Emergency bail out mode
	if (evt== ESCKEY and not val): 
		Exit() 
		# Display ye olde dialog here.

def shader_bevent(evt): # shader screen button events
	if evt == evt_materials_select: #Materials		
		# to avoid confusion:
		# Materials = the menu object (Blender.Draw.Menu)
		# Material = the String object (Blender.Draw.String)
		# MaterialNames = Array that tracks the actual names of the materials - Maybe this isn't neccessary		
		namelist = string.split(MaterialsNames, "|") # split up the menu string to get the selected item
		Material.val = namelist[Materials.val - 1] # change the label of the shader in the shadereditor to the one selected
		listlenght = len(namelist) # length of the list
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
		if shadertype.val == 1:			
			CurrentShaderList = ShadersSurface
		if shadertype.val == 2:			
			CurrentShaderList = ShadersDisplacement
		if shadertype.val == 3:			
			CurrentShaderList = ShadersLight
		if shadertype.val == 4:			
			CurrentShaderList = ShadersVolume			
		setShaderList()		
		haveValues = False
		parameditors = []
		paramvalues = []
		parameditortypes = []
		currentitem = 0
		
		Register(master_gui, master_event, master_bevent)

	elif evt == 1: #DeleteMaterial
		# Delete the material from the list here
		
		Register(master_gui, master_event, master_bevent)
	elif evt == 2: #DeleteShaderPath
		InsertCodeHere = 1
		Register(master_gui, master_event, master_bevent)
	elif evt == 3: #BrowseButton
		Blender.Window.FileSelector(dirselect, 'Choose any file')

	elif evt == 10: #ShaderMenu
		# shader selection...I should have to do nothing here
		# BECAUSE...the shader selected should already exist in 
		# my global variable
		# thus what I need to do...is set the current shader to the 
		# one that was just selected
		# get the scrolllayout here
		
		Register(master_gui, master_event, master_bevent)

	elif evt == 11: #ShaderPathMenu
		InsertCodeHere = 1

	elif evt == 4: #PreviewButton
		InsertCodeHere = 1

	elif evt == 5: #RestoreButton
		ResetParams()		
	Blender.Redraw()

def dirselect(pathname):
	global ShaderPaths, ShaderPath
	# use the registry here to generalize this function
	# file selection callback here.						
	idx = ShaderPathMenu.val # get the selected path
	pathname = os.path.dirname(pathname)		
	namelist = string.split(ShaderPaths, "|")
	namelist[idx - 1] = pathname
	ShaderPaths = string.join(namelist, "|")
	# update the string value of the string control
	ShaderPathMenu.val = pathname
	iterateShaders()
	Register(master_gui, master_event, master_bevent)

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
		# here, roll the shader names out of the shader list 
		# and process into the shader menu 
		sshaderx = x[1] # in CGKit, the shader name is index 1 in the list
		slist.append(sshaderx)		
	ShaderNames = string.join(slist, "|")

def iterateShaders():
	global ShaderPath, CurrentShader, CurrentShaderList, ShaderSurface, ShadersLight, ShadersVolume, ShadersDisplacement, ShadersImager
	# reset the global shaders lists
	ShadersSurface = []
	ShadersLight = []
	ShadersVolume = []
	ShadersDisplacement = []
	ShadersImager = []		
	path = ShaderPathName.val.strip()
	files = os.listdir(path)
	# determine the renderer	
	for x in files:
		if (x.endswith(".sl")):					
			# outx = os.path.splitext(x)			
			getShaderParams(x)
			# let's do the sorting here			
			if (parms[0] == "surface"):
				ShadersSurface.append(parms)
			elif (parms[0] == "imager"):
				ShadersImager.append(parms)
			elif (parms[0] == "displacement"):
				ShadersDisplacement.append(parms)
			elif (parms[0] == "volume"):
				ShadersVolume.append(parms)
			elif (parms[0] == "light"):
				ShadersLight.append(parms)	
	CurrentShaderList = ShadersSurface
	setShaderList()

def getShaderDimensions(thisshader):
	global scrolllayout
	# print "Shader data is", thisshader
	scrolllayout = []
	# list items 0 and 1 are shader name and type
	idx = 0
	for dat in thisshader:
			
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
	# print "Scrolllayout is:", scrolllayout		

def getShaderParams(selshader):
	maxpath = os.path.join(ShaderPathName.val.strip(), selshader)
	parms = slparams.slparams(maxpath) 
	return parms
		
	
