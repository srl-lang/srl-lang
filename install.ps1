# SRL Toolchain v0.3.0 Automated PowerShell Installer for Windows
$ErrorActionPreference = "Stop"

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  SRL (Serial Run Language) v0.3.0 Windows Installer" -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $ScriptDir

# 1. Check for CMake
Write-Host "[1/5] Checking build dependencies..." -ForegroundColor Yellow
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake is required to build SRL but was not found in system PATH. Please install CMake."
    exit 1
}

# 2. Build SRL Project
Write-Host "[2/5] Building SRL binaries with CMake..." -ForegroundColor Yellow
try {
    cmake -B build -S .
    cmake --build build --config Release
} catch {
    Write-Host "[Warn] Release build failed, attempting Debug fallback build..." -ForegroundColor Yellow
    cmake --build build --config Debug
}

# Locate compiled executable
$ExePath = ""
if (Test-Path "$ScriptDir\build\Release\srl.exe") {
    $ExePath = "$ScriptDir\build\Release\srl.exe"
} elseif (Test-Path "$ScriptDir\build\Debug\srl.exe") {
    $ExePath = "$ScriptDir\build\Debug\srl.exe"
} elseif (Test-Path "$ScriptDir\build\srl.exe") {
    $ExePath = "$ScriptDir\build\srl.exe"
} elseif (Test-Path "$ScriptDir\srl.exe") {
    $ExePath = "$ScriptDir\srl.exe"
} else {
    Write-Error "Could not locate compiled srl.exe binary."
    exit 1
}

# 3. Create Installation Directory Structure
$InstallDir = Join-Path $HOME ".srl"
$BinDir = Join-Path $InstallDir "bin"
$StdDir = Join-Path $InstallDir "std"
$IncludeDir = Join-Path $InstallDir "include"

Write-Host "[3/5] Creating installation directory structure at '$InstallDir'..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path $BinDir -Force | Out-Null
New-Item -ItemType Directory -Path $StdDir -Force | Out-Null
New-Item -ItemType Directory -Path $IncludeDir -Force | Out-Null

# 4. Copy Toolchain Binaries and Libraries
Write-Host "[4/5] Deploying binaries, standard library, and headers..." -ForegroundColor Yellow
Copy-Item -Path $ExePath -Destination (Join-Path $BinDir "srl.exe") -Force

if (Test-Path "$ScriptDir\srlc.exe") {
    Copy-Item -Path "$ScriptDir\srlc.exe" -Destination (Join-Path $BinDir "srlc.exe") -Force
} elseif (Test-Path "$ScriptDir\build\Release\srlc.exe") {
    Copy-Item -Path "$ScriptDir\build\Release\srlc.exe" -Destination (Join-Path $BinDir "srlc.exe") -Force
} elseif (Test-Path "$ScriptDir\build\Debug\srlc.exe") {
    Copy-Item -Path "$ScriptDir\build\Debug\srlc.exe" -Destination (Join-Path $BinDir "srlc.exe") -Force
}

if (Test-Path "$ScriptDir\std") {
    Copy-Item -Path "$ScriptDir\std\*" -Destination $StdDir -Recurse -Force
}
if (Test-Path "$ScriptDir\include") {
    Copy-Item -Path "$ScriptDir\include\*" -Destination $IncludeDir -Recurse -Force
}

# 5. Add to System PATH
Write-Host "[5/5] Configuring environment PATH variable..." -ForegroundColor Yellow
$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$BinDir*") {
    $NewPath = "$UserPath;$BinDir"
    [Environment]::SetEnvironmentVariable("PATH", $NewPath, "User")
    Write-Host "[OK] Added '$BinDir' to User PATH environment variable." -ForegroundColor Green
} else {
    Write-Host "[OK] '$BinDir' is already in User PATH." -ForegroundColor Green
}

# Update active process PATH
$env:PATH = "$env:PATH;$BinDir"

Write-Host ""
Write-Host "========================================================" -ForegroundColor Green
Write-Host "  SRL v0.3.0 Toolchain Installed Successfully!" -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green
Write-Host "  Installation Location : $InstallDir" -ForegroundColor White
Write-Host "  Executable Location   : $BinDir\srl.exe" -ForegroundColor White
Write-Host ""

# Verification
Write-Host "Verification test:" -ForegroundColor Yellow
& "$BinDir\srl.exe" version
