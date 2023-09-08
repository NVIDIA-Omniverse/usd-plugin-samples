:: Copyright 2023 NVIDIA CORPORATION
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

REM options defining what the script runs
set GENERATE=false
set BUILD=false
set CLEAN=false
set CONFIGURE=false
set STAGE=false
set HELP=false
set CONFIG=release
set HELP_EXIT_CODE=0

set DIRECTORIES_TO_CLEAN=_install _build _repo src\usd-plugins\schema\omniExampleSchema\generated src\usd-plugins\schema\omniExampleCodelessSchema\generated src\usd-plugins\schema\omniMetSchema\generated

REM default arguments for script - note this script does not actually perform
REM any build step so that you can integrate the generated code into your build
REM system - an option can be added here to run your build step (e.g. cmake)
REM on the generated files
:parseargs
if not "%1"=="" (
    if "%1" == "--clean" (
        set CLEAN=true
    )
    if "%1" == "--generate" (
        set GENERATE=true
    )
    if "%1" == "--build" (
        set BUILD=true
    )
    if "%1" == "--prep-ov-install" (
        set STAGE=true
    )
    if "%1" == "--configure" (
        set CONFIGURE=true
    )
    if "%1" == "--debug" (
        set CONFIG=debug
    )
    if "%1" == "--help" (
        set HELP=true
    )
    shift
    goto :parseargs
)

if not "%CLEAN%" == "true" (
    if not "%GENERATE%" == "true" (
        if not "%BUILD%" == "true" (
            if not "%STAGE%" == "true" (
                if not "%CONFIGURE%" == "true" (
                    if not "%HELP%" == "true" (
                        REM default action when no arguments are passed is to do everything
                        set GENERATE=true
                        set BUILD=true
                        set STAGE=true
                        set CONFIGURE=true
                    )
                )
            )
        )
    )
)

REM requesting how to run the script
if "%HELP%" == "true" (
    echo build.bat [--clean] [--generate] [--build] [--stage] [--configure] [--debug] [--help]
    echo --clean: Removes the following directories ^(customize as needed^):
    for %%a in (%DIRECTORIES_TO_CLEAN%) DO (
        echo       %%a
    )
    echo --generate: Perform code generation of schema libraries
    echo --build: Perform compilation and installation of USD schema libraries
    echo --prep-ov-install: Preps the kit-extension by copying it to the _install directory and stages the
    echo       built USD schema libraries in the appropriate sub-structure
    echo --configure: Performs a configuration step after you have built and
    echo       staged the schema libraries to ensure the plugInfo.json has the right information
    echo --debug: Performs the steps with a debug configuration instead of release
    echo       ^(default = release^)
    echo --help: Display this help message
    exit %HELP_EXIT_CODE%
)

REM should we clean the target directory?
if "%CLEAN%" == "true" (
    for %%a in (%DIRECTORIES_TO_CLEAN%) DO (
        if exist "%~dp0%%a/" (
            rmdir /s /q "%~dp0%%a"
        )
    )

    if !errorlevel! neq 0 (goto Error)

    goto Success
)

REM should we generate the schema code?
if "%GENERATE%" == "true" (

    REM pull down NVIDIA USD libraries
    REM NOTE: If you have your own local build, you can comment out this step
    call "%~dp0tools\packman\packman.cmd" pull deps/usd-deps.packman.xml -p windows-x86_64 -t config=%CONFIG%

    if !errorlevel! neq 0 ( goto Error )

    REM generate the schema code and plug-in information
    REM NOTE: this will pull the NVIDIA repo_usd package to do this work

    call "%~dp0tools\packman\python.bat" bootstrap.py usd --configuration %CONFIG%

    if !errorlevel! neq 0 ( goto Error )
)

REM should we build the USD schema?

REM NOTE: Modify this build step if using a build system other than cmake (ie, premake)
if "%BUILD%" == "true" (
    REM pull down target-deps to build dynamic payload which relies on CURL
    call "%~dp0tools\packman\packman.cmd" pull deps/target-deps.packman.xml -p windows-x86_64 -t config=%CONFIG%

    REM Below is an example of using CMake to build the generated files
    REM You may also want to explicitly specify the toolset depending on which
    REM version of Visual Studio you are using (e.g. -T v141)
    REM NVIDIA USD 22.11 was built with the v142 toolset, so we set that here
    REM Note that NVIDIA USD 20.08 was build with the v141 toolset
    cmake -B ./_build/cmake -T v142
    cmake --build ./_build/cmake --config=%CONFIG% --target install
)

REM is this a configure only?  This will configure the plugInfo.json files
if "%CONFIGURE%" == "true" (
    call "%~dp0tools\packman\python.bat" bootstrap.py usd --configure-pluginfo --configuration %CONFIG%

    if !errorlevel! neq 0 (goto Error)
)

REM do we need to stage?
if "%STAGE%" == "true" (
    REM Copy the kit-extension into the _install directory
    REM we can do this directly because it's a python extension, so there's nothing to compile
    REM but we want to combine it together with the output of the USD schema extension
    REM Why not build the USD schema extension to the exact structure we are creating here?  
    REM Because we are illustrating that you can build the C++ schemas and distribute them
    REM independent of anything kit related.  All the copy is doing is putting everything in 
    REM one structure that can be referenced as a complete kit extension
    echo D | xcopy "%~dp0src\kit-extension\exts\omni.example.schema" "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema" /s /Y
    if not exist "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleSchema" mkdir "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleSchema"
    echo F | xcopy "%~dp0_install\windows-x86_64\%CONFIG%\omniExampleSchema\OmniExampleSchema\*.*" "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleSchema" /Y
    echo D | xcopy "%~dp0_install\windows-x86_64\%CONFIG%\omniExampleSchema\include" "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleSchema\include" /s /Y
    echo D | xcopy "%~dp0_install\windows-x86_64\%CONFIG%\omniExampleSchema\lib" "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleSchema\lib" /s /Y
    echo D | xcopy "%~dp0_install\windows-x86_64\%CONFIG%\omniExampleSchema\resources" "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleSchema\resources" /s /Y
    if not exist "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleCodelessSchema" mkdir "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleCodelessSchema"
    echo D | xcopy "%~dp0_install\windows-x86_64\%CONFIG%\omniExampleCodelessSchema\resources" "%~dp0_install\windows-x86_64\%CONFIG%\omni.example.schema\OmniExampleCodelessSchema\resources" /s /Y
    
    if !errorlevel! neq 0 ( goto Error )
)

:Success
exit /b 0

:Error
exit /b !errorlevel!