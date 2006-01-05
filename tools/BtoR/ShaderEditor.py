import Blender
import string
import os
import sys
import popen2
import pdb
import CgKit


# initialized variables
globalshaderpathmenu = "Add New"
currentitem = 1
scrolllayout = []

# this is the original shader editor code that I created to dynamically generate editors for shader parameters. 
def event(evt, val):
	global currentitem, scrolllayout
	if (shadereditor.val == 1):
		if evt in [WHEELUPMOUSE, UPARROWKEY]:
			if not currentitem == 1:						
				currentitem = currentitem -1								
			Draw()
		if evt in [WHEELDOWNMOUSE, DOWNARROWKEY]:
			if not currentitem == len(scrolllayout):
				# find the last item
				currentitem = currentitem +1				
			Draw()
	if (evt == ESCKEY and not val): 
		Exit()


def shader_event():

	if (evt == evt_shader_select):						# Add shader menu
		namelist = string.split(sshader, "|") # the names of the shaders gets hacked up into a list
		surfaceshadername.val = namelist[surfaceshader.val - 1] # change the label of the shader in the shadereditor to the one selected
		listlenght = len(namelist) # length of the list
		if (surfaceshader.val == listlenght): # if the length of the list of shader names is the same as the surfaceshader menu add another item to the menu
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
			namelist.insert(listlenght - 1 , 'Surface.%s'%numbername)
			sshader = string.join(namelist, "|")
			surfaceshadername.val = namelist[surfaceshader.val - 1]
			saveShaderName(1)
			resetShaderVal()
			shadermenu.val = 1
		else:
			# by rights here, if I've selected a shader, I should be loading it from the file. Why am I not loading it?
			#saveload(0,0,'shader')
			print "hi"
		saveShaderName(1)
		shadernametemp = surfaceshadername.val
		Register (gui, event, bevent)
		
	if ((evt == evt_shader_delete) & (surfaceshadername.val != "")):				# Surface shader delete
		deleteshader = surfaceshader.val
		tempselected = selectedname
		namelist = string.split(sshader, "|")
		namelist.remove(surfaceshadername.val)
		surfaceshadername.val = namelist[surfaceshader.val - 1]
		namelist = map(str, namelist)
		sshader = string.join(namelist, "|")
		listlenght = len(namelist)
		if (surfaceshader.val == listlenght):
			surfaceshadername.val = namelist[surfaceshader.val - 2]
		surfaceshadername.val = ""
		surfaceshader.val = surfaceshader.val - 1
		saveShaderName(1)
		saveload(0,0,'shader')
		for objects in Blender.Object.Get():
			if (objects.getType() == "Mesh"):
				resetObject()
				selectedname = objects.name
				saveload(0,0,'object')
				if deleteshader == objectshadermenu.val:
					convertbm.val = 0
				elif deleteshader < objectshadermenu.val:
					objectshadermenu.val = objectshadermenu.val - 1
				saveload(1,0,'object')
		selectedname = tempselected
		saveload(0,0,'object')
		Register (gui, event, bevent)

	if (evt == evt_shader_rename):								
		if (len(surfaceshadername.val) != 0): 					
			namelist = string.split(sshader, "|")
			namelist[surfaceshader.val - 1]	= surfaceshadername.val
			sshader = string.join(namelist, "|")
			saveload(1,0,'shader')
			saveShaderName(1)
			Register (gui, event, bevent)	
			
	if (evt == evt_shader_path_modify):		
		namelist = string.split(globalshaderpathmenu, "|")
		namelist[shaderpath.val -1] = shaderpathname.val.strip()
		globalshaderpathmenu = string.join(namelist, "|")
		Register (gui, event, bevent)
		
	if (evt == evt_shader_path_select):
		# iterate the COMPILED shaders on a given path
		# so get the shader path value		
		namelist = string.split(globalshaderpathmenu, "|") # split the shader paths				
		shaderpathname.val = namelist[shaderpath.val - 1]
		listlenght = len(namelist) # length of the list		
		if (shaderpath.val == listlenght): # if the length of the list of paths is the same as the paths menu, add another item to the menu						
			namelist.insert(listlenght - 1, " ") # insert an empty value, so I can test it when I reach iterateShaders() 
			globalshaderpathmenu = string.join(namelist, "|")  #rejoins the namelist array into the a string for the menu again
			shaderpathname.val = namelist[listlenght - 1]
			#shaderpath.val = listlenght  #inserted at 
			saveload(1,0,'global') # save global options, shader search paths are global
		else:
			# what do I need to do here?
			# I need to display the selected path in the 
			# shaderpathname.val stringa
			# and then 
			shaderpathname.val = namelist[shaderpath.val -1]						
			iterateShaders()														
		Register(gui, event, bevent)		
		
	if (evt == evt_shader_type_select):				
		if shadertype.val == 1:			
			currentshaderlist = shaderssurface
		if shadertype.val == 2:			
			currentshaderlist = shadersdisp
		if shadertype.val == 3:			
			currentshaderlist = shaderslight
		if shadertype.val == 4:			
			currentshaderlist = shadersvolume			
		setShaderList()		
		haveValues = False
		parameditors = []
		paramvalues = []
		parameditortypes = []
		currentitem = 0
		
		Register(gui, event, bevent)
		
	if evt == evt_shader_path_browse:
		Blender.Window.FileSelector(dirselect, 'Choose any file')
	
	if evt > evt_shader_data_modify:
		# determine the affected control
		idx = evt - 500
		# the test: Assign the value of the editor (from parameditors) to paramvalues
		print "Parameter event index is: ", idx
		ptype = parameditortypes[idx]
		if ptype == "float" or ptype == "string":
			paramvalues[idx] = parameditors[idx].val					
		if ptype == "color":
			paramlist = []
			paramlist.append(parameditors[idx][0].val)
			paramlist.append(parameditors[idx][1].val)
			paramlist.append(parameditors[idx][2].val)
			paramlist.append(parameditors[idx][3].val)
		if ptype == "vector" or ptype == "point" or ptype == "normal":
			paramlist = []
			paramlist.append(parameditors[idx][0].val)
			paramlist.append(parameditors[idx][1].val)
			paramlist.append(parameditors[idx][2].val)
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
		
		Register(gui, event, bevent)
	if (evt == evt_sel_shader):
		# I've chosen one of the shaders in the shader list, reinitialize all the variables used
		haveValues = False 
		parameditors = []
		paramvalues = []
		parameditortypes = [] # this might be useless
		currentitem = 0
		#saveload(1, 0, 'shader')		
		Register (gui, event, bevent)

