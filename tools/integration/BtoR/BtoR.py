#!BPY

# """
# Name: 'Blender to Renderman'
# Blender: 242
# Group: 'Render'
# Tooltip: 'Launch BtoR(Blender to Renderman) System'
# """

from btor import BtoRGUIClasses as ui
from btor import BtoRMain
from btor import BtoRAdapterClasses
reload(BtoRMain)
# backGround = [0.65, 0.65, 0.65]

instBtoREvtManager = ui.EventManager()  # initialize the event manager first
setattr(BtoRMain, "instBtoREvtManager", instBtoREvtManager)

instBtoRLog = BtoRMain.TextWindow()
setattr(BtoRMain, "instBtoRLog", instBtoRLog)

instBtoRSettings = BtoRMain.BtoRSettings() # then bring up the settings
setattr(BtoRMain, "instBtoRSettings", instBtoRSettings)

instBtoRLightManager = BtoRMain.LightManager()
setattr(BtoRMain, "instBtoRLightManager", instBtoRLightManager)

instBtoRSceneSettings = BtoRMain.SceneSettings() # then scene settings
setattr(BtoRMain, "instBtoRSceneSettings", instBtoRSceneSettings)

setattr(BtoRMain, "instBtoRMaterials", BtoRMain.MaterialList())
instBtoRMaterials = getattr(BtoRMain, "instBtoRMaterials")

instBtoRGroupList = BtoRMain.GroupList()
setattr(BtoRMain, "instBtoRGroupList", instBtoRGroupList)

instBtoRHelp = BtoRMain.TextWindow()
setattr(BtoRMain, "instBtoRHelp", instBtoRHelp)

instBtoRObjects = BtoRMain.ObjectEditor()
setattr(BtoRMain, "instBtoRObjects", instBtoRObjects)


instBtoRMain = BtoRMain.MainUI()
setattr(BtoRMain, "instBtoRMain", instBtoRMain)

instBtoREvtManager.addElement(instBtoRMain.getEditor())
if instBtoRSettings.haveSetup == False:
	instBtoREvtManager.addElement(instBtoRSettings.getEditor())		

instBtoREvtManager.register() # fire it up