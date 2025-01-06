@echo off
setlocal enabledelayedexpansion

:: Path to clang-format executable
set CLANG_FORMAT=clang-format

:: Check if clang-format is available
where %CLANG_FORMAT% >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: clang-format not found. Please ensure it's installed and in your PATH.
    exit /b 1
)

:: Format files in root directory (non-recursive)
:: Best to exclude MTISerial.cpp and enumCOMs.cpp to prevent porting issues 
:: from our master C++ codebase
echo Formatting root directory files...
for %%F in (*.cpp *.h) do (
    if not "%%F"=="MTISerial.cpp" if not "%%F"=="enumCOMs.cpp" (
        echo Processing %%F
        %CLANG_FORMAT% -i -style=file "%%F"
    )
)

:: Format demo_source directory files (non-recursive)
echo Formatting demo_source directory files...
for %%F in (demo_source\*.cpp demo_source\*.h) do (
    echo Processing %%F
    %CLANG_FORMAT% -i -style=file "%%F"
)

:: Format include directory files (non-recursive)
echo Formatting include directory files...
for %%F in (include\*.cpp include\*.h) do (
    echo Processing %%F
    %CLANG_FORMAT% -i -style=file "%%F"
)

echo Formatting complete.
