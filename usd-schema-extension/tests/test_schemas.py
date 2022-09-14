import os
import subprocess
import pytest

from pxr import Usd, UsdGeom, Plug, Tf

import OmniExampleSchema

class TestSchemas(object):
    """
    Encapsulates sanity tests for the built schemas.
    """
    config = ""

    # This will register the plugins before the tests are run
    @pytest.fixture(autouse=True, scope="class")
    def setup(self):
        arch = "windows"
        if os.name != "nt":
            arch = "linux"

        arch_type = "x86_64"
        try:
            stdout = subprocess.check_output(["arch"]).decode("utf-8").strip()
            if stdout in ["aarch64", "x86_64"]:
                arch_type = stdout
        except:
            pass

        arch = arch + "-" + arch_type

        arch_config = arch + "_" + TestSchemas.config

        plugin_root = os.path.join(os.path.dirname(__file__), "..", "_install", "omniExampleSchema", arch_config, "usd", "OmniExampleSchema", "resources")
        if os.path.exists(plugin_root):
            Plug.Registry().RegisterPlugins(plugin_root)
        
        codeless_plugin_root = os.path.join(os.path.dirname(__file__), "..", "_install", "omniExampleCodelessSchema", "usd", "OmniExampleCodelessSchema", "resources")
        Plug.Registry().RegisterPlugins(codeless_plugin_root)

    def test_schema(self):
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

    def test_codeless_schema(self):
        stage = Usd.Stage.Open(os.path.join(os.path.dirname(__file__), "testStages", "codelessTest.usda"))
        obj = stage.GetPrimAtPath("/Object")
        prim = stage.GetPrimAtPath("/xformprim")

        tf_type = Tf.Type.FindByName('OmniExampleOmniSourceFormatMetadataAPI')

        assert obj.GetAttribute("omni:example:codeless:sourceFormatMetadata:partId").Get() != ""

        obj.ApplyAPI(tf_type)

        assert obj.GetAttribute("omni:example:codeless:sourceFormatMetadata:partId").Get() == ""
        assert prim.GetAttribute("omni:example:codeless:sourceFormatMetadata:itemId").Get() == "testID"