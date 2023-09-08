from pxr import Tf
from pxr.Usdviewq.plugin import PluginContainer
from .preferences import Preferences

def launchPreferences(usdviewApi):
    prim = usdviewApi.stage.GetPrimAtPath("/World/grid/Grid")
    attr = prim.GetAttribute("warp:sourceFile")
    _preferencesDlg = Preferences(usdviewApi.qMainWindow, attr)
    _preferencesDlg.show()
    _preferencesDlg = None

class OceanSimPluginContainer(PluginContainer):

    def registerPlugins(self, plugRegistry, usdviewApi):

        self._launchPreferences = plugRegistry.registerCommandPlugin(
            "OceanSimPluginContainer.launchPreferences",
            "Launch Preferences",
            launchPreferences)

    def configureView(self, plugRegistry, plugUIBuilder):

        tutMenu = plugUIBuilder.findOrCreateMenu("OceanSim")
        tutMenu.addItem(self._launchPreferences)

Tf.Type.Define(OceanSimPluginContainer)