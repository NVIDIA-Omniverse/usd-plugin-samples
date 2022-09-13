
## Introduction

Although the base USD distribution provides a wealth of functionality for defining and composing scene content, it is inevitable
that users will want to extend USD prims with additional data that can be persisted and processed by USD run-time components.
USD provides schema extensions as a mechanism for doing that.  Given the scope of Omniverse, we introduce the USD Schema Extension
Developer Experience with the goals of:

- Providing developers tools and guidelines for making the extension experience as easy as possible
- Standardizing the development and build experience such that all extensions are generated, built, and packaged in the same way (where possible)
- Naming extensions in such a way that it is easy to tell from which organization they originated

The sections below provide guidelines for schema developers to set up their schema projects, generate the code, build, and package the resulting libraries for use by `kit`.  In addition, we try to address how external USD developers in the community would be able to build their own schemas easily with a subset of our internal tooling environment.

## A Brief Introduction to USD Schemas

Recall that USD defines the concept of a "prim".  Prims abstractly serve as data containers for groups of logically related data.  These groups are referred to as _schemas_ (or more specificaly, _prim schemas_ to distinguish them from _property schemas_).  When we say that a prim has a type of _Mesh_, what we really mean is that the prim provides the logical set of related data defined by the `UsdGeomMesh` schema (e.g., vertices, faces, normals, etc.).

Schemas are divided into two major categories _IsA_ schemas and _API_ schemas.  _IsA_ schemas are meant to define a specific purpose for a prim.  In the case above, a _Mesh_ prim is a prim who's specific purpose is to represent a mesh.  A prim can only subscribe to a single _IsA_ schema - that is, it has a single well defined role in the scene hierarchy.  These types of schemas can be inherited and within that inheritance hierarchy schemas can be either _abstract_ or _concrete_.  Only concrete schemas are instantiable in the USD scene hierarchy.

_API Schemas_, on the other hand, serve only as additional data groups on prims as well as a well-defined API to get and set those values.  These schema types can be applied to any prim in the scene hierarchy (as long as the schema rules say it can be applied to that prim type). If a prim has a type, you will see that type in the scene hierarchy.  If a prim has an API schema applied, you won't see a difference in its type, but will still be able to ask that prim for the data contained within that schema.  These types of schemas can be either _non-applied_ or _applied_ API schemas, with the difference being that applied API schemes will be recorded in the USD file such that they retain that information on interchange of the scene data to other applications.  If a schema is an applied API schema, it can be either single instance (_single-apply_ API schemas, applied only once to a prim) or multi-instance (_multiple-apply_ API schemas, can be applied several times, each defining a unique instance).

USD ships with many schema definitions that you may be familiar with, including "IsA" schemas (e.g., Mesh, Xform) and "API" schemas (e.g., UsdShadeMaterialBindingAPI, UsdCollectionAPI, etc.).  These can all be found in their respective modules (e.g., the schema set provided by UsdGeom can be found in pxr/usd/usdGeom/schema.usda).

More information on schemas can be found here: https://graphics.pixar.com/usd/release/tut_generating_new_schema.html

## Creating Schema Extensions and Naming Conventions

Schema extensions are created by defining the schema using USD syntax, typically in a file called `schema.usda`.
Before setting up the gitlab project that will host your schema, you must determine the name of your schema.  Since the entire USD community can add schema extensions, it is important to be able to recognize from which organization / application a schema extension originates and to name them uniquely enough such that naming collisions do not occur across applications.  Across the NVIDIA organization, we want that our schema extensions are easily recognizeable by the community so it is clear what Omniverse will support and what 3rd party applications may not have support for.  In general, you can expect the following:

- `Omni` must be used as the recognizeable prefix used for our schema extensions. If `Omni` is not appropriate, other prefixes may be used as long
as they are distinct enough to recognize that they came from NVIDIA (e.g., `PhysX`).
- All applied API schemas must end with the `API` suffix (as well as adhering to the prefix rule above)
- All properties added by an API schema must start with a recognizeable namespacing prefix (e.g., `omni`) and be namespaced appropriately (e.g., `omni:graph:attrname`, etc.)
- Properties within a IsA schema should have namespace prefixes if derived from core USD schema types.

### Gitlab Project Setup

For a starting point, feel free to fork this repo.  It adheres to the internally recommended structure for both USD schema extensions and kit extensions that use those USD extensions.  If you are starting from scratch, the recommended project structure for the schema extension is as follows:

