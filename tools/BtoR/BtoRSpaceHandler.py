# SPACEHANDLER.VIEW3D.EVENT
#!BPY

# """
# Name: 'Script Name'
# Blender: 233
# Group: 'Export'
# Submenu: 'All' all
# Submenu: 'Selected' sel
# Submenu: 'Configure (gui)' gui
# Tooltip: 'Export to some format.'
# """

import Blender

evt = Blender.event

if evt == Blender.Draw.RIGHTMOUSE:
	# on a right mouse click assume I've selected an object or unselected it
	# simply set something in the registry to say "hey, something changed"
	changed = { "changed" : True }
	
	Blender.Registry.SetKey("BtoR_Space", changed)


 