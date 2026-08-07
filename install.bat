@echo off
setlocal
echo ========================================================
echo   Launching SRL v0.3.2 Windows Installer...
echo ========================================================
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1"
if %ERRORLEVEL% NEQ 0 (
    echo [Error] SRL Installation encountered an error.
    pause
    exit /b %ERRORLEVEL%
)
endlocal
