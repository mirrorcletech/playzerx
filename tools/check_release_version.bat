@echo off
:: ============================================================================
:: PlayzerX Version Verification Script
:: ============================================================================
:: This script is a wrapper for the PowerShell version checker script.
:: It verifies that version numbers in PlayzerX.rc match the specified tag.
::
:: Usage:
::   check_release_version.bat <version> <rc_file_path>
:: 
:: Example: 
::   .\tools\check_release_version.bat v2.1.0.0 PlayzerX.rc
::
:: Parameters:
::   version     - Version number WITH 'v' prefix (e.g., v2.1.0.0)
::   rc_file_path - Path to the PlayzerX.rc file to check
::
:: See check_release_version.ps1 for more details
:: ============================================================================

powershell -ExecutionPolicy Bypass -File "%~dp0check_release_version.ps1" %*
exit /b %ERRORLEVEL%