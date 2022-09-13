## Kit Sample Schema

### Introduction

This repository contains samples for building USD schema extensions and kit extensions that use those USD schema extensions.  It consists of two parts:

- `usd-schema-extension`: A collection of contrived USD schema extensions that illustrate various schema concepts.  These examples include both codeful and codeless schemas (contained under `usd-schema-extension/src`) as well as the complete `repo` toolset to generate, build, test, document, license, and package the resulting assets into zipped packages that can be uploaded to a package repository for others to consume.
- `kit-schema-extension`: A collection of kit extensions corresonding to each USD schema extension that illustrates how to bundle the USD schema extension for use in kit (contained under `kit-schema-extension/src`).  Like `usd-schema-extension`, it contains the complete `repo` toolset.

Both samples above contain their own `README` file that discusses how to use that section of the repository.

Feel free to fork this repo as a basis for creating your own USD schema extensions.  The `usd-schema-extension` portion is the same whether using internally to NVIDIA or externally.  The `kit-schema-extension` assumes the kit extensions are _external_ in the sense that they aren't built with `kit` itself, they are added after the fact by either dropping the unzipped package into an appropriately named directory in the installed `kit\exts` directory or by adding the directory where the extensions are unzipped / copied to the directories kit searches for extensions in.  

- [The USD schema extension example](./usd-schema-extension/README.md)
- [The Kit schema extension example](./kit-schema-extension/README.md)

Note that throughout the configuration and schema files, comments have been added to help explain what the options do to help guide you through setting up your own schema extensions.

### Building the Examples

The USD schema extensions can be built and packaged separately from the kit extensions using the `usd-schema-extension/build.bat` or `usd-schema-extension/build.sh` files.  The kit extensions cannot be built independently and instead rely on the USD schema extensions being built first before calling `kit-schema-extension/build.bat` or `kit-schema-extension/build.sh`.  

The repository root provides a convenience `build.bat` or `build.sh` file that can build both in sequence.
```
build.bat / build.sh
Usage: build.bat [configuration] [--package]

Args:
    configuration: (Optional) The configuration to build, one of [debug | release].  If no configuration is specified, it defaults to release.
    --package: (Optional) If specified, the build will package the built and staged extensions into .7z files for upload / distribution.
    --skip-test: (Optional) If specified, the build will skip testing of the artifacts.
```