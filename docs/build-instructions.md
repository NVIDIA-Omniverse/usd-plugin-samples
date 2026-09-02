## Additional Build Instructions

## Using CMake to Generate Schema Code and Build Files

These samples use `cmake` to drive generation of schema code (for codeful schemas) and building of OpenUSD plugins. The `cmake` infrastructure included here is setup to use pre-packaged OpenUSD builds (as well as some additional dependencies required by some of the plugins). The use of `packman` has been enabled in this repository to pull the relevant OpenUSD and Python packages (22.11 and 3.10 respectively) for a turnkey type solution that builds plugins compatible with NVIDIA Omniverse (106+).

The `cmake` build files are structured in the following way:

- The root `CMakeLists.txt` file, responsible for importing NVIDIA's OpenUSD plugin `cmake` helper, setting up the link between the pulled `packman` packages and the rest of the build commands, and the inclusion of the sub-directories containing OpenUSD plugin code.
- The `PackmanDeps.cmake` file, which setups paths to find `cmake` files to include from the `packman` packages that were pulled down.
- The `NvPxrPlugin.cmake` file from `nvopenusdbuildtools`, which contains helper functions for declaring OpenUSD schemas, plugin targets, and Python plugin targets.
- The individual `CMakeLists.txt` files for the OpenUSD plugins, which use the helper functions provided to build the individual plugins.

This works out of the box for both NVIDIA's customized OpenUSD 22.11 Python 3.10 build (i.e., `nv-usd`) and for stock OpenUSD 24.05 Python 3.10 by running the `build.bat`/`build.sh` files. To see all of the options available, run with the `--help` option. These files will:

- pull down the required `packman` packages from NVIDIA's package repositories (via `scripts/setup.py`)
- configure and build using `cmake`

Feel free to customize this sequence as well as the arguments passed to `cmake` as best suits your organization.


### Integrating your own OpenUSD/Python builds

If you would like to integrate your own build of OpenUSD and Python, you must:

- Change the `build.bat`/`build.sh` file such that the `--build-tools-only` option is passed to `scripts/setup.py` (This will ensure no package for OpenUSD or Python is pulled from the NVIDIA package repository, only the build support tools package)
- Edit the value of `PXR_OPENUSD_PYTHON_DIR` in `PackmanDeps.cmake` to point to the directory hosting the Python installation you want to use
- Edit the line in `PackmanDeps.cmake` that appends to the `CMAKE_PREFIX_PATH` – the value should point to your local OpenUSD build. Editing this will direct `cmake` to look for `pxrConfig.cmake` in your local directory rather than the directory of the pulled OpenUSD package from NVIDIA.

