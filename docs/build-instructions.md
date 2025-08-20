# Additional Build Instructions

## Overview

These samples use `cmake` as the build system to generate project files for the native compilers to build the OpenUSD plugins.  The `cmake` build uses a number of supporting `cmake` files located in the tools/build/cmake folder:

* `NvPxrPlugin.cmake`: Supplies helpful functions for building OpenUSD schemas and general plugins, for both the C++ based project and the Python based project.
* `NvOpenUSDPrebuilt.cmake`: Supplies options for building against NVIDIA prebuilt OpenUSD binaries, selecting the version, and importing those binaries into the project as dependencies via the `packman` tool.
* `xConfig.cmake`: `cmake` configuration files set up to use the vendored TBB, OpenSubDiv, and Imath dependencies included in NVIDIA's prebuilt OpenUSD binary packages.

## Selecting the OpenUSD Version to Build Against

By default, the project will use NVIDIA's prebuilt OpenUSD 22.11 binaries.  Two `cmake` options control the usage of these prebuilt binaries:

* `NV_USE_PREBUILT_OPENUSD_BINARIES` (default `On`)
* `NV_OPENUSD_BINARY_VERSION` (default `22.11`)

To change the version of the prebuilt binaries used, supply a different value for `NV_OPENUSD_BINARY_VERSION` to your `cmake` configure command, e.g.:

```
cmake -B ./_build/cmake -G "Visual Studio 16 2019" -DNV_OPENUSD_BINARY_VERSION=22.11
```
Supported versions are listed as `cmake` string properties on the `NV_OPENUSD_BINARY_VERSION` option.  Selecting the OpenUSD version will automatically select the appropriate pre-built Python package to go with that selected version.  These prebuilt binaries are supplied such that they are binary compatible with the version that the different versions of Omniverse `kit` uses.

If you would like to use your own OpenUSD build:

* set `NV_USE_PREBUILT_OPENUSD_BINARIES` to `Off`
* set `PXR_OPENUSD_PYTHON_DIR` to the directory containing your Python build you used to build your OpenUSD binaries
* add the directory containing your OpenUSD build to `CMAKE_PREFIX_PATH` such that it can find `pxrConfig.cmake`

For example, you may modify `CMakeLists.txt` at the root of the repo to include:
```
set (NV_USE_PREBUILT_OPENUSD_BINARIES OFF)
set (PXR_OPENUSD_PYTHON_DIR ${CMAKE_CURRENT_LIST_DIR}/../path/to/my/python)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/../path/to/my/OpenUSD/build)

# include NVIDIA Pixar Plugin build tools that simplify building OpenUSD plugins
# by default we have the option set to ON to use the prebuilt binaries
# this can be turned off by setting NV_USE_PREBUILT_OPENUSD_BINARIES to off
include(NvOpenUSDPrebuilt)
include(NvPxrPlugin)
```

### Using your Built Schemas in Omniverse Kit

If you would like to use the example schemas here inside of `kit` (or use the examples to build your own schemas and use those in `kit`), you must:

- Clone the `kit-app-template` from https://github.com/NVIDIA-Omniverse/kit-app-template
- Follow the instructions to create a sample extension and a sample app in which that extension can be hosted

Once you have an extension created, it's time to host the built schemas in that extension. The easiest way to do this is to source link in the built schemas from this repo into the `target-deps` directory of your app template. This can be done by creating a `usd-plugins.packman.xml` file in the `tools/deps` folder of your app template and placing the following content in:

```xml
<project toolsVersion="5.0">
  <dependency name="usd_plugins" linkPath="../../_build/target-deps/usd_plugins" tags="${config} non-redist">
    <source path="../../../../github_updates/usd-plugin-samples/_install" />
  </dependency>
</project>
```

This tells `packman` to create a symbolic link at `_build/target-deps/usd_plugins` at the root of the `kit-app-template` folder that links to the `_install` directory of this repo (replace the relative source path as needed for your setup as well as the target install folder if you changed `CMAKE_INSTALL_PREFIX`). We also need to tell `kit` to make sure this file is processed when processing the other `packman` files. To do this, open the `repo.toml` file at the root of your `kit-app-template` and add the following under the `repo_build` section:

