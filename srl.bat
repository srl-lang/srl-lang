@echo off
set SCRIPT_DIR=%~dp0
if exist "%SCRIPT_DIR%build\Release\srl.exe" (
    "%SCRIPT_DIR%build\Release\srl.exe" %*
) else if exist "%SCRIPT_DIR%build\Debug\srl.exe" (
    "%SCRIPT_DIR%build\Debug\srl.exe" %*
) else if exist "%SCRIPT_DIR%build\srl.exe" (
    "%SCRIPT_DIR%build\srl.exe" %*
) else (
    echo [SRL Error] SRL engine executable not found. Please build using cmake first.
    exit /b 1
)
