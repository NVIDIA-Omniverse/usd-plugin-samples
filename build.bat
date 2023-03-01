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
    if "%1" == "--stage" (
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

REM requesting how to run the script
if "%HELP%" == "true" (
    echo build.bat [--clean] [--generate] [--build] [--stage] [--configure] [--debug] [--help]
    echo --clean: Removes _install/_build/_repo directory ^(customize as needed^)
    echo --generate: Perform code generation of schema libraries
    echo --build: Perform compilation and installation of USD schema libraries
    echo --stage: Preps the kit-extension by copying it to the _install directory and stages the
    echo       built USD schema libraries in the appropriate sub-structure
    echo --configure: Performs a configuration step when using premake after you have built and
    echo       staged the schema libraries to ensure the plugInfo.json has the right information
    echo --debug: Performs the steps with a debug configuration instead of release
    echo       ^(default = release^)
    echo --help: Display this help message
)

REM should we clean the target directory?
if "%CLEAN%" == "true" (
    rmdir /s /q "%~dp0_install"
    rmdir /s /q "%~dp0_build"
    rmdir /s /q "%~dp0_repo"

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

    call "%~dp0tools\packman\python.bat" bootstrap.py usd

    if !errorlevel! neq 0 ( goto Error )
)

REM NOTE: this is where you can integrate your own build step if using premake
REM Below is an example of using CMake to build the generated files
REM You may also want to explicitly specify the toolset depending on which
REM version of Visual Studio you are using (e.g. -T v141)
if "%BUILD%" == "true" (
    cmake -B ./_build/cmake
    cmake --build ./_build/cmake --config=%CONFIG% --target install
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
exit /b !errorlevel!