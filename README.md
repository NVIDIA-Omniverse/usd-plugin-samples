# OpenUSD Plugin Samples

> ## ⚠️ No longer actively maintained
>
> These samples are proofs of concept, preserved as-is. They build against the
> specific OpenUSD and Python versions pinned here and are **not guaranteed to
> build or run against any other**, and we are not maintaining them against a
> moving toolchain.
> That's deliberate: their value is in the **data models and system
> architecture** they explore, not the implementation — so it doesn't depend on
> the code still building.
>
> Several of these concepts have informed, and continue to inform, OpenUSD
> design work:
>
> - **Geospatial scene index** (`src/hydra-plugins/omniGeoSceneIndex`) —
>   informed AOUSD AECO IG and Summit discussions, from which a geospatial
>   proposal is now being drafted.
> - **Dynamic payloads** (`src/usd-plugins/dynamicPayload/omniMetProvider`) —
>   informed [Composable Bindings](https://aka.ms/ComposableBindings)
>   (Microsoft/NVIDIA), which in turn informed the [Separation of Concerns for
>   Identifiers in USD](https://github.com/PixarAnimationStudios/OpenUSD-proposals/pull/105)
>   proposal.
> - **Metrics assembler** (`src/hydra-plugins/omniMetricsAssembler`) — a useful
>   datapoint as the community works toward a units solution.
>
> We expect further OpenUSD workstreams to build on this work.
>
> The **Kit extension** sample has been removed (it targeted an outdated version
> of Kit). The remaining samples are here for reference — fork and adapt as you
> like, but expect to do your own porting. We are not accepting contributions.
>
> Much of the build and dependency friction here is not specific to these
> samples: consuming OpenUSD from cmake portably is an ecosystem-wide problem.
> Work toward a portable, target-based OpenUSD cmake config is happening in the
> open at the [AOUSD Build Interest Group](https://github.com/aousd/build-ig-initiatives)
> (e.g. its effort to make `pxrConfig.cmake` fully portable) — the vendor-neutral
> venue where such fixes belong, rather than a bespoke build maintained here.
>
> Specific build problems people have reported are written up in the issue
> tracker. Some are fixed here; where the cause is upstream of this repository,
> the write-up says so and gives a local workaround where one exists. They're
> there for reference, not as an open support channel.

## Introduction

This repository contains working explorations of OpenUSD extensibility mechanisms — schemas, file format plugins, dynamic payloads, and Hydra 2 scene indices. The samples were developed as proofs of concept for specific technical problems and are documented in depth. Some have since informed proposals and discussions in the broader OpenUSD community.

The build infrastructure supports bringing your own OpenUSD build; see the [build instructions](./docs/build-instructions.md) for details.

## Quick Start

All samples included here use `cmake` to build the OpenUSD plugins.

> **Prerequisite:** You must have CMake 3.23.1+ installed on your system and available through the `PATH`.

If you want to directly build and try out the samples in `usdview`, you can use the provided tools to build the libraries and configure the environment to enable you to load the sample scenes in `usdview`. The commands below assume either a Linux environment or `git-bash` on Windows.

<details>
<summary>On Linux</summary>

```bash
# Builds the release build of the samples into "_install".
./build.sh

# Sets up a Python virtual environment (_venv), installs PySide and PyOpenGL, and sets the LD_LIBRARY_PATH/PYTHONPATH
# to the built sample libraries and the OpenUSD 23.05 distribution, sets the PXR_PLUGINPATH_NAME to include 
# paths to the sample "plugInfo.json" files.
source setenvlinux

# Opens usdview on the provided sample scene with a dynamic payload in an unloaded state.
usdview resources/scene.usda --unloaded
```
</details>

<details>
<summary>On Windows</summary>

```bat
REM Builds the release build of the samples into "_install".
.\build.bat

REM Sets up a Python virtual environment (_venv), installs PySide and PyOpenGL, and sets the PATH/PYTHONPATH
REM to the built sample libraries and the OpenUSD 23.05 distribution, sets the PXR_PLUGINPATH_NAME to include 
REM paths to the sample "plugInfo.json" files.
source setenvwindows

REM Opens usdview on the provided sample scene with a dynamic payload in an unloaded state.
usdview resources/scene.usda --unloaded
```
</details>

### Building Custom Schemas

For additional notes regarding building custom OpenUSD schemas, refer to [this guide](./docs/build-instructions.md).

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
/deps
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
setenvwindows
setenvlinux
setenvwindows.bat
```

All example source code is kept in the `src` directory, with each sub folder demonstrating a different type of OpenUSD plugin. The remaining files are there to support the build and execution infrastructure necessary to create the plugin libraries. This infrastructure uses an NVIDIA tool called `packman` to pull pre-built packages for use in these samples. These include the following:

- NVIDIA's customized OpenUSD 22.11 build for use in `kit`
- Stock OpenUSD 23.05 builds
- Python distributions used to build the above OpenUSD packages (Python 3.10/3.11)
- A set of build support files for `cmake` for schema generation and plugin building (`nvopenusdbuildtools`)
- The installation of PyOpenGL, PySide, and warp-lang to a virtual environment to support running the provided examples easily

By convention, all folders starting with `_` are derived artifacts and can be safely deleted when cleaning the repository. In particular, three of these folders are used:

- `_build` (default location for generated and intermediary build artifacts)
- `_install` (default location for built and staged plugins)
- `_venv` (a virtual environment created to setup the environment for trying the samples out in `usdview`)

Each set of samples is accompanied by a `README` containing additional information about the relevant part of OpenUSD being explored and how the sample is constructed. These can be found here:

- [Schemas, File Format Plugins, and Dynamic Payloads](./src/usd-plugins/README.md)
- [Hydra 2 Scene Indices](./src/hydra-plugins/README.md)


## Licensing Notes

The content herein is subject to the license located [here](./LICENSE). The dynamic payload example makes use of the Metropolitan Museum of Art Collection API, and usage of this is subject to terms and conditions specified [here](https://metmuseum.github.io). In particular, because the API does not require registration or use of an API key, request rates should be limited to 80 requests per second.


## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) to learn about our contribution guidelines for this repository.
