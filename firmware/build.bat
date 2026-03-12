@echo off
setlocal enabledelayedexpansion

:: ─── Arduino-CLI Consistent Build Script ─────────────────────
:: Compiles arduino_main with pinned core and library versions
:: to ensure identical binary output across all team machines.
::
:: Usage:
::   build.bat          - compile only (verify it fits)
::   build.bat upload   - compile + upload to board
:: ──────────────────────────────────────────────────────────────

set CONFIG=--config-file arduino-cli.yaml
set FQBN=arduino:megaavr:uno2018
set SKETCH=arduino_main\arduino_main.ino
set CORE=arduino:megaavr@1.8.8
set LIB=ArduinoBLE@1.5.0
set FLASH_LIMIT=48640

:: ─── Check arduino-cli is installed ──────────────────────────
where arduino-cli >nul 2>nul
if %errorlevel% neq 0 (
    echo.
    echo ERROR: arduino-cli not found on PATH.
    echo.
    echo Install it with one of:
    echo   winget install ArduinoSA.CLI
    echo   https://arduino.github.io/arduino-cli/latest/installation/
    echo.
    echo Then restart your terminal and run this script again.
    exit /b 1
)

echo [1/4] Updating board index...
arduino-cli core update-index %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to update board index.
    exit /b 1
)

echo [2/4] Installing core %CORE%...
arduino-cli core install %CORE% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install core %CORE%.
    exit /b 1
)

echo [3/4] Installing library %LIB%...
arduino-cli lib install %LIB% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install library %LIB%.
    exit /b 1
)

echo [4/4] Compiling %SKETCH%...
echo.
arduino-cli compile --fqbn %FQBN% %SKETCH% %CONFIG% --warnings default 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ============================================
    echo   BUILD FAILED
    echo ============================================
    exit /b 1
)

echo.
echo ============================================
echo   BUILD SUCCEEDED
echo   Flash limit: %FLASH_LIMIT% bytes
echo ============================================

:: ─── Upload if requested ─────────────────────────────────────
if /i not "%~1"=="upload" goto :done

echo.
echo Detecting connected boards...
echo.

set "TMPFILE=%TEMP%\arduino_boards_%RANDOM%.txt"
arduino-cli board list --format text %CONFIG% > "%TMPFILE%" 2>&1

set BOARD_COUNT=0
set HEADER_SKIPPED=0
for /f "usebackq tokens=1,* delims= " %%a in ("%TMPFILE%") do (
    if !HEADER_SKIPPED!==0 (
        set HEADER_SKIPPED=1
    ) else (
        echo %%a | findstr /r "^COM ^/dev" >nul 2>&1
        if !errorlevel!==0 (
            set /a BOARD_COUNT+=1
            set "PORT_!BOARD_COUNT!=%%a"
            set "INFO_!BOARD_COUNT!=%%b"
        )
    )
)

if %BOARD_COUNT%==0 (
    echo   No boards detected.
    echo.
    echo   Make sure your Arduino is:
    echo     - Plugged in via USB
    echo     - Drivers are installed
    echo     - Not in use by another program ^(Arduino IDE, Serial Monitor^)
    del "%TMPFILE%" >nul 2>&1
    exit /b 1
)

if %BOARD_COUNT%==1 (
    set "SELECTED_PORT=!PORT_1!"
    echo   Found board on !SELECTED_PORT!
    echo   !INFO_1!
    goto :upload
)

echo   Detected boards:
echo.
for /l %%i in (1,1,%BOARD_COUNT%) do (
    echo     %%i^) !PORT_%%i! - !INFO_%%i!
)
echo.
set /p CHOICE="  Select board (1-%BOARD_COUNT%): "

set "SELECTED_PORT="
if %CHOICE% geq 1 if %CHOICE% leq %BOARD_COUNT% (
    set "SELECTED_PORT=!PORT_%CHOICE%!"
)

if "%SELECTED_PORT%"=="" (
    echo   Invalid selection.
    del "%TMPFILE%" >nul 2>&1
    exit /b 1
)

:upload
del "%TMPFILE%" >nul 2>&1
echo.
echo Uploading to %SELECTED_PORT%...
arduino-cli upload --fqbn %FQBN% --port %SELECTED_PORT% %SKETCH% %CONFIG% 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ============================================
    echo   UPLOAD FAILED
    echo ============================================
    echo Make sure the board is plugged in and not
    echo in use by another program.
    exit /b 1
)
echo.
echo ============================================
echo   UPLOAD SUCCEEDED  [%SELECTED_PORT%]
echo ============================================

:done
exit /b 0
