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
set CLEAN=false
set CONFIGURE=false
set STAGE=false
set HELP=false

REM default arguments for script - note this script does not actually perform
REM any build step so that you can integrate the generated code into your build
REM system - an option can be added here to run your build step (e.g. cmake)
REM on the generated files
:parseargs
if not "%1"=="" (
    if "%1" == "--generate" (
        set GENERATE=true
    )
    if "%1" == "--clean" (
        set CLEAN=true
    )
    if "%1" == "--configure" (
        set CONFIGURE=true
    )
    if "%1" == "--stage" (
        set STAGE=true
    )
    if "%1" == "--help" (
        set HELP=true
    )
    shift
    goto :parseargs
)

REM requesting how to run the script
if "%HELP%" == "true" (
    echo "build.bat [--generate] [--clean] [--configure] [--stage]"
    echo "--clean: Removes _install directory (customize as needed)"
    echo "--generate: Perform generation of schema libraries"
    echo "--stage: Copies the sample kit-extension to the _install directory and stages the built schema libraries in the appropriate sub-structure"
    echo "--configure: Performs a configuration step when using premake after you have built and staged the schema libraries to ensure the plugInfo.json has the right information"
)

REM should we clean the target directory?
if "%CLEAN%" == "true" (
    rmdir "%~dp0_install"

    if !errorlevel! neq 0 (goto Error)

    goto Success
)

REM should we generate the schema code?
if "%GENERATE%" == "true" (
    REM pull down NVIDIA USD libraries
    REM NOTE: If you have your own local build, you can comment out these steps
    call "%~dp0tools\packman\packman.cmd" pull deps/usd-deps.packman.xml -p windows-x86_64 -t config=debug
    call "%~dp0tools\packman\packman.cmd" pull deps/usd-deps.packman.xml -p windows-x86_64 -t config=release

    if !errorlevel! neq 0 ( goto Error )

    REM generate the schema code and plug-in information
    REM NOTE: this will pull the NVIDIA repo_usd package to do this work
    call "%~dp0tools\packman\python.bat" bootstrap.py usd %*

    if !errorlevel! neq 0 ( goto Error )
)

REM NOTE: this is where you can integrate your own build step if using premake

REM do we need to stage?
if "%STAGE%" == "true" (
    REM Copy the kit-extension into the _install directory
    REM we can do this directly because it's a python extension, so there's nothing to compile
    REM but we want to combine it together with the output of the USD schema extension
    REM Why not build the USD schema extension to the exact structure we are creating here?  
    REM Because we are illustrating that you can build the C++ schemas and distribute them
    REM independent of anything kit related.  All the copy is doing is putting everything in 
    REM one structure that can be referenced as a complete kit extension
    echo D | xcopy "%~dp0src\kit-extension\exts\omni.example.schema" "%~dp0_install\windows-x86_64\omni.example.schema" /s /Y
    if not exist "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleSchema" mkdir "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleSchema"
    move /y "%~dp0_install\windows-x86_64\omniExampleSchema" "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleSchema"
    if not exist "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleCodelessSchema" mkdir "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleCodelessSchema"
    move /y "%~dp0_install\windows-x86_64\omniExampleCodelessSchema" "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleCodelessSchema"

    REM reparent the lib/python directory so we don't have to use [[lib.python.OmniExampleSchema]] in the extension.toml file
    move /y "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleSchema\lib\python\*.*" "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleSchema"
    rmdir "%~dp0_install\windows-x86_64\omni.example.schema\OmniExampleSchema\lib\python"

    if !errorlevel! neq 0 ( goto Error )
)

REM is this a configure only?
REM When using premake, the plugInfo.json files
REM do not get their tokens replaced as premake
REM does not have this functionality built in like cmake
if "%CONFIGURE%" == "true" (
    call "%~dp0tools\packman\python.bat" bootstrap.py usd --configure-plugInfo

    if !errorlevel! neq 0 (goto Error)

    goto Success
)

:Success
exit /b 0

:Error
exit /b %errorlevel%