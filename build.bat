:: Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
::
:: NVIDIA CORPORATION and its licensors retain all intellectual property
:: and proprietary rights in and to this software, related documentation
:: and any modifications thereto.  Any use, reproduction, disclosure or
:: distribution of this software and related documentation without an express
:: license agreement from NVIDIA CORPORATION is strictly prohibited.

@echo off
setlocal

:: build usd-schema-extensions first
call usd-schema-extension\build.bat %*

if %errorlevel% neq 0 ( goto Error )

:: build the kit-schema-extensions
call kit-schema-extension\build.bat %*

if %errorlevel% neq 0 ( goto Error )

call repo.bat docs --warn-as-error=0

if %errorlevel% neq 0 ( goto Error )

:Success
exit /b 0

:Error
exit /b %errorlevel%