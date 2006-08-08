#######################################################################
# GUI  - GUI for the material definition system
# Version 0.1a
# Author: Bobby Parker                                                                         
# This file provides the GUI for the export & materials system
# The shader parameter lister part of this GUI has functionality where it attempts to guess the intended use of 
# a parameter based on its default value or name, e.g. if a parameter has the name text or texture or map, 
# then it's assumed that the parameter is for an image, thus a filename selector is presentable.
# Note that guessed parameters can be overridden via an override button provided per editor.
#
#######################################################################
import os
from btor import BtoRGUIClasses as ui
from btor import BtoRAdapterClasses
from btor import BtoRTypes
reload(BtoRTypes)
reload(ui) # remove these when released for production use
reload(BtoRAdapterClasses)
import cgkit
import cgkit.rmshader
import cgkit.cgtypes
import cgkit.quadrics
from cgkit import ri as ri
from cgkit import ribexport as export
import Blender
import xml.dom.minidom
import new
import traceback
import math
from sets import Set
import sys
import StringIO	
import re

class BtoRSettings: # an instance of this class should be passed to every base-level BtoR object (objects that go into the root event manager.
	# someone with 3Delight, Prman, entropy etc can add further renderer information here
	# standard setup: Renderer name: [render binary, sltell binary, texture binary, shader extension, [home env vars], [shader env vars], [texture env vars], [ display env vars],
	#				            [ procedural env vars ], [plugin env vars], [archive env vars]] 
	# assign all lists and strings to None if there's no available info or that particular renderer has no equivalent value (see Aqsis for ex. of home env vars)
	renderers = {  "AQSIS" : ["aqsis", "aqsl", "aqsltell", "teqser", "slx", None, "AQSIS_SHADER_PATH", "AQSIS_TEXTURE_PATH", "AQSIS_DISPLAY_PATH", "AQSIS_PROCEDURAL_PATH", "AQSIS_PLUGIN_PATH", "AQSIS_ARCHIVE_PATH"],
			     "BMRT" : ["rendrib", "slc", "slctell","mkmip","BMRTHOME", "SHADERS", "TEXTURES",None,None,None,None], 
			     "Pixie" : ["rndr", "sdr", "sdrinfo", "texmake", "PIXIEHOME", "SHADERS", None, None, None, None, None],
			     "3Delight":["renderdl", "shaderdl", "shaderinfo", "tdlmake", "DELIGHT",  "DL_SHADERS", None, "DL_DISPLAYS", None, None, None],
			     "RenderDotC":["wrendrdc", "shaderdc", "soinfo", "texdc", "RDCROOT", "SHADERS", "MAPS", "DISPLAYS", "PROCEDURALS", None, "ARCHIVES"] }
	cpp = "cpp"
	# what else does BtoRSettings need to know? How about a projects directory? MRUs? TBD later I suppose
	
	def __init__(self):		
		sdict = globals()

		if sdict.has_key("instBtoREvtManager"):
			self.evt_manager = sdict["instBtoREvtManager"]
		#for key in keys:		
		#	if isinstance(dict[key], ui.EventManager):
		#		self.evt_manager = dict[key]
				# event manager instance
				
		self.getSettings(None)
		screen = Blender.Window.GetAreaSize()		
		self.editor = ui.Panel(10, screen[1] - 225, 600, 400, "GlobalSettings", "Global BtoR Settings:", None, False)
		self.editor.addElement(ui.Label(5, 30, "Renderer:", "Renderer:", self.editor, False))
		rendererList = self.renderers.keys()
		
		self.rendererMenu = ui.Menu(self.editor.get_string_width("Renderer:", 'normal') + 12, 30, 100, 20, "Renderer:", rendererList, self.editor, True)				
		self.rendererMenu.setValue(rendererList.index(self.renderer))
		self.rendererMenu.registerCallback("release", self.selectRenderer)
		
		self.useSlParams = ui.CheckBox(5, 65,"Use CGKit for Shader Parameters?", "Use CGKit for Shader Parameters?", True, self.editor, True)
		self.useSlParams.setValue(self.use_slparams)		
		
		#self.useSlParams.upColor = [158, 178, 181, 255]
		#self.useSlParams.normalColor = [158, 178, 181, 255]
		#self.useSlParams.downColor = [88, 108, 111, 255]
		#self.useSlParams.hoverColor = [68, 88, 91, 255]
		
		width = self.editor.get_string_width("Procedurals:", 'normal') + 30
		
		self.renderMatsOnStartup = ui.CheckBox(5, 90, "Render material previews on startup?", "Render material previews on startup?", False, self.editor, True)
		
		self.useShadowMaps = ui.CheckBox(300, 65, "Use Shadow Maps?", "Use Shadow Maps?", True, self.editor, True)

		offset = 125
		
		
		# search paths label
		self.editor.addElement(ui.Label(5, offset, "Search Paths:", "Search Paths:", self.editor, False))
		
		# binary search path
		self.editor.addElement(ui.Label(25, offset + 25, "Binaries:", "Binaries:", self.editor, False))		
		self.binarypath = ui.TextField(width, offset + 25, self.editor.width - (45 + width), 20, "BinaryPathText", self.binaryPath, self.editor, True)		
		self.binaryBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, offset + 25, 30, 20, "BinaryBrowse", "...", 'normal', self.editor, True)		
		self.binaryBrowse.registerCallback("release", self.browse)
		
		# shader search paths
		self.editor.addElement(ui.Label(25, offset + 50, "Shaders:", "Shaders:", self.editor, False))		
		self.shaderpaths = ui.TextField(width, offset + 50, self.editor.width - (45 + width), 20, "ShaderPathText", self.shaderPathList, self.editor, True)		
		self.shaderBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, offset + 50, 30, 20, "ShaderBrowse", "...", 'normal', self.editor, True)		
		self.shaderBrowse.registerCallback("release", self.browse)
		
		# texture search paths
		self.editor.addElement(ui.Label(25, offset + 75, "Textures:", "Textures:", self.editor, False))
		self.texturepaths = ui.TextField(width, offset + 75, self.editor.width - (45 + width), 20, "TexturePathText", self.texturePathList, self.editor, True)		
		self.textureBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, offset + 75, 30, 20, "TextureBrowse", "...", 'normal', self.editor, True)		
		self.textureBrowse.registerCallback("release", self.browse)
		
		# archive search paths
		self.editor.addElement(ui.Label(25, offset + 100, "Archives:", "Archives:", self.editor, False))		
		self.archivepaths = ui.TextField(width, offset + 100, self.editor.width - (45 + width), 20, "ArchivePathText", self.archivePathList, self.editor, True)		
		self.archiveBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, offset + 100, 30, 20, "ArchiveBrowse", "...", 'normal', self.editor, True)		
		self.archiveBrowse.registerCallback("release", self.browse)
		
		# procedural search paths
		self.editor.addElement(ui.Label(25, offset + 125, "Procedurals:", "Procedurals:", self.editor, False))		
		self.proceduralpaths = ui.TextField(width, offset + 125, self.editor.width - (45 + width), 20, "ProceduralsPathText", self.proceduralPathList, self.editor, True)		
		self.proceduralBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, offset + 125, 30, 20, "ProceduralBrowse", "...", 'normal', self.editor, True)		
		self.proceduralBrowse.registerCallback("release", self.browse)
		
		# display search paths
		self.editor.addElement(ui.Label(25, offset + 150, "Displays:", "Displays:", self.editor, False))
		self.displaypaths = ui.TextField(width, offset + 150, self.editor.width - (45 + width), 20, "DisplaysPathText", self.displayPathList, self.editor, True)		
		self.displayBrowse = ui.Button(self.binarypath.x + self.binarypath.width + 2, offset + 150, 30, 20, "DisplayBrowse", "...", 'normal', self.editor, True)
		self.displayBrowse.registerCallback("release", self.browse)
		
		# file paths label
		self.editor.addElement(ui.Label(10, offset + 175, "File Paths:", "File Paths:", self.editor, False))
		
		# output path
		self.editor.addElement(ui.Label(25, offset + 200, "Output Path:", "Output Path:", self.editor, False))
		self.outputpath = ui.TextField(width, offset + 200, self.editor.width - (45 + width), 20, "OutputPathText", self.outputPath, self.editor, True)
		self.outputBrowse = ui.Button(self.outputpath.x + self.outputpath.width + 2, offset + 200, 30, 20, "OutputBrowse", "...", 'normal', self.editor, True)
		self.outputBrowse.registerCallback("release", self.browse)
		
		self.cancelButton = ui.Button(5, self.editor.height - 45, 60, 20, "Cancel", "Cancel", 'normal', self.editor, True)		
		self.okButton = ui.Button(self.editor.width - (60 + 15), self.editor.height - 45, 60, 20, "OK", "OK", 'normal', self.editor, True)
		
		self.okButton.registerCallback("release", self.saveSettings)
		self.cancelButton.registerCallback("release", self.getSettings)
		self.cancelButton.registerCallback("release", self.update)
		self.okButton.registerCallback("release", self.close)
		self.cancelButton.registerCallback("release", self.close)
		
		# get the first shader path in the search list
		paths = self.shaderpaths.getValue().split(";")
		
		self.shadersSurface = {}
		self.shadersDisplacement = {}
		self.shadersImager = {}
		self.shadersVolume = {}
		self.shadersLight = {}
		self.surfaceFiles = {}
		self.dispFiles = {}
		self.volumeFiles = {}
		self.imagerFiles = {}
		self.lightFiles = {}
		

		
		if self.use_slparams:
			self.getShaderSourceList(paths[0]) 
		else:
			self.getCompiledShaderList(paths[0]) 
			
		self.shadows = os.sep + "shadows" + os.sep
		self.images = os.sep + "images" + os.sep
		self.textures = os.sep + "textures" + os.sep
		self.archives = os.sep + "archives" + os.sep
		

	def getShaderSearchPaths(self):
		pathList = self.shaderpaths.getValue().split(";")
		return pathList
			
	def selectRenderer(self, button):
		self.renderer = self.rendererMenu.getValue() 
			
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
		
		try:
			parms = cgkit.slparams.slparams(file) 
		except cgkit.slparams.SyntaxError:
			# the shader's source-code was incomplete, don't include it.
			parms = None 
		return parms
			
	def getCompiledShaderList(self, shaderPath):
		# reset the shaders lists, since I'm looking at a new path		
		self.shadersSurface[shaderPath] = []
		self.shadersLight[shaderPath] = []
		self.shadersVolume[shaderPath] = []
		self.shadersDisp[shaderPath] = []
		self.shadersImager[shaderPath] = []						
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
							self.shadersSurface[shaderPath].append(parms)
						elif (parms[1] == "imager"):
							self.shadersImager[shaderPath].append(parms)
						elif (parms[1] == "displacement"):
							self.shadersDisp[shaderPath].append(parms)
						elif (parms[1] == "volume"):
							self.shadersVolume[shaderPath].append(parms)
						elif (parms[1] == "light"):
							self.shadersLight[shaderPath].append(parms)	
		else:
			# display error dialog
			error_state = 1
			error_message = "Invalid search path!"
			print error_message
	
	def getShaderSourceList(self, shaderPath): 
		# maybe add some iterative thing here that pops up a progress bar
		# reset the shader lists
	
		path = shaderPath.strip()
		# test for a valid path, and bail if not valid
		if os.path.exists(path):

			# reset the global shaders lists
			self.shadersSurface[shaderPath] = []
			self.surfaceFiles[shaderPath] = []
			self.shadersLight[shaderPath] = []
			self.lightFiles[shaderPath] = []
			self.shadersVolume[shaderPath] = []
			self.volumeFiles[shaderPath] = []
			self.shadersDisplacement[shaderPath] = []
			self.dispFiles[shaderPath] = []
			self.shadersImager[shaderPath] = []		
			self.imagerFiles[shaderPath] = []

			files = os.listdir(path)
			# determine the renderer	
			for file in files:
				if (file.endswith(".sl")):
					
					parms = self.getShaderSourceParams(path + os.sep + file)					
					if parms == None:
						print "Error getting parameters for shader ", file
					else:
		
						if (parms[0][0] == "surface"):
							self.shadersSurface[shaderPath].append(parms[0])
							self.surfaceFiles[shaderPath].append(file)
						elif (parms[0][0] == "imager"):
							self.shadersImager[shaderPath].append(parms[0])
							self.imagerFiles[shaderPath].append(file)
						elif (parms[0][0] == "displacement"):
							self.shadersDisplacement[shaderPath].append(parms[0])
							self.dispFiles[shaderPath].append(file)
						elif (parms[0][0] == "volume"):
							self.shadersVolume[shaderPath].append(parms[0])
							self.volumeFiles[shaderPath].append(file)
						elif (parms[0][0] == "light"):
							self.shadersLight[shaderPath].append(parms[0])
							self.lightFiles[shaderPath].append(file)
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
				self.binaryPath = r'C:\Program Files\Aqsis\\bin'
				self.shaderPathList = r"C:\Program Files\Aqsis\Shaders" 
				self.texturePathList = r"C:\Program Files\Aqsis\Textures"
				self.displayPathList = r"C:\Program Files\Aqsis\Displays"
				self.archivePathList = r"C:\Program Files\Aqsis\Archives"
				self.proceduralPathList = r"C:\Program Files\Aqsis\Procedurals"
				self.outputPath = r'C:\temp'
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
		#print self.rendererMenu.getValue()
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
		
	def update(self, obj):
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
		self.update(None)
		
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
	filter_menu = ["None Selected", "box", "triangle", "catmull-rom", "sinc", "gaussian"]
	def __init__(self):
				
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		#setattr(BtoRAdapterClasses, "instBtoREvtManager", self.evt_manager)
		#scene = Blender.Scene.GetCurrent()
		#render = scene.getRenderingContext()
		#gammaLevel = render.gammaLevel()
		#gausFilterSize = render.gaussFilterSize()
		
		self.editor = ui.Panel(200, 500, 500, 300, "Scene Settings:", "Scene Settings:", None, False)
		
		self.saveButton = ui.Button(self.editor.width - 160, 30, 150, 25, "Save Scene Setup", "Save Scene Setup", 'normal', self.editor, True)
		self.saveButton.registerCallback("release", self.saveSceneData)
			
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		
		self.useAO = ui.CheckBox(250, 90, "Generate Ambient Occlusion Maps?", "Generate Ambient Occlusion Maps?", True, self.editor, True)
		self.editor.addElement(ui.Label(250, 110, "AO shader", "AO Shader:", self.editor, True))
		self.shaderButton = ui.Button(350, 110, 100, 25, "AO Shader", "None Selected", 'normal', self.editor, True)
		self.shaderButton.registerCallback("release", self.showAOShader)
		setattr(self.editor, "shaderButton", self.shaderButton)
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		
		offset = 70
		x_off = self.editor.get_string_width("Pixel Samples:", 'normal') + 15
		self.editor.addElement(ui.Label(5, 30, "Render Settings:", "Render Settings:", self.editor, False))
		self.editor.addElement(ui.Label(5, offset, "Shading Rate:", "Shading Rate:", self.editor, False))
		self.shadingRate = ui.TextField(x_off, offset, 20, 20, "Shading Rate:", 1, self.editor, True)
		
		self.editor.addElement(ui.Label(5, offset + 30, "Pixel Samples:", "Pixel Samples:", self.editor, False))
		self.pixelSamplesX = ui.TextField(x_off , offset + 30, 20, 20, "Pixel Samples", 2, self.editor, True)
		self.pixelSamplesY = ui.TextField(self.pixelSamplesX.width + self.pixelSamplesX.x + 5,offset + 30, 20, 20, "Pixel Samples", 2, self.editor, True)
		
		self.editor.addElement(ui.Label(5, offset + 60, "Gain", "Gain:", self.editor, False))
		self.gain = ui.TextField(x_off, offset + 60, 20, 20, "Gain", 1.0, self.editor, True)
		
		self.editor.addElement(ui.Label(5, offset + 90, "Gamma", "Gamma:", self.editor, False))
		self.gamma = ui.TextField(x_off, offset + 90, 20, 20, "Gamma", 1.0, self.editor, True)		
		
		self.editor.addElement(ui.Label(5, offset + 120, "Pixel Filter:", "Pixel Filter:", self.editor, True))
		
		self.pixelFilterMenu = ui.Menu(self.editor.get_string_width("Pixel Filter:", 'normal', ) + 15, offset + 120, 120, 20, "Pixel Filter", self.filter_menu, self.editor, True)
		
		# filtersize X
		self.editor.addElement(ui.Label(self.pixelFilterMenu.x + 140, offset + 120, "Filter Size - X:", "Filter Size - X:", self.editor, False))
		xoff = self.pixelFilterMenu.x + 140 + self.editor.get_string_width("Filter Size - X:", 'normal') + 5
		self.filterSizeX = ui.TextField(xoff, offset + 120, 25, 20, "FilterSizeX", 1, self.editor, True)
		
		# filtersize Y
		self.editor.addElement(ui.Label(xoff + 35, offset + 120, "Y:", "Y:", self.editor, False))
		xoff = xoff + self.editor.get_string_width("Y:", 'normal') + 30
		self.filterSizeY = ui.TextField(xoff + 10, offset + 120, 25, 20, "FilterSizeX", 1, self.editor, True)
		
		self.object_data = {} # fetch what will now be adapter objects for a given scene.
		self.camera_data = {} # I doubt this is neccessary, since the shader will now be saved per camera
		self.light_data = {} # ditto
		self.object_groups = {} # this *is* neccessary for instancing support
		self.occlusion_map = []
		
		self.lightMultiplier = 1
		self.renderContext = Blender.Scene.GetCurrent().getRenderingContext()
		
		self.suzanne = BtoRAdapterClasses.IObjectAdapter(BtoRTypes.BtoRPreview(Blender.Mesh.Primitives.Monkey()))
		self.ao_shader = GenericShader(None, "light", self)
		
	def showAOShader(self, button):
		self.evt_manager.addElement(self.ao_shader.getEditor())
		
	def render(self):
		# generate Ri calls for the scene
		# scene = Blender.Scene.GetCurrent()
		# render = scene.getRenderingContext(
		paths = self.settings.getShaderSearchPaths()
		# search path options
		for path in paths:
			ri.RiOption("searchpath", "shader", path + ":&")
		
		
		settings = {}
		settings["PixelSamples"] = self.pixelSamplesX.getValue() + " " + self.pixelSamplesY.getValue()
		settings["ShadingRate"] = self.shadingRate.getValue()
		settings["Gain"] = self.gain.getValue()
		settings["Gamma"] = self.gamma.getValue()
		
		ri.RiPixelSamples(self.pixelSamplesX.getValue(), self.pixelSamplesY.getValue())
		ri.RiShadingRate(self.shadingRate.getValue())
		ri.RiExposure(self.gain.getValue(), self.gamma.getValue())
		if self.pixelFilterMenu.getSelectedIndex() > 0:
			ri.RiPixelFilter(self.pixelFilterMenu.getValue(), self.filterSizeX.getValue(), self.filterSizeY.getValue())
			
		
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def renderScene(self, exportSettings):
		""" export/render the whole scene """		
		paths = self.settings.shaderpaths.getValue().split(";")
		
		#yeeehaw, it's render time!
		# hook me up with some paths!
		outputPath = self.settings.outputpath.getValue()
		if not os.path.exists(outputPath):
			os.mkdir(outputPath)
		if not os.path.exists(outputPath + self.settings.shadows):
			os.mkdir(outputPath + self.settings.shadows)
		if not os.path.exists(outputPath + self.settings.archives):
			os.mkdir(outputPath + self.settings.archives)
		if not os.path.exists(outputPath + self.settings.images):
			os.mkdir(outputPath + self.settings.images)
		if not os.path.exists(outputPath + self.settings.textures):
			os.mkdir(outputPath + self.settings.textures)
		# get required precursor info
		bScene = Blender.Scene.GetCurrent()
		rend = bScene.getRenderingContext()
		startFrame = rend.startFrame()
		endFrame = rend.endFrame()
		self.frames = range(startFrame, endFrame)
		
		toFile = False
		toRender = False
		if exportSettings.export_menu.getSelectedIndex() == 0:
			toFile = True			
			filename = exportSettings.filename.getValue()
		elif exportSettings.export_menu.getSelectedIndex() == 1:
			toRender = True			
			filename = exportSettings.filename.getValue()
		else:
			toFile = False
			filename = exportSettings.textName.getValue()
			
		# step 1, sort out the scene objects
		self.cameras = [] # temporary storage
		self.lights = []
		self.objects = []
		self.shadows = {}
		objs = Blender.Scene.GetCurrent().getChildren()
		# print len(objs), " objects found."
		# sort out the objects in this scene by type
		objSequence = 1
		for obj in objs:
			print obj.getType()
			if self.object_data.has_key(obj.getName()):
				adapter = self.object_data[obj.getName()]
			else:
				bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
				self.object_data[obj.getName()] = BtoRAdapterClasses.IObjectAdapter(bObj) # add to the master object list for possible use later
				adapter = self.object_data[obj.getName()]
			if obj.getType() == "Camera":						
				self.cameras.append(adapter)
			elif obj.getType() == "Lamp":
				self.lights.append(adapter)
			else:
				self.objects.append(adapter)
			adapter.sequence = objSequence
			objSequence = objSequence + 1

		if exportSettings.renderAnimation == False:
			self.frames = [1] # reset the frame list to one frame - simple fix eh?
		
		self.generateShadowMaps()
			
		for frame in self.frames:	
			# frame block
			# Blender.Set("curfame", frame) 
			# frame info here
			cam = 1
			for camera in self.cameras:
				# split the filename off
				outputPath = os.path.split(filename)[1]
				outputname = os.path.splitext(filename)[0] + "_cam_%d_frame_%d" % (cam, frame)				
				filename = outputname + ".rib"	 
				imgFile = outputname + ".tif" 
					# Main Render Start
				
				if toFile:
					ri.RiBegin(filename)
				elif toRender:
					ri.RiBegin("aqsis")
				else:
					ri.RiBegin()
				# make the occlusion pass
				if len(self.occlusion_map) > 0:
					maps = "["
					for map in self.occlusion_map:
						maps = maps + '"' + map + '" '
					maps = maps + "]"
					ri._ribout.write("MakeOcclusion " + maps + " " + '"' + "occlusion.sm" + '"' + "\n")
					self.occlusion_map_file = outputPath + "occlusion.sm"
				for path in paths:
					ri.RiOption("searchpath", "shader", path + ":&")
					
				ri.RiFrameBegin(frame)	
				# Global scene options
				self.render()
				ri.RiDisplay(imgFile, "file", "rgb")
				# self.renderFrame(exportSettings)		
				# ri.RiTransformBegin() this is potential
				camera.render()		
				# world block
				self.renderWorld()
				# ri.RiTransformEnd()
				cam = cam + 1
				ri.RiFrameEnd()			
				ri.RiEnd()
		
	def generateShadowMaps(self):
		self.occlusion_map = []
		shadowPath = self.settings.outputpath.getValue() + self.settings.shadows
		for light in self.lights:
			if light.getProperty("GenShadowMap"): # if this lamp generates a shadowMap...
				# create a shadowmap RIB		
				shadowList = {}
				for direction in light.getRenderDirections():
					# if the light's animated, then animate the shadow map
				
					if light.isAnimated: # the light is animated, so iterate all of the frames
						for frame in self.frames:
							Blender.Set("curfame", frame) 
							shadowFile = shadowPath + light.objData["name"] + direction  + "_" + frame + ".z"
							shadowName = shadowPath + light.objData["name"] + direction + "_" + frame + ".tx"
							shadowRIB = shadowPath + light.objData["name"] + direction + "_" + frame + ".rib"
							self.renderShadowMap(light, direction, shadowRIB, shadowName, shadowFile)							
							shadowList[direction + "_" + frame] = {"rib" : shadowRIB, "shadowName" : shadowName, "shadowFile" : shadowFile }
							
					else:						
						shadowName = shadowPath + light.objData["name"] + direction + ".tx"
						shadowFile = shadowPath + light.objData["name"] + direction + ".z"
						shadowRIB = shadowPath + light.objData["name"]  + direction + ".rib"
						self.renderShadowMap(light, direction, shadowRIB, shadowName, shadowFile)
						shadowList[direction] = {"rib" : shadowRIB, "shadowName" : shadowName, "shadowFile" : shadowFile}
					print "Created shadowmap ", shadowName, " using RIB ", shadowRIB
				# add in the shadow data to the shadows array
				self.shadows[light.objData["name"]] = shadowList # something
				
	def renderShadowMap(self, light, direction, shadowRIB, shadowName, shadowFile):									
		self.lightDebug = False

		if self.lightDebug:
			ri.RiBegin(shadowRIB + "test.rib")	
			paths = self.settings.shaderpaths.getValue().split(";")
			for path in paths:
				ri.RiOption("searchpath", "shader", path + ":&")			
			ri.RiScale(-1, -1, -1)
			ri.RiPixelSamples(1, 1)
			ri.RiPixelFilter("box", 1, 1)
			ri.RiHider("hidden", "jitter", 0)
			ri.RiDisplay(shadowFile + ".tiff", "file", "rgb")
			#ri.RiDisplay(shadowFile, "zfile", "z")
			projection = light.getRenderProjection()
			ri.RiProjection(projection, "fov", 91) # 92 degrees projection
			ri.RiShadingRate(4)
			light.doCameraTransform(direction)
			ri.RiWorldBegin()			
			#ri.RiLightSource("ambientlight", "intensity", "1")
			ri.RiTransformBegin()
			lightParams = { "from" : [3, 3, 3], "uniform float intensity" : [8] }
			ri.RiLightSource("pointlight", lightParams) 
			lightParams = { "from" : [-3, 3, 3], "uniform float intensity" : [8] }
			ri.RiLightSource("pointlight", lightParams) 
			lightParams = { "from" : [-3, -3, 3], "uniform float intensity" : [8] }
			ri.RiLightSource("pointlight", lightParams) 
			lightParams = { "from" : [3, -3, 3], "uniform float intensity" : [8] }
			ri.RiLightSource("pointlight", lightParams) 
			ri.RiTransformEnd()
			self.renderObjects()
			ri.RiWorldEnd()
			#ri.RiMakeShadow(shadowFile, shadowName)
			ri.RiEnd()
		else:
			ri.RiBegin(shadowRIB)
			paths = self.settings.shaderpaths.getValue().split(";")
			for path in paths:
				ri.RiOption("searchpath", "shader", path + ":&")	
			ri.RiPixelSamples(1, 1)
			ri.RiPixelFilter("box", 1, 1)			
			ri.RiHider("hidden", {"jitter" : [0], "depthfilter" : "midpoint"})
			# ri.RiDisplay(shadowFile + ".tiff", "file", "rgb")
			ri.RiDisplay(shadowFile, "zfile", "z")
			#ri.RiDisplay("view_from_light", "framebuffer", "rgb")
			#params = { "uniform cd ..float intensity" : 1, "uniform point from" : [0, 0, 0], "uniform point to" : [0, 0, 1] }
			#ri.RiLightSource("distantlight", params)
			projection = light.getRenderProjection()			
			ri.RiProjection(projection, "fov", 95) # 92 degrees projection
			ri.RiShadingRate(4)
			light.doCameraTransform(direction)
			ri.RiWorldBegin()
			# ri.RiReverseOrientation()
			self.renderObjects()
			ri.RiWorldEnd()
			ri.RiMakeShadow(shadowFile, shadowName)
			ri.RiEnd()
			
		self.occlusion_map.append(shadowFile)
		
	def renderWorld(self):
		ri.RiWorldBegin()		
		# if we're shadowmapping, make sure to generate shadowmaps each of these - of course the question becomes how...
		self.renderLights()		
		ri.RiIdentity()
		self.renderObjects()
		ri.RiWorldEnd()
	def renderWorldArchive(self):
		""" render the folowing objects to archives """
		for obj in self.objects:
			obj.renderArchive()

	def renderLights(self):
		# occlusion setup here
		if self.useAO.getValue() and self.ao_shader.shader != None and len(self.occlusion_map) > 0:
			# using occlusion
			self.ao_shader.updateShaderParams()				
			ri.RiLightSource(self.ao_shader.shader.shadername, self.ao_shader.shader.params())
			for light in self.lights:
				if light.getProperty("IncludeWithAO"):
					if light.getProperty("GenShadowMap"):
						shadows = self.shadows[light.objData["name"]]
						light.setShadowParams(self.shadows)
					light.render()
		else:
			for light in self.lights:
				# get the fame info
				# frame = Blender.Get("curframe")
				if light.getProperty("IncludeWithAO"):
					shadows = self.shadows[light.objData["name"]]			
					if light.getProperty("GenShadowMap"):
						light.setShadowParms(shadows)		
					light.render() 
					
		
		
	def renderObjects(self):
		for obj in self.objects:
			# get all the objects in the scene that aren't lights or cameras
			obj.render()
		
		
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
		
		obj = self.saveObjects(newdoc)
		
		root.appendChild(obj)
		# self.saveSceneSettings(root_element)
		try:
			text = Blender.Text.Get("BtoRXML")
			found = True
		except NameError: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
			# not found, create a new one
			text = Blender.Text.New("BtoRXML")
			found = False
		
		try:
			xmlOut = root.toprettyxml()
			text.clear()
			# doesn't matter, it's getting written
			text.write(xmlOut) # that should be that.
		except:
			# not found, spawn a dialog
			traceback.print_exc()
			self.evt_manager.showConfirmDialog("Error!", "Something went wrong. Look at the console!", None, False)
			
	def loadSceneData(self):
		try:
			text = Blender.Text.Get("BtoRXML")
			lines = text.asLines()
			xmlData = ""
			for line in lines:
				xmlData = xmlData + line
			# do something fun here
			found = True
		except: 
			traceback.print_exc()
			self.evt_manager.showConfirmDialog("Error parsing XML!", "There was an error in the BtoRXML text file!", None, False)
			return None
		if found:
			try:
				xmlScene = xml.dom.minidom.parseString(xmlData)
				hasXMLData = True
			except xml.parsers.expat.ExpatError:
				hasXMLData = False	
				# pop up an error dialog!
				traceback.print_exc()
				self.evt_manager.showConfirmDialog("Error parsing XML!", "There was an error in the BtoRXML text file!", None, False)
				return None
			if hasXMLData:
				xmlObjects = xmlScene.getElementsByTagName("Object")
				if len(xmlObjects) > 0:
					for xmlObject in xmlObjects:
						# get the name of the object in question. They should all have names.
						objName = xmlObject.getAttribute("name")
						# find the object in blender
						obj = Blender.Object.Get(objName)
						# load the target object...
						bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
						objData = BtoRAdapterClasses.IObjectAdapter(bObj) # get an adapter
						# test to ensure that I've got the same object type...
						if obj.getType() != xmlObject.getAttribute("type"):
							# just in case something went wrong here, discard most of the object data and re-initialize with just a material def
							if xmlObject.getAttribute("material") != "":
								objData.objData["material"] = xmlObject.getAttribute("material")
						else:
							# otherwise, load up the full data from the object definition
							objData.loadData(xmlObject)
						self.object_data[objName] = objData # stuff the adapter component into the object data
			# return some statistics here 
			self.evt_manager.showConfirmDialog("Scene Data Loaded!", "%d objects were successfully loaded." % len(xmlObjects), None, False)
			
	def saveObjects(self, xml):
		
		#create a root object for the XML
		objectRoot = xml.createElement("Object_Defs")
		
		# object save routines for each adapter
		# get the current list of objects in the scene
		scene = Blender.Scene.GetCurrent()
		
		objList = scene.getChildren()
		for obj in objList:
			if self.object_data.has_key(obj.getName()):
				objXml = self.object_data[obj.getName()].saveData(xml)
				print "XML received for ", obj.getName(), "of type ", obj.getType(), "..."
			else:
				# create a new adapter in case there's no data defined for it.
				bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
				adapter = BtoRAdapterClasses.IObjectAdapter(bObj)	
				objXml = adapter.saveData(xml)
				self.object_data[obj.getName()] = adapter
				print "New adapter generated for ", obj.getName(), " of type ", obj.getType(), "..."
			objectRoot.appendChild(objXml)	# that should be successful one way or another.		
		
		return objectRoot
					
	def saveSimpleObject(self, objData, xml):
		objXml = xml.createElement("Object")
		for key in objData:
			objXml.setAttribute(key, objData[key])
		
		return objXml
		
	def saveObject(self, objData, xml):
		objXml = xml.createElement("object")
		for key in objData:			
			if key == "shaderparms":
				shaderparms = objData["shaderparms"]
				# I need to spawn an RMShader to deal with this effectively.
				# Before I can do that, the restore needs to be in place to shove the data back into the shader 
				for skey in shaderparms:
					#print type(shaderparms[skey])
					shader.setAttribute(skey, shaderparms[skey])
					
				objXml.appendChild(shader)
			else:
				objXml.setAttribute(key, objData[key])
		return objXml				
			
		
	def saveShaderParms(self, shader, xml): # I only need to pass the shader in here
		shaderNode = xml.createElement("Shader")				
		shaderNode.setAttribute("name", mat.surfaceShaderName())
		if shader.filename != None:
			shaderNode.setAttribute("path", os.path.normpath(shader.filename))
		else:
			shaderNode.setAttribute("path", "None")
		
			
		for parm in shader.shaderparams:
			# get the param and stuff into my dict				
			# create the node
			parmNode = xml.createElement("Param")
			value = getattr(mat.surface, parm[1])
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
			shaderNode.appendChild(parmNode)
		return shaderNode
			

