@echo off
set SCRIPT_DIR=%~dp0
if exist "%SCRIPT_DIR%srlc\build\Release\srlc.exe" (
    "%SCRIPT_DIR%srlc\build\Release\srlc.exe" %*
) else if exist "%SCRIPT_DIR%srlc\build\srlc.exe" (
    "%SCRIPT_DIR%srlc\build\srlc.exe" %*
) else (
    echo [SRLC Error] SRL compiler executable not found. Please build using cmake first.
    exit /b 1
)
