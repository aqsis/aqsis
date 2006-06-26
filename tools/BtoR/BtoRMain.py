#######################################################################
# GUI  - GUI for the material definition system
# Version 0.1a
# Author: Bobby Parker                                                                         
# This file provides the GUI for the materials system.
# The shader parameter lister part of this GUI has functionality where it attempts to guess the intended use of 
# a parameter based on its default value or name, e.g. if a parameter has the name text or texture or map, 
# then it's assumed that the parameter is for an image, thus a filename selector is presentable.
# Note that guessed parameters can be overridden via an override button provided per editor.
#
#######################################################################
import os
from btor import BtoRGUIClasses as ui
reload(ui)
import cgkit
import cgkit.rmshader
import cgkit.cgtypes
import cgkit.quadrics
from cgkit import ri as ri
from cgkit import ribexport as export
import Blender
import xml.dom.minidom
import new

def type_select(obj):
	print obj.value 

def stickyTest(obj):
	print obj.value
	
def add_new(button):
	print button.title
	print button.normalColor

def launchDialog(button):
	evt_manager.showConfirmDialog("Test Confirm Dialog", "HI! I'm a dialog! Say YES to everything I say!",  printResults, True)
	
def printResults(dialog):
	if dialog.state == dialog.OK:
		print "Everything's OK!"
	elif dialog.state == dialog.DISCARD:
		print "Toss it!"
	else:
		print "Do nuttin'!"
	

backGround = [0.65, 0.65, 0.65]

evt_manager = ui.EventManager()  # initialize the event manager GUI stuff first, since I need that for things like dialogs

