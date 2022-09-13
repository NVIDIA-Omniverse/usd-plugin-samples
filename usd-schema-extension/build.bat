:: Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
::
:: NVIDIA CORPORATION and its licensors retain all intellectual property
:: and proprietary rights in and to this software, related documentation
:: and any modifications thereto.  Any use, reproduction, disclosure or
:: distribution of this software and related documentation without an express
:: license agreement from NVIDIA CORPORATION is strictly prohibited.

@echo off
setlocal enabledelayedexpansion

pushd %~dp0

:: default config is release
set CONFIG=release
set PACKAGE=false
set SKIPTEST=false

:parseargs
if not "%1"=="" (
    if "%1" == "debug" (
        set CONFIG=debug
    )
    if "%1" == "--package" (
        set PACKAGE=true
    )
    if "%1" == "--skip-test" (
        set SKIPTEST=true
    )
    shift
    goto :parseargs
)

echo Building %CONFIG% configuration

if "%CONFIG%" == "debug" (
    set BUILD_OPTION=debug-only
)

if "%CONFIG%" == "release" (
    set BUILD_OPTION=release-only
)

:: Step 1: Run usdgenschema (note this is the same whether configuration is debug or release)
if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Generate schema']
call repo.bat usdgenschema
if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Generate schema']

if !errorlevel! neq 0 ( goto Error )

:: Step 2: Build C++ / Python libraries as needed
:: Note: we wouldn't perform this step if the schema is codeless
if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Build libraries']
call repo.bat build --%BUILD_OPTION%
if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Build libraries']

if !errorlevel! neq 0 ( goto Error )

:: Step 3: Test schema 
if "%SKIPTEST%" EQU "false" (
    if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Run tests']
    set "PATH=%~dp0_install\omniExampleSchema\windows-x86_64_%CONFIG%\lib;%~dp0_repo\deps\repo_usdgenschema\_build\deps\usd_py37_%CONFIG%\lib;%PATH%"
    call repo.bat test --config %CONFIG%
    if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Run tests']

    if !errorlevel! neq 0 ( goto Error )
)

:: Step 4: Gather up license dependencies
if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Gather licenses']
call repo.bat licensing gather -d src/omniExampleSchema -p _repo/deps/repo_usdgenschema/deps/usd-deps.packman.xml --platform windows-x86_64 --config %CONFIG%
call repo.bat licensing gather -d src/omniExampleCodelessSchema -p _repo/deps/repo_usdgenschema/deps/usd-deps.packman.xml --platform windows-x86_64 --config %CONFIG%
if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Gather licenses']

if !errorlevel! neq 0 ( goto Error )

:: the license file should be generated in a local _build directory under where the PACKAGE-INFO.yaml file is
:: need to copy it to the _install location
:: NOTE - as of now, repo_licensing doesn't seem to gather up the USD license dependencies
:: if this is ever fixed, those should also be copied to _install
:: NOTE - this is a little wasteful because for every configuration we will actually create the codeless schema package 
:: many times, but it's always the same package since there's no code so the configuration doesn't matter
:: we do both debug and release of this thing so that packman configuration doesn't need special cases to only pull one version
if not exist _install\omniExampleSchema\windows-x86_64_%CONFIG%\PACKAGE-LICENSES mkdir _install\omniExampleSchema\windows-x86_64_%CONFIG%\PACKAGE-LICENSES
if not exist _install\omniExampleCodelessSchema\PACKAGE-LICENSES mkdir _install\omniExampleCodelessSchema\PACKAGE-LICENSES
echo F | xcopy src\omniExampleSchema\_build\PACKAGE-LICENSES\omni-example-schema-LICENSE.txt _install\omniExampleSchema\windows-x86_64_%CONFIG%\PACKAGE-LICENSES\omni-example-schema-LICENSE.txt /Y
echo F | xcopy src\omniExampleCodelessSchema\_build\PACKAGE-LICENSES\omni-example-codeless-schema-LICENSE.txt _install\omniExampleCodelessSchema\PACKAGE-LICENSES\omni-example-codeless-schema-LICENSE.txt /Y

if !errorlevel! neq 0 ( goto Error )

:: Step 5: Create the repo packages
if "%PACKAGE%" == "true" (
    if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Create packages']
    call repo.bat package --mode omni-example-schema --platform-target windows-x86_64 --root . --config %CONFIG%
    call repo.bat package --mode omni-example-codeless-schema --platform-target windows-x86_64 --root . --config %CONFIG%
    if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Create packages']

    if !errorlevel! neq 0 ( goto Error )
)

:Success
exit /b 0

:Error
exit /b !errorlevel!