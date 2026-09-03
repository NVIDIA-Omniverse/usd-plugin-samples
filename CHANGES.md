# Changelog

## 5.0.0

- Deprecated repository; preserved as a conceptual reference only,
  no longer actively maintained against current OpenUSD or Kit versions
- Removed `src/kit-extension` (targeted an outdated Kit version)
- Removed Kit/Omniverse-specific packaging instructions from build docs
- Fixed the Python module install on Linux: `install(TARGETS ...)` for the
  Python extension declared only a RUNTIME destination, so the `.so` was
  installed to the library directory instead of the module directory and
  the module could not be imported (thanks @andrewwhitecdw)
- Marked `tools/packman/python.sh` executable so `./build.sh` runs on a
  fresh Linux clone
- Fixed the Python root derivation on Linux: `PXR_OPENUSD_PYTHON_DIR` is the
  root of the Python installation, but the non-Windows branch took its parent,
  so configure failed looking for headers one directory too high (issue #28)
- Fixed the schema-generation virtual environment on Linux: the interpreter was
  derived as `<python-dir>/python`, but the Linux Python package ships only
  `python3`/`python3.x` in `bin/`

## 4.0.0

- Simplified build infrastructure such that standard tooling is used
  everywhere except pulling down packman packages
- Removed kit-extension and updated README with instructions on
  integrating schema builds with standard kit-app-template extensions
  (note: this entry was inaccurate; `src/kit-extension` remained in the
  tree and was removed in 5.0.0)
- Eliminated repo-tooling based generators

## 3.0.0

- Added several examples for Hydra 2 scene index plugins
- Fixed issue in build plugInfo.json file configuration for debug builds
- Updated dependencies to stock USD 23.05
- Updated openssl and libcurl dependencies

## 2.0.0

- Added support for general USD plugins beyond schemas
- Updated repo_usd to support flexible build files
- Updated dependencies to USD 22.11 and Python 3.10
- Added sample for dynamic payloads and file format plugins

## 1.0.0

- Initial open source release