```
root
  deps
    host-deps.packman.xml
    repo-deps.packman.xml
  src
    schema_name
       __init__.py
       module.cpp
       schema.usda
       PACKAGE-INFO.yaml
  tests
      testStages
          testStage.usda
      test.py
  tools
    packman
    repoman
  build.bat
  build.sh
  CHANGES.md
  LICENSE.txt
  README.md
  repo.sh
  repo.bat
  repo.toml
  VERSION.md
```

Most of these files are fairly standard for any repository that uses packman and repoman.  These files can be acquired via a project that already uses them.

At a minimum, to work with schemas you need the following repo tools:
- `repo_usdgenschema`
- `repo_build`
- `repo_test`
- `repo_package`

That is, at minimum your `deps/repo-deps.packman.xml` file should contain the following:
```
<project toolsVersion="6.45">
    <dependency name="repo_man" linkPath="../_repo/deps/repo_man">
        <package name="repo_man" version="1.5.1" />
    </dependency>
    <dependency name="repo_usdgenschema" linkPath="../_repo/deps/repo_usdgenschema">
        <package name="repo_usdgenschema" version="0.8.0" />
    </dependency>
    <dependency name="repo_build" linkPath="../_repo/deps/repo_build">
        <package name="repo_build" version="0.22.0" />
    </dependency>
    <dependency name="repo_test" linkPath="../_repo/deps/repo_test">
        <package name="repo_test" version="1.4.1" />
    </dependency>
    <dependency name="repo_package" linkPath="../_repo/deps/repo_package">
        <package name="repo_package" version="5.6.1" />
    </dependency>
</project>
```

Your `host-deps.packman.xml` file may vary, but we recommend the following configurations for MSVC and the Windows SDK:
```
<project toolsVersion="6.45">
    <dependency name="premake" linkPath="../_build/host-deps/premake" tags="non-redist" >
        <package name="premake" version="5.0.9-nv-main-68e9a88a-${platform}" platforms="windows-x86_64 linux_x86_64 linux-aarch64" />
    </dependency>
  <dependency name="msvc" linkPath="../_build/host-deps/msvc" tags="non-redist">
        <package name="msvc" version="2017-15.9.17-winsdk-19041" platforms="windows-x86_64" />
  </dependency>
  <dependency name="winsdk" linkPath="../_build/host-deps/winsdk" tags="non-redist">
        <package name="winsdk" version="10.0.19041.0" platforms="windows-x86_64" />
  </dependency>
</project>
```

All schema extensions must define a `PACAKGE-INFO.yaml` file.  This is similar to what would normally be defined for a new project, except it must contain one extra key, `USD Version` that defines the semantic version of USD the schema extension used to generate / build the schema libraries.

An example from the omni-geospatial-schema repository:
```
Package : omni-geospatial-schema
Maintainers : Edward Slavin
Description : Schema extension for attributing prims with geodetic coordinates and transform information
SWIPAT NvBug :
Repository : https://gitlab-master.nvidia.com/omniverse/omni-geospatial-schema
License Type : NVIDIA
USD Version : 20.08
```

## Using Repo Tools to Generate, Build, and Package Schemas

### Generating the Schema and Code Files

USD schema extensions can be fully generated, built, and packaged within the framework of `repo` tools with the addition of `repo_usdgenschema`.
This repo tool does all the heavy lifting of using `usdgenschema` to generate your schema and code files based on your schema definition as well
as generating the premake files necessary to pass to `repo_build` to build the schema libraries.  It can handle multiple separate schemas in one repository as well as schemas that are both code-based and codeless.

Like other `repo` based tools, configuration is defined in your `repo.toml` file that typically sits at the root of your gitlab repository.
Each schema definition you would like to generate is represented as a separate key in the `[repo_usdgenschema]` section of the configuration file.
The only pre-requisite for supporting multiple schema definitions in the same repository as that their schema definitions exist in separate directories.

For each schema in your repository, you must create a separate configuration section where each section has the following format:

```
[repo_usdgenschema.x]                   (x=libraryName of the schema defined in the GLOBALS section of the schema definition)
schema_file = "path_to_schema_file"     (required, typically pathed relative to repo ${root} variable)
codeless = true | false                 (optional, default = false, codeless schemas will skip generation of C++ / Python code and premake project)
schema_module_name = "libraryPrefix"    (optional, defaults to ProperCase value of 'x', typically set to the value of libraryPrefix in the GLOBALS section of the schema definition)
additional_cpp_files = []               (optional, defaults to [], list of paths to additional files to include in the C++ premake project generated)
                                        (each path is typically pathed relative to repo ${root} variable)
additional_python_files = []            (optional, defaults to [], list of paths to additional files to include in the Python premake project generated)
                                        (each path is typically pathed relative to repo ${root} variable)
                                        (although optional, this will almost always contain at least a `module.cpp` file required to wrap types for boost::python)
python_init_files="path_to_init_file"   (required, path to the __init__.py file that will accompany the compiled schema Python library)
package_info_file="path_to_package-info_file"   (optional, strongly recommended, path to the PACKAGE-INFO.yaml file to include with the built binaries)
```

