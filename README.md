## Introduction

This repository is intended to contain a set of samples that illustrate authoring of different kinds of plug-ins for USD.  In particular, this repository is intended to illustrate samples for:

- USD schemas (both codeful and codeless)
- AR 2.0 resolvers
- AR 1.0 resolvers (for 104.x Kit-based applications)
- File Format Plugins
- Dynamic Payloads

Currently, the repository hosts example USD schemas along with a set of tools that can be used to generate schema code and templates that can be used for both CMake and Premake to build the plug-ins using compiler / linker settings consistent with those used when building USD libraries.  As more samples become available, they will be added to this repository.  Feel free to fork this repository, delete the portions you don't need, and customize the remaining in whatever way suits your USD environment.

## Prerequisites

To build the sample as is contained in this repository, a few things are required:

- Standard C++ build tools for your platform (e.g. Visual Studio, g++)
- `cmake` (to create makefiles and invoke the build tools)
- `curl` (to use the NVIDIA tool `packman` to pull down dependency packages - see below

## General Project Structure

The repository is structured as follows:

```
deps
src
    kit-extension
    usd-plugins
        schema
tools
bootstrap.py
build.bat
build.sh
repo.toml
```

All example source code is kept in the `src` diretory, with each sub folder in the `usd-plugins` directory demonstrating a different type of USD plug-in.  The remaining files are there to support the schema generation and build infrastructure necessary to create the plug-in libraries.  This infrastructure uses an NVIDIA tool called `packman` to pull packages required for schema and makefile generation.  These include the following:

- USD builds compatible with NVIDIA Omniverse 104.x kit-based applications (USD 20.08)
- A python distribution used to build the above USD packages (Python 3.7)
- A tool (`repo_usd`) used to generate schema code and makefiles in the desired format (Cmake / Premake)
- The jinja python package (3.1.2) and its dependencies to support `usdGenSchema` (installed to a local folder via `repo_usd`)

By convention, all folders starting with `_` are derived artifacts and can be safely deleted when cleaning the repository.  In particular, three of these folders are used:

- _repo (stores the download `repo_usd` package for use)
- _build (default location for generated and intermediary build artifacts)
- _install (default location for built and staged plug-ins)

These folders are used or not depending on various configuration options you provide to the `repo_usd` tool via the `repo.toml` file.  Options that can be provided, as well as command line options that can be passed to the `build.bat` / `build.sh` scripts are described in the section `Tool Options` below.

Additionally, the repository contains a sample NVIDIA OMniverse kit extension based off of the template found here: https://github.com/NVIDIA-Omniverse/kit-extension-template.  This example shows the wrapping of the built schema libraries such that they can be loaded into and used from NVIDIA Omniverse based applications.

## Quick Start

The provided build files (`build.bat` / `build.sh`) provide a very smiple integration of the steps required to generate, build, and stage everything in such a way that the sample kit extension properly integrates the built USD schema and loads into kit (when setting the extension path to the build location, by default `_install/${platform}/${env:CONFIG}`).

These files provide a number of options to use to run each of the steps independently or together as required:

- `--generate`: runs the schema generation step for the included schemas in the `repo.toml` file
- `--build`: runs the usd-plugin build step by invoking `cmake` 
- `--stage`: copies the built schema libraries and kit extension into the install directory such that it can be added to kit from there
- `--clean`: removes the `_build` and `_install` directories (but leaves the generated code, as `usdGenSchema` will detect changes here)
- `--debug`: indicates that a debug build should be made rather than a release build (the default)

To run all steps to generate, build, and stage:
```
build.bat --generate --build --stage          (Windows, release)
build.bat --generate --build --stage --debug  (Windows, debug)
./build.sh --generate --build --stage           (Linux, release)
./build.sh --generate --build --stage --debug   (Linux, debug)
```

The options you can specify to the tool are described in further detail below.

## USD Schemas

Recall that USD defines the concept of a "prim".  Prims abstractly serve as data containers for groups of logically related data.  These groups are referred to as _schemas_ (or more specificaly, _prim schemas_ to distinguish them from _property schemas_).  When we say that a prim has a type of _Mesh_, what we really mean is that the prim provides the logical set of related data defined by the `UsdGeomMesh` schema (e.g., vertices, faces, normals, etc.).

Schemas are divided into two major categories _IsA_ schemas and _API_ schemas.  _IsA_ schemas are meant to define a specific purpose for a prim.  In the case above, a _Mesh_ prim is a prim who's specific purpose is to represent a mesh.  A prim can only subscribe to a single _IsA_ schema - that is, it has a single well defined role in the scene hierarchy.  These types of schemas can be inherited and within that inheritance hierarchy schemas can be either _abstract_ or _concrete_.  Only concrete schemas are instantiable in the USD scene hierarchy.

_API Schemas_, on the other hand, serve only as additional data groups on prims as well as a well-defined API to get and set those values.  These schema types can be applied to any prim in the scene hierarchy (as long as the schema rules say it can be applied to that prim type). If a prim has a type, you will see that type in the scene hierarchy.  If a prim has an API schema applied, you won't see a difference in its type, but will still be able to ask that prim for the data contained within that schema.  These types of schemas can be either _non-applied_ or _applied_ API schemas, with the difference being that applied API schemes will be recorded in the USD file such that they retain that information on interchange of the scene data to other applications.  If a schema is an applied API schema, it can be either single instance (_single-apply_ API schemas, applied only once to a prim) or multi-instance (_multiple-apply_ API schemas, can be applied several times, each defining a unique instance).

USD ships with many schema definitions that you may be familiar with, including "IsA" schemas (e.g., `Mesh`, `Xform`) and "API" schemas (e.g., `UsdShadeMaterialBindingAPI`, `UsdCollectionAPI`, etc.).  These can all be found in their respective modules (e.g., the schema set provided by `UsdGeom` can be found in pxr/usd/usdGeom/schema.usda).

More information on schemas can be found here: https://graphics.pixar.com/usd/release/tut_generating_new_schema.html

### Creating new Schema Extensions and Naming Conventions

Schema extensions are created by defining the schema using USD syntax, typically in a file called `schema.usda`.  Before defining your schema classes, you must determine the name of your schema library.  Since the entire USD community can add schema extensions, it is important to be able to recognize from which organization / application a schema extension originates and to name them uniquely enough such that naming collisions do not occur across applications.  For example, across the NVIDIA organization, we want that our schema extensions are easily recognizeable by the community so it is clear what Omniverse will support and what 3rd party applications may not have support for.  In general, you can expect the following:

- `Omni` is used as the recognizeable prefix used for our schema extensions. If `Omni` is not appropriate, other prefixes may be used as long as they are distinct enough to recognize that they came from NVIDIA (e.g., `PhysX`)
- All applied API schemas will end with the `API` suffix (as well as adhering to the prefix rule above)
- All properties added by an API schema will start with a recognizeable namespacing prefix (e.g., `omni`) and be namespaced appropriately (e.g., `omni:graph:attrname`, etc.)
- Properties within an IsA schema may have namespace prefixes if derived from core USD schema types.

The samples provide examples for two types of schemas, codeful and codeless.  The former will have C++ / Python code generated for it, the latter will only have USD plug-in information generated.  These are provided in the `src/usd-plugins/schema/omniExampleSchema` and `src/usd-plugins/schema/omniExampleCodelessSchema` in their respective `schema.usda` files.  

### Configuring Schema Generation

These samples use a tool called `repo_usd` to generate the schema code and plug-in information that will be built and distributed.  This is a small wrapper around `usdGenSchema`, provided with a USD build, with several options that control where the information is generated and optionally if build information should be generated to support build of the schema libraries.  All options are defined in a `toml` file called `repo.toml` at the root of the repository.

First, we must tell `repo_usd` where the USD dependencies are so that it knows where to find `usdGenSchema` (for code / plug-in information generation only) and associated include and library paths (for makefile generation, if selected).  Let's consider generation only first - `repo_usd` needs two locations to function properly:

- The path to the USD build that will be used (referred to as `usd_release_root`)
- The path to a Python environment consistent with that which built the USD libraries (referred to as `usd_python_root`)

Since the samples here rely on NVIDIA's USD libraries pulled by `packman`, the `repo.toml` file can be configured as follows:

```
[repo_usd]
usd_release_root = "${root}/_build/usd-deps/nv-usd/release"
usd_python_root = "${root}/_build/usd-deps/python"
```

A couple of things to note here:

- `${root}` is a special token that `repo_usd` will replace with the path to the root of the repository.  All paths defined in `repo.toml` should be specified relative to `${root}` unless noted otherwise.
- The paths here reference the `linkPath` property that is defined in the `deps/usd-deps.packman.xml` file.  When `packman` pulls these packages down, it will place them in these locations.
- To change the USD build you are referencing, simply change the path specified here (relative to `${root}`).  Make sure that the python environment you use is consistent with that which was used to build the USD libraries being referenced (e.g. if you built USD with Python 3.10, make sure your `usd_python_root` points to an environment containing Python 3.10).

Next, we need to configure the options for our two schemas.  `src/usd-plugins/schema/omniExampleSchema` defines a sample _codeful_ schema and `src/usd-plugins/schema/omniExampleCodelessSchema` defines a sample _codeless_ schema.  In both cases, the schema definition conforms to that expected by USD v20.08, since that is the version currently being used for Omniverse kit-based applications.  Comments were added to these schema files to indicate where these definitions will change with later USD versions.  Each schema is configured independently in the `repo.toml` file and at minimum requires the following:

- The path to the `schema.usda` file defining the schema classes
- The path to a directory to generate the code / plug-in information into
- The `library_prefix` that is set to the same value as `libraryPrefix` in the `schema.usda` file (necessary for generating the python wrapper code, only necessary for codeful schemas)
- The set of USD libraries that the schema depends on.  Typically at minimum this consists of `arch`, `tf`, `vt`, `sdf`, and `usd` (and only necessary for codeful schemas)
- Whether or not the schema is codeless (by default, the schema is codeful, so this is only necessary if you have a codeless schema)

These options can be configured as follows in the `repo.toml` file:

```
[repo_usd]
usd_release_root = "${root}/_build/usd-deps/nv-usd/release"
usd_python_root = "${root}/_build/usd-deps/python"

[repo_usd.schema.omniExampleSchema]
schema_file = "${root}/src/schema/omniExampleSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleSchema/generated"
library_prefix = "OmniExample"
usd_lib_dependencies = [
    "arch",
    "tf",
    "vt",
    "sdf",
    "usd"
]

[repo_usd.schema.omniExampleCodelessSchema]
schema_file = "${root}/src/schema/omniExampleCodelessSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleCodelessSchema/generated"
is_codeless = true
```

Note that all USD dependencies are declared with their base library names.  If using a USD build with a custom prefix, you may specify this prefix in the `repo_usd` configuration and it will be applied to all USD base names when forming the library dependency.  For example, if your USD libraries were of the form lib_arch, lib_tf, etc., you would specify the prefix as follows:

```
[repo_usd]
usd_release_root = "${root}/_build/usd-deps/nv-usd/release"
usd_python_root = "${root}/_build/usd-deps/python"
usd_lib_prefix = "lib_"

[repo_usd.schema.omniExampleSchema]
schema_file = "${root}/src/schema/omniExampleSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleSchema/generated"
library_prefix = "OmniExample"
usd_lib_dependencies = [
    "arch",
    "tf",
    "vt",
    "sdf",
    "usd"
]

[repo_usd.schema.omniExampleCodelessSchema]
schema_file = "${root}/src/schema/omniExampleCodelessSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleCodelessSchema/generated"
is_codeless = true
```

Note that specifying the USD library dependencies is required for codeful schemas regardless of if you wish to generate makefiles for the schema plug-ins or not.  This is because one of the generated files (`moduleDeps.cpp`) declares the dependencies in the source and must have knowledge of them to generate the right boilerplate code.

__Be aware there are two settings, `library_prefix` and `usd_lib_prefix` that sound similar but are used differently.  `library_prefix` matches the the value of `libraryPrefix` in the `schema.usda` file and is used to ensure the python wrapper code is generated correctly.  `usd_lib_prefix` is used when you need to tell the tool that the USD libraries you are linking to were built with a specific prefix that needs to be prepended to the base USD library name to ensure the proper library is found.__

This should be enough to generate the schema C++ and Python source code (if codeful) and, in all cases, the `plugInfo.json` and `generatedSchema.usda` files that need to be distributed with your plug-in so that the USD Schema Registry understands your schema types.  The provided scripts (`build.bat` / `build.sh`) perform the steps necessary to:

- Download the NVIDIA USD packages via `packman` (both the debug and release builds)
- Generate the schema information for the two schemas

If you choose to generate against a local USD build, you may remove the lines from these files that pull down the packages.

To generate the schema code, simply run:
```
./build.bat --generate  (Windows)
./build.sh --generate   (Linux)
```

### Generating Makefiles

`repo_usd` can also generate makefiles in either `cmake` or `premake` format (`cmake` is the default).  If you would like to take advantage of this functionality, there are a few more options in the `repo.toml` file that are required:

- The path to the debug build of USD that will be used to set debug configurations in the makefiles
- The makefile format you would like to generate
- Whether or not you would like to generate the _root_ makefiles.  By default, this option is `false` (see below for further explanation)

These options can be set in the `repo.toml` file as follows:

```
[repo_usd]
usd_debug_root = "${root}/_build/usd-deps/nv-usd/debug"
usd_release_root = "${root}/_build/usd-deps/nv-usd/release"
usd_python_root = "${root}/_build/usd-deps/python"
generate_schema_makefiles = true
schema_makefile_format = "cmake"

[repo_usd.schema.omniExampleSchema]
schema_file = "${root}/src/schema/omniExampleSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleSchema/generated"
usd_lib_dependencies = [
    "arch",
    "tf",
    "vt",
    "sdf",
    "usd"
]

[repo_usd.schema.omniExampleCodelessSchema]
schema_file = "${root}/src/schema/omniExampleCodelessSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleCodelessSchema/generated"
is_codeless = true
```

Note that if you want to generate `Premake` files, simply change `schema_makefile_format` to `premake`.  When the `generate_schema_makefiles` option is on, a makefile in the specified format will be generated in the specified `generate_dir` for each schema.  By default, the following directories are used (per-schema) for artifact outputs:

- `install_root`: path at which to root the plug-in output files (default = `"${root}/_install/${platform}/${schema_name}`)
- `include_dir`: path relative to `install_root` where public headers for the schema will be copied
- `lib_dir`: path relative to `install_root` where the schema C++ library will be built to
- `module_dir`: path relative to `install_root` where the schema Python library will be built to and python module files copied
- `resources_dir`: path relative to `install_root` where resource files (such as `plugInfo.json`, `generatedSchema.usda`, etc.) will be copied

You can change any of these options per-schema by specifying values for each of the above keys in the `repo.toml` file.

For premake generation, you may also specify:

- `build_dir`: path relative to the repository root where you would like the projects created by premake to be generated.  The premake file itself (`premake5.lua`) is generated to the same directory as the source, but the artifacts of the premake file will be generated to this location.  By default, this location is `${root}/_build/${schema_name}`.  Note that this option is only valid for premake - CMake artifacts are generated in the directory specified when running CMake configure and mirrors the source tree.

With these options, only makefiles specific for each schema are generated in their respective `generate_dir` locations and it is your responsibility to use these makefiles in whatever way fits your own local build infrastructure.  For a more turnkey type setup, you can also instruct `repo_usd` to generate _root makefiles_ which are CMake or Premake files that are generated at the root of the repository (`CMakeLists.txt` and `premake5.lua`, respectively).  These files ensure that the appropriate included makefiles can be found as well as includes each makefile from each schema such that a single build will build everything that was generated.

__NOTE: Ensure that you do not have a CMakeLists.txt or premake5.lua file already at your repository root or setting this option will overwrite it!__

Note that the makefiles generated will include macros provided by `repo_usd` that set compiler and linker switches, take care of copying public headers and resource files, etc.  Interested parties can examine these files in `_repo/repo_usd/templates`.

To turn this option on, set the `generate_root_makefile` option to `true` in the `repo.toml` file:

```
[repo_usd]
usd_debug_root = "${root}/_build/usd-deps/nv-usd/debug"
usd_release_root = "${root}/_build/usd-deps/nv-usd/release"
usd_python_root = "${root}/_build/usd-deps/python"
generate_schema_makefiles = true
generate_root_makefile = true
schema_makefile_format = "cmake"

[repo_usd.schema.omniExampleSchema]
schema_file = "${root}/src/schema/omniExampleSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleSchema/generated"
library_prefix = "OmniExample"
usd_lib_dependencies = [
    "arch",
    "tf",
    "vt",
    "sdf",
    "usd"
]

[repo_usd.schema.omniExampleCodelessSchema]
schema_file = "${root}/src/schema/omniExampleCodelessSchema/schema.usda"
generate_dir = "${root}/src/schema/omniExampleCodelessSchema/generated"
is_codeless = true
```

Note that this option is turned on (not the default) in the sample repository.  If you want to integrate the schema plug-in makefiles with your own infrastructure, be aware that the generated makefile uses functions provided by the templates.  For both CMake and Premake you will need to add the templates to set everything up appropriately.  At a minimum, for CMake this means the following:

```
# declare your minimum version of cmake e.g.:
# cmake_minimum_required(VERSION 3.13)

# create a projet e.g.:
# project(usd-plugins)

# include the cmake file required for declaring a plugin
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} _repo/repo_usd/templates/cmake)
include(PxrPlugin)

# add each schema subdirectory to the list
add_subdirectory(src/schema/omniExampleSchema/generated)
add_subdirectory(src/schema/omniExampleCodelessSchema/generated)
```

Similarly for Premake, this means the following:

```
require("_repo/repo_usd/templates/premake/premake5-usdplugin")

-- declare a workspace or use your existing one e.g.:
-- workspace(usd-plugins)

-- make sure these definitions are in the context of a workspace
-- they do not declare their own
require("src/schema/omniExampleSchema/generated/premake5")
require("src/schema/omniExampleCodelessSchema/generated/premake5")
```

### Additional Configuration Options

The `repo_usd` tool supports some additional advanced options via the `repo.toml` file if required in your build environment.

- `additional_include_dirs`: A list of additional directories that should be used to find include files the plug-in may depend on
- `additional_library_dirs`: A list of additional directories that should be used to find library files the plug-in may depend on
- `additional_libs`: A list of additional libs that need to be linked to the plug-in to build
- `additional_cpp_files`: A list of additional `.h` / `.cpp` files to include as part of the plug-in build
- `additional_module_files`: A list of additonal `.py` files to include as part of the python module distribution for the plug-in
