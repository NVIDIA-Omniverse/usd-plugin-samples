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
    set "PATH=%~dp0_install\omniExampleSchema\windows-x86_64_%CONFIG%\lib;%~dp0_build\deps\usd_%CONFIG%\lib;%PATH%"
    call repo.bat test --config %CONFIG%
    if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Run tests']

    if !errorlevel! neq 0 ( goto Error )
)

:: Step 4: Create the repo packages
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