Register (gui, event, bevent)
		
def dirselect(pathname):
	global globalshaderpathmenu, shaderpath
	# file selection callback here.						
	idx = shaderpath.val # get the selected path
	pathname = os.path.dirname(pathname)		
	namelist = string.split(globalshaderpathmenu, "|")
	namelist[idx - 1] = pathname
	globalshaderpathmenu = string.join(namelist, "|")
	# update the string value of the string control
	shaderpathname.val = pathname
	iterateShaders()
	Register(gui, event, bevent)

def shaderseditor():
	# construct the shader editor screen
	

	global menuval, currentshadertell, prmanshadertell, bmrtshadertell, entropyshadertell, dlshadertell, aqsisshadertell, pixieshadertell
	global surfaceshader, sshader, surfaceshadername, shadertype, shaderpath, shadertypes, globalshaderpathmenu, shaderpathname
	global shadermenu, currentshaderlist, paramvalues, haveValues, parameditors, parameditortypes, gotEnvVars 
	
	# test for environment varaibles that might have a shader path in them
	if gotEnvVars == False:
		shaderspath = string.split(os.environ["SHADERS"], ";")	
		shaderspath.append("Add New")
		globalshaderpathmenu = string.join(shaderspath, "|")
					
	# fill globalshaderpathmenu with the output of this
	shaderpath = Menu(globalshaderpathmenu, evt_shader_path_select, 320, 270, 20, 20, shaderpath.val, "Shader Search Paths")
			
	if (shaderpath.val != 0):				
		if (len(globalshaderpathmenu) > 7):
			shaderpathname = String("PATH: ", evt_shader_path_modify, 340, 270, 190, 20, shaderpathname.val, 256, "Shader Search Path")	
			shaderdeletebutton = Button("X", evt_shader_path_delete, 530, 270, 20, 20, "Delete Shader Path")
			shaderbrowsebutton = Button("Browse", evt_shader_path_browse, 550, 270, 60, 20, "Select shader path")
			shadertype = Menu(shadertypes, evt_shader_type_select, 200, 270, 100, 20, shadertype.val, "Shader Type")			
	if gotEnvVars == False:
		iterateShaders()
		gotEnvVars = True
					
	if (menuval.val == 1):
		currentshadertell = prmanshadertell		
	elif (menuval.val == 2):
		currentshadertell = bmrtshadertell
	elif (menuval.val == 3):
		currentshadertell = entropyshadertell
	elif (menuval.val == 4):
		currentshadertell = dlshadertell
	elif (menuval.val == 5):
		currentshadertell = aqsisshadertell
	elif (menuval.val == 6):
		currentshadertell = pixieshadertell			
	surfaceshader = Menu(sshader, evt_shader_select, 10,  270, 20,  20, surfaceshader.val, "Choose shader")			
	if (surfaceshader.val != 0):
		surfaceshadername = String("SH: ", evt_shader_rename, 30,  270, 140, 20, surfaceshadername.val, 200)
		Button("X", evt_shader_delete, 171, 270, 20,  20, "Delete shader")		

	if surfaceshadername.val:
		
		Button("Test Render Shader", evt_shader_testrender,  220, 230, 150,  20)
		Button("Set default values", evt_shader_set_default_values, 380, 230, 150,  20)			
		if (currentshaderlist): 
			shadermenu = Menu(shader, evt_sel_shader,  10,  230, 200,  20, shadermenu.val)				
			# working dynamic area: upperBound = 225, lowerbound = 10, leftbound = 10, rightbound = 600					
			myshader = currentshaderlist[shadermenu.val - 1]				
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
						paramvalues.append(x[3]) # AND the parameter value stack	
						thisVal = x[3] # yank the default value into thisVal for use below 
						parameditortypes.append(x[2])
					else:
						# we've got the values initialized, so thisVal should be the value from the paramvalues list
						thisVal = paramvalues[idx] # should be broken out already, so it's just a matter of yanking from the paramvalues list															
						parameditors[idx] = thisVal
					if count >= currentitem and (currenty - scrolllayout[idx - 2]) > 10:													
						# so now if this item is displayed, 								
						#pdb.set_trace()												
						if x[2] == "float": 								
							if isinstance(thisVal, float):
								finit = thisVal
							else:
								finit = thisVal.val
							currenty = currenty - 22							
							parameditors[idx] = Number(x[0], eventidx, 10, currenty, 200, 20, finit, -10, 10, x[0])														
							
						if x[2] == "string":
							if isinstance(thisVal, basestring):
								sinit = thisVal								
							else:							
								sinit = thisVal.val	
							if sinit == "none":
								sinit = ""
							currenty = currenty - 22
							parameditors[idx] = String(x[0] + ": ", eventidx, 10, currenty, 250, 20, sinit, 50, x[0])
						
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
								
							ctype = []
							
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
								xa = controllist[0]
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
	
	
	
