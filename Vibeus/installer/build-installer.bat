@echo off
REM Quick Installer Build Script for Vibeus
REM Requires: Inno Setup 6.x installed
REM Usage: Double-click after building Release

setlocal EnableDelayedExpansion
pushd %~dp0

echo ========================================
echo Vibeus Installer Builder
echo ========================================
echo.

REM Resolve Inno Setup compiler
set "ISCC_EXE="
if defined INNO_PATH if exist "%INNO_PATH%" set "ISCC_EXE=%INNO_PATH%"

if not defined ISCC_EXE (
    for %%P in ("%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" "%ProgramFiles%\Inno Setup 6\ISCC.exe") do (
        if exist %%~fP set "ISCC_EXE=%%~fP"
    )
)

if not defined ISCC_EXE goto :INNO_MISSING

REM Check if build output exists
if not exist "..\build\Release\Vibeus.exe" goto :BUILD_MISSING

echo [1/3] Inno Setup compiler: %ISCC_EXE%
echo [2/3] Found build output
echo [3/3] Compiling installer...
echo.

"%ISCC_EXE%" "vibeus-installer.iss"
if %ERRORLEVEL% neq 0 goto :COMPILE_FAILED

echo.
echo ========================================
echo SUCCESS! Installer created:
echo .\output\Vibeus-*-setup.exe
echo ========================================
echo.
echo You can now distribute this installer!
echo.
popd
pause
exit /b 0

:COMPILE_FAILED
echo.
echo ERROR: Installer compilation failed!
echo Check the output above for details.
popd
pause
exit /b 1

:INNO_MISSING
echo ERROR: Inno Setup compiler not found.
echo Tried INNO_PATH and common install locations.
echo Install from https://jrsoftware.org/isdl.php or set INNO_PATH to ISCC.exe
popd
pause
exit /b 1

:BUILD_MISSING
echo ERROR: Build output not found!
echo Please build your Release configuration first.
echo Expected: ..\build\Release\Vibeus.exe
popd
pause
exit /b 1