USD schema generation is performed such that the output is generated at `repo_man.repo_folders['build']/usdgenschema/generate`.  Each schema will be in a sub-directory of this directory with the name of the sub-directory equal to the schema name (`x` in the definition above).  The premake file will be generated in the root `generate` directory.  Note that this premake file contains a single workspace as well as a C++ and Python project for each code-based schema defined in the configuration.

Once a schema has been generated and compiled, its default directory structure (and root for packaging) looks like this:

```
package_root
    PACKAGE-INFO.yaml
    include
        schema_name
            .h files
    lib
        schema_name.dll / schema_name.so
        schema_name.lib
        python
            module_name
                __init__.py
                _schema_name.pyd / _schema_name.so
    usd
        module_name
            resources
                generatedSchema.usda
                plugInfo.json
                module_name
                    schema.usda
```

This structure can be distributed independently for users who wish to use the schema libraries or packaged as a part of a `kit` extension for use within `kit` applications.

### Defining Custom USD Dependencies

By default, `repo_usdgenschema` uses the USD version targeted by the latest `kit` release.  In this case, this is `nv_usd` version `20.08` using the following `packman` configuration:

```
<project toolsVersion="6.45">
    <dependency name="nv_usd_py37_release" linkPath="../_build/deps/usd_py37_release" >
        <package name="nv-usd" version="20.08.nv.1.2.2404.6bf74556-win64_py37_release-main" platforms="windows-x86_64" />
        <package name="nv-usd" version="20.08.nv.1.2.2404.6bf74556-linux64_py37-centos_release-main" platforms="linux-x86_64" />
        <package name="nv-usd" version="20.08.nv.1.2.2404.6bf74556-linux-aarch64_py37_release-main" platforms="linux-aarch64" />
    </dependency>
    <dependency name="nv_usd_py37_debug" linkPath="../_build/deps/usd_py37_debug" >
        <package name="nv-usd" version="20.08.nv.1.2.2404.6bf74556-win64_py37_debug-main" platforms="windows-x86_64" />
        <package name="nv-usd" version="20.08.nv.1.2.2404.6bf74556-linux64_py37-centos_debug-main" platforms="linux-x86_64" />
        <package name="nv-usd" version="20.08.nv.1.2.2404.6bf74556-linux-aarch64_py37_debug-main" platforms="linux-aarch64" />
    </dependency>
</project>

```

`repo_usdgenschema` supports building against different USD configurations defined in your own `packman` files.  If you do use a different USD version, you must let `repo_usdgenschema` know so that it knows where to find `usdgenschema` as well as the include / lib paths necessary for premake file generation.  This can be done using the following configuration in the `repo.toml` file:

```
[repo_usdgenschema]
usd_debug_root = "path_to_usd_debug_installation"       (required if using a custom USD configuration,
                                                         typically pathed relative to repo ${root} variable)
usd_release_root = "path_to_usd_release_installation"   (required if using a custom USD configuration,
                                                         typically pathed relative to repo ${root} variable)
```

If these keys are not defined, `repo_usdgenschema` will use its own configuration (as defined above).

### Linking Schema Libraries to Additional External Libraries

In some cases, schema libraries may include additional code (through the `additional_cpp_files` or `additional_python_files` configuration keys) that rely on non-USD libraries to provide some of the functionality.  When this functionality is included, it is necessary for the final schema library to link to these external libraries when building but also to optionally include their `.dll / .so` files in the schema library distribution package.

**This feature is not yet supported by `[repo_usdgenschema]`, coming soon!**

### Building the Schema Libraries

For code-based schemas, `repo_build` is used to build the generated schema code into C++ and Python libraries that can be used downstream.  Because `repo_usdgenschema` took care of creating the premake file necessary to perform this action, you must tell `repo_build` the location of this premake file so that it knows what to build. You do this by adding configuration to your `repo.toml` file for `repo`:

```
# tell repo_build where the premake file that was generated by repo_usdgenschema is
[repo.folders]
premake_file = "{root}/_build/usdgenschema/generate/premake5.lua"
```

If your `build` key for `repo.folders` is different, make sure to modify the path above accordingly.

