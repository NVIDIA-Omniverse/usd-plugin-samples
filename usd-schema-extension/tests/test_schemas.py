import os
import OmniExampleSchema
from pxr import Usd, UsdGeom, Plug, Tf
import pytest

# This will register the plugins before the tests are ran
@pytest.fixture
def setup():
    archConfigs = ["windows-x86_64_debug", "windows-x86_64_release", "linux-aarch64_debug", "linux-aarch64_release", "linux-x86_64_debug", "linux-x86_64_release"]
    for archConfig in archConfigs:
        plugin_root = os.path.join(os.path.dirname(__file__), "..", "_install", "omniExampleSchema", archConfig, "usd", "OmniExampleSchema", "resources")
        if os.path.exists(plugin_root):
            Plug.Registry().RegisterPlugins(plugin_root)
            break
    
    codeless_plugin_root = os.path.join(os.path.dirname(__file__), "..", "_install", "omniExampleCodelessSchema", "usd", "OmniExampleCodelessSchema", "resources")
    Plug.Registry().RegisterPlugins(codeless_plugin_root)

def test_schema(setup):
    stage = Usd.Stage.Open(os.path.join(os.path.dirname(__file__), "testStages", "test.usda"))
    obj = stage.GetPrimAtPath("/Object")
    prim = UsdGeom.Xform.Define(stage, "/xformprim")
    ml = stage.GetPrimAtPath("/MeshLod")

    assert not prim.GetPrim().HasAPI(OmniExampleSchema.OmniTemperatureDataAPI)
   
    OmniExampleSchema.OmniTemperatureDataAPI.Apply(prim.GetPrim())
    meshLod = OmniExampleSchema.OmniMeshLod(ml)

    assert meshLod.GetLodTransitionSchemeAttr().Get() == "blend"
    assert obj.HasAPI(OmniExampleSchema.OmniExternalDataSourceAPI)
    assert prim.GetPrim().HasAPI(OmniExampleSchema.OmniTemperatureDataAPI)

def test_codeless_schema(setup):
    stage = Usd.Stage.Open(os.path.join(os.path.dirname(__file__), "testStages", "codelessTest.usda"))
    obj = stage.GetPrimAtPath("/Object")
    prim = stage.GetPrimAtPath("/xformprim")

    tf_type = Tf.Type.FindByName('OmniExampleOmniSourceFormatMetadataAPI')

    assert obj.GetAttribute("omni:example:codeless:sourceFormatMetadata:partId").Get() != ""

    obj.ApplyAPI(tf_type)

    assert obj.GetAttribute("omni:example:codeless:sourceFormatMetadata:partId").Get() == ""
    assert prim.GetAttribute("omni:example:codeless:sourceFormatMetadata:itemId").Get() == "testID"