class ObjectEditor:
	def __init__(self):
		
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		self.groupList = sdict["instBtoRGroupList"]
		setattr(BtoRAdapterClasses, "instBtoRSettings", sdict["instBtoRSettings"])
		setattr(BtoRAdapterClasses, "instBtoREvtManager", sdict["instBtoREvtManager"])
		setattr(BtoRAdapterClasses, "instBtoRSceneSettings", sdict["instBtoRSceneSettings"])
		setattr(BtoRAdapterClasses, "instBtoRMaterials", sdict["instBtoRMaterials"])
		setattr(BtoRAdapterClasses, "instBtoRObjects", self)
		
		self.debug = False
				
		self.selected_objects = []
		
		# master panel
		screen = Blender.Window.GetAreaSize()	
		self.editorPanel = ui.Panel(225, screen[1] - 10, 500, 400, "Object Editor:", "Object Properties:", None, False)
		self.editorPanel.addElement(ui.Label(10, 30, "Object Name:", "Object Name:", self.editorPanel, False))
		
		self.objectName = ui.Label(15 + self.editorPanel.get_string_width("Object Name:", 'normal'), 30, "None Selected", "None Selected", self.editorPanel, True)
		self.objectMenu = ui.Menu(self.objectName.x, 30, 150, 25, "Object menu", ["None Selected"], self.editorPanel, True)
		self.objectMenu.isVisible = False
		self.objectMenu.registerCallback("release", self.selectObject_menu)
		
		containerOffset = 110
		containerHeight = 280
								
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)

		self.objEditorPanel = ui.Panel(4, 65, 491,  280, "Empty Panel", "", None, False) # keep this one invisible
		self.objEditorPanel.isVisible = False
		self.objEditorPanel.hasHeader = False
		self.objEditorPanel.cornermask = 0
		self.objEditorPanel.shadowed = False
		self.objEditorPanel.outlined = False
		
		self.editorPanel.addElement(self.objEditorPanel) 
		
		self.export_funcs = []
		#ok, this is now unneccessary
		# objFunc = self.editorPanel.callFactory(self.exportObject, "hi!")
		
		# set up the listener for the spacehandler 
		self.evt_manager.registerCallback("draw", self.getSelected)
		
		# self.infoButton = ui.Button(250, 60, 150, 25, "Show object info", "Show object info", 'normal', self.editorPanel, True)
		# self.infoButton.registerCallback("release", self.showObjData)
		# material selector setup		
		self.materials.select_functions.append(self.selectMaterial)
		
		# group selector
		self.groupList.select_functions.append(self.selectGroup)

		self.materials.loadMaterials()
		self.scene.loadSceneData()
		
	def getEditor(self):
		return self.editorPanel

	
	def exportScene(self):
		pass
		
	def selectionChanged(self):
		try:
			changed = Blender.Registry.GetKey("BtoR_Space", False)
			selection = changed["changed"]
			changed["changed"] = False			
		except:
			selection = False
		return selection
		
	def selectObject_menu(self, obj):
		object = self.objects[self.objectMenu.getSelectedIndex()]
		self.selectObject(object)
		
	def selectObject(self, obj):
		""" An object's been selected, make the magic perform. """
		# print "selected an object!"
		name = obj.getName()
		self.objectName.setValue(name)
		objType = obj.getType()		
		# so instead I simply retrieve the object in question
		if self.scene.object_data.has_key(obj.getName()): 
			self.objData = self.scene.object_data[obj.getName()] # fetch the adapter 
			print self.objData
		else:
			# no adapter found in the scene settings dictionary, so
			# create a new one based on a returned BtoR type
			bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
			self.objData = BtoRAdapterClasses.IObjectAdapter(bObj)		
			self.scene.object_data[obj.getName()] = self.objData # assign the new adapter to the scene settings dict
		
		# editor panel setup
		self.objEditor = self.objData.getEditor()
		self.editorPanel.removeElement(self.objEditorPanel)
		self.objEditorPanel = self.objEditor.getEditor()
		self.objEditorPanel.parent = self.editorPanel
		self.objEditorPanel.invalid = True
		self.editorPanel.addElement(self.objEditorPanel)
		
		# and finally, object checks/resets for interested objects
		self.objData.checkReset()
		
	def selectMaterial(self, button):
		# set the material in the current object adapter
		self.evt_manager.removeElement(self.materials.getSelector())
		matName = button.title
		mat = self.materials.getMaterial(matName)
		self.objData.setMaterial(mat)
		
	def selectGroup(self, button):
		self.evt_manager.removeElement(self.groupList.getEditor())
		groupName = button.title
		self.objData.setGroup(groupName)
		
		
	def showObjData(self, button):
		self.objData.getInfo()
		
	def exportSingleObject(self):
		""" export a single object """
		# ok so here, I get the scene settings from the scene object and react to those
		exportSettings = self.objData.objEditor.exportSettings
		# get the filename and export path first
		if exportSettings.camera_menu.getSelectedIndex() == 0:
				makeFullScene = True
				# but which camera do I get?
				# for now, support only one camera, that being the first one I find.
				
		toFile = False
		toRender = False
		if exportSettings.export_menu.getSelectedIndex() == 0:
			toFile = True			
			filename = exportSettings.filename.getValue()
		elif exportSettings.export_menu.getSelectedIndex() == 1:
			toRender = True			
			filename = exportSettings.filename.getValue()
		else:
			toFile = False
			filename = exportSettings.textName.getValue()
		
		# ok, so the scene settings
		if makeFullScene:
			if toFile:
				ri.RiBegin(filename)
			elif toRender:
				ri.RiBegin("aqsis")
			else:
				ri.RiBegin()
			ri.RiFrameBegin(1) # just one frame for now		
			
			# Renderman scene stuff, exposure, samples, shading rate, etc.
			self.scene.render()
			imgFile = os.path.splitext(filename)[0] + ".tif "
			ri.RiDisplay(imgFile, "file", "rgb")
			
			# the blender camera (or somehow find the current 3D view, not sure how to do that yet.
			blenderScene = Blender.Scene.GetCurrent()
			
			camera = blenderScene.getCurrentCamera()
			if self.scene.object_data.has_key(camera.getName()):
				camAdapter = self.scene.object_data[camera.getName()] # fetch the camera adapter for this object
			else:
				# create a new camera adapter.
				bObj = BtoRTypes.__dict__["BtoR" + camera.getType()](camera)
				camAdapter = BtoRAdapterClasses.IObjectAdapter(bObj)		
				self.scene.object_data[camera.getName()] = camAdapter # assign the new adapter to the scene settings dict
			
			
			camAdapter.render()
			ri.RiWorldBegin()
			# grab all the lights	
			objs = blenderScene.getChildren()
			for obj in objs:
				if obj.getType() == "Lamp":
					if self.scene.object_data.has_key(obj.getName()):
						self.scene.object_data[obj.getName()].render()
					else:
						bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
						self.scene.object_data[obj.getName()] = BtoRAdapterClasses.IObjectAdapter(bObj)
						self.scene.object_data[obj.getName()].render() # move this redundant crap to it's own func
		
		ri.RiIdentity() # normalize the transform matrix here.
		
		ri.RiAttributeBegin()
		ri.RiTransformBegin()
		
		# if making a full scene here, include transform data
		if makeFullScene:
			# objMatrix = cgkit.cgtypes.mat4(self.objData.object.matrix)

			# so how to compensate for the weird Z axis thing?
			ri.RiTransform(self.objData.object.matrix)
		self.objData.render()
		
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
		
		if makeFullScene:
			ri.RiWorldEnd()
			# add possible support for exporting object animation stuff, but that should really be in the main scene rib
			# need to find out exactly how to do that.
			ri.RiFrameEnd()
			ri.RiEnd()
	
	def noPanel(self):
		self.editorPanel.removeElement(self.objectEditorPanel)
		
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
					self.noPanel()
					
				elif len(objects) > 1:
					self.objectName.isVisible = False
					self.objectMenu.isVisible = True
					self.obj_menu = [] # reset the menu
					self.obj_types = []
					for obj in objects:
						self.obj_menu.append(obj.getName())
						
					self.objects = objects
					self.objectMenu.re_init(self.obj_menu)
					self.objectMenu.setValue(1)
					self.selectObject_menu(None)
				else:
					self.objectName.isVisible = True
					self.objectMenu.isVisible = False
					obj = objects[0]
					self.selectObject(obj)
								
	def initializeBtoRObjectData(self): # this changes to support generation of BtoR export adapters
		# get everything in the current scene.
		# I should probably get the scene name here as well for reference, since BtoR *should* be able to store seperate scene setups in a .blend 
		# with multiple scenes in it.
		objList = Blender.Scene.getChildren()
		objectID = 0 # this ensures that all objects have a unique handle

		for object in objList:			
			self.scene.objData[object.getName()] = protocols.adapt(object.getType(), IObjectAdapter)	
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
	