class BtoRSettings: # an instance of this class should be passed to every base-level BtoR object (objects that go into the root event manager.
	# someone with 3Delight, Prman, entropy etc can add further renderer information here
	# standard setup: Renderer name: [render binary, sltell binary, texture binary, shader extension, [home env vars], [shader env vars], [texture env vars], [ display env vars],
	#				            [ procedural env vars ], [plugin env vars], [archive env vars]] 
	# assign all lists and strings to None if there's no available info or that particular renderer has no equivalent value (see Aqsis for ex. of home env vars)
	renderers = {  "AQSIS" : ["aqsis", "aqsl", "aqsltell", "teqser", "slx", None, "AQSIS_SHADER_PATH", "AQSIS_TEXTURE_PATH", "AQSIS_DISPLAY_PATH", "AQSIS_PROCEDURAL_PATH", "AQSIS_PLUGIN_PATH", "AQSIS_ARCHIVE_PATH"],
			     "BMRT" : ["bmrt", "slc", "slctell","mkmip","BMRTHOME", "SHADERS", "TEXTURES",None,None,None,None], 
			     "Pixie" : ["pixie", "sdr", "sdrinfo", "texmake", "PIXIEHOME", "SHADERS", None, None, None, None, None],
			     "3Delight":["renderdl", "shaderdl", "shaderinfo", "tdlmake", "DELIGHT",  "DL_SHADERS", None, "DL_DISPLAYS", None, None, None],
			     "RenderDotC":["wrendrdc", "shaderdc", "soinfo", "texdc", "RDCROOT", "SHADERS", "MAPS", "DISPLAYS", "PROCEDURALS", None, "ARCHIVES"] }
	cpp = "cpp"
	# what else does BtoRSettings need to know? How about a projects directory? MRUs? TBD later I suppose
	
	
	def __init__(self):		
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
			elif isinstance(dict[key], ui.EventManager):
				self.evt_manager = dict[key]
				# event manager instance
				
		self.getSettings(None)
		screen = Blender.Window.GetAreaSize()		
		self.editor = ui.Panel(10, screen[1] - 225, 600, 400, "GlobalSettings", "Global BtoR Settings:", None, False)
		self.editor.addElement(ui.Label(5, 30, "Renderer:", "Renderer:", self.editor, False))
		rendererList = self.renderers.keys()
		
		self.rendererMenu = ui.Menu(self.editor.get_string_width("Renderer:", 'normal') + 12, 30, 100, 20, "Renderer:", rendererList, self.editor, True)				
		self.rendererMenu.setValue(rendererList.index(self.renderer))
		
		self.useSlParams = ui.ToggleButton(180, 30, 250, 20, "Use CGKit for Shader Parameters?", "Use CGKit for Shader Parameters?", 'normal', self.editor, True)
		self.useSlParams.setValue(self.use_slparams)		
		
		self.useSlParams.upColor = [158, 178, 181, 255]
		self.useSlParams.normalColor = [158, 178, 181, 255]
		self.useSlParams.downColor = [88, 108, 111, 255]
		self.useSlParams.hoverColor = [68, 88, 91, 255]
		
		width = self.editor.get_string_width("Procedurals:", 'normal') + 30
		
		# search paths label
		self.editor.addElement(ui.Label(5, 65, "Search Paths:", "Search Paths:", self.editor, False))
		
		# binary search path
		self.editor.addElement(ui.Label(25, 90, "Binaries:", "Binaries:", self.editor, False))		
		self.binarypath = ui.TextField(width, 90, self.editor.width - (45 + width), 20, "BinaryPathText", self.binaryPath, self.editor, True)		
		self.binaryBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, 90, 30, 20, "BinaryBrowse", "...", 'normal', self.editor, True)		
		self.binaryBrowse.release_functions.append(self.browse)
		
		# shader search paths
		self.editor.addElement(ui.Label(25, 115, "Shaders:", "Shaders:", self.editor, False))		
		self.shaderpaths = ui.TextField(width, 115, self.editor.width - (45 + width), 20, "ShaderPathText", self.shaderPathList, self.editor, True)		
		self.shaderBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, 115, 30, 20, "ShaderBrowse", "...", 'normal', self.editor, True)		
		self.shaderBrowse.release_functions.append(self.browse)
		
		# texture search paths
		self.editor.addElement(ui.Label(25, 140, "Textures:", "Textures:", self.editor, False))
		self.texturepaths = ui.TextField(width, 140, self.editor.width - (45 + width), 20, "TexturePathText", self.texturePathList, self.editor, True)		
		self.textureBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, 140, 30, 20, "TextureBrowse", "...", 'normal', self.editor, True)		
		self.textureBrowse.release_functions.append(self.browse)
		
		# archive search paths
		self.editor.addElement(ui.Label(25, 165, "Archives:", "Archives:", self.editor, False))		
		self.archivepaths = ui.TextField(width, 165, self.editor.width - (45 + width), 20, "ArchivePathText", self.archivePathList, self.editor, True)		
		self.archiveBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, 165, 30, 20, "ArchiveBrowse", "...", 'normal', self.editor, True)		
		self.archiveBrowse.release_functions.append(self.browse)
		
		# procedural search paths
		self.editor.addElement(ui.Label(25, 190, "Procedurals:", "Procedurals:", self.editor, False))		
		self.proceduralpaths = ui.TextField(width, 190, self.editor.width - (45 + width), 20, "ProceduralsPathText", self.proceduralPathList, self.editor, True)		
		self.proceduralBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, 190, 30, 20, "ProceduralBrowse", "...", 'normal', self.editor, True)		
		self.proceduralBrowse.release_functions.append(self.browse)
		
		# display search paths
		self.editor.addElement(ui.Label(25, 215, "Displays:", "Displays:", self.editor, False))
		self.displaypaths = ui.TextField(width, 215, self.editor.width - (45 + width), 20, "DisplaysPathText", self.displayPathList, self.editor, True)		
		self.displayBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, 215, 30, 20, "DisplayBrowse", "...", 'normal', self.editor, True)
		self.displayBrowse.release_functions.append(self.browse)
		
		# file paths label
		self.editor.addElement(ui.Label(10, 240, "File Paths:", "File Paths:", self.editor, False))
		
		# output path
		self.editor.addElement(ui.Label(25, 260, "Output Path:", "Output Path:", self.editor, False))
		self.outputpath = ui.TextField(width, 260, self.editor.width - (45 + width), 20, "OutputPathText", self.outputPath, self.editor, True)
		self.outputBrowse = ui.Button(self.outputpath.x + self.outputpath.width + 2, 260, 30, 20, "OutputBrowse", "...", 'normal', self.editor, True)
		self.outputBrowse.release_functions.append(self.browse)
		
		self.cancelButton = ui.Button(5, self.editor.height - 45, 60, 20, "Cancel", "Cancel", 'normal', self.editor, True)		
		self.okButton = ui.Button(self.editor.width - (60 + 15), self.editor.height - 45, 60, 20, "OK", "OK", 'normal', self.editor, True)
		
		self.okButton.release_functions.append(self.saveSettings)
		self.cancelButton.release_functions.append(self.getSettings)
		self.cancelButton.release_functions.append(self.update)
		self.okButton.release_functions.append(self.close)
		self.cancelButton.release_functions.append(self.close)
		
		
		# get the first shader path in the search list
		paths = self.shaderpaths.getValue().split(";")
		
		
		if self.use_slparams:
			self.getShaderSourceList(paths[0]) 
		else:
			self.getCompiledShaderList(paths[0]) 
			
	def getShaderList(self, path):
		
		if self.use_slparams:
			self.getShaderSourceList(path) 
		else:
			self.getCompiledShaderList(path) 

			
	def getShaderParams(self, shader):	
		# run the shader params 
		# let's assume that if I'm here, I have a valid path		
		shaderparms = []
		maxpath = os.path.join(self.searchPaths.getValue(), shader)			
		isdefvalue = False
		havetype = False	
		type = None
		# the shader name is already passed in		
		if (os.name != "posix"): # must deal with linux here at some point
			command = '%s "%s"'%(self.renderers[self.settings.renderer][4], maxpath)						
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
						# shaderparms.append(typex)
					else: #not the first item
						# find out where we are		
						parameter = []
						paramline = linedata.split()
						if not isdefvalue:
							# parse the parameter name and type, ignoring AOV for the moment, and figure out if something is an array or not as well							
							parametername = paramline[0]																				
							if (paramline[1] == "parameter"):
								# for aqsis & 3delight sltells								
								storageclass = paramline[2]																											
								storagetype = paramline[3]
							else:
								storageclass = paramline[1]
								storagetype = paramline[2]								
							isdefvalue = True 
						else: 	
							if storagetype.find("[") > -1: # i have an array
									index = storagetype[3].index("[")
									# get the array length
									arrayLen = storagetype[3][index + 1:len(storagetype[3]) - 1]
									# now trim the array Length off of the string.
									storagetype = storagetype[:index]
							else: 
								arrayLen = None
								
							# Modify parameter returns to be in line with how CgKit.slparams returns them so I only have to worry about one type of data structure
							if storagetype == "float":									
								# on the float value type
								fdefvalue = float(paramline[2])
								# output specifier is always blank here, leave it to the user to configure this								
								parmset = ('', storageclass, storagetype, arrayLen, parametername, None, fdefvalue) 
								shaderparms.append(parmset)	
							
							elif storagetype == "color" or storagetype == "vector" or storagetype == "normal" or storagetype == "point":
								# the default value for colors, vectors, points, or normals will be two values, so expand this section 
								odefvalue = [paramline[2].strip(""), float(paramline[3].strip("[")), float(paramline[4]), float(paramline[5].strip("]"))] 
								# pulls the coordinate space (or color space) and values
								parmset = ('', storageclass, storagetype, arrayLen, parametername, odefvalue[0], [odefvalue[1], odefvalue[2], odefvalue[3]])
								# parmset = [parametername, storageclass, storagetype, odefvalue]							
								shaderparms.append(parmset)	
								
							elif storagetype == "string":							
								sdefvalue = paramline[2].strip("")
								if sdefvalue == "":
									sdefvalue = None
								# sdefvalue = None
								# sdefvalue = paramline[2]
								parmset = ('', storageclass, storagetype, arrayLen, parametername, None, sdefvalue) 
								# parmset = [parametername, storageclass, storagetype, sdefvalue]	
								shaderparms.append(parmset)
								
							elif storagetype == "matrix":
								# how would the default value be formatted?
								mdefvalue = [] # this is a nice array, row-major
								for x in range(2, len(paramline)):
									mdefvalue.append(paramline[x])
									
								parmset = ('', storageclass, storagetype, arrayLen, parametername, None, sdefvalue) 
								# parmset = [parametername, storageclass, storagetype, mdefvalue]
								shaderparms.append(parmset)							
									
							isdefvalue = False 
						
		shaderdata = (typex, shader, shaderparms) # this tuple should be exactly like what cgkit.slparams returns now
		return shaderdata
			
	def getShaderSourceParams(self, file):
		print file
		try:
			parms = cgkit.slparams.slparams(file) 
		except cgkit.slparams.SyntaxError:
			# the shader's source-code was incomplete, don't include it.
			parms = None 
		return parms
			
	def getCompiledShaderList(self, shaderPath):
		# reset the shaders lists, since I'm looking at a new path		
		self.shadersSurface = []
		self.shadersLight = []
		self.shadersVolume = []
		self.shadersDisp = []
		self.shadersImager = []						
		if os.path.exists(shaderPath):
			files = os.listdir(shaderPath)		
			for file in files:
				if (file.endswith(self.renderers[self.renderer][4])):		
					outx = os.path.splitext(file)
					currentshaderlist.append(outx[0]) 
					parms = self.getShaderParams(file)
					# let's do the sorting here			
					if (parms[1] == self.stype):
						self.shaders.append(parms)
					elif self.type == None:
						if (parms[1] == "surface"):
							self.shadersSurface.append(parms)
						elif (parms[1] == "imager"):
							self.shadersImager.append(parms)
						elif (parms[1] == "displacement"):
							self.shadersDisp.append(parms)
						elif (parms[1] == "volume"):
							self.shadersVolume.append(parms)
						elif (parms[1] == "light"):
							self.shadersLight.append(parms)	
		else:
			# display error dialog
			error_state = 1
			error_message = "Invalid search path!"
			print error_message
	
	def getShaderSourceList(self, shaderPath): 
		# maybe add some iterative thing here that pops up a progress bar
		# reset the shader lists
		self.shadersSurface = []
		self.shadersDisplacement = []
		self.shadersImager = []
		self.shadersVolume = []
		self.shadersLight = []
		
		path = shaderPath.strip()
		# test for a valid path, and bail if not valid
		if os.path.exists(path):

			# reset the global shaders lists
			self.shadersSurface = []
			self.surfaceFiles = []
			self.shadersLight = []
			self.lightFiles = []
			self.shadersVolume = []
			self.volumeFiles = []
			self.shadersDisplacement = []
			self.dispFiles = []
			self.shadersImager = []		
			self.imagerFiles = []

			files = os.listdir(path)
			# determine the renderer	
			for file in files:
				if (file.endswith(".sl")):
					
					parms = self.getShaderSourceParams(path + os.sep + file)					
					if parms == None:
						print "Error getting parameters for shader ", file
					else:
						print "Got parms for shader ", file
						if (parms[0][0] == "surface"):
							self.shadersSurface.append(parms[0])
							self.surfaceFiles.append(file)
						elif (parms[0][0] == "imager"):
							self.shadersImager.append(parms[0])
							self.imagerFiles.append(file)
						elif (parms[0][0] == "displacement"):
							self.shadersDisplacement.append(parms[0])
							self.dispFiles.append(file)
						elif (parms[0][0] == "volume"):
							self.shadersVolume.append(parms[0])
							self.volumeFiles.append(file)
						elif (parms[0][0] == "light"):
							self.shadersLight.append(parms[0])
							self.lightFiles.append(file)
				# self.makeSourceMenus() # is this even neccessary?
				
		else:
			# display error dialog
			error_state = 1
			error_message = "Invalid search path!"
			print "Something wicked happened!"
	
	def getSettings(self, button):
		# let's try the Registry!
		try:
			settings = Blender.Registry.GetKey("BtoR", True) # check everywhere for BtoR settings.
		except:
			settings = False
			
		if settings: # settings were found, btor was setup previously, let's get the information
			self.haveSetup = True
			# I've got the information
			self.renderer = settings["renderer"] # string
			self.binaryPath = settings["Binaries"] # string
			self.shaderPathList = settings["Shaders"] # string
			self.texturePathList = settings["Textures"] # string
			self.displayPathList = settings["Displays"] # string
			self.archivePathList = settings["Archives"] # string
			self.proceduralPathList = settings["Procedurals"] # string
			self.outputPath = settings["outputPath"] # string
			self.use_slparams = settings["use_slparams"] # boolean		
				
		else:			
			self.haveSetup = False
			# no info		
			self.renderer = "AQSIS" # this should eventually change to use settings stored in either the blender registry, or something else
			# default to aqsis, since we've got no actual information
			if os.name <> "posix" or os.name <> "mac":
				self.binaryPath = 'C:\Program Files\Aqsis\\bin'
				self.shaderPathList = "C:\Program Files\Aqsis\Shaders" 
				self.texturePathList = "C:\Program Files\Aqsis\Textures"
				self.displayPathList = "C:\Program Files\Aqsis\Displays"
				self.archivePathList = "C:\Program Files\Aqsis\Archives"
				self.proceduralPathList = "C:\Program Files\Aqsis\Procedurals"
				self.outputPath = "C:\temp"
			else:
				self.binaryPath = "/usr/aqsis/bin"
				self.shaderPathList = "/usr/aqsis/shaders/"
				self.texturePathList = "/usr/aqsis/textures"
				self.displayPathList = "/usr/aqsis/displays"
				self.archivePathList = "/usr/aqsis/archives"
				self.proceduralPathList = "/usr/aqsis/procedurals"
				self.outputPath = "/temp/"
			self.use_slparams = True # set this to true by default
			
			# surf the environment and get the shader paths from the env vars for this renderer
			# step one, test the $HOME/shaders thing
			# news flash: Set this up on failure of the settings load				
			if self.renderers[self.renderer][5] <> None:			
				# the renderer root should have no need of multiple search paths
				if os.getenv(self.renderers[self.renderer][5]) <> None:
					self.shaderPathList = self.shaderPathList + (os.getenv(self.renderers[self.renderer][5]) + os.sep + "shaders" + os.sep) + os.pathsep
			if self.renderers[self.renderer][6] <> None:
				if os.getenv(self.renderers[self.renderer][6]) <> None:
					#if self.renderers[self.renderer][6].find
					self.shaderPathList = self.shaderPathList + os.getenv(self.renderers[self.renderer][6]) # that should be that.
						
		
	
	def saveSettings(self, button):
		# save the settings to the registry here. Mostly it's simple stuff, will probably expand, 
		# but for a first release, this should work OK
		print self.rendererMenu.getValue()
		settings = { "renderer" : self.rendererMenu.getValue(), 
				"Binaries" : self.binarypath.getValue(),
				"Shaders" : self.shaderpaths.getValue(),
				"Archives" : self.archivepaths.getValue(),
				"Textures" : self.texturepaths.getValue(),
				"Procedurals" : self.proceduralpaths.getValue(),
				"Displays" : self.displaypaths.getValue(),
				"outputPath" : self.outputpath.getValue(),
				"use_slparams" : self.useSlParams.getValue() }
		
		if self.haveSetup:
			Blender.Registry.RemoveKey("BtoR", True)
		Blender.Registry.SetKey("BtoR", settings, True)
		self.haveSetup = True
		
	def update(self):
		self.rendererMenu.setValue(self.rendererMenu.menu.index(self.renderer))
		self.useSlParams.setValue(self.use_slparams)
		self.binarypath.setValue(self.binaryPath)
		self.shaderpaths.setValue(self.shaderPathList)
		self.texturepaths.setValue(self.texturePathList)
		self.displaypaths.setValue(self.displayPathList)
		self.proceduralpaths.setValue(self.proceduralPathList)
		self.archivepaths.setValue(self.archivePathList)
		self.outputpath.setValue(self.outputPath)
		

	
	def cancel(self, button):
		self.getSettings() # recover the stuff from the registry
		self.update()
		
	def browse(self, button):
		self.activeButton = button
		Blender.Window.FileSelector(self.select, 'Choose any file')
	
	def close(self, button):
		self.evt_manager.removeElement(self.editor) # remove myself from the event manager
		
	def select(self, file):
		path = os.path.dirname(file)
		if self.activeButton == self.binaryBrowse:
			self.binaryPath = path		
		elif self.activeButton == self.shaderBrowse:
			self.shaderPathList = path		
		elif self.activeButton == self.textureBrowse:
			self.texturePathList = path
		elif self.activeButton == self.displayBrowse:
			self.displayPathList = path
		elif self.activeButton == self.proceduralBrowse:
			self.proceduralPathList = path
		elif self.activeButton == self.archiveBrowse:
			self.archivePathList = path
		elif self.activeButton == self.outputBrowse:
			self.outputPath = path
		self.update()
		
	
	def getEditor(self):
		return self.editor	
		

		
		
