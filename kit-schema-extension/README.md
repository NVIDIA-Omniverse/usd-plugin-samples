## Introduction

Generating and building a USD schema extension provides all that is needed to add the new schema to USD and work with USD APIs to use that schema.  However, packaging that USD schema extension for easy use in `kit` requires a few more steps.  As part of the USD Schema Extension Developer Experience, this section of the repository provides basic examples of putting together `kit` extensions that setup the USD schema extensions for direct consumption in `kit` applications.

The sections below provide guidelines for schema developers to setup their kit extensions and package them together with their USD schema extensions.  In particular, the repository is setup such that the extensions can be used without building and bundling them as part of the `kit` build infrastructure.  If you also need to build the extensions as part of the `kit` build, an additional section entitled `Integrating Kit Extension Source Into Kit Builds` is provided for reference.

Note that this repository only covers kit extensions insofar as they pertain to being able to use USD schema extensions inside of `kit`.  For a more general reference in building `kit` extensions, see https://github.com/NVIDIA-Omniverse/kit-extension-template.

## Creating Kit Extensions and Naming Conventions

In general, the kit extension that exposes the schema to `kit` should follow a similar naming scheme to that of the USD schema extension.  Convention for the C++ version of the USD schema extension names tend to follow camel casing that includes the fully namespaced name (e.g. `omniGeospatialSchema`) and the Python version of the USD schema extension names tend to follow pascal casing that includes the fully namespaced name (e.g. `OmniGeospatialSchema`).  In contrast, `kit` naming conventions favor a namespaced name separated by the `.` character (e.g. `omni.geospatial.schema`).  In general, we recommend that the kit extension be named the same as the USD schema extension, but with the `.` character separating the different parts of the properly namespaced name.

### Gitlab Project Setup

For a starting point, feel free to fork this repo.  It adheres to the internally recommended structure for both USD schema extensions and kit extensions that use those USD extensions.  If you are starting from scratch, the recommended project structure for the kit extension is as follows:

```
root
  deps
    repo-deps.packman.xml
  src
    extension_name
      config
        extension.toml
      extension
        name
          __init__.py
      PACKAGE-INFO.yaml
   tools
     packman
     repoman
  build.bat
  build.sh
  CHANGES.md
  README.md
  repo.sh
  repo.bat
  repo.toml
  VERSION.md
```

Most of these files are fairly standard for any repository that uses packman and repoman and can be obtained in the same way as was done for your USD schema extension.  Note that this README always assumes that the USD schema extension and the kit extension are in the same repository as in this example.  If this is not the case, building the kit extension would work slightly differently (see `Integrating Kit Extension Source Into Kit Builds`).

To gather licenses and package the kit extensions, at a minimum your `deps/repo-deps.packman.xml` file should contain the following:

```
<project toolsVersion="6.45">
    <dependency name="repo_man" linkPath="../_repo/deps/repo_man">
        <package name="repo_man" version="1.5.1" />
    </dependency>
    <dependency name="repo_package" linkPath="../_repo/deps/repo_package">
        <package name="repo_package" version="5.6.1" />
    </dependency>
    <dependency name="repo_licensing">
        <package name="repo_licensing" version="1.9.6" />
    </dependency>
</project>
```

All kit extensions should have `PACKAGE-INFO.yaml` files defined.  Note that these files are **per extension**.

## Using Repo Tooling to Package Extension

Simple kit extensions that wrap the schema generally don't need additional build tooling.  Instead, support for packaging the kit extension together with the USD schema extension is required.  Assuming they are in the same repository as this `README` assumes, then packaging generally consists of staging the kit extension content to a staging directory (typically `_install\kit_extension_name`) and copying the corresponding staged USD schema extension (typically found in `usd-schema-extension\_install\usd_schema_extension_name`).  Packaging can then be accomplished by calling `repo_package` to zip up the staging directory for distribution.

The `build.bat` and `build.sh` files provide an example of setting up this staging area by copying the kit and USD schema extension files to their appropriate destinations.

Note that packaging (zipping) isn't always strictly required.  As such, all `build.bat` and `build.sh` files take an optional argument `--package` which controls if packaging takes place or not.  In the absence of the `--package` argument, the default behavior is to not zip the staging location up.

## Using the Kit Extension

When the kit extension is built, it pulls the appropriate files needed by the extension from the usd-schema portion of the build.  To use the kit extension, add an extension path to kit that points to the `kit-schema-extension/_install/${platform}_${config}` directory (or the directory you copy the extension content to).  Loading the extension should then allow you to use the schema from the python scripting environment via `import OmniExampleSchema`.