```lua
fetch.packman_target_files_to_pull = [
    "${root}/tools/deps/host-deps.packman.xml",
    "${root}/tools/deps/kit-sdk.packman.xml",
    "${root}/tools/deps/kit-sdk-deps.packman.xml",
    "${root}/tools/deps/usd-plugins.packman.xml"
]
```

We then need to copy the output of our `_install` folder into the `kit` extension directory. We can do this by opening up the `premake5.lua` file for our extension and adding the following:

```lua
-- Copy in the schema output libraries and resources
repo_build.prebuild_copy
{
    { target_deps.."/usd_plugins/**", ext.target_dir }
}
```

Since we source linked in our `_install` directory to `_build/target-deps/usd_plugins`, this copy command copies all of that content into the extension's build target directory.

Next, we have to modify the `extension.toml` file of our sample extension so that it loads the schema libraries. To do that, add the following at the top:

```toml
[core]
# Load at the start, load all schemas with order -100 (with order -1000 the OpenUSD libs are loaded)
order = -100
```

This ensures the extension, when set to load with the application, will load early, which is necessary to make sure our schema libraries are loaded by OpenUSD prior to the `UsdSchemaRegistry` being created. Next, we add the native libraries to load and the Python module for the codeful schema:

```toml
[[native.library]]
"filter:platform"."linux-x86_64"."path" = "lib/${lib_prefix}omniExampleSchema${lib_ext}"
"filter:platform"."windows-x86_64"."path" = "bin/${lib_prefix}omniExampleSchema${lib_ext}"

[[python.module]]
name = "OmniExampleSchema"
```

Finally, we add a dependency to `omni.usd.libs`, which is the extension in `kit` that hosts the OpenUSD libraries:

```toml
[dependencies]
"omni.usd.libs" = {}
```

Now we need to make sure the schemas are registered as plugins with OpenUSD. To do this, we will perform explicit registration via the `__init__.py` file parallel to your `extension.py` file in your `kit` extension by adding the following content (note, the specific path for `pluginsRoot` below will depend on the relative directory structure between your Python file performing the registration and the location of the `plugins` directory copied from the schema build artifacts):

```python
from pxr import Plug

pluginsRoot = os.path.join(os.path.dirname(__file__), '../../plugins')
omniExampleSchemaPath = os.path.join(pluginsRoot, "omniExampleSchema", "resources")
omniExampleCodelessSchemaPath = os.path.join(pluginsRoot, "omniExampleCodelessSchema", "resources")

Plug.Registry().RegisterPlugins(omniExampleSchemaPath)
Plug.Registry().RegisterPlugins(omniExampleCodelessSchemaPath)
```

Now build your extension and app using the `kit-app-template` instructions (usually `repo.bat build` or `./repo.sh build`).

Once the application and extension are built, it's time to make sure our extension gets auto-loaded into the application. This is necessary because plugins need to be registered with OpenUSD as early as possible to ensure the relevant singleton manager picks them up. Launch your application with developer extensions enabled:

**On Linux:**
```bash
./repo.sh launch -d
```

**On Windows:**
```batch
repo.bat launch -d
```

Open the extension manager, find the extension you created, and enable it. Open the extension's properties and select the `Autoload` box at the top. Then close and restart your `kit` application. Alternatively, you can add your extension to the `[dependencies]` section of the `app` configuration. Once restarted, the schemas should be loaded inside of `kit`. To test this, you can open up the scripting window and use the following script:

```python
import OmniExampleSchema
import omni.usd

from pxr import UsdGeom

# Create new mesh prim and apply one of the API schemas to it
stage = omni.usd.get_context().get_stage()
prim = UsdGeom.Mesh.Define(stage, "/World/MyMesh")
OmniExampleSchema.OmniTemperatureDataAPI.Apply(prim.GetPrim())

# Now apply the example codeless schema
prim.GetPrim().ApplyAPI("OmniExampleCodelessOmniSourceFormatMetadataAPI")
```

To verify that the relevant schema properties have been applied to the prim, select the `/World/MyMesh` prim and examine the `Raw USD` properties in the property window. If you'd like these properties to show up in their own respective groups, you would need to add a property window extension to achieve that behavior.