class SceneSettings:
	def __init__(self):
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
			elif isinstance(dict[key], ui.EventManager):
				self.evt_manager = dict[key]
				# event manager instance
				
		
		self.editor = ui.Panel(200, 500, 500, 300, "Scene Settings:", "Scene Settings:", None, False)
		
		self.saveButton = ui.Button(self.editor.width - 160, 30, 150, 25, "Save Scene Setup", "Save Scene Setup", 'normal', self.editor, True)
		self.saveButton.release_functions.append(self.saveSceneData)
		
		self.objectButton = ui.Button(10, 80, 150, 25, "Object Properties:", "Object Properties:", 'normal', self.editor, True)
		self.objectButton.release_functions.append(self.showObjectEditor)
		
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.release_functions.append(self.close)
		
	
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def getEditor(self):
		return self.editor
		

		
	def saveSceneData(self, obj):
		
		# get the material list
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			if isinstance(dict[key], MaterialList):
				self.materials = dict[key]
				# material list
		newdoc =xml.dom.minidom.Document()
		root = newdoc.createElement("BtoR")
		newdoc.appendChild(root)
		
		mat = self.materials.saveMaterials(newdoc)
		root.appendChild(mat)
		
		# self.saveSceneSettings(root_element)
		try:
			text = Blender.Text.Get("BtoRXML")
			found = True
		except NameError: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
			# not found, create a new one
			text = Blender.Text.New("BtoRXML")
			found = False
			
		text.clear()
		# doesn't matter, it's getting written
		text.write(root.toprettyxml()) # that should be that.
			
	def showObjectEditor(self, obj):
		self.evt_manager.addElement(self.objectEditor)
		self.evt_manager.raiseElement(self.objectEditor)

class ObjectEditor:
		
	def __init__(self):
		
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
			elif isinstance(dict[key], ui.EventManager):
				self.evt_manager = dict[key]
				# event manager instance
				
		self.selected_objects = []
		# master panel
		self.editorPanel = ui.Panel(200, 200, 200, 200, "Object Editor:", "Object Editor:", None, False)
		
		
		self.editorPanel.addElement(ui.Label(10, 30, "Object Name:", "Object Name:", self.editorPanel, False))
		self.objectName = ui.Label(15 + self.editorPanel.get_string_width("Object Name:", 'normal'), 30, "None Selected", "None Selected", self.editorPanel, True)
		self.objectMenu = ui.Menu(self.objectName.x, 30, 150, 30, "Object menu", ["None Selected"], self.editorPanel, True)
		self.objectMenu.isVisible = False
		
		self.editorPanel.addElement(ui.Label(10, 80, "Object Type:", "Object Type:", self.editorPanel, False))
		self.objectType = ui.Label(15 + self.editorPanel.get_string_width("Object Name:", 'normal'), 80, "No_type", "No_type", self.editorPanel, True)
		
		
		self.object_output_options = ["Mesh", "Renderman Primitive", "RA Proxy", "RA Procedural"]
		self.light_options = ["basic", "Light Shader"]		
		self.mesh_output_options = ["basic", "SubDiv"]
		
		self.evt_manager.draw_functions.append(self.getSelected)
		
	def getEditor(self):
		return self.editorPanel
		
	def selectionChanged(self):
		# let's try the Registry!
		try:
			changed = Blender.Registry.GetKey("BtoR_Space", False)
			selection = changed["changed"]
			changed["changed"] = False
			
		except:
			selection = False
		return selection
		
	def getSelected(self):
		if self.editorPanel in self.evt_manager.elements: #only if this instance is up and running.
			if self.selectionChanged():
				objects = Blender.Object.GetSelected()
				# get the first object
				if len(objects) < 1:
					self.objectName.isVisible = True
					self.objectMenu.isVisible = False

					self.objectName.setValue("None Selected")
					self.objectType.setValue("No Type")
				elif len(objects) > 1:
					self.objectName.isVisible = False
					self.objectMenu.isVisible = True
					self.obj_menu = [] # reset the menu
					self.obj_types = []
					for obj in objects:
						self.obj_menu.append(obj.getName())
						
					self.objects = objects
					self.objectMenu.re_init(self.obj_menu)
				else:
					self.objectName.isVisible = True
					self.objectMenu.isVisible = False
					obj = objects[0]
					self.objectName.setValue(obj.getName())
					self.objectType.setValue(obj.getType())
	
