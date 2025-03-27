@echo off
setlocal
REM ============================================================================
REM Windows Build and Package Script for PlayzerX
REM ============================================================================
REM
REM This script builds the PlayzerX project in Release x64 configuration, 
REM creates a clean delivery directory with only the required files, and 
REM packages them into a zip archive.
REM
REM Usage:
REM   .\tools\windows_build_repo.bat [VERSION]
REM
REM Parameters:
REM   VERSION    (Optional) Version string to include in the zip filename.
REM              If not provided, "dev" will be used as default.
REM
REM Example:
REM   .\tools\windows_build_repo.bat v2.1.0.0
REM   
REM   This will build the project and create:
REM   PlayzerX-v2.1.0.0-Windows-x86_64.zip
REM
REM Output:
REM   - PlayzerX-[VERSION]-Windows-x86_64.zip containing:
REM     - exe/PlayzerX-Demo.exe
REM     - exe/PlayzerX64.lib
REM     - exe/PlayzerX64.dll
REM     - exe/butterfly.smp
REM ============================================================================

REM Get version from first argument or use "dev" as default
set VERSION=%1
if "%VERSION%"=="" set VERSION=dev

echo Building PlayzerX (Release x64)...
call msbuild PlayzerX-All.sln /p:Configuration=Release /p:Platform=x64

echo Creating delivery directory...
if exist delivery rmdir /s /q delivery
mkdir delivery\exe

echo Copying required files...
copy x64\Release\PlayzerX-Demo.exe delivery\exe\
copy x64\Release\PlayzerX64.lib delivery\exe\
copy x64\Release\PlayzerX64.dll delivery\exe\
copy butterfly.smp delivery\exe\

echo Creating ZIP archive...
powershell -Command "Compress-Archive -Path .\delivery\* -DestinationPath .\PlayzerX-%VERSION%-Windows-x86_64.zip"

echo Delivery package prepared successfully. 