When building for windows, it may be necessary to inform `repo_build` what Windows SDK version you are using and where the include / lib / binary paths are for the  version of the compiler (`msvc`) being used.  By default, the assumption is that the version `10.0.19041.0` of the Windows SDK will be used in conjunction with the `msvc` version compatible with that Windows SDK version.  That is, it assumes that the following configuration is in your `packman` files (typically `host-deps.packman.xml`):

```
<project toolsVersion="6.45">
    <dependency name="premake" linkPath="../_build/host-deps/premake" tags="non-redist" >
        <package name="premake" version="5.0.9-nv-main-68e9a88a-${platform}" platforms="windows-x86_64 linux_x86_64 linux-aarch64" />
    </dependency>
  <dependency name="msvc" linkPath="../_build/host-deps/msvc" tags="non-redist">
        <package name="msvc" version="2017-15.9.17-winsdk-19041" platforms="windows-x86_64" />
  </dependency>
  <dependency name="winsdk" linkPath="../_build/host-deps/winsdk" tags="non-redist">
        <package name="winsdk" version="10.0.19041.0" platforms="windows-x86_64" />
  </dependency>
</project>
```

When using this configuration, the generated premake file will have the appropriate defaults in place for the SDK version and paths, but you must still let it know where the tools are.  In addition, you may choose to use a different Windows SDK / msvc version setup.  In both cases, you must add configuration for `repo_build` in your `repo.toml` file so that it can find the libraries and compiler tools it needs to build the schema libraries.  These can be passed to `premake` via the `extra_args` argument of `repo_build`.  These extra arguments are as follows:

- `--winsdkroot`: Required when building for windows, must point to the root path you've configured for the winsdk in your `host-deps` packman file
- `--msvcroot`: Required when building for windows, must point to the root path you've configured for msvc in your `host-deps` packman file
- `--winsdkversion`: Required when building for windows, but only if you are using a different Windows SDK version than the default (`10.0.19041.0`)
- `--msvcbinarypaths`: Required when building for windows to customize the paths where the build system will find binaries such as VC.exe, rc.exe, etc.
                       (Only required if using a different msvc version than the default `2017-15.9.17-winsdk-19041`)
                       (Each path must be separated by a `;` character)
- `--msvcincludepaths`: Required when building for windows to customize the paths where standard CRT includes can be found
                        (Only required if using a different msvc version than the default `2017-15.9.17-winsdk-19041`)
                        (Each path must be separated by a `;` character)
- `--msvclibpaths`: Required when building for windows to customize the paths where standard CRT libs can be found
                    (Only required if using a different msvc version than the default `2017-15.9.17-winsdk-19041`)
                    (Each path must be separated by a `;` character)
- `--usdlibprefix`: Required when the USD libraries you are linking to have a specialized prefix (e.g. libpxr_, etc.)

Altogether, this looks like:

```
[repo_build.premake]
extra_args = [
    "--winsdkroot=${root}/_build/host-deps/winsdk"
    "--msvcroot=${root}/_build/host-deps/msvc"
    "--winsdkversion=x"
    "--msvcbinarypaths=x;y;z"
    "--msvcincludepaths=x;y;z"
    "--msvclibpaths=x;y;z"
    "--usdlibprefix=x"
]
```

Additionally, you must let `repo_build` know where the solution file is for MSBuild when building on windows via the `repo_build.msbuild` configuration key:

```
[repo_build.msbuild]
sln_file = "${root}/_build/usdgenschema/generate/usdgenschema_workspace.sln"
```

Don't forget to adjust that path if your `build` key of `repo.folders` is different than the default.

Note that if your schema is codeless, no premake file will be generated and you do not have to invoke `repo_build`.  If at least one schema is code-based, then invoking `repo_build` will build all code-based schema libraries in your configuration.

### Packaging up the Schema Libraries for Packman

When `repo_usdgenschema` creates the premake file, it adds post-build events to ensure that the built libraries, generated schema definition, and include files are copied together into the appropriate directory structure expected by by `kit` described above.  The package root is always of the form `_install/${schema_name}/${platform}_${config}`. `repo_package` can be used in order to package up the generated schemas.

For each schema contained within the repository, a new key must be created in the `repo.toml` configuration for `repo_package` of the form:

```
[repo_package.packages."x"]             (x = the name of the package for the schema extension, which should conform to naming conventions described above)
omniverse_flow_version_scheme = true
package_per_config = true
```

In addition, you must create a `package.toml` file that defines the packaging logic for each schema package you defined.  It must contain (for each schema package):

- the files that will be included in the package no matter the configuration (typically at least your `PACKAGE_INFO.yaml` file)
- the package root for each platform / configuration combination being supported