class Material:	
	def __init__(self, material, parent):		
		# material should be an RMMaterial	
		dict = globals()		
		keys = dict.keys()
		for key in keys:					
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
			elif isinstance(dict[key], ui.EventManager):
				self.evt_manager = dict[key]
				# event manager instance
				
				
		if material <> None:
			self.material = material
			render = True
		else:			
			self.material = cgkit.rmshader.RMMaterial()
			render = False
		
		# panel init		
		screen = Blender.Window.GetAreaSize()	
		self.editorPanel = ui.Panel(225, screen[1] - 10, 300, 350, "shaders", "Material Settings:", None, False)
		self.editorPanel.addElement(ui.Label(5, 32, "Material Name:", "Material Name:", self.editorPanel, True))
		
		# material name
		fieldOffset = self.editorPanel.get_string_width("Material Name:", 'normal') + 12
		self.materialName = ui.TextField(fieldOffset, 32, self.editorPanel.width - (fieldOffset + 5), 18, "MaterialName", self.material.name, self.editorPanel, True)
		self.materialName.key_functions.append(self.rename)
		
		# self.editorPanel.addElement(self.materialName)
		self.surface = Shader(self.material.surface, "surface", self)
		# surface shader button
		self.editorPanel.addElement(ui.Label(5, 65, "Surface Shader:", "Surface Shader:", self.editorPanel, True))
		self.surfaceShader = ui.Button(160, 65, 130, 20, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		self.editorPanel.addElement(self.surfaceShader)
		self.surfaceShader.release_functions.append(self.showSurfaceEditor)
		
		self.displacement = Shader(self.material.displacement, "displacement", self)
		# diplacement shader button
		self.editorPanel.addElement(ui.Label(5, 90, "Displacement Shader:", "Displacement Shader:", self.editorPanel, True))
		self.displacementShader = ui.Button(160, 90, 130, 20, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		self.editorPanel.addElement(self.displacementShader)
		self.displacementShader.release_functions.append(self.showDispEditor)
		
		self.volume = Shader(self.material.interior, "volume", self)
		# volume shader button
		self.editorPanel.addElement(ui.Label(5, 115, "Volume Shader:", "Volume Shader:", self.editorPanel, True))
		self.volumeShader = ui.Button(160, 115, 130, 20, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		self.editorPanel.addElement(self.volumeShader)
		self.volumeShader.release_functions.append(self.showVolumeEditor)
		
		
		# Color
		self.editorPanel.addElement(ui.Label(5, 160, "Color", "Color:", self.editorPanel, True))
		
		self.editorPanel.addElement(ui.Label(55, 160, "R", "R", self.editorPanel, False))
		self.color_red = ui.TextField(70, 160, 35, 20, "Red", self.material.color()[0], self.editorPanel, True)
		self.color_red.update_functions.append(self.updateColor)
		
		self.editorPanel.addElement(ui.Label(110, 160, "G", "G", self.editorPanel, False))
		self.color_green = ui.TextField(130, 160, 35, 20, "Green", self.material.color()[1], self.editorPanel, True)
		self.color_green.update_functions.append(self.updateColor)
		
		self.editorPanel.addElement(ui.Label(175, 160, "B", "B", self.editorPanel, True))		
		self.color_blue = ui.TextField(190, 160, 35, 20, "Blue", self.material.color()[2], self.editorPanel, True) 
		self.color_blue.update_functions.append(self.updateColor)
		
		self.color = ui.ColorButton(230, 160, 25, 20, "Color", [float(self.color_red.getValue()) * 255, float(self.color_green.getValue()) * 255, float(self.color_blue.getValue()) * 255, 255], self.editorPanel, True)
		self.editorPanel.addElement(self.color)
		self.color.picking = False
		self.color.picker.ok_functions.append(self.updateColorFields)
		
		# opacity
		self.editorPanel.addElement(ui.Label(5, 190, "Opacity", "Opacity:", self.editorPanel, True))
		
		self.editorPanel.addElement(ui.Label(55, 190, "R", "R", self.editorPanel, False))
		self.opacity_red = ui.TextField(70, 190, 35, 20, "Red", 1.0, self.editorPanel, True)
		self.opacity_red.update_functions.append(self.updateOpacity)
		
		self.editorPanel.addElement(ui.Label(110, 190, "G", "G", self.editorPanel, False))
		self.opacity_green = ui.TextField(130, 190, 35, 20, "Green", 1.0, self.editorPanel, True)
		self.opacity_green.update_functions.append(self.updateOpacity)
		
		self.editorPanel.addElement(ui.Label(175, 190, "B", "B", self.editorPanel, True))
		self.opacity_blue = ui.TextField(190, 190, 35, 20, "Blue", 1.0, self.editorPanel, True) 
		self.opacity_blue.update_functions.append(self.updateOpacity)
		
		self.opacity = ui.ColorButton(230, 190, 25, 20, "Color", [255, 255, 255, 255], self.editorPanel, True)
		self.editorPanel.addElement(self.opacity)		
		self.opacity.picking = False
		self.opacity.picker.ok_functions.append(self.updateOpacityFields)
		
		# displacement bound
		self.editorPanel.addElement(ui.Label(5, 220, "DisplacementBound", "Displacement Bound:", self.editorPanel, True))
		fieldOffset = self.editorPanel.get_string_width("Displacement Bound:", 'normal') + 12
		self.dispBound = ui.TextField(fieldOffset, 220, 25, 20, "DispBound", 2, self.editorPanel, True)
		self.editorPanel.addElement(self.dispBound)				
		
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		#close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editorPanel)
		self.close_button.release_functions.append(self.close)
		
		self.preview_button = ui.Button(15, 275, 135, 25, "Preview Material:", "Preview Material:", 'normal', self.editorPanel, True)
		self.preview_button.release_functions.append(self.renderPreview)
		
		# preview image
		self.image = Blender.Image.New(self.material.name, 128, 128, 24)
		self.editorPanel.addElement(ui.Image(165, 215, 128, 128, self.image, self.editorPanel, False))
		
		
		# editor button that displays the material editor
		self.editorButton = ui.ToggleButton(0, 0, 180, 65, self.material.name, self.material.name, 'normal', parent, True)
		self.editorButton.shadowed = False
		self.editorButton.outlined = True
		self.editorButton.textlocation = 1
		
		self.editorButton.release_functions.append(self.showMaterial)		
		self.editorButton.image = ui.Image(120, 5, 56, 56,  self.image, self.editorButton, False)
		
		self.selectorButton = ui.ToggleButton(0, 0, 180, 65, self.material.name, self.material.name, 'normal', None, False)
		self.selectorButton.image = ui.Image(120, 5, 56, 56, self.image, self.selectorButton, False)
			
		if render:
			# I've got a prebuilt material, so setup the names and what-not.
			if self.surface.shader.shaderName() != None:
				self.surfaceShader.title = self.surface.shader.shaderName()
			if self.displacement.shader.shaderName() != None:
				self.displacementShader.title = self.displacement.shader.shaderName()
			if self.volume.shader.shaderName() != None:
				self.volumeShader.title = self.volume.shader.shaderName()
			self.renderPreview(None)
			
		
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		
	def rename(self, obj, key, val):
		self.editorButton.title = self.materialName.getValue()
		
	def getEditor(self):
		return self.editorPanel
		
	def getMaterialButton(self):
		return self.editorButton
		
	def getSelectorButton(self):
		return self.selectorButton
		
	def showMaterial(self, button):
		self.evt_manager.addElement(self.editorPanel)		
		
	def showSurfaceEditor(self, button):		
		self.evt_manager.addElement(self.surface.getEditor())
		# there should be no update neccessary since the shader REFERENCE should do all the work
	
	def showDispEditor(self, button):
		self.evt_manager.addElement(self.displacement.getEditor())
		# there should be no update neccessary since the shader REFERENCE should do all the work
		
	def showVolumeEditor(self, button):
		self.evt_manager.addElement(self.volume.getEditor())
		# there should be no update neccessary since the shader REFERENCE should do all the work

	def draw(self):
		self.editorPanel.draw()
		
	def updateColorFields(self, obj):
		color = self.color.getValue()
		
		# color values in the text boxes are assigned via ye olde renderman method, i.e. 0-1
		self.color_red.setValue(float(float(color[0]) / 255))
		self.color_green.setValue(float(float(color[1]) / 255))
		self.color_blue.setValue(float(float(color[2]) / 255))
		
		self.material._color = (float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue()))
		
	def updateOpacityFields(self, obj):
		color = self.opacity.getValue()
		
		# color values in the text boxes are assigned via ye olde renderman method, i.e. 0-1
		self.opacity_red.setValue(float(float(color[0]) / 255))
		self.opacity_green.setValue(float(float(color[1]) / 255))
		self.opacity_blue.setValue(float(float(color[2]) / 255))
		
		self.material._opacity = (float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue()))
	
	def updateColor(self, obj):
		if obj.isEditing == False:
			# convert to a float value
			try:
				if float(self.color_red.getValue()) > 1.0:
					red = 1.0
				else:
					red = float(self.color_red.getValue())
					
				if float(self.color_green.getValue()) > 1.0:
					green = 1.0
				else:
					green = float(self.color_green.getValue())
				
				if float(self.color_blue.getValue()) > 1.0:
					blue = 1.0
				else:
					blue = float(self.color_blue.getValue())
			except: 
				print "hey, I want a number!"
				
			rgb = self.editorPanel.rgb2Float([red, green, blue])
			self.color.setValue(rgb) 			
			
			self.material._color = (float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue()))
	
	def updateOpacity(self, obj):
		if obj.isEditing == False:
			# convert to a float value
			try:
				if float(self.opacity_red.getValue()) > 1.0:
					red = 1.0
				else:
					red = float(self.opacity_red.getValue())
					
				if float(self.opacity_green.getValue()) > 1.0:
					green = 1.0
				else:
					green = float(self.opacity_green.getValue())
				
				if float(self.opacity_blue.getValue()) > 1.0:
					blue = 1.0
				else:
					blue = float(self.opacity_blue.getValue())
			except: 
				print "hey, I want a number!"
				
			rgb = self.editorPanel.rgb2Float([red, green, blue])
			self.opacity.setValue(rgb) 
			
			self.material._opacity = (float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue()))
	
	def renderPreview(self, button):
		# here, render a preview image using the current material
		# step 1 is to find a place to put this, so consult the BtoR settings to see where that is.
		filename = os.path.normpath(self.settings.outputPath + os.sep + "material" + self.material.name + ".tif")
		ri.RiBegin(self.settings.renderer)
		# get the shader options
		paths = self.settings.shaderpaths.getValue().split(";")
		# setup the shader paths
		for path in paths:
			ri.RiOption("searchpath", "shader", "&:" + path[0])
			
		ri.RiFormat(128, 128, 1)
		ri.RiAttribute("displacementbound", "coordinatesystem", "shader", "sphere", 1.5)
		ri.RiDisplay(filename, "file", "rgba")
		ri.RiTranslate(0, 0, 2.7)
		ri.RiWorldBegin()
		ri.RiAttributeBegin()
		ri.RiCoordinateSystem("world")
		ri.RiAttributeEnd()
		ri.RiTransformBegin()
		ri.RiLightSource("ambientlight", "lightcolor", [0.151, 0.151, 0.151])
		ri.RiLightSource("distantlight", "lightcolor", [0.8, 0.8, 0.8], "from", [1, 1.5, -1], "to", [0, 0, 0], "intensity", 1)
		ri.RiLightSource("distantlight", "lightcolor", [0.2, 0.2, 0.2], "from", [-1.3, -1.2, -1.0], "to", [0, 0, 0], "intensity", 1)
		ri.RiTransformEnd()
		ri.RiAttributeBegin() 
		# ok here I need to get the material settings out of the material object somehow.
		# check the value of the surface shader
		if self.surface.shader.shadername != None:	
			self.surface.updateShaderParams()
			parms = dict()
			
			for parm in self.material.surface.shaderparams:
				# get the param and stuff into my dict				
				parms[parm[1]] = self.material.surface.__dict__[parm[1] + "_slot"].getValue()			
				ri.RiDeclare(parm[1], parm[0])
			# now	
			ri.RiSurface(self.material.surface.shadername, parms)
		if self.displacement.shader.shadername != None:				
			self.displacement.updateShaderParams()
			parms = dict()
			for parm in self.material.displacement.shaderparams:
				parms[parm[1]] = self.material.displacement.__dict__[parm[1] + "_slot"].getValue()
				ri.RiDeclare(parm[1], parm[0])
			ri.RiDisplacement(self.material.displacement.shadername, parms)
			
		if self.volume.shader.shadername != None:			
			parms = dict()
			self.volume.updateShaderParams()
			for parm in self.material.interior.shaderparams:
				parms[parm[1]] = self.material.interior.__dict__[parm[1] + "_slot"].getValue()
				ri.RiDeclare(parm[1], parm[0])
				
			ri.RiAtmosphere(self.material.interior.shadername, parms)
		color = self.material.color()
		opacity = self.material.opacity()
		ri.RiTransformBegin()
		ri.RiRotate(45, 45, 45, 0)
		ri.RiColor([color[0], color[1], color[2]])		
		ri.RiOpacity([opacity[0], opacity[1], opacity[2]])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		ri.RiSphere(0.75, -0.75, 0.75, 360)
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
		#ri.RiAttributeBegin()
		#ri.RiTranslate(0,0,0.5)
		#ri.RiScale(1, 1, 1)
		#ri.RiColor([1, 1, 1])		
		#ri.RiSurface("plastic")
		#ri.RiPatch("bilinear", "P", [-1, 1, 0, 1, 1, 0, -1, -1, 0, 1 -1, 0])
		#ri.RiAttributeEnd()
		ri.RiWorldEnd()
		ri.RiEnd()
		
		# here I want to load the image I just rendered into the material and draw it.		
		self.image.setFilename(filename)
		self.image.reload()
		
	
	