def setShaderList():
	global shader, currentshaderlist
	slist = []
	for x in currentshaderlist:
		# here, roll the shader names out of the shader list 
		# and process into the shader menu 
		sshaderx = x[0] #should be the name of the shader in question
		slist.append(sshaderx)		
	shader = string.join(slist, "|")
	
	

def iterateShaders():
	global shaderpathname, currentshaderext, shader, currentshaderlist, shaderssurface, shaderslight, shadersvolume, shadersdisp, shadersimager
	# reset the global shaders lists
	shaderssurface = []
	shaderslight = []
	shadersvolume = []
	shadersdisp = []
	shadersimager = []		
	path = shaderpathname.val.strip()
	files = os.listdir(path)
	# determine the renderer	
	for x in files:
		if (x.endswith(currentshaderext)):		
			outx = os.path.splitext(x)
			currentshaderlist.append(outx[0]) 
			parms = getShaderParams(x)
			# let's do the sorting here			
			if (parms[1] == "surface"):
				shaderssurface.append(parms)
			elif (parms[1] == "imager"):
				shadersimager.append(parms)
			elif (parms[1] == "displacement"):
				shadersdisp.append(parms)
			elif (parms[1] == "volume"):
				shadersvolume.append(parms)
			elif (parms[1] == "light"):
				shaderslight.append(parms)	
	currentshaderlist = shaderssurface
	setShaderList()
	
