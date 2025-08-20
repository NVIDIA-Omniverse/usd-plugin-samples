# OpenUSD Plugin Samples

## Introduction

This repository contains a set of samples that illustrate authoring of different kinds of plugins for OpenUSD. In particular, this repository contains plugin samples for:

- OpenUSD schemas (both codeful and codeless)
- File Format plugins
- Dynamic Payloads
- Hydra 2 Scene Indices

The sections below introduce these samples with the hope of helping you get started on your OpenUSD plugin journey. Feel free to fork this repository, delete the portions you don't need, and customize the remaining in whatever way suits your OpenUSD environment.

This repository is set up in such a way that you can either:
- quickly get started using prebuilt OpenUSD binaries from NVIDIA (22.11)
- "bring your own" OpenUSD build to build the samples against

Details on selecting a version or bringing your own build can be found [here](./docs/build-instructions.md).

## Quick Start

All samples included here use `cmake` to build the OpenUSD plugins.

> **Prerequisite:** You must have CMake 3.23.1+ installed on your system and available through the `PATH`.

If you want to directly build and try out the samples in `usdview`, you can use the provided tools to build the libraries and configure the environment to enable you to load the sample scenes in `usdview`. The commands below assume either a Linux environment or `x64 Visual Studio Native Command Prompt` on Windows.

<details>
<summary>On Linux</summary>

```bash
# Builds the release build of the samples into "_install" using prebuilt NVIDIA OpenUSD 22.11 binaries.
./build.sh

# Sets up a Python virtual environment (_venv), installs PySide and PyOpenGL, and sets the LD_LIBRARY_PATH/PYTHONPATH
# to the built sample libraries and the OpenUSD 22.11 distribution, sets the PXR_PLUGINPATH_NAME to include 
# paths to the sample "plugInfo.json" files.
source setenvlinux

# Opens usdview on the provided sample scene with a dynamic payload in an unloaded state.
usdview resources/scene.usda --unloaded
```
</details>

<details>
<summary>On Windows</summary>

```bat
REM Builds the release build of the samples into "_install" using prebuilt NVIDIA OpenUSD 22.11 binaries.
build.bat

REM Sets up a Python virtual environment (_venv), installs PySide and PyOpenGL, and sets the PATH/PYTHONPATH
REM to the built sample libraries and the OpenUSD 22.11 distribution, sets the PXR_PLUGINPATH_NAME to include 
REM paths to the sample "plugInfo.json" files.
setenvwindows.bat

REM Opens usdview on the provided sample scene with a dynamic payload in an unloaded state.
usdview resources/scene.usda --unloaded
```

</details>

### Advanced Build Instructions

For additional notes regarding building against different prebuilt NVIDIA OpenUSD binaries or your own OpenUSD build as well as using built schemas with NVIDIA Omniverse Extensions, refer to [this guide](./docs/build-instructions.md).

## Sample Scenes

Different samples rely on different sample scenes to showcase their capabilities:

### Dynamic Payloads

Open `usdview` with the `resources/scene.usda` sample stage. Once `usdview` has been opened, you can load the dynamic payload by right-clicking on `MetropolitanMuseumOfArt` and selecting `Load`. Using the default metadata configuration, this will load the payload as a set of deferred reads invoking REST APIs to retrieve department and object data for the Metropolitan Museum of Art. Alternatively, you can open `usdview` fully loaded without the `--unloaded` option. Note that this sample does not render anything – it is there to illustrate the dynamic scene structure created from the information received via the REST API.

### Hydra 2 Geospatial Coordinates

Open `usdview` with the `resources/wgs84/deutschebahn-rails.usda` sample stage. In this example, source WGS84 coordinates a resolved in Hydra 2 and a reference map displayed in the background to ensure correct resolution.

We thank [Digitale Schiene Deutschland](https://digitale-schiene-deutschland.de/en) for the collaboration and for providing exemplary railway map data.

### Hydra 2 Metrics Assembler

Open `usdview` with the `resources/metrics_assembler.usda` sample stage. You can play with the value of `metersPerUnit` in the `metrics_assembler_2.usda` layer to observe what happens when this value is different from that of the `metersPerUnit` value of the root stage.

### Hydra 2 Warp

Several examples are provided to illustrate the use of NVIDIA's warp in conjunction with scene indices:

- `warp_demo_mesh.usda`: Sample for using warp to deform a mesh
- `warp_demo_sim.usda`: Sample demonstrating the use of warp to simulate physics against a set of sphere particles


## General Project Structure

The repository is structured as follows:
```
/src
├─/hydra-plugins
│ ├─/omniGeoSceneIndex
│ ├─/omniMetricsAssembler
│ └─/omniWarpSceneIndex
└─/usd-plugins
  ├─/dynamicPayload
  ├─/fileFormat
  └─/schema
/tools
build.bat
build.sh
setenvlinux
setenvwindows.bat
```

All example source code is kept in the `src` directory, with each sub folder demonstrating a different type of OpenUSD plugin. The remaining files are there to support the build and execution infrastructure necessary to create the plugin libraries. This infrastructure uses an NVIDIA tool called `packman` to pull pre-built packages for use in these samples. These include the following:

- NVIDIA's prebuilt OpenUSD binaries that are used in Omniverse `kit` (and their Python versions)
- The installation of PyOpenGL, PySide, and warp-lang to a virtual environment to support running the provided examples easily

By convention, all folders starting with `_` are derived artifacts and can be safely deleted when cleaning the repository. In particular, three of these folders are used:

- `_build` (default location for generated and intermediary build artifacts)
- `_install` (default location for built and staged plugins)
- `_venv` (a virtual environment created to setup the environment for trying the samples out in `usdview`)

Each set of samples is accompanied by a `README` containing additional information about the relevant part of OpenUSD being explored and how the sample is constructed. These can be found here:

- [General Overview of OpenUSD Plugins](./docs/about-openusd-plugins.md)
- [Schemas, File Format Plugins, and Dynamic Payloads](./src/usd-plugins/README.md)
- [Hydra 2 Scene Indices](./src/hydra-plugins/README.md)


## Licensing Notes

The content herein is subject to the license located [here](./LICENSE). The dynamic payload example makes use of the Metropolitan Museum of Art Collection API, and usage of this is subject to terms and conditions specified [here](https://metmuseum.github.io). In particular, because the API does not require registration or use of an API key, request rates should be limited to 80 requests per second.


## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) to learn about our contribution guidelines for this repository.