class Shader:
	# what does this need to know?
	# the shader path for one thing...and the shader type...and the 
	# current shader itself
	# the shader, as an RMShader object doesn't know its own type probably, so I have to supply it
	# this needs to at some point construct the set of parameters that belongs to the selected shader.
	def __init__(self, shader, stype, material):
		dict = globals()		
		keys = dict.keys()				

		for key in keys:					
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
			elif isinstance(dict[key], ui.EventManager):
				self.evt_manager = dict[key]
				# event manager instance
				
		if shader <> None:
			self.shader = shader
		else:
			self.shader = None # no selection, we'll be assigning it as we go.
			
		searchpaths = self.settings.shaderpaths.getValue().split(";")		
		self.stype = stype
		self.material = material # parent material
		
		# initialize the panel
		# find the center of the screen...again
		screen = Blender.Window.GetAreaSize()
		
		self.editorPanel = ui.Panel(225, screen[1] - 370, 350, 500, "ShaderEditor", "Shader Parameters:" + self.stype, None, False)
		self.editorPanel.dialog = True
		# on the shader panel, we need the shader search path (with the ability to add to the list)
		self.editorPanel.addElement(ui.Label(5, 30, "SearchPathLabel", "Shader Search Path:", self.editorPanel, True))
		offset = self.editorPanel.get_string_width("Shader Search Path:", 'normal') + 12
		self.searchPaths = ui.Menu(offset, 30, 210, 20, "SearchPath",  searchpaths, self.editorPanel, True)
		self.searchPaths.release_functions.append(self.listShaders)
		
		# next we need the shader type, so I know what I'm working with.
		self.editorPanel.addElement(ui.Label(5, 50, "ShaderType", "Shader Type: " + stype, self.editorPanel, False))
		
		self.editorPanel.addElement(ui.Label(5, 80, "Shader", "Shader:", self.editorPanel, False))
		
		# the starting shader type is Surface
		self.shaders = []
		self.makeShaderMenu()
				
		self.shader_menu = ui.Menu(self.editorPanel.get_string_width("Shader:", 'normal') + 12, 80, 150, 20, "Shaders", self.shadersMenu, self.editorPanel, True)
		self.shader_menu.release_functions.append(self.selectShader)
		# now that THAT nastiness is done...
		self.scroller = ui.ScrollPane(5, 115, self.editorPanel.width - 10, self.editorPanel.height - 125, "Param Scroller", "parmScroller", self.editorPanel, True)
		self.scroller.normalColor = [185, 185, 185, 255]
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		#close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editorPanel)
		self.close_button.release_functions.append(self.close)
		
		if self.shader.shaderparams != None:
			self.setupParamEditors()
		
		print self.shader.shaderName
		if self.shader.shaderName() != None:
			# set the shader name to the correct shader name in the menu
			sIndex = self.shadersMenu.index(self.shader.shaderName())
			self.shader_menu.setValue(sIndex)
		#else:
		#	self.selectShader(None)
		# otherwise, I'll be selecting a shader at the outset...
		
	def listShaders(self, obj):
		self.settings.getShaderList(self.searchPaths.getValue())
		# reset the shader list menu
		self.shaders = []
		self.makeShaderMenu()
		self.shader_menu.re_init(self.shadersMenu)
		
		
	def close(self, button):		
		self.evt_manager.removeElement(self.editorPanel)
		self.material.renderPreview(None)
		
	def getEditor(self):		
		return self.editorPanel
		
	def makeShaderMenu(self):
		self.shadersMenu = []
		self.shadersMenu.append("None Selected")
		if self.stype == "surface":
			self.shaders = self.settings.shadersSurface
			for shader in self.settings.shadersSurface:
				self.shadersMenu.append(shader[1])
		elif self.stype == "displacement":
			self.shaders = self.settings.shadersDisplacement
			for shader in self.settings.shadersDisplacement:				
				self.shadersMenu.append(shader[1])
		elif self.stype == "volume":
			self.shaders = self.settings.shadersVolume
			for shader in self.settings.shadersVolume:
				self.shadersMenu.append(shader[1])
		elif self.stype == "light":
			self.shaders = self.settings.shadersLight
			for shader in self.settings.shadersLight:
				self.shadersMenu.append(shader[1])
		elif self.stype == "imager":
			self.shaders = self.settings.shadersImager
			for shader in self.settings.shadersImager:
				self.shadersMenu.append(shader[1])
				
	def selectShader(self, button):
		# select the shader in question and then setup the params for it.
		# ditch the current parameter editors	
		
		self.scroller.clearElements()	
		self.shader = None	# clear all the old data.
		if self.shader_menu.getSelectedIndex() > 0:			
			shaderID = self.shader_menu.getSelectedIndex() - 1
			if self.stype == "surface":
				self.shaderParms = self.settings.shadersSurface[shaderID]
			elif self.stype == "displacement":
				self.shaderParms = self.settings.shadersDisplacement[shaderID]
			elif self.stype == "volume":
				self.shaderParms = self.settings.shadersVolume[shaderID]
			elif self.stype == "light":
				self.shaderParms = self.settings.shadersLight[shaderID]
			elif self.stype == "imager":
				self.shaderParms = self.settings.shadersImager[shaderID]				
				
			# initialize a new RMShader object
			if self.settings.use_slparams: 
				if self.stype == "surface":
					file = self.settings.surfaceFiles[shaderID]
				elif self.stype == "displacement":				
					file = self.settings.dispFiles[shaderID]
				elif self.stype == "volume":
					file = self.settings.volumeFiles[shaderID]
				elif self.stype == "light":
					file = self.settings.lightFiles[shaderID]
				elif self.stype == "imager":
					file = self.settings.imagerFiles[shaderID]
					
				# get the search path from the settings object, and use that to find the shader source
				# hopefully it's the same as the shader name
				path = self.settings.shaderPathList
				self.shader = cgkit.rmshader.RMShader(self.searchPaths.getValue() + os.sep + file)
				self.setupParamEditors()
			else:
				self.shader = cgkit.rmshader.RMShader()
				self.initShaderParams(self.shaderParms, self.shader)				
				self.setupParamEditors()
				 
		if self.shader == None:
			self.shader = cgkit.rmshader.RMShader() # blank shader
				
		if self.material <> None:
			if self.stype == "surface":
				self.material.surfaceShader.title = self.shader_menu.getValue()
				self.material.material.surface = self.shader
			elif self.stype == "volume":
				self.material.volumeShader.title = self.shader_menu.getValue()
				self.material.material.interior = self.shader
			elif self.stype == "displacement":
				self.material.displacementShader.title = self.shader_menu.getValue()
				self.material.material.displacement = self.shader
			elif self.stype == "light":
				self.material.lightShader.title = self.shader_menu.getValue()
			elif self.stype == "imager":
				self.material.imagerShader.title = self.shader_menu.getValue()
		
			
			
	
	def setupParamEditors(self):
		# iterate the current shader and setup the parameter editors for it.
		# here I'm using the rmshader object so react accordingly
		# iterate the slots in the RMShader object
		# delete the current set of elements in the scroller object		

		count = 0
		for param in self.shader.shaderparams:
			normalColor = [216, 214, 220, 255]
			#if count % 2:
			#	normalColor = [190, 190, 190, 255]
			p_type = param[0].split()	
			# get the default value from the shader
			iParams = self.shader.params()
			defValue = iParams[(param[0] + " " + param[1])]
			if p_type[1] == "float":
				# create a float editor
				self.scroller.addElement(ui.FloatEditor(0, 0, self.scroller.width - 30, ui.FloatEditor.height, param[1], defValue, self.scroller, False))
			elif p_type[1] == "string":
				self.scroller.addElement(ui.TextEditor(0, 0, self.scroller.width - 30, ui.TextEditor.height, param[1], defValue, self.scroller, False))
			elif p_type[1] == "color":
				color = []				
				color.append(defValue[0])
				color.append(defValue[1])
				color.append(defValue[2])
				self.scroller.addElement(ui.ColorEditor(0, 0, self.scroller.width - 30, ui.ColorEditor.height, param[1], color, self.scroller, False))
			elif p_type[1] == "point":	
				self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param[1], defValue, self.scroller, False))
			elif p_type[1] == "vector":
				self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param[1], defValue, self.scroller, False))
			elif p_type[1] == "normal":
				self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param[1], defValue, self.scroller, False))
			elif p_type[1] == "matrix":
				self.scroller.addElement(ui.MatrixEditor(0, 0, self.scroller.width - 30, ui.MatrixEditor.height, param[1], defValue, self.scroller, False))
			#self.scroller.elements[count].normalColor = normalColor			
			count = count + 1
		self.scroller.invalid = True
		self.scroller.currentItem = 1
				
	def initShaderParams(self, params, shader):		
		convtypes = {"float":"double",
				"string":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		for param in params:
			atts = param.attributes
			switch = atts["type"].nodeValue
			if switch == "color":
				val = cgkit.cgtypes.vec3(float(atts["value_a"].nodeValue), float(atts["value_b"].nodeValue), float(atts["value_c"].nodeValue))
				# no color space support and most blender people won't have a clue what this means anyway
			elif switch == "float":						
				val = cgkit.cgtypes.double(float(atts["value"].nodeValue))
			elif switch == "string":
				val = atts["value"].nodeValue
			elif switch == "matrix":
				val = cgkit.cgtypes.mat4(float(atts["value_0"].nodeValue), 
										float(atts["value_1"].nodeValue), 
										float(atts["value_2"].nodeValue), 
										float(atts["value_3"].nodeValue), 
										float(atts["value_4"].nodeValue), 
										float(atts["value_5"].nodeValue), 
										float(atts["value_6"].nodeValue), 
										float(atts["value_7"].nodeValue), 
										float(atts["value_8"].nodeValue), 
										float(atts["value_9"].nodeValue), 
										float(atts["value_10"].nodeValue), 
										float(atts["value_11"].nodeValue), 
										float(atts["value_12"].nodeValue), 
										float(atts["value_13"].nodeValue), 
										float(atts["value_14"].nodeValue), 
										float(atts["value_15"].nodeValue))				
			
			elif switch == "point" or switch == "vector" or switch == "normal":
				val = cgkit.cgtypes.vec3(float(atts["value_a"].nodeValue), float(atts["value_b"].nodeValue), float(atts["value_c"].nodeValue))
			# slot this attribute into the shader
			shader.createSlot(atts["name"].nodeValue, convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here		
	
	def updateShaderParams(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		for element in self.scroller.elements:
			p_type = element.paramtype
			name = element.param_name
			if p_type == "float" or p_type == "string": # lo, all my single-variable types
				self.shader.__dict__[name + "_slot"].setValue(self.scroller.elements[index].getValue()) 
				
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = self.scroller.elements[index].getValue()
				self.shader.__dict__[name + "_slot"].setValue(cgkit.cgtypes.vec3(val[0], val[1], val[2]))
				
			elif p_type == "matrix": # matrix types				
				val = self.scroller.elements[index].getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				self.shader.__dict__[name + "_slot"].setValue(matrix)
			
			index = index + 1


class MainUI:
	# this is the main UI object, the "main menu" of sorts from which all things spring.
	def __init__(self):
		# find my settings
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
				
			elif isinstance(dict[key], ui.EventManager):
				self.evt_manager = dict[key]
				# UI event manager
				
			elif isinstance(dict[key], MaterialList):
				self.materials = dict[key]
				# material list
				
			elif isinstance(dict[key], SceneSettings):
				self.scenesettings = dict[key]
				# scene setup
				
			elif isinstance(dict[key], ObjectEditor):
				self.objecteditor = dict[key]
				# scene setup
				
		screen = Blender.Window.GetAreaSize()		
		self.editor = ui.Panel(10, screen[1] - 10, 200, 205, "BtoR", "Blender to Renderman:", None, False)		
		self.editor.outlined = True
		
		self.globalSettingsButt = ui.Button(50, 30, 100, 25, "Settings", "Global Settings", 'normal', self.editor, True) # renderer settings and paths
		self.globalSettingsButt.release_functions.append(self.showGlobal)
		
		self.sceneSettingsButt = ui.Button(50, 65, 100, 25, "Scene Settings", "Scene Settings", 'normal', self.editor, True) # scene settings, DOF, shadows, etc.
		self.sceneSettingsButt.release_functions.append(self.showSceneSettings)
		
		self.objectButt = ui.Button(50, 100, 100, 25, "Objects", "Objects", 'normal', self.editor, True) # launches a low-quality preview, preview shading rate determined by global settings
		self.objectButt.release_functions.append(self.showObjectEditor)
		
		self.renderButt = ui.Button(50, 135, 100, 25, "Render", "Render", 'normal', self.editor, True)
		
		self.materialsButt = ui.Button(50, 170, 100, 25, "Materials", "Materials", 'normal', self.editor, True)	
		self.materialsButt.release_functions.append(self.showMaterials)
		
		
	def showGlobal(self, button):
		editor = self.settings.getEditor()
		editor.dialog = True
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showMaterials(self, button):
		editor = self.materials.getEditor()		
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def doRender(self, button):
		pass
		
	def showObjectEditor(self, button):
		editor = self.objecteditor.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showSceneSettings(self, button):
		editor = self.scenesettings.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
	
	def getEditor(self):
		return self.editor
		
		
class MaterialList:
	def __init__(self):
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			if isinstance(dict[key], BtoRSettings):				
				self.settings = dict[key]
				# BtoRSettings object, globals
			elif isinstance(dict[key], ui.EventManager):		
				self.evt_manager = dict[key]
				# event manager instance
			elif isinstance(dict[key], SceneSettings):
				self.scenesettings = dict[key]
				
	
		screen = Blender.Window.GetAreaSize()
		
		# self.picker.x = self.absX
		self.editor = ui.Panel(10, screen[1] - 225, 200, 400, "Materials", "Materials:", None, False)
		self.selector = ui.Panel(10, screen[1] - 225, 200, 375, "Materials", "Materials:", None, False)
		
		self.add_button = ui.Button(0, 370, 99, 30, "Add New", "Add New", 'normal', self.editor, True)
		self.add_button.shadowed = False
		self.add_button.cornermask = 8
		self.add_button.radius = 10
		self.add_button.release_functions.append(self.showAddDialog)
		
		self.delete_button = ui.Button(101, 370, 99, 30, "Delete", "Delete", 'normal', self.editor, True)
		self.delete_button.shadowed = False
		self.delete_button.cornermask = 4 
		self.delete_button.radius = 10
		self.delete_button.release_functions.append(self.showDeleteDialog)
		
		self.scroller = ui.ScrollPane(0, 27, 200, 340, "MaterialScroller", "Scroller", self.editor, True)
		self.scroller.normalColor = [185,185,185,255]
		
		self.sel_scroller = ui.ScrollPane(0, 27, 200, 340, "MaterialScroller", "Scroller", self.selector, True)
		self.sel_scroller.normalColor = [185,185,185,255]
			
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.release_functions.append(self.close)
			
		self.materials = self.getSavedMaterials()

		for material in self.materials:
			# create a button editor for each material			
			#self.scroller.elements.append(material.getMaterialButton())
			material.editorButton.release_functions.append(self.toggleOn)
			button = material.getSelectorButton()
			button.parent = self.sel_scroller
			self.sel_scroller.addElement(button)
			
			
		self.stickyToggle = False
			
	def toggleOn(self, button):
		# sticky toggle keeps the button set, no matter what 		
		if self.stickyToggle:
			for element in self.scroller.elements:
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
				for element in self.scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.name 
			
	def getEditor(self):
		return self.editor
	
	def getSelector(self):
		return self.selector
	
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def showAddDialog(self, button): # coroutine, open the naming dialog
		self.evt_manager.showSimpleDialog("New Material:", "New Material Name:", "Material %d" % len(self.materials), self.addNew)
		
	def addNew(self, dialog):
		if dialog.state == dialog.OK:
			# assign the name
			# create the new RMMaterial
			rm_mat = cgkit.rmshader.RMMaterial(dialog.getValue())
			material = Material(rm_mat, self.scroller)
			self.materials.append(material)			
			self.scroller.lastItem = 0
						
		material.editorButton.release_functions.append(self.toggleOn)
		# self.evt_manager.removeElement(dialog)
	
	
		
	
				
	def showDeleteDialog(self, button):
		self.evt_manager.showConfirmDialog("Delete Material?", "Delete selected material?",  self.deleteMaterial, False)
	def deleteMaterial(self, dialog):
		if dialog.state == dialog.OK:
			# find the selected material by chasing through the elements in the scroller and deleting the one that's found.
			for element in self.scroller.elements:
				if element.getValue() == True:
					# find the parent material
					for  material in self.materials:
						if element == material.getMaterialButton():
							self.materials.remove(material)
							break
					self.scroller.removeElement(element)					
					parent = element.parent
					del element
					self.materials
					
					break
					
	def getSavedMaterials(self):
		# this looks for XML stored in a blender text object by the name of BtoRXMLData
		# look to see if there's an XML object here
		materials = []
		try:
			text = Blender.Text.Get("BtoRXML")
			found = True
		except NameError: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
			found = False
		if found:
			xmldat = ""
			lines = text.asLines()
			for line in lines:
				xmldat = xmldat + line
			try:				
				# parse the settings tree and see what we have for materials
				xmlsettings = xml.dom.minidom.parseString(xmldat)
				hasXMLData = True
			except xml.parsers.expat.ExpatError:
				hasXMLData = False			
			if hasXMLData:
				xmlMaterials = xmlsettings.getElementsByTagName("Material")				
				# now I have a DOM object that contains (hopefully) my materials list
				if len(xmlMaterials) > 0:			
					# iterate the materials and create the objects 
					for xmlMaterial in xmlMaterials:						
						# the RMMaterial gets created last						
						name = xmlMaterial.getAttribute("name")
						print "Material name is ", name
						surface = xmlMaterial.getElementsByTagName("Surface")
						displacement = xmlMaterial.getElementsByTagName("Displacement")
						volume = xmlMaterial.getElementsByTagName("volume")
						
						colorElement = xmlMaterial.getElementsByTagName("Color")[0]
						matColor = [float(colorElement.getAttribute("red")), float(colorElement.getAttribute("green")), float(colorElement.getAttribute("blue"))]
						
						opacityElement = xmlMaterial.getElementsByTagName("Opacity")[0]
						matOpacity = [float(opacityElement.getAttribute("red")), float(opacityElement.getAttribute("green")), float(opacityElement.getAttribute("blue"))]
						
						if len(surface) > 0:
							# new CgKit surface shader
							
							if surface[0].getAttribute("filename") == "None":
								# I don't know where the path is. Initialize a basic shader and set the slots up manually
								surfShader = cgkit.rmshader.RMShader()
								initialized = False
							else:
								if self.settings.use_slparams:
									surfShader = cgkit.rmshader.RMShader(os.path.normpath(surface[0].getAttribute("path")))
								else:
									surfShader = cgkit.rmshader.RMShader(surface[0].getAttribute("path") + os.sep + surface[0].getAttribute("filename") + self.settings.renderers[self.settings.renderer][4])
								initialized = True
								
							# setup the parameters
							
							parms = surface[0].getElementsByTagName("Param")																					
							self.populateShaderParams(parms, surfShader, initialized)
						else:
							surfShader = cgkit.rmshader.RMShader()
						
						if len(displacement) > 0:
							# new CgKit displacement Shader
							
							if displacement[0].getAttribute("filename") == "None":
								dispShader = cgkit.rmshader.RMShader()
							else:
								if self.settings.use_slparams:
									dispShader = cgkit.rmshader.RMShader(os.path.normpath(displacement[0].getAttribute("path")))
								else:
									dispShader = cgkit.rmshader.RMShader(displacement[0].getAttribute("path") + os.sep + displacement[0].getAttribute("name") + self.settings.renderers[self.settings.renderer][4])
				
								
							parms = displacement[0].getElementsByTagName("Param")
							self.populateShaderParams(parms, dispShader, initialized)
						else:
							dispShader = cgkit.rmshader.RMShader()
						
						if len(volume) > 0:
							
							if volume[0].getAttribute("filename") == "None":
								volShader = cgkit.rmshader.RMShader()
							else:
								# new CgKit volume shader
								if self.settings.use_slparams:
									volumeShader = cgkit.rmshader.RMShader(os.path.normpath(volume[0].getAttribute("path")))
								else:
									volumeShader = cgkit.rmshader.RMShader(volume[0].getAttribute("path") + os.sep + volume[0].getAttribute("name") + self.settings.renderers[self.settings.renderer][4])
				
							parms = volume.getElementsByTagName("Param")			
							self.populateShaderParams(parms, volumeShader, initialized)
						else:
							volumeShader = cgkit.rmshader.RMShader()
							
						# base RMMaterial
						
						rmmaterial = cgkit.rmshader.RMMaterial(name = name.encode("utf-8"), surface = surfShader, displacement = dispShader, 
													interior = volumeShader, color = matColor, opacity = matOpacity)
						print "Color: ", matColor
						# B2RMaterial Editor here
						BtoRmat = Material(rmmaterial, self.scroller)
						# set the title
						BtoRmat.editorButton.title = name
						BtoRmat.materialName.setValue(name)
						materials.append(BtoRmat) 
						
						# that was easier than it seemed at first blush
						
		else: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
			# not found, spawn a dialog
			self.evt_manager.showConfirmDialog("No materials were found!", "No saved materials were found in this document.", None, False)
			
		return materials
	
	def populateShaderParams(self, parms, shader, initialized):
		
		for parm in parms:	
			convtypes = {"float":"double",
					"string":"string",
					"color":"vec3",
					"point":"vec3",
					"vector":"vec3",
					"normal":"vec3",
					"matrix":"mat4"}			
			p_type = parm.getAttribute("type")
			p_name = parm.getAttribute("name")
				
			if p_type == "float":
				parm_value = float(parm.getAttribute("value"))
			elif p_type == "string":
				parm_value = parm.getAttribute("value")
			elif p_type == "color":
				parm_value = cgkit.cgtypes.vec3(float(parm.getAttribute("red")), float(parm.getAttribute("green")), float(parm.getAttribute("blue")))
			elif p_type in ["normal", "vector", "point"]:
				parm_value = cgkit.cgtypes.vec3(float(parm.getAttribute("x")), float(parm.getAttribute("y")), float(parm.getAttribute("z")))
			elif p_type == "matrix":
				mat_value = []
				sep = "_"
				index = 0
				for x in range(4):
					for y in range(4):
						mat_value.append(float(parmNode.getAttribute(sep.join(["value", index]))))
						index = index + 1
				parm_value = cgkit.cgtypes.mat4(mat_value[0], 
										mat_value[1], 
										mat_value[2], 
										mat_value[3], 
										mat_value[4], 
										mat_value[5], 
										mat_value[6], 
										mat_value[7], 
										mat_value[8], 
										mat_value[9],
										mat_value[10],
										mat_value[11],
										mat_value[12],
										mat_value[13],
										mat_value[14],
										mat_value[15])		
						
			if initialized == False:
				shader.createSlot(p_name, convtypes[p_type], None, parm_value) # Here we set the default value to the parameters incoming value.
			
			# and set the value 
			shader.__dict__[p_name + "_slot"].setValue(parm_value)
				
		
	
	def saveMaterials(self, xmlDoc):
		
		# save materials to XML format.
		# the format for the materials section of the XML is thus		
		# <Material_Defs>
		#	<Material name="name">
		#		<Color red="1.0" green="1.0" blue="1.0"/>
		# 		<Opacity red="1.0" green="1.0" blue="1.0"/>
		#		<Surface shader="shadername">
		#			<Param type="float" name="whatever" value="whatever"/>
		#			<Param name="color" type="color" red="1.0" green="1.0" blue="1.0" />
		#			<Param name="matrix_name" type="matrix" value_1="1.0"/> # matrix values will increase in row-major order.
		#			<Param type="string" name="name" value="value"/>
		#			<Param name="name" type="point/normal/vector" x="1.0" y="1.0" z="1.0"/>
		#			<Param name="name" type="array">
		#				<some params>
		#			</Param>
		#
				
		matRoot = xmlDoc.createElement("Materials_Defs")
		# xmlDoc.appendChild(matRoot)
				
		for material in self.materials:
			mat = material.material			
			surface = mat.surface
			disp = mat.displacement
			volume = mat.interior
			color = mat.color()
			
			opacity = mat.opacity()
			dispBound = material.dispBound.getValue()
			
			matNode = xmlDoc.createElement("Material")
			matNode.setAttribute("name", material.materialName.getValue())
			matNode.setAttribute("displacementBound", material.dispBound.getValue()) # that should be a string value here
			
			# color
			colorNode = xmlDoc.createElement("Color")
			colorNode.setAttribute("red", '%f' % color[0])
			colorNode.setAttribute("green", '%f' % color[1])
			colorNode.setAttribute("blue", '%f' % color[2])
			
			#opacity
			opacityNode = xmlDoc.createElement("Opacity")
			opacityNode.setAttribute("red", '%f' % opacity[0])
			opacityNode.setAttribute("green", '%f' % opacity[1])
			opacityNode.setAttribute("blue", '%f' % opacity[2])
			
			matNode.appendChild(colorNode)
			matNode.appendChild(opacityNode)
			
			if mat.surfaceShaderName() != None:
				
				surfNode = xmlDoc.createElement("Surface")				
				# update all the stuff...
				material.surface.updateShaderParams()						
				# get the shader information
				surfNode.setAttribute("name", mat.surfaceShaderName())
				if mat.surface.filename != None:
					surfNode.setAttribute("path", os.path.normpath(mat.surface.filename))
				else:
					surfNode.setAttribute("path", "None")
				
					
				for parm in surface.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = mat.surface.__dict__[parm[1] + "_slot"].getValue()	
					# create an XML element for this value.
					s_type = parm[0].split()[1]
					# setup as much of the node element as I can here
					parmNode.setAttribute("name", parm[1])
					parmNode.setAttribute("type", s_type)

					if s_type == "float":
						parmNode.setAttribute("value", '%f' % value)
					elif s_type == "string":
						parmNode.setAttribute("value", value)
					elif s_type == "color":
						parmNode.setAttribute("red", '%f' % value[0])
						parmNode.setAttribute("green", '%f' % value[1])
						parmNode.setAttribute("blue", '%f' % value[2])
					elif s_type in ["point", "normal", "vector"]:
						parmNode.setAttribute("x", '%f' % value[0])
						parmNode.setAttribute("y", '%f' % value[1])
						parmNode.setAttribute("z", '%f' % value[2])
					elif s_stype == "matrix":
						sep = "_"
						index = 0
						for x in range(4):
							for y in range(4):
								parmNode.setAttribute(sep.join(["value", index]), '%f' % value[x, y])
								index = index + 1
					
					# now commit this node to the shader node
					surfNode.appendChild(parmNode)
				
				# add to the material node
				matNode.appendChild(surfNode)
							
			if mat.displacementShaderName() != None:
				dispNode = xmlDoc.createElement("Displacement")
				# update all the stuff...
				material.displacement.updateShaderParams()						
				# get the shader information
				dispNode.setAttribute("name", mat.displacementShaderName())
				if mat.displacement.filename != None:
					dispNode.setAttribute("path", os.path.normpath(mat.displacement.filename))
					
				else:
					dispNode.setAttribute("path", "None")
					
				for parm in disp.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = mat.displacement.__dict__[parm[1] + "_slot"].getValue()	
					# create an XML element for this value.
					s_type = parm[0].split()[1]
					# setup as much of the node element as I can here
					parmNode.setAttribute("name", parm[1])
					parmNode.setAttribute("type", s_type)

					if s_type == "float":
						parmNode.setAttribute("value", '%f' % value)
					elif s_type == "string":
						parmNode.setAttribute("value", value)
					elif s_type == "color":
						parmNode.setAttribute("red", '%f' % value[0])
						parmNode.setAttribute("green", '%f' % value[1])
						parmNode.setAttribute("blue", '%f' % value[2])
					elif s_type in ["point", "normal", "vector"]:
						parmNode.setAttribute("x", '%f' % value[0])
						parmNode.setAttribute("y", '%f' % value[1])
						parmNode.setAttribute("z", '%f' % value[2])
					elif s_stype == "matrix":
						sep = "_"
						index = 0
						for x in range(4):
							for y in range(4):
								parmNode.setAttribute(sep.join(["value", index]), value[x, y])
								index = index + 1
					
					# now commit this node to the shader node
					dispNode.appendChild(parmNode)
					
				# add this node to the material node
				matNode.appendChild(dispNode)

			if mat.interiorShaderName() != None:
				volumeNode = xmlDoc.createElement("Volume")
				# update all the stuff...
				mat.volume.updateShaderParams()						
				# get the shader information
				volumeNode.setAttribute("name", mat.interiorShaderName())
				
				if material.interior.filename != None:
					volumeNode.setAttribute("path", os.path.normpath(mat.interior.filename))
				else:
					volumeNode.setAttribute("path", "None")
					
				for parm in displacement.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = mat.interior.__dict__[parm[1] + "_slot"].getValue()	
					# create an XML element for this value.
					s_type = parm[0].split()[1]
					# setup as much of the node element as I can here
					parmNode.setAttribute("name", parm[1])
					parmNode.setAttribute("type", s_type)

					if s_type == "float":
						parmNode.setAttribute("value", '%f' % value)
					elif s_type == "string":
						parmNode.setAttribute("value", value)
					elif s_type == "color":
						parmNode.setAttribute("red", '%f' % value[0])
						parmNode.setAttribute("green", '%f' % value[1])
						parmNode.setAttribute("blue", '%f' % value[2])
					elif s_type in ["point", "normal", "vector"]:
						parmNode.setAttribute("x", '%f' % value[0])
						parmNode.setAttribute("y", '%f' % value[1])
						parmNode.setAttribute("z", '%f' % value[2])
					elif s_stype == "matrix":
						sep = "_"
						index = 0
						for x in range(4):
							for y in range(4):
								parmNode.setAttribute(sep.join(["value", index]), value[x, y])
								index = index + 1

					# now commit this node to the shader node
					volumeNode.appendChild(parmNode)
					
				# since I'm done, commit this to the material node
				matNode.appendChild(volumeNode)
				
			matRoot.appendChild(matNode)
			
		return matRoot

settings = BtoRSettings()
scenesettings = SceneSettings()
materials = MaterialList()
objects = ObjectEditor()

main = MainUI()

if settings.haveSetup == False:
	evt_manager.addElement(settings.getEditor())		
else:
	#matEditor = Material(None, None)
	evt_manager.addElement(main.getEditor())
	#evt_manager.addElement(matEditor.getEditor())

evt_manager.register() # fire it up