@echo off
setlocal enabledelayedexpansion
rem ============================================================================
rem  NucEngine - build and run
rem  Double-click this file (or run it from a terminal) to compile the engine
rem  and launch it. Optional argument: "release" or "debug" (default: debug).
rem
rem  It locates MSBuild and the newest installed Windows SDK automatically, and
rem  builds with the v143 toolset (Visual Studio 2022). NOMINMAX and
rem  GLM_FORCE_CTOR_INIT are set in the project, so no extra flags are needed.
rem ============================================================================

rem Work from the repository root (this script's folder) so asset paths resolve.
cd /d "%~dp0"

rem --- Configuration -----------------------------------------------------------
set "CFG=Debug"
if /i "%~1"=="release" set "CFG=Release"
if /i "%~1"=="debug"   set "CFG=Debug"
set "TOOLSET=v143"

echo(
echo === NucEngine : build %CFG% ^| x64 ===
echo(

rem --- Locate MSBuild via vswhere ---------------------------------------------
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo ERROR: vswhere.exe not found.
	echo Install Visual Studio 2019/2022 with the "Desktop development with C++" workload.
	goto :fail
)
set "MSBUILD="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"
if not defined MSBUILD (
	echo ERROR: MSBuild.exe not found through vswhere.
	goto :fail
)
echo MSBuild : !MSBUILD!

rem --- Detect the newest installed Windows 10/11 SDK ---------------------------
set "SDK="
set "SDKINC=%ProgramFiles(x86)%\Windows Kits\10\Include"
if exist "%SDKINC%" (
	for /f "usebackq tokens=*" %%d in (`dir /b /ad /o-n "%SDKINC%" 2^>nul`) do (
		if not defined SDK if exist "%SDKINC%\%%d\um\windows.h" set "SDK=%%d"
	)
)
if defined SDK (echo SDK     : !SDK!) else (echo SDK     : ^(using project default^))

rem --- Build -------------------------------------------------------------------
echo(
set "SDKARG="
if defined SDK set "SDKARG=/p:WindowsTargetPlatformVersion=!SDK!"
"!MSBUILD!" NucEngine.sln /nologo /m /v:minimal /p:Configuration=%CFG% /p:Platform=x64 /p:PlatformToolset=%TOOLSET% !SDKARG!
if errorlevel 1 (
	echo(
	echo *** BUILD FAILED ***
	goto :fail
)

rem --- Run ---------------------------------------------------------------------
set "EXE=x64\%CFG%\NucEngine.exe"
if not exist "%EXE%" (
	echo ERROR: build reported success but "%EXE%" was not found.
	goto :fail
)
echo(
echo Build OK. Launching %EXE% ...
start "NucEngine" "%EXE%"
endlocal
exit /b 0

:fail
echo(
pause
endlocal
exit /b 1