class Material:	
	def __init__(self, material, parent, selector):	
		
		# material should be an RMMaterial			
		sdict = globals()		
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		#keys = dict.keys()
		#for key in keys:					
		#	if isinstance(dict[key], BtoRSettings):				
		#		self.settings = dict[key]
		#		# BtoRSettings object, globals
		#	elif isinstance(dict[key], ui.EventManager):
		#		self.evt_manager = dict[key]
		#		# event manager instance
		#	elif isinstance(dict[key], MaterialList):
		#		self.material_list = dict[key]
		#	elif isinstance(dict[key], SceneSettings):
		#		self.scene = dict[key]
				
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
		self.materialName.update_functions.append(self.rename)
		self.name = self.material.name
		
		# self.editorPanel.addElement(self.materialName)
		self.surface = Shader(self.material.surface, "surface", self)
		# surface shader button
		self.editorPanel.addElement(ui.Label(5, 65, "Surface Shader:", "Surface Shader:", self.editorPanel, True))
		self.surfaceShader = ui.Button(160, 65, 130, 20, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		self.editorPanel.addElement(self.surfaceShader)
		self.surfaceShader.registerCallback("release", self.surface.showEditor)
		
		self.displacement = Shader(self.material.displacement, "displacement", self)
		# diplacement shader button
		self.editorPanel.addElement(ui.Label(5, 90, "Displacement Shader:", "Displacement Shader:", self.editorPanel, True))
		self.displacementShader = ui.Button(160, 90, 130, 20, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		self.editorPanel.addElement(self.displacementShader)
		self.displacementShader.registerCallback("release", self.displacement.showEditor)
		
		self.volume = Shader(self.material.interior, "volume", self)
		# volume shader button
		self.editorPanel.addElement(ui.Label(5, 115, "Volume Shader:", "Volume Shader:", self.editorPanel, True))
		self.volumeShader = ui.Button(160, 115, 130, 20, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		self.editorPanel.addElement(self.volumeShader)
		self.volumeShader.registerCallback("release", self.volume.showEditor)
		
		# Color
		self.editorPanel.addElement(ui.Label(5, 160, "Color", "Color:", self.editorPanel, True))
		
		self.editorPanel.addElement(ui.Label(55, 160, "R", "R", self.editorPanel, False))
		self.color_red = ui.TextField(70, 160, 35, 20, "Red", float(self.material.color()[0]), self.editorPanel, True)
		self.color_red.update_functions.append(self.updateColor)
		
		
		self.editorPanel.addElement(ui.Label(110, 160, "G", "G", self.editorPanel, False))
		self.color_green = ui.TextField(130, 160, 35, 20, "Green", float(self.material.color()[1]), self.editorPanel, True)
		self.color_green.update_functions.append(self.updateColor)
		
		
		self.editorPanel.addElement(ui.Label(175, 160, "B", "B", self.editorPanel, True))		
		self.color_blue = ui.TextField(190, 160, 35, 20, "Blue", float(self.material.color()[2]), self.editorPanel, True) 
		self.color_blue.update_functions.append(self.updateColor)
		
		
		self.color = ui.ColorButton(230, 160, 25, 20, "Color", [float(self.color_red.getValue()) * 255, float(self.color_green.getValue()) * 255, float(self.color_blue.getValue()) * 255, 255], self.editorPanel, True)
		self.editorPanel.addElement(self.color)
		self.color.picking = False
		self.color.picker.ok_functions.append(self.updateColorFields)
		
		# opacity
		self.editorPanel.addElement(ui.Label(5, 190, "Opacity", "Opacity:", self.editorPanel, True))
		
		self.editorPanel.addElement(ui.Label(55, 190, "R", "R", self.editorPanel, False))
		self.opacity_red = ui.TextField(70, 190, 35, 20, "Red", float(self.material.opacity()[0]), self.editorPanel, True)
		self.opacity_red.update_functions.append(self.updateOpacity)
		
		self.editorPanel.addElement(ui.Label(110, 190, "G", "G", self.editorPanel, False))
		self.opacity_green = ui.TextField(130, 190, 35, 20, "Green", float(self.material.opacity()[1]), self.editorPanel, True)
		self.opacity_green.update_functions.append(self.updateOpacity)
		
		self.editorPanel.addElement(ui.Label(175, 190, "B", "B", self.editorPanel, True))
		self.opacity_blue = ui.TextField(190, 190, 35, 20, "Blue", float(self.material.opacity()[2]), self.editorPanel, True) 
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
		self.close_button.registerCallback("release", self.close)
		#self.previewLightMenu = ui.Menu(15, 283, 135, 25, "Preview Light Setup", ["Pointlights", "Spotlight", "Distantlight", "Hemilight"], self.editorPanel, True)
		self.previewTypeMenu = ui.Menu(15, 250, 135, 25, "Preview Type", ["Bicubic Sphere", "Cube", "Suzanne - Mesh", "Suzanne - Subdiv", "Teapot"], self.editorPanel, True)
		
		self.preview_button = ui.Button(15, 315, 135, 25, "Preview Material:", "Preview Material:", 'normal', self.editorPanel, True)
		self.preview_button.registerCallback("release", self.renderPreview)
		
		# preview image
		# Try to find my image, 
		try:
			self.image = Blender.Image.Get(self.material.name)
		except:
			self.image = Blender.Image.New(self.material.name, 128, 128, 24)
			
		self.editorPanel.addElement(ui.Image(165, 215, 128, 128, self.image, self.editorPanel, False))
		
		
		# editor button that displays the material editor
		self.editorButton = ui.ToggleButton(0, 0, 180, 65, self.material.name, self.material.name, 'normal', parent, True)
		self.editorButton.shadowed = False
		self.editorButton.outlined = True
		self.editorButton.textlocation = 1
		
		self.editorButton.registerCallback("release", self.showEditor)		
		self.editorButton.image = ui.Image(120, 5, 56, 56,  self.image, self.editorButton, False)
		
		self.selectorButton = ui.ToggleButton(0, 0, 180, 65, self.material.name, self.material.name, 'normal', selector, True)
		self.selectorButton.shadowed = False
		self.selectorButton.outlined = True
		self.selectorButton.textlocation = 1
		self.selectorButton.image = ui.Image(120, 5, 56, 56, self.image, self.selectorButton, False)
		self.selectorButton.title = self.editorButton.title
					
		self.select_functions = []
		if render:
			# I've got a prebuilt material, so setup the names and what-not.
			if self.surface.shader.shaderName() != None:
				self.surfaceShader.title = self.surface.shader.shaderName()
			if self.displacement.shader.shaderName() != None:
				self.displacementShader.title = self.displacement.shader.shaderName()
			if self.volume.shader.shaderName() != None:
				self.volumeShader.title = self.volume.shader.shaderName()
			self.renderPreview(None)

		
			
	def setImage(self, image):
		self.image = image
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		
	def rename(self, objl):
		# add in redundancy checking here to ensure that I don't have two materials of the same name.
		# see if he name exists in the material list already
		
		for button in self.material_list.scroller.elments:
			if self.button.title == self.materialName.getValue():
				duplicate_name = True
				break
		if duplicate_name:
			self.materialName.setValue
			self.evt_manager.showConfirmDialog("Material name Already Exists!", "That material name already exists. Please choose another.", None, False)
			
		else:
			self.editorButton.title = self.materialName.getValue()
			# also on a material rename, I should probably search the scene data to figure out if any objects reference this material and change the referenc to match.
			# thus			
			for object in self.scene.object_data.itervalues():
				try:
					if object["material"] == self.name:
						object["material"] = self.materialName.getValue()
				except:
					print "Some kind of error occured"
			self.name = self.materialName.getValue()
									
		
	def getEditor(self):
		return self.editorPanel
		
	def getMaterialButton(self):
		return self.editorButton
		
	def getSelectorButton(self):
		return self.selectorButton
	
	def selectMaterial(self):
		for func in self.select_functions:
			func(self)
		
	def showEditor(self, button):
		self.evt_manager.addElement(self.editorPanel)		
		
	def draw(self):
		self.editorPanel.draw()
		
	def updateColorFields(self, obj):
		color = self.color.getValue()
		
		# color values in the text boxes are assigned via ye olde renderman method, i.e. 0-1
		self.color_red.setValue(float(float(color[0]) / 255))
		self.color_green.setValue(float(float(color[1]) / 255))
		self.color_blue.setValue(float(float(color[2]) / 255))
		setattr(self.material, "_color", cgkit.cgtypes.vec3(float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue())))
		# self.material._color = (float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue()))
		
	def updateOpacityFields(self, obj):
		color = self.opacity.getValue()
		
		# color values in the text boxes are assigned via ye olde renderman method, i.e. 0-1
		self.opacity_red.setValue(float(float(color[0]) / 255))
		self.opacity_green.setValue(float(float(color[1]) / 255))
		self.opacity_blue.setValue(float(float(color[2]) / 255))
		
		# self.material._opacity = (float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue()))
		setattr(self.material, "_opacity", cgkit.cgtypes.vec3(float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue())))
	
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
			
			setattr(self.material, "_color", cgkit.cgtypes.vec3(float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue())))
			# self.material._color = (float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue()))
			
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
			
			setattr(self.material, "_opacity", cgkit.cgtypes.vec3(float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue())))
			# self.material._opacity = (float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue()))
	
	def renderPreview(self, button):
		# here, render a preview image using the current material
		# step 1 is to find a place to put this, so consult the BtoR settings to see where that is.
		filename = os.path.normpath(self.settings.outputPath + os.sep + "material_" + self.material.name + ".tif")
		# print "Selected renderer: ", self.settings.renderers[self.settings.renderer][0]
		ri.RiBegin(self.settings.renderers[self.settings.renderer][0] + " -v=3") # replace this with render flags
		# ri.RiBegin(os.path.normpath(self.settings.outputPath + os.sep + "material_" + self.material.name + ".rib"))
		# get the shader options
		paths = self.settings.shaderpaths.getValue().split(";")
		# setup the shader paths
		#sPath = ""
		#for path in paths:
		#	sPath = sPath + ":" + path
		#sPath = sPath + ":@"
		#ri.RiOption("searchpath", "shader", sPath)
		#ri.RiOption("searchpath", "shader", paths)
		for path in paths:
			ri.RiOption("searchpath", "shader", path + ":&")
			
		ri.RiFormat(128, 128, 1)
		# ri.RiProjection("perspective", "fov", 22.5)
		ri.RiAttribute("displacementbound", "coordinatesystem", "shader", "sphere", 1.5)
		ri.RiDisplay(filename, "file", "rgba")
		ri.RiTranslate(0, 0, 2.7)
		ri.RiWorldBegin()
		# ri.RiRotate(45, 1, 1, 0)
		ri.RiAttributeBegin()
		ri.RiCoordinateSystem("world")
		ri.RiAttributeEnd()
		self.renderLights()
		ri.RiAttributeBegin() 
		# ok here I need to get the material settings out of the material object somehow.
		# check the value of the surface shader
		if self.surface.shader.shadername != None:	
			self.surface.updateShaderParams()
			parms = dict()
			
			for parm in self.material.surface.shaderparams:
				# get the param and stuff into my dict				
				parms[parm] = getattr(self.material.surface, parm)
				ri.RiDeclare(parm, self.material.surface.shaderparams[parm])
			# now	
			ri.RiSurface(self.material.surface.shadername, parms)
		if self.displacement.shader.shadername != None:				
			self.displacement.updateShaderParams()
			parms = dict()
			for parm in self.material.displacement.shaderparams:
				parms[parm] = getattr(self.material.displacement, parm)
				ri.RiDeclare(parm, self.material.displacement.shaderparams[parm])
				
			ri.RiDisplacement(self.material.displacement.shadername, parms)
			
		if self.volume.shader.shadername != None:			
			parms = dict()
			self.volume.updateShaderParams()
			for parm in self.material.interior.shaderparams:
				parms[parm] = getattr(self.material.interior, parm)
				ri.RiDeclare(parm,self.material.interior.shaderparams[parm])				
			ri.RiAtmosphere(self.material.interior.shadername, parms)
		color = self.material.color()
		opacity = self.material.opacity()
		ri.RiTransformBegin()
		ri.RiScale(1, 1, 1)
		ri.RiColor([color[0], color[1], color[2]])		
		ri.RiOpacity([opacity[0], opacity[1], opacity[2]])

		previewType = self.previewTypeMenu.getValue()
		if previewType == "Bicubic Sphere":
			self.renderBicubicSphere()
		elif previewType == "Cube":
			self.renderUnitCube()
		elif previewType == "Suzanne - Mesh":
			self.renderSuzanneMesh()
		elif previewType == "Suzanne - Subdiv":
			self.renderSuzanneSubdiv()
		elif previewType == "Teapot":
			self.renderTeapot()
		else:
			self.renderBicubicSphere()
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
		# self.renderBackground()
		ri.RiWorldEnd()
		ri.RiEnd()
		
		# here I want to load the image I just rendered into the material and draw it.		
		self.image.setFilename(filename)
		self.image.reload()
		
	def renderBicubicSphere(self):
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		ri.RiSphere(0.75, -0.75, 0.75, 360)
		ri.RiTransformEnd()
		
	def renderSuzanneMesh(self):
		self.scene.suzanne.isSubdiv = False
		ri.RiRotate(180, 0, 1, 0)
		ri.RiRotate(27, 1, 0, 0)
		ri.RiRotate(-30, 0, 1, 0)
		ri.RiScale(.74, .74, .74)
		self.scene.suzanne.render()
		
	def renderSuzanneSubdiv(self):
		ri.RiRotate(180, 0, 1, 0)
		ri.RiRotate(27, 1, 0, 0)
		ri.RiRotate(-30, 0, 1, 0)
		ri.RiScale(.74, .74, .74)
		self.scene.suzanne.isSubdiv = True
		self.scene.suzanne.render()
	
	def renderTeapot(self):
		ri.RiTransformBegin()
		ri.RiTranslate(0, -.25, .5)
		ri.RiRotate(180, 0, 0, 1)
		ri.RiRotate(125, 1, 0, 0)
		ri.RiRotate(-45, 0, 0, 1)
		#ri.RiRotate(45, 0, 0, 1)
		ri.RiScale(0.35, 0.35, 0.35)
		teapotRIB = """
		AttributeBegin
		Basis "bezier" 3 "bezier" 3
		PatchMesh "bicubic" 13 "nonperiodic" 10 "nonperiodic" "P" [1.5 0 0 1.5 0.828427 0 0.828427 1.5 0 0 1.5 0 -0.828427 1.5 0 -1.5 0.828427 0 -1.5 0 0 -1.5 -0.828427 0 -0.828427 -1.5 0 0 -1.5 0 0.828427 -1.5 0 1.5 -0.828427 0 1.5 0 0 1.5 0 0.075 1.5 0.828427 0.075 0.828427 1.5 0.075 0 1.5 0.075 -0.828427 1.5 0.075 -1.5 0.828427 0.075 -1.5 0 0.075 -1.5 -0.828427 0.075 -0.828427 -1.5 0.075 0 -1.5 0.075 0.828427 -1.5 0.075 1.5 -0.828427 0.075 1.5 0 0.075 2 0 0.3 2 1.10457 0.3 1.10457 2 0.3 0 2 0.3 -1.10457 2 0.3 -2 1.10457 0.3 -2 0 0.3 -2 -1.10457 0.3 -1.10457 -2 0.3 0 -2 0.3 1.10457 -2 0.3 2 -1.10457 0.3 2 0 0.3 2 0 0.75 2 1.10457 0.75 1.10457 2 0.75 0 2 0.75 -1.10457 2 0.75 -2 1.10457 0.75 -2 0 0.75 -2 -1.10457 0.75 -1.10457 -2 0.75 0 -2 0.75 1.10457 -2 0.75 2 -1.10457 0.75 2 0 0.75 2 0 1.2 2 1.10457 1.2 1.10457 2 1.2 0 2 1.2 -1.10457 2 1.2 -2 1.10457 1.2 -2 0 1.2 -2 -1.10457 1.2 -1.10457 -2 1.2 0 -2 1.2 1.10457 -2 1.2 2 -1.10457 1.2 2 0 1.2 1.75 0 1.725 1.75 0.966498 1.725 0.966498 1.75 1.725 0 1.75 1.725 -0.966498 1.75 1.725 -1.75 0.966498 1.725 -1.75 0 1.725 -1.75 -0.966498 1.725 -0.966498 -1.75 1.725 0 -1.75 1.725 0.966498 -1.75 1.725 1.75 -0.966498 1.725 1.75 0 1.725 1.5 0 2.25 1.5 0.828427 2.25 0.828427 1.5 2.25 0 1.5 2.25 -0.828427 1.5 2.25 -1.5 0.828427 2.25 -1.5 0 2.25 -1.5 -0.828427 2.25 -0.828427 -1.5 2.25 0 -1.5 2.25 0.828427 -1.5 2.25 1.5 -0.828427 2.25 1.5 0 2.25 1.4375 0 2.38125 1.4375 0.793909 2.38125 0.793909 1.4375 2.38125 0 1.4375 2.38125 -0.793909 1.4375 2.38125 -1.4375 0.793909 2.38125 -1.4375 0 2.38125 -1.4375 -0.793909 2.38125 -0.793909 -1.4375 2.38125 0 -1.4375 2.38125 0.793909 -1.4375 2.38125 1.4375 -0.793909 2.38125 1.4375 0 2.38125 1.3375 0 2.38125 1.3375 0.738681 2.38125 0.738681 1.3375 2.38125 0 1.3375 2.38125 -0.738681 1.3375 2.38125 -1.3375 0.738681 2.38125 -1.3375 0 2.38125 -1.3375 -0.738681 2.38125 -0.738681 -1.3375 2.38125 0 -1.3375 2.38125 0.738681 -1.3375 2.38125 1.3375 -0.738681 2.38125 1.3375 0 2.38125 1.4 0 2.25 1.4 0.773198 2.25 0.773198 1.4 2.25 0 1.4 2.25 -0.773198 1.4 2.25 -1.4 0.773198 2.25 -1.4 0 2.25 -1.4 -0.773198 2.25 -0.773198 -1.4 2.25 0 -1.4 2.25 0.773198 -1.4 2.25 1.4 -0.773198 2.25 1.4 0 2.25 ]
		Basis "bezier" 3 "bezier" 3
		PatchMesh "bicubic" 13 "nonperiodic" 7 "nonperiodic" "P" [1.3 0 2.25 1.3 0.71797 2.25 0.71797 1.3 2.25 0 1.3 2.25 -0.71797 1.3 2.25 -1.3 0.71797 2.25 -1.3 0 2.25 -1.3 -0.71797 2.25 -0.71797 -1.3 2.25 0 -1.3 2.25 0.71797 -1.3 2.25 1.3 -0.71797 2.25 1.3 0 2.25 1.3 0 2.4 1.3 0.71797 2.4 0.71797 1.3 2.4 0 1.3 2.4 -0.71797 1.3 2.4 -1.3 0.71797 2.4 -1.3 0 2.4 -1.3 -0.71797 2.4 -0.71797 -1.3 2.4 0 -1.3 2.4 0.71797 -1.3 2.4 1.3 -0.71797 2.4 1.3 0 2.4 0.4 0 2.4 0.4 0.220914 2.4 0.220914 0.4 2.4 0 0.4 2.4 -0.220914 0.4 2.4 -0.4 0.220914 2.4 -0.4 0 2.4 -0.4 -0.220914 2.4 -0.220914 -0.4 2.4 0 -0.4 2.4 0.220914 -0.4 2.4 0.4 -0.220914 2.4 0.4 0 2.4 0.2 0 2.55 0.2 0.110457 2.55 0.110457 0.2 2.55 0 0.2 2.55 -0.110457 0.2 2.55 -0.2 0.110457 2.55 -0.2 0 2.55 -0.2 -0.110457 2.55 -0.110457 -0.2 2.55 0 -0.2 2.55 0.110457 -0.2 2.55 0.2 -0.110457 2.55 0.2 0 2.55 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0.8 0 3 0.8 0.441828 3 0.441828 0.8 3 0 0.8 3 -0.441828 0.8 3 -0.8 0.441828 3 -0.8 0 3 -0.8 -0.441828 3 -0.441828 -0.8 3 0 -0.8 3 0.441828 -0.8 3 0.8 -0.441828 3 0.8 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 ]
		Basis "bezier" 3 "bezier" 3
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [-2 0 0.75 -2 0.3 0.75 -1.9 0.3 0.45 -1.9 0 0.45 -2.5 0 0.975 -2.5 0.3 0.975 -2.65 0.3 0.7875 -2.65 0 0.7875 -2.7 0 1.425 -2.7 0.3 1.425 -3 0.3 1.2 -3 0 1.2 -2.7 0 1.65 -2.7 0.3 1.65 -3 0.3 1.65 -3 0 1.65 -2.7 0 1.875 -2.7 0.3 1.875 -3 0.3 2.1 -3 0 2.1 -2.3 0 1.875 -2.3 0.3 1.875 -2.5 0.3 2.1 -2.5 0 2.1 -1.6 0 1.875 -1.6 0.3 1.875 -1.5 0.3 2.1 -1.5 0 2.1 ]
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [2.8 0 2.25 2.8 0.15 2.25 3.2 0.15 2.25 3.2 0 2.25 2.9 0 2.325 2.9 0.25 2.325 3.45 0.15 2.3625 3.45 0 2.3625 2.8 0 2.325 2.8 0.25 2.325 3.525 0.25 2.34375 3.525 0 2.34375 2.7 0 2.25 2.7 0.25 2.25 3.3 0.25 2.25 3.3 0 2.25 2.3 0 1.95 2.3 0.25 1.95 2.4 0.25 1.875 2.4 0 1.875 2.6 0 1.275 2.6 0.66 1.275 3.1 0.66 0.675 3.1 0 0.675 1.7 0 1.275 1.7 0.66 1.275 1.7 0.66 0.45 1.7 0 0.45 ]
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [-1.9 0 0.45 -1.9 -0.3 0.45 -2 -0.3 0.75 -2 0 0.75 -2.65 0 0.7875 -2.65 -0.3 0.7875 -2.5 -0.3 0.975 -2.5 0 0.975 -3 0 1.2 -3 -0.3 1.2 -2.7 -0.3 1.425 -2.7 0 1.425 -3 0 1.65 -3 -0.3 1.65 -2.7 -0.3 1.65 -2.7 0 1.65 -3 0 2.1 -3 -0.3 2.1 -2.7 -0.3 1.875 -2.7 0 1.875 -2.5 0 2.1 -2.5 -0.3 2.1 -2.3 -0.3 1.875 -2.3 0 1.875 -1.5 0 2.1 -1.5 -0.3 2.1 -1.6 -0.3 1.875 -1.6 0 1.875 ]
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [3.2 0 2.25 3.2 -0.15 2.25 2.8 -0.15 2.25 2.8 0 2.25 3.45 0 2.3625 3.45 -0.15 2.3625 2.9 -0.25 2.325 2.9 0 2.325 3.525 0 2.34375 3.525 -0.25 2.34375 2.8 -0.25 2.325 2.8 0 2.325 3.3 0 2.25 3.3 -0.25 2.25 2.7 -0.25 2.25 2.7 0 2.25 2.4 0 1.875 2.4 -0.25 1.875 2.3 -0.25 1.95 2.3 0 1.95 3.1 0 0.675 3.1 -0.66 0.675 2.6 -0.66 1.275 2.6 0 1.275 1.7 0 0.45 1.7 -0.66 0.45 1.7 -0.66 1.275 1.7 0 1.275 ]
		AttributeEnd """
		ri._ribout.write(teapotRIB)
		ri.RiTransformEnd()

		
	def renderUnitCube(self):	
		ri.RiRotate(63, 1, 0, 0)
		ri.RiRotate(45, 0, 0, 1)
		
		#ri.RiScale(.70, .70, .70)
		# Far
		unitsize = .65	
		ri.RiTransformBegin()
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# right 
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# near
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# left
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# bottom
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# top
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
	
	def renderLights(self):
		ri.RiTransformBegin()
		# case checking needed here for lighting information
		ri.RiLightSource("ambientlight", "lightcolor", [0.151, 0.151, 0.151])
		ri.RiLightSource("distantlight", "lightcolor", [1, 1, 1], "from", [1, 1.5, -1], "to", [0, 0, 0], "intensity", 1)
		ri.RiLightSource("distantlight", "lightcolor", [0.7, 0.7, 0.7], "from", [-1.3, -1.2, -1.0], "to", [0, 0, 0], "intensity", 1)
		# ri.RiLightSource("distantlight", "lightcolor", [1, 1, 1], "from", [-1, -1.5, -1], "to", [0, 0, 0], "intensity", 1)
		# ri.RiLightSource("distantlight", "lightcolor", [1, 1, 1], "from", [-1, 1.5, -1], "to", [0, 0, 0], "intensity", 1)
		ri.RiTransformEnd()
		
	def renderBackground(self):
		ri.RiAttributeBegin()
		ri.RiTranslate(0, 0, 0.5)
		ri.RiRotate(90, 0, 1, 1)

		ri.RiColor([1, 1, 1])		
		ri.RiDeclare("frequency", "uniform float")
		ri.RiSurface("checker", "frequency", 1)
		ri.RiTransformBegin()
		# Far
		unitsize = 1.5
		ri.RiRotate(45, 1, 1, 1)
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# right 
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# near
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# left
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# bottom
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# top
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		# ri.RiPatch("bilinear", "P", [-2, 2, 0, 2, 2, 0, -2, -2, 0, 2 -2, 0])
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
	
