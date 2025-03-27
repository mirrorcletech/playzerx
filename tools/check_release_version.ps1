<#
.SYNOPSIS
    Verifies version numbers in PlayzerX.rc file match the specified tag.

.DESCRIPTION
    This script verifies that the version numbers in the PlayzerX.rc file match the 
    specified version tag. It checks all version fields:
      - FILEVERSION (comma-separated)
      - PRODUCTVERSION (comma-separated)
      - FileVersion string (dot-separated)
      - ProductVersion string (dot-separated)

.PARAMETER Version
    Version number with 'v' prefix (e.g., v2.1.0.0)

.PARAMETER RcFile
    Path to the PlayzerX.rc file to check
    
.EXAMPLE
    .\check_release_version.ps1 -Version "v2.1.0.0" -RcFile "PlayzerX.rc"
#>

param (
    [Parameter(Mandatory=$true, Position=0)]
    [string]$Version,
    
    [Parameter(Mandatory=$true, Position=1)]
    [string]$RcFile
)

# Verify version has 'v' prefix
if (-not $Version.StartsWith('v')) {
    Write-Host "Error: Version must start with 'v' prefix (e.g., v2.1.0.0)" -ForegroundColor Red
    Write-Host "  Provided version: $Version" -ForegroundColor Red
    exit 1
}

# Remove 'v' prefix for comparison
$VersionNumber = $Version.Substring(1)
$versionComma = $VersionNumber -replace '\.',','

Write-Host "Checking version numbers in $($RcFile)"
Write-Host "Expected version: $VersionNumber ($versionComma)"
Write-Host ""

# Read the file content
try {
    $fileContent = Get-Content -Path $RcFile -Raw -ErrorAction Stop
}
catch {
    Write-Host "Error: Unable to read file '$RcFile': $_"
    exit 1
}

$hasError = $false

# Function to check version fields
function Test-VersionField {
    param (
        [string]$Pattern,
        [string]$FieldName,
        [string]$ExpectedValue
    )
    
    $match = [regex]::Match($fileContent, $Pattern)
    if ($match.Success) {
        $foundValue = $match.Groups[1].Value
        Write-Host "$FieldName`t$foundValue"
        
        if ($foundValue -ne $ExpectedValue) {
            Write-Host "> Error: Expected $ExpectedValue" -ForegroundColor Red
            return $false
        }
        return $true
    }
    else {
        Write-Host "$FieldName`tNot found" -ForegroundColor Yellow
        Write-Host "> Error: Field not found in RC file" -ForegroundColor Red
        return $false
    }
}

# Define the version fields to check with their patterns
$versionChecks = @(
    @{
        Pattern = 'FILEVERSION\s+([0-9,]+)'; 
        FieldName = 'FILEVERSION:'; 
        ExpectedValue = $versionComma
    },
    @{
        Pattern = 'PRODUCTVERSION\s+([0-9,]+)'; 
        FieldName = 'PRODUCTVERSION:'; 
        ExpectedValue = $versionComma
    },
    @{
        Pattern = 'VALUE\s+"FileVersion",\s+"([0-9\.]+)"'; 
        FieldName = 'FileVersion:'; 
        ExpectedValue = $VersionNumber
    },
    @{
        Pattern = 'VALUE\s+"ProductVersion",\s+"([0-9\.]+)"'; 
        FieldName = 'ProductVersion:'; 
        ExpectedValue = $VersionNumber
    }
)

# Check each version field
foreach ($check in $versionChecks) {
    $result = Test-VersionField -Pattern $check.Pattern -FieldName $check.FieldName -ExpectedValue $check.ExpectedValue
    if (-not $result) {
        $hasError = $true
    }
}

Write-Host ""
if ($hasError) {
    Write-Host "Version verification failed! Please update all version numbers to match." -ForegroundColor Red
    exit 1
}
else {
    Write-Host "Version verification successful! All versions match." -ForegroundColor Green
    exit 0
} 