def getShaderDimensions(thisshader):
	global scrolllayout
	# print "Shader data is", thisshader
	scrolllayout = []
	# list items 0 and 1 are shader name and type
	idx = 0
	for dat in thisshader:
		print dat		
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
	print "Scrolllayout is:", scrolllayout		

def getShaderParams(selshader):	
	# run the shader params 
	# let's assume that if I'm here, I have a valid path		
	shaderparms = []
	maxpath = os.path.join(shaderpathname.val.strip(), selshader)			
	isdefvalue = False
	havetype = False	
	shaderparms.append(selshader)
	if (os.name != "posix"):		
		command = '%s "%s"'%(currentshadertell.val, maxpath)						
		fromchild, tochild = popen2.popen2(command)		
		paramdata = fromchild.readlines()
		# print paramdata
		tochild.close()
		fromchild.close()						
		for line in paramdata:						
			if not line.strip():			
				# nothing here
				ztemp = 0
			else:
				
				# for each shader, we want to process the avilable parameters.
				# the first line should contain the type of shader it is
				linedata = line.strip().replace('"', '')
				linex = linedata.split()[0]						
				if linex == "surface" or linex == "light" or linex == "volume" or linex == "displacement" or linex == "imager":	 # test for shader type declaration										
					typex = linex	
					shaderparms.append(typex)
				else: #not the first item
					# find out where we are					
					paramline = linedata.split()
					if not isdefvalue:
						# parse the parameter name and type								
						parametername = paramline[0]																				
						if (paramline[1] == "parameter"):
							# only for Aqsis
							storageclass = paramline[2]
							storagetype = paramline[3]
						else:
							storageclass = paramline[1]
							storagetype = paramline[2]								
						isdefvalue = True 
					else:
						# parse the default value											
						if (storagetype == "float"):
							fdefvalue = float(paramline[2])
							parmset = [parametername, storageclass, storagetype, fdefvalue]
							shaderparms.append(parmset)	
												
						elif storagetype == "color" or storagetype == "vector" or storagetype == "normal" or storagetype == "point":
							odefvalue = [paramline[2].strip(""), float(paramline[3].strip("[")), float(paramline[4]), float(paramline[5].strip("]"))] 
							# pulls the coordinate space (or color space) and values
							parmset = [parametername, storageclass, storagetype, odefvalue]							
							shaderparms.append(parmset)	
							
						elif storagetype == "string":							
							sdefvalue = "none"
							#sdefvalue = paramline[2]
							parmset = [parametername, storageclass, storagetype, sdefvalue]	
							shaderparms.append(parmset)
							
						elif storagetype == "matrix":
							parmset = [parametername, storageclass, storagetype, mdefvalue]
							shaderparms.append(parmset)							
							
						isdefvalue = False 						
		return shaderparms
		