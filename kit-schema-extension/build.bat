:: Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
::
:: NVIDIA CORPORATION and its licensors retain all intellectual property
:: and proprietary rights in and to this software, related documentation
:: and any modifications thereto.  Any use, reproduction, disclosure or
:: distribution of this software and related documentation without an express
:: license agreement from NVIDIA CORPORATION is strictly prohibited.

@echo off
setlocal

pushd %~dp0

:: default config is release
set CONFIG=release
set PACKAGE=false

:parseargs
if not "%1"=="" (
    if "%1" == "debug" (
        set CONFIG=debug
    )
    if "%1" == "--package" (
        set PACKAGE=true
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

:: Step 1: TODO: Need to test the kit extensions here
:: (not sure if we have to do it pre-copy or post-copy yet)

:: Step 2: Aggregate everything under the _install directory
if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Copying schema extension with kit extension']
if not exist _install mkdir _install
if not exist _install\windows-x86_64_%CONFIG% mkdir _install\windows-x86_64_%CONFIG%
if not exist _install\windows-x86_64_%CONFIG%\omni.example.schema mkdir _install\windows-x86_64_%CONFIG%\omni.example.schema
if not exist _install\windows-x86_64_%CONFIG%\omni.example.schema\lib mkdir _install\windows-x86_64_%CONFIG%\omni.example.schema\lib
if not exist _install\windows-x86_64_%CONFIG%\omni.example.schema\OmniExampleSchema mkdir _install\windows-x86_64_%CONFIG%\omni.example.schema\OmniExampleSchema
echo F | xcopy src\omni.example.schema\PACKAGE-INFO.yaml _install\windows-x86_64_%CONFIG%\omni.example.schema\PACKAGE-INFO.yaml /Y
echo F | xcopy ..\usd-schema-extension\_install\omniExampleSchema\windows-x86_64_%CONFIG%\lib\omniExampleSchema.dll _install\windows-x86_64_%CONFIG%\omni.example.schema\lib\omniExampleSchema.dll /Y
echo F | xcopy ..\usd-schema-extension\_install\omniExampleSchema\windows-x86_64_%CONFIG%\lib\python\OmniExampleSchema\__init__.py _install\windows-x86_64_%CONFIG%\omni.example.schema\OmniExampleSchema\__init__.py /Y
echo F | xcopy ..\usd-schema-extension\_install\omniExampleSchema\windows-x86_64_%CONFIG%\lib\python\OmniExampleSchema\_omniExampleSchema.pyd _install\windows-x86_64_%CONFIG%\omni.example.schema\OmniExampleSchema\_omniExampleSchema.pyd /Y
echo D | xcopy ..\usd-schema-extension\_install\omniExampleSchema\windows-x86_64_%CONFIG%\usd _install\windows-x86_64_%CONFIG%\omni.example.schema\usd /Y /s
echo D | xcopy src\omni.example.schema\config _install\windows-x86_64_%CONFIG%\omni.example.schema\config /Y /s
echo D | xcopy src\omni.example.schema\omni _install\windows-x86_64_%CONFIG%\omni.example.schema\omni /Y /s

if not exist _install\windows-x86_64_%CONFIG%\omni.example.codeless.schema mkdir _install\windows-x86_64_%CONFIG%\omni.example.codeless.schema
echo F | xcopy src\omni.example.codeless.schema\PACKAGE-INFO.yaml _install\windows-x86_64_%CONFIG%\omni.example.codeless.schema\PACKAGE-INFO.yaml /Y
echo D | xcopy src\omni.example.codeless.schema\config _install\windows-x86_64_%CONFIG%\omni.example.codeless.schema\config /Y /s
echo D | xcopy src\omni.example.codeless.schema\omni _install\windows-x86_64_%CONFIG%\omni.example.codeless.schema\omni /Y /s
echo D | xcopy ..\usd-schema-extension\_install\omniExampleCodelessSchema\usd _install\windows-x86_64_%CONFIG%\omni.example.codeless.schema\usd /Y /s

if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Copying schema extension with kit extension']

if %errorlevel% neq 0 ( goto Error )

:: Step 3: Package if the option was set
if "%PACKAGE%" == "true" (
    if defined TEAMCITY_VERSION echo ##teamcity[blockOpened name='Create packages']
    call repo.bat package --mode omni-example-schema --platform-target windows-x86_64 --root . --config %CONFIG%
    call repo.bat package --mode omni-example-codeless-schema --platform-target windows-x86_64 --root . --config %CONFIG%
    if defined TEAMCITY_VERSION echo ##teamcity[blockClosed name='Create packages']

    if %errorlevel% neq 0 ( goto Error )
)

:Success
exit /b 0

:Error
exit /b %errorlevel%