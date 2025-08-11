:: Copyright 2023-2024 NVIDIA CORPORATION
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::    http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

@echo off
setlocal enabledelayedexpansion

pushd %~dp0

:: customize this as needed
:: the _install directory matches what is set for CMAKE_INSTALL_PREFIX
:: and the _build directory matches what we send to cmake below to store the build files
if "%~1" == "--clean" (
    rmdir /s /q _build
    rmdir /s /q _install

    goto :Success
)

:: configure cmake
:: this setup uses the NVIDIA prebuilt OpenUSD binaries that match
:: versions kit was released with as well as the latest OpenUSD release
:: kit 106 - 22.11
:: kit 107 - 24.05
:: kit 108 - 25.02
:: OpenUSD 25.08
:: by default, this setup will use OpenUSD 25.02
:: you can select a different prebuilt binary by passing the version
:: to the cmake variable NV_OPENUSD_BINARY_VERSION
:: you may also use your own OpenUSD build by doing the following:
:: set NV_USE_PREBUILT_OPENUSD_BINARIES to OFF
:: set PXR_OPENUSD_PYTHON_DIR to the path of your Python build
:: add the path to your OpenUSD build to CMAKE_PREFIX_PATH
cmake -B ./_build/cmake -G "Visual Studio 16 2019" -DNV_OPENUSD_BINARY_VERSION=22.11

:: invoke cmake build
cmake --build ./_build/cmake --config=Release --target=install

:Success
exit /b 0

:Error
exit /b !errorlevel!