class Shader:
	# what does this need to know?
	# the shader path for one thing...and the shader type...and the 
	# current shader itself
	# the shader, as an RMShader object doesn't know its own type probably, so I have to supply it
	# this needs to at some point construct the set of parameters that belongs to the selected shader.
	def __init__(self, shader, stype, material):
		sdict = globals()		
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
				
		if shader <> None:
			self.shader = shader
		else:
			self.shader = None # no selection, we'll be assigning it as we go.
			
		self.searchpaths = self.settings.shaderpaths.getValue().split(";")		
		self.stype = stype
		self.material = material # parent material
		
		# initialize the panel
		# find the center of the screen...again
		screen = Blender.Window.GetAreaSize()
		
		self.editorPanel = ui.Panel(225, screen[1] - 370, 350, 500, "ShaderEditor", "Shader Parameters:" + self.stype, None, False)
		#self.editorPanel.dialog = True
		# on the shader panel, we need the shader search path (with the ability to add to the list)
		self.editorPanel.addElement(ui.Label(5, 30, "SearchPathLabel", "Shader Search Path:", self.editorPanel, True))
		offset = self.editorPanel.get_string_width("Shader Search Path:", 'normal') + 12
		self.searchPaths = ui.Menu(offset, 30, 210, 20, "SearchPath",  self.searchpaths, self.editorPanel, True)
		self.searchPaths.registerCallback("release", self.listShaders)
		
		# next we need the shader type, so I know what I'm working with.
		self.editorPanel.addElement(ui.Label(5, 50, "ShaderType", "Shader Type: " + stype, self.editorPanel, False))
		
		self.editorPanel.addElement(ui.Label(5, 80, "Shader", "Shader:", self.editorPanel, False))
		
		# the starting shader type is Surface
		self.shaders = []
		self.makeShaderMenu()
				
		self.shader_menu = ui.Menu(self.editorPanel.get_string_width("Shader:", 'normal') + 12, 80, 150, 20, "Shaders", self.shadersMenu, self.editorPanel, True)
		self.shader_menu.registerCallback("release", self.selectShader)
		# now that THAT nastiness is done...
		self.scroller = ui.ScrollPane(5, 115, self.editorPanel.width - 10, self.editorPanel.height - 125, "Param Scroller", "parmScroller", self.editorPanel, True)
		self.scroller.normalColor = [185, 185, 185, 255]
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		#close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editorPanel)
		self.close_button.registerCallback("release", self.close)
		
		if self.shader.shaderparams != None:
			self.setupParamEditors()
		

		if self.shader.shaderName() != None:
			# set the shader name to the correct shader name in the menu
			# first, discover the path in use
			path = os.path.split(self.shader.filename)[0]
			# if the path isn't the same as the selected path, generate the shader list for it and set everything up
			if path != self.searchPaths.getValue():
				pIndex = self.searchpaths.index(path)
				self.searchPaths.setValue(pIndex)
				self.listShaders(None)
			sIndex = self.shadersMenu.index(self.shader.shaderName())
			self.shader_menu.setValue(sIndex)
		#else:
		#	self.selectShader(None)
		# otherwise, I'll be selecting a shader at the outset...
		self.value_tables = {} # vts are referenced per shader parameter
		self.hasValueTable = False
		
		
	def getValueTable(self, parameter):
		return value_tables[parameter]
		
	def listShaders(self, obj):
		self.settings.getShaderList(self.searchPaths.getValue())
		# reset the shader list menu
		self.shaders = []
		self.makeShaderMenu()
		self.shader_menu.re_init(self.shadersMenu)
		
	def getShaderName(self):
		return self.shader.shadername
	
	def getShaderFilename(self):
		return self.shader.shaderfilename
		
		
	def close(self, button):		
		self.evt_manager.removeElement(self.editorPanel)
		self.material.renderPreview(None)
		
	def getEditor(self):		
		return self.editorPanel
	
	def showEditor(self, button):
		self.evt_manager.addElement(self.editorPanel)
		
	def makeShaderMenu(self):
		# get the current shader path
		path = self.searchPaths.getValue()
		self.shadersMenu = []
		self.shadersMenu.append("None Selected")
		if self.stype == "surface":
			self.shaders = self.settings.shadersSurface[path]
			for shader in self.settings.shadersSurface[path]:
				self.shadersMenu.append(shader[1])
		elif self.stype == "displacement":
			self.shaders = self.settings.shadersDisplacement[path]
			for shader in self.settings.shadersDisplacement[path]:				
				self.shadersMenu.append(shader[1])
		elif self.stype == "volume":
			self.shaders = self.settings.shadersVolume[path]
			for shader in self.settings.shadersVolume[path]:
				self.shadersMenu.append(shader[1])
				
	def selectShader(self, button):
		# select the shader in question and then setup the params for it.
		# ditch the current parameter editors	
		path = self.searchPaths.getValue()
		self.scroller.clearElements()	
		self.shader = None	# clear all the old data.
		if self.shader_menu.getSelectedIndex() > 0:			
			shaderID = self.shader_menu.getSelectedIndex() - 1
			if self.stype == "surface":
				self.shaderParms = self.settings.shadersSurface[path][shaderID]
			elif self.stype == "displacement":
				self.shaderParms = self.settings.shadersDisplacement[path][shaderID]
			elif self.stype == "volume":
				self.shaderParms = self.settings.shadersVolume[path][shaderID]
			elif self.stype == "light":
				self.shaderParms = self.settings.shadersLight[path][shaderID]
			elif self.stype == "imager":
				self.shaderParms = self.settings.shadersImager[path][shaderID]				
				
			# initialize a new RMShader object
			if self.settings.use_slparams: 
				if self.stype == "surface":
					file = self.settings.surfaceFiles[path][shaderID]
				elif self.stype == "displacement":				
					file = self.settings.dispFiles[path][shaderID]
				elif self.stype == "volume":
					file = self.settings.volumeFiles[path][shaderID]
				elif self.stype == "light":
					file = self.settings.lightFiles[path][shaderID]
				elif self.stype == "imager":
					file = self.settings.imagerFiles[path][shaderID]
					
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
				
		if self.material != None:
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
			# test here for an array value
			normalColor = [216, 214, 220, 255]
			#if count % 2:
			#	normalColor = [190, 190, 190, 255]			
			p_type = self.shader.shaderparams[param].split()[1]
			
			if "[" in p_type:
				# this is an array value, react accordingly
				# perhaps I can simply look at the _slot in the shader and decide from there?
				arrayLen = self.shader.__dict__[param + "_slot"].size() # this should be there if everything was initialized correctly!
				# and the type name
				idx = p_type.index("[")
				isArray = True
				p_type = p_type[:idx]
				# if of type array, setup 
			else:
				isArray = False
			# get the default value from the shader
			iParams = self.shader.params()			
			defValue = getattr(self.shader, param)
			size = [0, 0, self.scroller.width - 30, 0]
			if isArray:
				bParm = BtoRTypes.__dict__["BtoRArrayParam"](param = param, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
			else:
				bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize() + "Param"](param = param, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
			editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick
			self.scroller.addElement(editor)
			count = count + 1
		self.scroller.invalid = True
		self.scroller.currentItem = 1
	
	def declareParams(self):
		for parm in self.shaderparams:
			ri.RiDeclare(parm[0], parm[1])
			
	
	def obfuscate(self):
		p_type = "None"
		if True:
			if p_type == "float":
				# create a float editor
				if isArray:
					self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.FloatEditor.height, param, "FloatEditor", defValue, arrayLen, self.scroller, False))
				else:
					self.scroller.addElement(ui.FloatEditor(0, 0, self.scroller.width - 30, ui.FloatEditor.height, param, defValue, self.scroller, False))
			elif p_type == "string":
				# time for Black Magick and v00d0u
				if ("tex" in param or "map" in param or "name" in param):
					if isArray:
						self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.FileEditor.height, param, "FileEditor", defValue, arrayLen, self.scroller, False))
					else:
						self.scroller.addElement(ui.FileEditor(0, 0, self.scroller.width - 30, ui.FileEditor.height, param, defValue, self.scroller, False))
				elif "space" in param or "Space" in param:
					if isArray:
						self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.SpaceEditor.height, param, "SpaceEditor", defValue, arrayLen, self.scroller, False))
					else:
						self.scroller.addElement(ui.SpaceEditor(0, 0, self.scroller.width - 30, ui.SpaceEditor.height, param, defValue, self.scroller, False))
				elif "proj" in param or "Proj" in param:
					if isArray:
						self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.SpaceEditor.height, param, "ProjectionEditor", defValue, arrayLen, self.scroller, False))
					else:
						self.scroller.addElement(ui.ProjectionEditor(0, 0, self.scroller.width - 30, ui.SpaceEditor.height, param, defValue, self.scroller, False))
				else:
					if isArray:
						self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.TextEditor.height, param, "TextEditor", defValue, arrayLen, self.scroller, False))
					else:
						self.scroller.addElement(ui.TextEditor(0, 0, self.scroller.width - 30, ui.TextEditor.height, param, defValue, self.scroller, False))
			elif p_type == "color":
				color = []				
				color.append(defValue[0])
				color.append(defValue[1])
				color.append(defValue[2])
				if isArray:
					self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.ColorEditor.height, param, "ColorEditor", defValue, arrayLen, self.scroller, False))
				else:
					self.scroller.addElement(ui.ColorEditor(0, 0, self.scroller.width - 30, ui.ColorEditor.height, param, color, self.scroller, False))
			elif p_type in ["point", "vector", "normal"]:	
				if isArray:
					self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param, "CoordinateEditor", defValue, arrayLen, self.scroller, False))
				else:
					self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param, defValue, self.scroller, False))
			elif p_type == "matrix":
				if isArray:
					self.scroller.addElement(ui.ArrayEditor(0, 0, self.scroller.width - 30, ui.MatrixEditor.height, param, "MatrixEditor", defValue, arrayLen, self.scroller, False))
				else:
					self.scroller.addElement(ui.MatrixEditor(0, 0, self.scroller.width - 30, ui.MatrixEditor.height, param, defValue, self.scroller, False))
					
			#self.scroller.elements[count].normalColor = normalColor	
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
			shader.declare(atts["name"], type=convtypes[switch], default = val)
			# shader.createSlot(atts["name"].nodeValue, convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here		
	
	
	def updateShaderParams(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		for element in self.scroller.elements:
			p_type = element.paramtype
			name = element.param_name
			if p_type == "float" or p_type == "string": # lo, all my single-variable types
				setattr(self.shader, name, self.scroller.elements[index].getValue()) 
				
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = self.scroller.elements[index].getValue()
				vec = cgkit.cgtypes.vec3(val[0], val[1], val[2])
				setattr(self.shader, name, vec)
				
			elif p_type == "matrix": # matrix types				
				val = self.scroller.elements[index].getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				setattr(self.shader, name, matrix)
			
			index = index + 1

class GenericShader:
	def __init__(self, shader, s_type, parent):
		self.parent = parent
		sdict = globals()
			
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
				
		if shader <> None:
			self.shader = shader
		else:
			self.shader = None # no selection, we'll be assigning it as we go.
			
		searchpaths = self.settings.shaderpaths.getValue().split(";")	
		self.s_type = s_type
		
		# initialize the panel
		# find the center of the screen...again
		screen = Blender.Window.GetAreaSize()
		
		self.editorPanel = ui.Panel(225, screen[1] - 370, 350, 500, "ShaderEditor", "Shader Parameters: Light", None, False)
		self.editorPanel.dialog = True
		# on the shader panel, we need the shader search path (with the ability to add to the list)
		self.editorPanel.addElement(ui.Label(5, 30, "SearchPathLabel", "Shader Search Path:", self.editorPanel, True))
		offset = self.editorPanel.get_string_width("Shader Search Path:", 'normal') + 12
		self.searchPaths = ui.Menu(offset, 30, 210, 20, "SearchPath",  searchpaths, self.editorPanel, True)
		self.searchPaths.registerCallback("release", self.listShaders)
		
		# next we need the shader type, so I know what I'm working with.
		self.editorPanel.addElement(ui.Label(5, 50, "ShaderType", "Shader Type: Light", self.editorPanel, False))
		
		self.editorPanel.addElement(ui.Label(5, 80, "Shader", "Shader:", self.editorPanel, False))
		
		self.shaders = []
		self.makeShaderMenu()
				
		self.shader_menu = ui.Menu(self.editorPanel.get_string_width("Shader:", 'normal') + 12, 80, 150, 20, "Shaders", self.shadersMenu, self.editorPanel, True)
		self.shader_menu.registerCallback("release", self.selectShader)
		# now that THAT nastiness is done...
		self.scroller = ui.ScrollPane(5, 115, self.editorPanel.width - 10, self.editorPanel.height - 125, "Param Scroller", "parmScroller", self.editorPanel, True)
		self.scroller.normalColor = [185, 185, 185, 255]
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		#close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editorPanel)
		self.close_button.registerCallback("release", self.close)
		
		if self.shader != None:
			if self.shader.shaderparams != None:
				self.setupParamEditors()
		
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
		
	def getShaderName(self):
		if self.shader == None:
			return "None Selected"
		else:
			return self.shader.shadername
		
	def getShaderFilename(self):
		return self.shader.shaderfilename
		
	def close(self, button):		
		self.updateShaderParams()
		self.evt_manager.removeElement(self.editorPanel)
		# no preview neccessar here
		
	def getEditor(self):		
		return self.editorPanel
	def showEditor(self, button):
		self.evt_manager.addElement(self.editorPanel)
	def setParamValue(self, parameter, value):
		for element in self.scroller.elements:
			if element.param_name == parameter:
				element.setValue(value)
		
	def makeShaderMenu(self):
		self.shadersMenu = []
		path = self.searchPaths.getValue()
		self.shadersMenu.append("None Selected")
		if self.s_type == "light":
			self.shaders = self.settings.shadersLight[path]
			for shader in self.settings.shadersLight[path]:
				self.shadersMenu.append(shader[1])
		elif self.s_type == "imager":
		    self.shaders = self.settings.shadersImager[path]
		    for shader in self.settings.shadersImager[path]:
		        self.shadersMenu.append(shader[1])
                

	def selectShader(self, button):
		# select the shader in question and then setup the params for it.
		# ditch the current parameter editors	
		path = self.searchPaths.getValue()
		self.scroller.clearElements()	
		self.shader = None	# clear all the old data.
		if self.shader_menu.getSelectedIndex() > 0:			
			shaderID = self.shader_menu.getSelectedIndex() - 1
			if self.s_type == "light":
				self.shaderParms = self.settings.shadersLight[path][shaderID]
			elif self.s_type == "imager":
				self.shaderParms = self.settings.shadersImager[path][shaderID]
			# initialize a new RMShader object
			if self.settings.use_slparams: 
				if self.s_type == "light":
					file = self.settings.lightFiles[path][shaderID]
				elif self.s_type == "imager":
					file = self.settings.imagerFiles[path][shaderID]
					
				# get the search path from the settings object, and use that to find the shader source
				# hopefully it's the same as the shader name
				self.shader = cgkit.rmshader.RMShader(self.searchPaths.getValue() + os.sep + file)
				self.setupParamEditors()
			else:
				self.shader = cgkit.rmshader.RMShader()
				self.initShaderParams(self.shaderParms, self.shader)				
				self.setupParamEditors()
				 
		if self.shader == None:
			self.shader = cgkit.rmshader.RMShader() # blank shader
			
		if self.parent != None:
			self.parent.getEditor().shaderButton.title = self.shader_menu.getValue()
			
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
			p_type = self.shader.shaderparams[param].split()[1]
			# get the default value from the shader
			iParams = self.shader.params()			
			defValue = getattr(self.shader, param)
			if "[" in p_type:
				# this is an array value, react accordingly
				# perhaps I can simply look at the _slot in the shader and decide from there?
				arrayLen = self.shader.__dict__[param + "_slot"].size() # this should be there if everything was initialized correctly!
				# and the type name
				idx = p_type.index("[")
				isArray = True
				p_type = p_type[:idx]
				# if of type array, setup 
			else:
				isArray = False
			# I should be able to instantiate an editor like this
			size = [0, 0, self.scroller.width - 30, 0]
			if isArray:
				bParm = BtoRTypes.__dict__["BtoRArrayParam"](param = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
			else:
				bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize() + "Param"](param = param, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
			editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick
			self.scroller.addElement(editor)
			count = count + 1
		self.scroller.invalid = True
		self.scroller.currentItem = 1
		
	def obfuscate(self):
		p_type = "none"
		if True:
			if p_type == "float":
				# create a float editor
				self.scroller.addElement(ui.FloatEditor(0, 0, self.scroller.width - 30, ui.FloatEditor.height, param, defValue, self.scroller, False))
			elif p_type == "string":
				self.scroller.addElement(ui.TextEditor(0, 0, self.scroller.width - 30, ui.TextEditor.height, param, defValue, self.scroller, False))
			elif p_type == "color":
				color = []				
				color.append(defValue[0])
				color.append(defValue[1])
				color.append(defValue[2])
				self.scroller.addElement(ui.ColorEditor(0, 0, self.scroller.width - 30, ui.ColorEditor.height, param, color, self.scroller, False))
			elif p_type == "point":	
				self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param, defValue, self.scroller, False))
			elif p_type == "vector":
				self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param, defValue, self.scroller, False))
			elif p_type == "normal":
				self.scroller.addElement(ui.CoordinateEditor(0, 0, self.scroller.width - 30, ui.CoordinateEditor.height, param, defValue, self.scroller, False))
			elif p_type == "matrix":
				self.scroller.addElement(ui.MatrixEditor(0, 0, self.scroller.width - 30, ui.MatrixEditor.height, param, defValue, self.scroller, False))
			#self.scroller.elements[count].normalColor = normalColor			
	def updateShaderParams(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		for element in self.scroller.elements:
			p_type = element.paramtype
			name = element.param_name
			print "updating ", element.param_name, " to value: ", element.getValue()
			if p_type == "float" or p_type == "string": # lo, all my single-variable types
				setattr(self.shader, name, self.scroller.elements[index].getValue())
				
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = self.scroller.elements[index].getValue()
				vec = cgkit.cgtypes.vec3(val[0], val[1], val[2])
				setattr(self.shader, name, vec)
				
			elif p_type == "matrix": # matrix types				
				val = self.scroller.elements[index].getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				setattr(self.shader, name, matrix)
			print getattr(self.shader, name)
			index = index + 1

	def initShaderParamsList(self, paramlist, shader):
		convtypes = {"float":"double",
				"str":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		for key in paramlist:
			param = paramlist[key]
		
			if type(param) == "list":
				ptype = param[0]
				if ptype == "point" or "vector" or "normal" or "color":
					val = cgkit.types.vec3(float(ptype[1]), float(ptype[2]), float(ptype[3]))
									
				elif ptype == "matrix":
					val = cgkit.types.mat4(float(param[1]),
							float(param[2]),
							float(param[3]),
							float(param[4]),
							float(param[5]),
							float(param[6]),
							float(param[7]),
							float(param[8]),
							float(param[9]),
							float(param[10]),
							float(param[11]),
							float(param[12]),
							float(param[13]),
							float(param[14]),
							float(param[15]),
							float(param[16]))
			elif type(param) == "float" or type(param) == "int":
				ptype = "float"
				val = float(param)
			elif type(param) == "str":
				ptype = "string"
				val = param	
			shader.declare(key, type=convtypes[switch], default=val)
			# shader.createSlot(key + "_slot", convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here	
			
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
			shader.declare(atts["name"], type=convtypes[switch], default=val)
			# shader.createSlot(atts["name"].nodeValue, convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here	
class GeneticParamModifier:
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		screen = Blender.Window.GetAreaSize()
		self.editorPanel = ui.Panel(screen[0] - 200, screen[1] - 300, 400, 600, "Parameter modification:", "Parameter Modification:", None, False)
		self.shaderName = ui.Label(5, 27, "Shader Name: %s" % shader.shadername, "Shader Name: %s" % shader.shadername, self.editorPanel, True)
		self.parm_menu = []

		self.editorPanel.addElement(ui.Label(5, 40, "Parameter:", "Parameter:", self.editorPanel, False))
		self.parmMenu = ui.Menu(45, 40, "Parameters", self.parm_menu, self.editorPanel, True)
		self.addButton = ui.Button(self.parmMenu.width + 45, 40, "Add Parameter", "Add Parameter", self.editorPanel, True)
		self.addButton.registerCallback("release", self.addParm)
		self.scrollpane = ui.ScrollPane(2, 60, self.editorPanel.width - 4, self.editorPanel.height - 70, "Scroller", "Scroller", self.editorPanel, True)
		# I have nothing here.
		self.assigned_parms = []
	def init_load(self, parms):
		""" initialize the list of ranges """
		# remember to fix this
	def setShader(self, shader):
		self.assigned_parms = []
		self.shader = shader
		self.shaderName.setValue(shader.shaderName())
		# get the list of parameters
		for param in self.shader.shaderparams:
			self.parm_menu.append(param[0])
			self.parmMenu.reinit(self.parm_menu)
			
	def addParm(self, button):
		# somehow test for duplicates		
		parm = self.parmMenu.getValue()
		if parm in self.assigned_parms:
			self.evt_manager.showConfirmDialog("Error!", "You've already added that parameter!", None, False)
		else:
			parmType = self.shader.shaderparams[parm].split()[1].capitalize()
			if "[" in parmType:
				bObj = BtoRTypes.__dict__["BtoRArrayParam"](self.matName, self.shader, parm, "Array") # you know, there's no reason why this won't work...
			else:
				bObj = BtoRTypes.__dict__["BtoR" + parmType + "Param"](self.matName, self.shader, parm, parmType)			
			pObj = BtoRAdapterClasses.IShaderParamUI(bObj)
			editor = pObj.getEditor()
			containerPane = ui.Panel(0, 0, editor.width + 15, editor.height, "container", "container", self.scrollpane, False)
			containerPane.addElement(ui.CheckBox(0, 2, "", "", False, containerPane, False)) 
			iPane = ui.Panel(20, 0, editor.width, editor.height, "none", "none", self.containerPane, True)
			iPane.addElement(editor)
			self.scrollpane.addElement(containerPane) # should be all done			
			self.assigned_parms.append(parm)
		
	def removeSelectedParms(self, button):
		# somehow determine what the selected parameter is.
		# I suppose I have to embed all this stuff into a large toggle button?
		toRemove = []
		for element in self.scroller.elements:
			if element.elements[0].getValue():
				toRemove.append(element)
		for element in toRemove:
			self.scroller.removeElement(element)
	


class MainUI:
	# this is the main UI object, the "main menu" of sorts from which all things spring.
	def __init__(self):
		# find my settings
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		self.objecteditor = sdict["instBtoRObjects"]
		self.export = sdict["instBtoRExport"]
				
		screen = Blender.Window.GetAreaSize()		
		self.editor = ui.MenuBar(None, False)
		#self.editor = ui.Panel(10, screen[1] - 10, 200, 205, "BtoR", "Blender to Renderman:", None, False)		
		self.editor.outlined = True
		p = self.editor
		offset = 10
		self.file_menu = ["Save", "Save to file", "Exit"]
		width = self.editor.get_string_width("File", 'normal') + 5
		
		self.fileMenu = ui.Menu(5, 5, 50, 18, "File", self.file_menu, self.editor, True, 
										enableArrowButton = False, 
										noSelect = True,
										baseButtonTitle = "File",
										shadowed = False)
				
		self.fileMenu.registerCallbackForElementIndex(0, "release",  self.dialogTest)
				
		self.material_menu = ["Material List", "Import from XML", "Import from another .blend", "Export to XML"]
		self.materialMenu = ui.Menu(60, 5, 80, 18, "Materials", self.material_menu, self.editor, True, 
										enableArrowButton = False,
										noSelect = True,
										baseButtonTitle = "Materials",
										shadowed = False)
										
		self.materialMenu.width = self.fileMenu.get_string_width("Material", 'normal') + 5
								
		self.materialMenu.registerCallbackForElementIndex(0, "release", self.showMaterials)
		
		self.settings_menu = ["Global Settings", "Scene Settings"]
		self.settingsMenu = ui.Menu(150, 5, 80, 18, "Settings", self.settings_menu, self.editor, True, enableArrowButton = False,
										noSelect = True,
										baseButtonTitle = "Settings",
										shadowed = False)
		self.settingsMenu.registerCallbackForElementIndex(0, "release", self.showGlobal)
		self.settingsMenu.registerCallbackForElementIndex(1, "release", self.showSceneSettings)
		

		
		self.objectButt = ui.Button(240, 5, 80, 18, "Objects", "Objects", 'normal', self.editor, True) # launches a low-quality preview, preview shading rate determined by global settings
		self.objectButt.registerCallback("release", self.showObjectEditor)
		self.objectButt.shadowed = False
		#self.objectButt.registerCallback("release", p.callFactory(None, self.showEditor(self.settings.getEditor())))
		
		self.exportButt = ui.Button(350, 5, 50, 18, "Export", "Export", 'normal', self.editor, True)
		#self.exportButt.registerCallback("release", p.callFactory(None, self.showEditor(self.export.getEditor())))
		self.exportButt.registerCallback("release", self.showExport)
		self.exportButt.shadowed = False
		
		#self.materialsButt.registerCallback("release", p.callFactory(None, self.showEditor(self.materials.getEditor())))
		
	def dialogTest(self, button):
		self.evt_manager.showConfirmDialog("Some prompt!", "Another Prompt!", self.hideDialog, True)
	def hideDialog(self, dialog):
		self.evt_manager.removeElement(dialog)
		
	def showEditor(self, editor):
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showExport(self, button):
		editor = self.export.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showGlobal(self, button):
		editor = self.settings.getEditor()
		editor.dialog = True
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showMaterials(self, button):
		editor = self.materials.getEditor()		
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
				
	def showObjectEditor(self, button):
		editor = self.objecteditor.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showSceneSettings(self, button):
		editor = self.scene.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
	
	def getEditor(self):
		return self.editor
		
		
class MaterialList:
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		
		screen = Blender.Window.GetAreaSize()
		
		# self.picker.x = self.absX
		self.editor = ui.Panel(10, screen[1] - 225, 200, 400, "Material Selector", "Material Editor:", None, False)
		self.selector = ui.Panel(10, screen[1] - 225, 200, 375, "Material Selector", "Material Selector:", None, False)
		
		self.add_button = ui.Button(0, 370, 99, 30, "Add New", "Add New", 'normal', self.editor, True)
		self.add_button.shadowed = False
		self.add_button.cornermask = 8
		self.add_button.radius = 10
		self.add_button.registerCallback("release", self.showAddDialog)
		
		self.delete_button = ui.Button(101, 370, 99, 30, "Delete", "Delete", 'normal', self.editor, True)
		self.delete_button.shadowed = False
		self.delete_button.cornermask = 4 
		self.delete_button.radius = 10
		self.delete_button.registerCallback("release", self.showDeleteDialog)
		
		self.scroller = ui.ScrollPane(0, 27, 200, 340, "MaterialScroller", "Scroller", self.editor, True)
		self.scroller.normalColor = [185,185,185,255]
				
		self.sel_scroller = ui.ScrollPane(0, 27, 200, 340, "MaterialScroller", "Scroller", self.selector, True)
		self.sel_scroller.normalColor = [185,185,185,255]
			
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)

		self.materials = []
		
		self.stickyToggle = False
		self.sel_sticky = True
		self.select_functions = []
		
	def loadMaterials(self):
		self.materials = self.getSavedMaterials()

		for material in self.materials:
			# create a button editor for each material			
			#self.scroller.elements.append(material.getMaterialButton())
			material.editorButton.registerCallback("release", self.toggleOn)
			material.selectorButton.registerCallback("release", self.select)
	
	def getMaterial(self, materialName):
		for material in self.materials:
			# print material.materialName.getValue()
			if material.materialName.getValue() == materialName:
				return material
		return None
			
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
	
	def select(self, button):				
		
		if self.sel_sticky:
			for element in self.sel_scroller.elements:
				if element <> button:
					element.set_state(False)
			# simply set the state of this button and move on
			button.set_state(True)
			self.value = button.title
		else:
			# print button.state
			if button.state == False:
				self.value = None # no value, since nothing's on
				# do nothing, the button has been turned off
			else: # the button was turned on, turn everything else off
				for element in self.sel_scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.title
		# now what do I do? Somehow get the selected material back to the object editor.
	
		for func in self.select_functions:
			func(button) # links back to whatever's interested in selections.
				
	def getEditor(self):
		return self.editor
	
	def getSelector(self):
		return self.selector
	def setSelected(self, ID):
		# print "selecting ", ID
		for element in self.sel_scroller.elements:
			if element.title == ID:				
				element.set_state(True)
			else:
				element.set_state(False)
	def getSelected(self):
		button = None
		for element in self.sel_scroller.elements:
			if element.getValue():
				button = element
				break
				
		return button
	
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def showAddDialog(self, button): # coroutine, open the naming dialog
		self.evt_manager.showSimpleDialog("New Material:", "New Material Name:", "Material %d" % len(self.materials), self.addNew)
		
	def addNew(self, dialog):
		if dialog.state == dialog.OK:
			# assign the name
			# create the new RMMaterial
			rm_mat = cgkit.rmshader.RMMaterial(dialog.getValue())
			material = Material(rm_mat, self.scroller, self.sel_scroller)
			self.materials.append(material)			
			self.scroller.lastItem = 0
			self.sel_scroller.lastItem = 0
			
			material.selectorButton.registerCallback("release", self.select)
			material.editorButton.registerCallback("release", self.toggleOn)
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
						# print "Material name is ", name
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
						# print "Color: ", matColor
						# B2RMaterial Editor here
						BtoRmat = Material(rmmaterial, self.scroller, self.sel_scroller)
						# set the title
						BtoRmat.editorButton.title = name
						BtoRmat.selectorButton.title = name
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
				parm_value = parm.getAttribute("value").encode("ascii")
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
				shader.declare(p_name, type=convtypes[p_type], default=parm_value)
				# shader.createSlot(p_name, convtypes[p_type], None, parm_value) # Here we set the default value to the parameters incoming value.
			
			# and set the value 	
			setattr(shader, p_name, parm_value)
				
		
	
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
					value = getattr(mat.surface, parm)	
					# create an XML element for this value.
					s_type = surface.shaderparams[parm].split()[1]
					# setup as much of the node element as I can here
					parmNode.setAttribute("name", parm)
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
					value = getattr(disp, parm)
					# create an XML element for this value.
					s_type = disp.shaderparams[parm].split()[1]
					# setup as much of the node element as I can here
					parmNode.setAttribute("name", parm)
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
				material.volume.updateShaderParams()						
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
					value = getattr(volume, parm)
					# create an XML element for this value.
					s_type = volume.shaderparams[parm].split()[1]
					# setup as much of the node element as I can here
					parmNode.setAttribute("name", parm)
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
class GroupList:
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		setattr(BtoRAdapterClasses, "instBtoRGroupList", self)
		screen = Blender.Window.GetAreaSize()
		
		# self.picker.x = self.absX
		self.editor = ui.Panel((screen[0] / 2) - 200 , (screen[1] / 2) + 100, 200, 400, "Object Groups", "Object Groups:", None, False)

		self.add_button = ui.Button(0, 370, 99, 30, "Add New", "Add New", 'normal', self.editor, True)
		self.add_button.shadowed = False
		self.add_button.cornermask = 8
		self.add_button.radius = 10
		self.add_button.registerCallback("release", self.showAddDialog)
		
		self.delete_button = ui.Button(101, 370, 99, 30, "Delete", "Delete", 'normal', self.editor, True)
		self.delete_button.shadowed = False
		self.delete_button.cornermask = 4 
		self.delete_button.radius = 10
		self.delete_button.registerCallback("release", self.showDeleteDialog)
		
		self.sel_scroller = ui.ScrollPane(0, 27, 200, 340, "ObjectScroller", "Scroller", self.editor, True)
		self.sel_scroller.normalColor = [185,185,185,255]
				
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		
		self.stickyToggle = False
		self.sel_sticky = True
		
		self.select_functions = []
		
	def getEditor(self):
		return self.editor
		
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def select(self, button):				
		
		if self.sel_sticky:
			for element in self.sel_scroller.elements:
				if element <> button:
					element.set_state(False)
			# simply set the state of this button and move on
			button.set_state(True)
			self.value = button.title
		else:
			# print button.state
			if button.state == False:
				self.value = None # no value, since nothing's on
				# do nothing, the button has been turned off
			else: # the button was turned on, turn everything else off
				for element in self.sel_scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.title
		# now what do I do? Somehow get the selected material back to the object editor.
	
		for func in self.select_functions:
			func(button) # links back to whatever's interested in selections.
			
	def toggleOn(self, button):
		# sticky toggle keeps the button set, no matter what 		
		if self.stickyToggle:
			for element in self.sel_scroller.elements:
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
				for element in self.sel_scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.name 
				
	def showAddDialog(self, button): # coroutine, open the naming dialog
		self.evt_manager.showSimpleDialog("New Object Group:", "New Object Group Name:", "ObjGroup %d" % len(self.scene.object_groups.keys()), self.addNew)
		
	def addNew(self, dialog):
		self.scene.object_groups[dialog.getValue()] = [] # make it empty for the time being.
		selectorButton = ui.ToggleButton(0, 0, 180, 35, dialog.getValue(), dialog.getValue(), 'normal', self.sel_scroller, True)
		selectorButton.shadowed = False
		selectorButton.outlined = True
		selectorButton.textlocation = 1		
		selectorButton.title = dialog.getValue()
		self.sel_scroller.addElement(selectorButton)
		selectorButton.registerCallback("release", self.toggleOn)
		selectorButton.registerCallback("release", self.select)
		
	def showDeleteDialog(self, button):
		self.evt_manager.showConfirmDialog("Delete Object Group?", "Delete selected object group?",  self.deleteGroup, False)
	def deleteGroup(self, button):
		if dialog.state == dialog.OK:
			# find the selected material by chasing through the elements in the scroller and deleting the one that's found.
			for element in self.sel_scroller.elements:
				if element.getValue() == True:
					groupName = element.getTitle()
					self.scene.object_groups.popitem(groupName)					
					self.sel_scroller.removeElement(element)
					break
	def initializeGroups(self):
		for key in self.scene.object_groups.keys():
			selectorButton = ui.ToggleButton(0, 0, 180, 35, dialog.getValue(), dialog.getValue(), 'normal', selector, True)
			selectorButton.shadowed = False
			selectorButton.outlined = True
			selectorButton.textlocation = 1		
			selectorButton.title = dialog.getValue()
			self.sel_scroller.addElement(selectorButton)
			
	def setSelected(self, ID):
		# print "selecting ", ID
		for element in self.sel_scroller.elements:
			if element.title == ID:				
				element.set_state(True)
			else:
				element.set_state(False)
				
	def getSelectedValue(self):
		button = None
		for element in self.sel_scroller.elements:
			if element.getValue():
				button = element
				break
				
		return button.title
	
	
		
		
class ExportSettings:	
	def __init__(self):	
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
			
		screen = Blender.Window.GetAreaSize()		
		self.editorPanel = ui.Panel((screen[0] / 2) - 225 , (screen[1] / 2) + 150 , 450, 250, "Export Object", "Export Selected Object:", None, False)
		self.editorPanel.dialog = True
		
		self.editorPanel.addElement(ui.Label(10, 28, "Export to:", "Export to:", self.editorPanel, False))
		export_options = ["File...", "To selected renderer...", "To a Blender Text"]
		self.export_menu = ui.Menu(self.editorPanel.get_string_width("Export to:", 'normal') + 20, 28, 150, 20, "Export To?", export_options, self.editorPanel, True)
		self.export_menu.registerCallback("release",self.selectExportType)
		
		self.textLabel = ui.Label(10, 63, "Text Name:", "Text Name:", self.editorPanel, False)
		self.textLabel.isvisible = False
		self.editorPanel.addElement(self.textLabel)		
		self.textName = ui.TextField(self.editorPanel.get_string_width("Text Name:", 'normal') + 20, 63, 300, 20, "Text Name", "RIBOutput.rib", self.editorPanel, True)
		self.textName.isvisible = False
				
		self.fileLabel = ui.Label(10, 63, "Export Filename:", "Export Filename:", self.editorPanel, False)
		self.editorPanel.addElement(self.fileLabel)
		self.filename = ui.TextField(20 + self.editorPanel.get_string_width("Export Filename:", 'normal'), 63, 250, 20, "Filename", self.settings.outputpath.getValue() + os.sep + "output.rib", self.editorPanel, True)
		# the filename should probably reflect the object name in question.
		self.browseButton = ui.Button(self.filename.x + self.filename.width + 5, 63, 60, 20, "Browse", "Browse", 'normal', self.editorPanel, True)
		self.browseButton.registerCallback("release",self.openBrowseWindow)
		
		self.editorPanel.addElement(ui.Label(10, 95, "Export coordinates:", "Export coordinates:", self.editorPanel, False))
		coordinate_options = ["Use world coordinates", "Use object coordinates"]
		self.coordinate_menu = ui.Menu(self.editorPanel.get_string_width("Export coordinates:", 'normal') + 20, 95, 250, 20, "Coordinates to use:", coordinate_options, self.editorPanel, True)
		self.coordinate_menu.registerCallback("release",self.selectCoords)
		
		self.editorPanel.addElement(ui.Label(10, 135, "Export Materials?", "Export Materials?", self.editorPanel, False))
		self.material_menu = ui.Menu(self.editorPanel.get_string_width("Export Materials?", 'normal') + 20, 135, 80, 20, "Export Materials?", ["Yes", "No"], self.editorPanel, True)
		
		self.camera_label = ui.Label(10, 165, "Create renderable file?", "Include camera settings? (Creates renderable file)", self.editorPanel, True)
		self.camera_menu = ui.Menu(self.editorPanel.get_string_width("Include camera settings? (Creates renderable file)", 'normal') + 20, 165, 80, 20, "Include Camera?", ["No", "Current Camera", "Current 3D View"], self.editorPanel, True)
		
				
		# self.editorPanel.addElement(ui.Label(10, 160, "Duplicate Object by:", "Duplicate Object By:", self.editorPanel, False))
		# additive transform matrix, (or subtractive, multiplicative, divisive)
		# per-vertex of some other object
		# some kind of random script
		
		
		self.closeButton = ui.Button(10, self.editorPanel.height - 40, 100, 25, "Close", "Cancel", 'normal', self.editorPanel, True)
		self.closeButton.registerCallback("release",self.close)
		
		# the export button should be the last thing on the panel
		self.exportButton = ui.Button(self.editorPanel.width - 110, self.editorPanel.height - 40, 100, 25, "Export Object", "Export Object", 'normal',  self.editorPanel, True)
		self.exportButton.registerCallback("release",self.doExport)
				

		self.export_functions = []
		self.selectExportType(None)
		
	def getEditor(self):
		return self.editorPanel
		
	def selectCoords(self, button):
		if self.coordinate_menu.getSelectedIndex() == 0:
			self.camera_label.isVisible = True
			self.camera_menu.isVisible = True
		else:
			self.camera_label.isVisible = False
			self.camera_menu.isVisible = False
			
	def selectExportType(self, button):
		if self.export_menu.getSelectedIndex() == 0:
			self.fileLabel.isVisible = True			
			self.browseButton.isVisible = True
			self.filename.isVisible = True			
			self.textLabel.isVisible = False
			self.textName.isVisible = False
		elif self.export_menu.getSelectedIndex() == 1:
			self.textLabel.isVisible = False
			self.textName.isVisible = False
			self.fileLabel.isVisible = False
			self.browseButton.isVisible = False
			self.filename.isVisible = False			
		else:
			self.fileLabel.isVisible = False
			self.browseButton.isVisible = False
			self.filename.isVisible = False
			self.textLabel.isVisible = True
			self.textName.isVisible = True
		
	def doExport(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		for func in self.export_functions:
			func()
	
	def openBrowseWindow(self, button):
		Blender.Window.FileSelector(self.select, 'Choose a file')
		
	def select(self, file):
		#self.filename.value = file
		self.filename.setValue(file)
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
	
	def getValue(self):
		return self.filename.getValue()

class ExportUI:
	
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]

		screen = Blender.Window.GetAreaSize()		
		self.editorPanel = ui.Panel((screen[0] / 2) - 225 , (screen[1] / 2) + 150 , 450, 250, "Export Scene", "Export Scene:", None, False)
		self.editorPanel.dialog = True
		
		self.editorPanel.addElement(ui.Label(10, 28, "Export to:", "Export to:", self.editorPanel, False))
		export_options = ["Single file...", "File with archives...", "To selected renderer...", "To a Blender Text"]
		self.export_menu = ui.Menu(self.editorPanel.get_string_width("Export to:", 'normal') + 20, 28, 150, 20, "Export To?", export_options, self.editorPanel, True)
		self.export_menu.registerCallback("release",self.selectExportType)
		
		self.textLabel = ui.Label(10, 63, "Text Name:", "Text Name:", self.editorPanel, False)
		self.textLabel.isVisible = False
		self.editorPanel.addElement(self.textLabel)		
		self.textName = ui.TextField(self.editorPanel.get_string_width("Text Name:", 'normal') + 20, 63, 300, 20, "Text Name", "RIBOutput.rib", self.editorPanel, True)
		self.textName.isVisible = False
		
		self.fileLabel = ui.Label(10, 63, "Export Filename:", "Export Filename:", self.editorPanel, False)
		self.editorPanel.addElement(self.fileLabel)
		self.filename = ui.TextField(20 + self.editorPanel.get_string_width("Export Filename:", 'normal'), 63, 250, 20, "Filename", self.settings.outputpath.getValue() + os.sep + 'output.rib', self.editorPanel, True)
		# the filename should probably reflect the object name in question.
		self.browseButton = ui.Button(self.filename.x + self.filename.width + 5, 63, 60, 20, "Browse", "Browse", 'normal', self.editorPanel, True)
		self.browseButton.registerCallback("release",self.openBrowseWindow)
		
		self.archiveLabelA = ui.Label(10, 85, "Archives", "Archives will be written to an 'archive' subdirectory relative to the export file.", self.editorPanel, True)
		self.archiveLabelA.isVisible = False
		self.archiveLabelB = ui.Label(10, 105, "Archives", "Note: This overrides per-object settings.", self.editorPanel, True)
		self.archiveLabelB.isVisible = False
		
		self.exportButton = ui.Button(self.editorPanel.width - 110, self.editorPanel.height - 40, 100, 25, "Export Scene", "Export Scene", 'normal',  self.editorPanel, True)
		self.exportButton.registerCallback("release", self.doExport) # that should do it.
		
		self.closeButton = ui.Button(10, self.editorPanel.height - 40, 100, 25, "Close", "Cancel", 'normal', self.editorPanel, True)
		self.closeButton.registerCallback("release",self.close)
		self.renderAnimation = False
		
	def getEditor(self):
		return self.editorPanel
	
	def doExport(self, button):
		self.evt_manager.removeElement(self.getEditor())
		self.scene.renderScene(self)
		
	def selectExportType(self, button):
		if self.export_menu.getSelectedIndex() in [0, 1]:
			self.fileLabel.isVisible = True			
			self.browseButton.isVisible = True
			self.filename.isVisible = True			
			self.textLabel.isVisible = False
			self.textName.isVisible = False
			if self.export_menu.getSelectedIndex() == 0:
				self.archiveLabelA.isVisible = False
				self.archiveLabelB.isVisible = False

			else:
				self.archiveLabelA.isVisible = True
				self.archiveLabelB.isVisible = True
				
		elif self.export_menu.getSelectedIndex() == 2:
			self.textLabel.isVisible = False
			self.textName.isVisible = False
			self.fileLabel.isVisible = False
			self.browseButton.isVisible = False
			self.filename.isVisible = False
			self.archiveLabelA.isVisible = False	
			self.archiveLabelB.isVisible = False			
		else:
			self.fileLabel.isVisible = False
			self.browseButton.isVisible = False
			self.filename.isVisible = False
			self.textLabel.isVisible = True
			self.textName.isVisible = True
			self.archiveLabelA.isVisible = False
			self.archiveLabelB.isVisible = False
			
	def openBrowseWindow(self, button):
		Blender.Window.FileSelector(self.select, 'Choose a file')
		
	def select(self, file):
		self.filename.setValue(file)
		
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)

