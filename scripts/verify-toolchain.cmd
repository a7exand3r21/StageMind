@echo off
setlocal EnableExtensions

set "DEV_CMD=%~dp0dev-cmd.cmd"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSINSTALL=%%I"
)

set "VSNINJA=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

echo Checking MSVC...
call "%DEV_CMD%" cl
if errorlevel 1 exit /b %errorlevel%

echo Checking CMake...
call "%DEV_CMD%" cmake --version
if errorlevel 1 exit /b %errorlevel%

echo Checking MSBuild...
call "%DEV_CMD%" msbuild -version -nologo
if errorlevel 1 exit /b %errorlevel%

echo Checking Ninja...
call "%DEV_CMD%" "%VSNINJA%" --version
if errorlevel 1 exit /b %errorlevel%

echo Checking Windows SDK tools...
call "%DEV_CMD%" where rc
if errorlevel 1 exit /b %errorlevel%
call "%DEV_CMD%" where mt
if errorlevel 1 exit /b %errorlevel%

echo Toolchain OK.
