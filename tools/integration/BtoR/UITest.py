#######################################################################
# UITest.py - Test program for the BtoR GUI Library
# Version 0.1a
# Author: Bobby Parker                                                                         
#  This is a rudimentary test program to put BtoRGUIClasses through a somewhat limited set of "tests"
#  Basically I show some controls, all of the Shader parameter types, and a dialog pops up when you click the "Delete" button, 
#  and oh yeah, there's a toggleGroup in evidence on materials panel.
# 
#######################################################################

import Blender
from btor import BtoRGUIClasses as ui
reload(ui)

def type_select(obj):
	print obj.value 

def stickyTest(obj):
	print obj.value
	
def add_new(button):
	print button.title
	print button.normalColor

def launchDialog(button):
	evt_manager.showConfirmDialog("Test Confirm Dialog", "HI! I'm a dialog! Say YES to everything I say!",  printResults, False)
	
def printResults(dialog):
	if dialog.state == dialog.OK:
		print "Everything's OK!"
	elif dialog.state == dialog.DISCARD:
		print "Toss it!"
	else:
		print "Do nuttin'!"
	
conv = 0.00392156
theme = Blender.Window.Theme.Get()[0]
backGround = [0.65, 0.65, 0.65]

evt_manager = ui.EventManager()  # initialize the event manager GUI stuff first, since I need that for things like dialogs

# the two panels I need
materialPanel = ui.Panel(100, 600, 200, 400, "materials", "Materials:", None, False)
paramPanel = ui.Panel(315, 600, 500, 600, "shaders", "Material Settings:", None, False)
# colorPicker = ColorPicker(100, 1000, 350, 300, "colorPicker", "ColorPicker:", None)

# some buttons and stuff
SurfaceBut = ui.ToggleButton(0, 25, 50, 25, "Surface", "Surface", 'small', materialPanel, True)
SurfaceBut.shadowed = False
SurfaceBut.cornermask = 0
SurfaceBut.push_offset = 2

DispBut = ui.ToggleButton(50, 25, 50,  25, "Disp", "Displace", 'small', materialPanel, True)
DispBut.shadowed = False
DispBut.cornermask = 0
DispBut.push_offset = 2

VolumeBut = ui.ToggleButton(100, 25, 50, 25, "Volume", "Volume", 'small', materialPanel, True)
VolumeBut.shadowed = False
VolumeBut.cornermask = 0
VolumeBut.push_offset = 2

LightBut = ui.ToggleButton(150, 25, 50, 25, "Light", "Light", 'small', materialPanel, True)
LightBut.shadowed = False
LightBut.cornermask = 0
LightBut.push_offset = 2

# a toggle group for the above buttons
tGroup = ui.ToggleGroup([SurfaceBut, DispBut, VolumeBut, LightBut])
tGroup.funcs.append(type_select)

AddNew = ui.Button(0, 375, 100, 25, "AddNew", "Add New", 'small', materialPanel, True)
AddNew.radius = 10
AddNew.cornermask = 8
AddNew.shadowed = False
AddNew.push_offset = 2
AddNew.release_functions.append(add_new)

Delete = ui.Button(100, 375, 100, 25, "Delete", "Delete", 'small', materialPanel, True)
Delete.radius = 10
Delete.cornermask = 4
Delete.shadowed = False
Delete.push_offset = 2
Delete.release_functions.append(launchDialog)

materialPanel.offset = 7


ShaderTypes = ["Surface", "Displacement", "Volume", "Light"]

paramPane = ui.ScrollPane(0, 27, 500, 565, "ParameterList", "ParmList", paramPanel, True)
paramPane.normalColor = paramPanel.normalColor
newColor = [0, 0, 0, 255]
color = ui.UIElement.normalColor
newColor[0] = color[0] - 15
newColor[1] = color[1] - 15
newColor[2] = color[2] 
x = 1
paramPane.addElement(ui.ColorEditor(0, (x * 50), 500, 50, "Color Editor", [255, 255, 255, 255], paramPane, False))	
paramPane.addElement(ui.SpaceEditor(0, (x * 100), 500, 50, "Space Editor", "Hello", paramPane, False)) # this needs a way of getting a default value	
paramPane.addElement(ui.ColorSpaceEditor(0, (x * 150), 500, 50, "Color Space Editor", "Hello", paramPane, False)) # this needs a way of getting a default value	
paramPane.addElement(ui.ProjectionEditor(0, (x * 200), 500, 50, "Projection Editor", "Hello", paramPane, False)) # this needs a way of getting a default value	
paramPane.addElement(ui.CoordinateEditor(0, (x * 250), 500, 50, "Coordinate Editor", [0, 0, 0], paramPane, False)) # this needs a way of getting a default value	
paramPane.addElement(ui.MatrixEditor(0, (x * 50), 500, 50, "Matrix Editor", [[1.0, 1.0, 1.0, 1.0], [1.0, 1.0, 1.0, 1.0], [1.0, 1.0, 1.0, 1.0],[1.0, 1.0, 1.0, 1.0]], paramPane, False))	
paramPane.addElement(ui.FileEditor(0, (x * 50), 500, 50, "File Editor", "filename", paramPane, False))
paramPane.addElement(ui.TextEditor(0, (x * 50), 500, 50, "Text Editor", "Hi, I'm some text!", paramPane, False))
paramPane.addElement(ui.FloatEditor(0, (x * 50), 500, 50, "Float Editor", 1.0, paramPane, False))

	
	# slightly brighter than normal	
i = 0
for element in paramPane.elements:
	if i % 2:		
		element.normalColor = newColor
	i = i + 1
	
paramPanel.offset = 20
paramPanel.addElement(paramPane)
#paramPanel.addElement(xMenu)
#paramPanel.addElement(zMenu)

evt_manager.addElement(materialPanel)
evt_manager.addElement(paramPanel)
evt_manager.register() # fire it up