class LightManager:
	""" This is the light manager. It provides direct access to lights in the scene without having to select them in the blender window. 
	
	     this will allow me to load light setups/looks
	"""
	def __init__(self):
		# center the panel
		screen = Blender.Window.GetAreaSize()		
		self.editorPanel = ui.Panel((screen[0] / 2) - 225 , (screen[1] / 2) + 250 , 450, 250, "Export Scene", "Export Scene:", None, False)
		
		
		
		
class HelpWindow:
	# this is based on a simple panel
	width = 400
	# the height is based on the text size of normal text
	# thus, the height is a factor of the number of lines I wish to display. I figure 80 to 100 lines should suffice for the moment.		
	
	def __init__(self, lines, font_size = 'normal'):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		
		setattr(BtoRAdapterClasses, "instBtoRHelp", self) # and done		
		
		height = (ui.UIElement.font_sizes[font_size] + 4) * lines # font cell height, + a 2 pixel gutter on top and bottom for 50 lines		
		screen = Blender.Window.GetAreaSize()
		
		self.editorPanel = ui.Panel((screen[0] / 2) - 100,  screen[1] - (height + 34) / 2, 400, height + 34, "Help", "Help", None, False)
		self.editorPanel.normalColor = [255, 255, 208, 255]
		# now that I have my panel, setup my scroller		
		self.scroller = ui.ScrollPane(0, 27, 398, height, "Scroller", "Scroller", self.editorPanel, True)
		self.scroller.normalColor = [255, 255, 208, 255]
		self.font_size = font_size
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		self.text = "There is no help for you!"
		self.calculateLines()
		
	def getEditor(self):
		return self.editorPanel
		
	def setText(self, text):		
		self.scroller.clearElements()
		print text
		self.text = text
		if text != None:
			if len(text) > 0:
				self.calculateLines() # and done
	
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		
	def calculateLines(self):
		# the first thing I have to look for is new lines in the string and 
		print self.text
		words = re.compile('[ ]+').split(self.text)
		# words = self.text.split()
		
		lines = []
		line = ""				
		currentWidth = 0
		for word in words:
			if "\n" in word:
				while "\n" in word:				
					if len(word) == 1:
						# this entire word is a newline
						lines.append(line)
					else:
						idx = word.index("\n")
						# now that I have this, I need to remove the first part of this word					
						if idx == len(word) - 1: # the newline will be at the end of the word in most cases						
							word = word[:idx - 1] # get everything before that.		
							line = line + word 
							lines.append(line)
						elif idx == 0: # the newline is at the beginning of this word
							word = word[idx + 1:]
							line = line + word
							lines.append(line)
						else: # the newline is in the middle of the word
							prefix = word[:idx - 1] # text up to
							word = word[idx + 1:] # and the last snip
							line = line + prefix
							lines.append(line)
					line = ""
			else:
				word = word + " " # add a space on each end
				wordWidth = self.editorPanel.get_string_width(word, self.font_size)
				if currentWidth + wordWidth < self.width - 10: # 5 pixel buffer zone on either side
					line = line + word
					currentWidth = currentWidth + wordWidth
				else:
					lines.append(line)
					line = ""
					currentWidth = 0
		label_y = 30
		for line in lines:					
			self.scroller.addElement(ui.Label(5, label_y, line, line, self.scroller, False))
			label_y = label_y + self.editorPanel.font_cell_sizes[self.font_size][1] + 4 # 4 pixel gap here
			
		field_y = label_y
		for element in self.scroller.elements:
			element.normalColor = [255, 255, 208, 255] # get that nice "help screen yellow!"
		
		
