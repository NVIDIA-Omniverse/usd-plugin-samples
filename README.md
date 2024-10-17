## Introduction

This repository contains a set of samples that illustrate authoring of different kinds of plug-ins for OpenUSD.  In particular, this repository contains plug-in samples for:

- USD schemas (both codeful and codeless)
- File Format Plugins
- Dynamic Payloads
- Hydra 2 Scene Indices

The sections below introduce these samples with the hope of helping you get started on your OpenUSD plug-in journey.  Feel free to fork this repository, delete the portions you don't need, and customize the remaining in whatever way suits your OpenUSD environment.

While the repository is set up in such a way that you can quickly get started using pre-created builds of OpenUSD (in this case, either `23.05` or NVIDIA's custom OpenUSD build for `22.11` which is currently used in `kit` 106), the intent is to enable you to "bring your own" OpenUSD builds by specifying where it resides in configuration for the included tooling.  This is described in further detail below.

## Quick Start

All samples included here use `cmake` to build the OpenUSD plug-ins.

**Prerequisite: You must have CMake 3.23.1+ installed on your system and available through the `PATH`**

If you want to directly build and try out the samples in `usdview`, you can use the provided tools to build the libraries and configure the environment to enable you to load the sample scenes in `usdview`.  The commands below assume either a Linux environment or `git-bash` on Windows.

```
./build.bat # builds the release build of the samples into _install (Windows X86)
./build.sh  # builds the release build of the samples into _install (Linux X86, Linux ARM)

source setenvwindows      # sets up a python virtual environment (_venv), installs PySide and PyOpenGL, and sets the PATH / PYTHONPATH
                          # to the built sample libraries and the USD 23.05 distribution, sets the PXR_PLUGINPATH_NAME to include 
                          # paths to the sample plugInfo.json files (Windows)
source setenvlinux        # sets up a python virtual environment (_venv), installs PySide and PyOpenGL, and sets the LD_LIBRARY_PATH / PYTHONPATH
                          # to the built sample libraries and the USD 23.05 distribution, sets the PXR_PLUGINPATH_NAME to include 
                          # paths to the sample plugInfo.json files (Linux)

usdview resources/scene.usda  --unloaded    # opens usdview on the provided sample scene with a dynamic payload in an unloaded state
```

Different samples rely on different sample scenes:

__Dymamic Payloads__

Open `usdview` with the `resources/scene.usda` sample stage.  Once `usdview` has been opened, you can load the dynamic payload by right-clicking on `MetropolitanMuseumOfArt` and selecting `Load`.  Using the default metadata configuration, this will load the payload as a set of deferred reads invoking REST APIs to retrieve department and object data for the Metropolitan Museum of Art.  Alternatively, you can open `usdview` fully loaded without the `--unloaded` option.  Note that this sample does not render anything - it is there to illustrate the dynamic scene structure created from the information received via the REST API.

__Hydra 2 Geospatial Coordinates__

Open `usdview` with the `resources/wgs84/deutschebahn-rails.usda` sample stage.  In this example, source WGS84 coordinates a resolved in Hydra 2 and a reference map displayed in the background to ensure correct resolution.

We thank [Digitale Schiene Deutschland](https://digitale-schiene-deutschland.de/en) for the collaboration and for providing exemplary railway map data.

__Hydra 2 Metrics Assembler__

Open `usdview` with the `resources/metrics_assembler.usda` sample stage.  You can play with the value of `metersPerUnit` in the `metrics_assembler_2.usda` layer to observe what happens when this value is different from that of the `metersPerUnit` value of the root stage.

__Hydra 2 Warp__

Several examples are provided to illustrate the use of NVIDIA's warp in conjunction with scene indices:

- `warp_demo_mesh.usda` - Sample for using warp to deform a mesh
- `warp_demo_sim.usda` - Sample demonstrating the use of warp to simulate physics against a set of sphere particles

## Licensing Notes

The content herein is subject to the license [here](LICENSE).  The dynamic payload example makes use of the Metropolitan Museum of Art Collection API, and usage of this is subject to terms and conditions specified [here](https://metmuseum.github.io).  In particular, because the API does not require registration or use of an API key, request rates should be limited to _80 requests per second_.

## General Project Structure

The repository is structured as follows:

```
deps
src
  hydra-plugins
    omniGeoSceneIndex
    omniMetricsAssembler
    omniWarpSceneIndex
  kit-extension
  usd-plugins
    dynamicPayload
    fileFormat
    schema
tools
build.bat
build.sh
setenvwindows
setenvlinux
setenvwindows.bat
```

All example source code is kept in the `src` diretory, with each sub folder demonstrating a different type of USD plug-in.  The remaining files are there to support the build and execution infrastructure necessary to create the plug-in libraries.  This infrastructure uses an NVIDIA tool called `packman` to pull pre-built packages for use in these samples.  These include the following:

- NVIDIA's customized OpenUSD 22.11 build for use in `kit`
- Stock OpenUSD 23.05 builds
- Python distributions used to build the above USD packages (Python 3.10 / 3.11)
- A set of build support files for `cmake` for schema generation and plug-in building (`nvopenusdbuildtools`)
- The installation of PyOpenGL, PySide, and warp-lang to a virtual environment to support running the provided examples easily

By convention, all folders starting with `_` are derived artifacts and can be safely deleted when cleaning the repository.  In particular, three of these folders are used:

- _build (default location for generated and intermediary build artifacts)
- _install (default location for built and staged plug-ins)
- _venv (a virtual environment created to setup the environment for trying the samples out in `usdview`)

Each set of samples is accompanied by a `README` containing additional information about the relevant part of USD being explored and how the sample is constructed.  These can be found here:

- [Schemas, File Format Plugins, and Dynamic Payloads](src/usd-plugins/README.md)
- [Hydra 2 Scene Indices](src/hydra-plugins/README.md)

The remainder of this document explores the USD plugin system in general and the tooling provided to build the samples in greater depth.

## OpenUSD Plugins

OpenUSD provides many different extensibility points to allow additional data to be represented, loaded, and worked with as prims and attributes within the OpenUSD runtime.  These extensiblity points are implemented via _plugins_, which provide definition of additional data (schemas) and certain runtime behaviors (data loading and asset resolution).  Plugins are implemented via libraries that contain classes that implement base APIs provided by USD to interact with the OpenUSD runtime and declared to OpenUSD via information contained in a `plugInfo.json` file.

In general, the plugin system of OpenUSD works in the same way regardless of plugin type.  OpenUSD needs a few things for the plugin system to work:

- An abstract base class declaring the API for the plugin type that is registered with the type system
- A factory function for creating new instances
- A mechanism for loading the plugin into the system

For example, in the case of implementing a custom file format extension, OpenUSD provides:

- An abstract base class via `SdfFileFormat`
- A factory object reponsible for creating instances of the plugin type via `Sdf_FileFormatFactory`
- An object that reads information of plugins that implement `SdfFileFormat` and loads them into the runtime via `Sdf_FileFormatRegistry`

This can be illustrated in the diagram below:

![OpenUSD Plugin Architecture](images/usd_plugin_architecture.png)

To implement a plugin, a developer needs to do a few things:

- Create a class that implements the API of the abstract base class
- Register the type with `Tf`
- Declare the plugin and the information contained therein in a `plugInfo.json` file
- Register the plug-in with the system (either implicitly by specifying a path to the `plugInfo.json` file in `PXR_PLUGINPATH_NAME` or explicitly via a `RegisterPlugins` call in controlling code)

From the point of view of the OpenUSD runtime, plugins are read generically by interpreting the `plugInfo.json` file and deriving information about the plugin that can be used to load the plugin into the runtime when requested.  This is the responsibility of the `plug` OpenUSD library.  The metadata of all plugins (regardless of type) is held in the `PlugRegistry` object.  On OpenUSD runtime startup, the information in all `plugInfo.json` files accessible from paths declared in `PXR_PLUGINPATH_NAME` are loaded, interpreted, and stored in the `PlugRegistry` singleton instance.  Additionally, any `plugInfo.json` files that are found via a `RegisterPlugins` call on the singleton are loaded, interpreted, and stored (more on this later, because order of operations are important!).  The metadata of each plugin is represented by a `PlugPlugin` instance.

Objects that are interested in plugins of a certain type may query the `PlugRegistry` for all plugins that derive from a type registered in the type system via a call to `GetAllDerivedTypes`.  This returns all registered types that derive from the requested type; from this the object can get the actual plugin metadata via a call to `GetPluginForType`.  Finally, the object can load the plugin from the metadata via `Load` in order to work with the specific API implemented by the plugin.  This loads the library associated with the plugin into memory and makes the types in it accessible to the system.  In general, the objects that manage this information are specific for a particular plugin type (e.g. `SdfFileFormat`) and are typically singletons in the USD runtime (e.g., `Sdf_FileFormatRegistry`).  

The file format objects that implement the OpenUSD plugin architecture for `SdfFileFormat` are given below:

![File Format Plugin Example](images/file_format_plugin_example.png)

In all cases, the singleton objects that load plugins do so __once__ at the time of first access and cache that information.  __This means that any code that performs a call to `RegisterPlugins` must be executed prior to the first call to the singleton object managing plugins of that type!__.

For illustrative purposes, the `edf` file format plugin contained within this sample also has an example of a type that manages plugins (of type `IEdfDataProvider`) in a similar way that the built-in OpenUSD managers manage their own plugin types.

## Using CMake to Generate Schema Code and Build Files

These samples use `cmake` to drive generation of schema code (for codeful schemas) and building of OpenUSD plug-ins.  The `cmake` infrastructure included here is setup to use pre-packaged OpenUSD builds (as well as some additional dependencies required by some of the plug-ins).  The use of `packman` has been enabled in this repository to pull the relevant OpenUSD and python packages (22.11 and 3.10 respectively) for a turnkey type solution that builds plugins compatible with NVIDIA Omniverse (106+).

The `cmake` build files are structured in the following way:

- The root `CMakeLists.txt` file, responsible for importing NVIDIA's OpenUSD plug-in `cmake` helper, setting up the link between the pulled `packman` packages and the rest of the build commands, and the inclusion of the sub-directories containing OpenUSD plug-in code.
- The `PackmanDeps.cmake` file, which setups paths to find `cmake` files to include from the `packman` packages that were pulled down.
- The `NvPxrPlugin.cmake` file from `nvopenusdbuildtools`, which contains helper functions for declaring OpenUSD schemas, plug-in targets, and python plug-in targets.
- The individual `CMakeLists.txt` files for the OpenUSD plug-ins, which use the helper functions provided to build the individual plug-ins.

This works out of the box for both NVIDIA's customized OpenUSD 22.11 Python 3.10 build (i.e., `nv-usd`) and for stock OpenUSD 24.05 Python 3.10 by running the `build.bat` / `build.sh` files.  To see all of the options available, run with the `--help` option.  These files will:

- pull down the required `packman` packages from NVIDIA's package repositories (via `scripts/setup.py`)
- configure and build using `cmake`

Feel free to customize this sequence as well as the arguments passed to `cmake` as best suits your organization.

### Integrating your own OpenUSD / Python builds

If you would like to integrate your own build of OpenUSD and Python, you must:

- Change the `build.bat` / `build.sh` file such that the `--build-tools-only` option is passed to `scripts/setup.py` (This will ensure no package for OpenUSD or Python is pulled from the NVIDIA package repository, only the build support tools package)
- Edit the value of `PXR_OPENUSD_PYTHON_DIR` in `PackmanDeps.cmake` to point to the directory hosting the Python installation you want to use
- Edit the line in `PackmanDeps.cmake` that appends to the `CMAKE_PREFIX_PATH` - the value should point to your local OpenUSD build.  Editing this will direct `cmake` to look for `pxrConfig.cmake` in your local directory rather than the directory of the pulled OpenUSD package from NVIDIA.

## Contributing

The source code for this repository is provided as-is and we are not accepting outside contributions at this time.
