:: Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
::
:: NVIDIA CORPORATION and its licensors retain all intellectual property
:: and proprietary rights in and to this software, related documentation
:: and any modifications thereto.  Any use, reproduction, disclosure or
:: distribution of this software and related documentation without an express
:: license agreement from NVIDIA CORPORATION is strictly prohibited.

@echo off

:: if we are using NVIDIA pre-built binaries, use the prebuilt
:: python to create a virtual environment we can install dependencies
:: to - otherwise it's the devs responsibility to set it up
if not exist %~dp0_venv (
    if exist %~dp0_build\usd-deps\python (
        %~dp0_build\usd-deps\python\python.exe -m venv %~dp0_venv
        call "%~dp0_venv\Scripts\activate.bat"
        pip install PySide2
        pip install PyOpenGL
    )
) else (
    call "%~dp0_venv\Scripts\activate.bat"
)

:: if we are using NVIDIA pre-built binaries, use the prebuilt
:: OpenUSD to set up the paths
:: otherwise its the devs responsibility to add those here
if exist %~dp0_build\usd-deps\usd (
    set PYTHONPATH=%PYTHONPATH%;%~dp0_build\usd-deps\usd\lib\python
    set "PATH=%PATH%;%~dp0_build\usd-deps\usd\bin;%~dp0_build\usd-deps\usd\lib;%~dp0_build\usd-deps\python"
)

:: setup pythonpath and path to the build artifacts to run the samples
set PYTHONPATH=%PYTHONPATH%;%~dp0_install
set "PATH=%PATH%;%~dp0_install\bin;%~dp0_build\target-deps\zlib\lib\rt_dynamic\release"
set PXR_PLUGINPATH_NAME=%~dp0_install\plugins\omniMetSchema\resources;%~dp0_install\plugins\edfFileFormat\resources;%~dp0_install\plugins\omniMetProvider\resources;%~dp0_install\plugins\omniExampleSchema\resources;%~dp0_install\plugins\omniExampleCodelessSchema\resources