An example `package.toml` for a single schema `mySchema` is given below:

```
[mySchema]
files = [
    ["PACKAGE-INFO.yaml"]
]
[mySchema.windows-x86_64.debug]
files = [
    ["_install/mySchema/windows-x86_64_debug" "."]
]
[mySchema.windows-x86_64.release]
files = [
    ["_install/mySchema/windows-x86_64_relesae" "."]
]
[mySchema.linux-x86_64.debug]
files = [
    ["_install/mySchema/linux-x86_64_debug" "."]
]
[mySchema.linux-x86_64.release]
files = [
    ["_install/mySchema/linux-x86_64_release" "."]
]
[mySchema.linux-aarch64.debug]
files = [
    ["_install/mySchema/linux-aarch64_debug" "."]
]
[mySchema.linux-aarch64.release]
files = [
    ["_install/mySchema/linux-aarch64_release" "."]
]
```

This should be enough information for `repo_package` to create and upload the package to packman.  All packages will receive the following naming convention:

`${schema_name}@${schema_version}+${branch}.$(build_number).${git_hash}.${build_location}.${platform}.${config}`

where:
- schema_name: the name of the schema package
- schema_version: the version of the schema package, taken from the VERSION file if available
- branch: the git branch the package is being built from
- build_number: a generated build number when building on teamcity
- git-hash: the first 8 characters of the git-hash of the commit being built
- build_location: teamcity or local
- platform: the target platform the binaries were built for
- config: the configuration the binaries were built for

**Note that under this scheme, if you have multiple schemas in the same repository they cannot be versioned independently**

Note that packaging will only take place if the `--package` option is passed to `./build.bat` / `./build.sh`.

## Testing, Documentation, and Licensing

USD schema extensions can be tested with the use of `repo_test`. This `repo` tool allows the build script to run all your tests for any amount of schemas in your `src/` directory. You can create your own test stages and tests to assert that your schema extensions behave as expected.

An example of this is seen in the `tests/` directory with a structure like this:

```
tests
    testStages
        testStage.usda
    test.py
```

You can define multiple test stages and files, and `repo_test` can run all of your test files. To use `repo_test` the dependency can be added to `deps/repo-deps.packman.xml` like this:

```
<dependency name="repo_test" linkPath="../_repo/deps/repo_test">
    <package name="repo_test" version="1.4.1" />
</dependency>
```

Now, like other `repo` based tools, configuration is defined in your `repo.toml` file that typically sits at the root of your gitlab repository. To let `repo_test` find your test suites, you should define `[repo_test.suites.<test_suite>]` in your `repo.toml` file. In our example we use `pytest`, and a template example of `pytest` would look like this:

```
# Example of Python "pytest"
[repo_test.suites.pytest]
kind = "pytest"

# Record test results to file. Provide a file path ending in .xml extension
# to record a junit compatible report.
# If --coverage is provided, a coverage.xml will be generated in the same
# directory as log_file(defaults to repository root).
log_file = "results.xml"

# Specify extra paths to include in PYTHONPATH. Supports wildcards.
# Note pythonpath is also supported for backwards compatibility.
pythonpath = [
  "${root}/_repo/deps/repo_usdgenschema/_build/deps/usd_py37_${config}/lib/python",
  "${root}/_install/omniExampleSchema/${platform}_${config}/lib/python",
]

# Gather Python tests from a module. Will override discover_path
module_path = "${root}/tests/test_schemas.py"
# Alternate: Discover python tests in directory
discover_path = "${root}/tests"
# The command-line --filter-files arg will override all of this. Specifiy
# a directory, a file, or a pytest test nodeid,
# e.g.: tests/test_run_pytest.py::test_run_pytest_sunny

# Where the Python source lives. This is needed for a sane coverage report, otherwise
# your repository root is used.
python_source_path = "omni/repo/*"

```

One thing to note is when using `discover_path`, test names must start with `test_`. For example instead of `testSchemas.py`, it must be `test_schemas.py` for `repo_test` to be able to find your test files. Also, if you defined a custom USD root, make sure to update your `pythonpath` accordingly to ensure that the right USD configuration is used.

Lastly, in the `build.sh/build.bat` scripts, it calls `repo_test` as the third step. Before calling `repo_test`, the build script sets your `PATH`/`LD_LIBRARY_PATH` to point to the library paths for the schema extension and USD. This is necessary for code-based schemas in order for `repo_test` to find your `dll's`. Again, if you defined a custom USD root, make sure to update the `PATH`/`LD_LIBRARY_PATH` accordingly.

With all this put together, you can now test any amount of schemas (both code-based and codeless) with `repo_test`.