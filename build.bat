@echo off
echo Building ESP32-S2 project...
echo.

REM Try different ways to run PlatformIO
python -m platformio run
if %errorlevel% equ 0 goto :end

python3 -m platformio run
if %errorlevel% equ 0 goto :end

if exist "%USERPROFILE%\.platformio\penv\Scripts\platformio.exe" (
    "%USERPROFILE%\.platformio\penv\Scripts\platformio.exe" run
    goto :end
)

pio run
if %errorlevel% equ 0 goto :end

platformio run
if %errorlevel% equ 0 goto :end

echo.
echo Error: PlatformIO not found!
echo Please install PlatformIO:
echo   1. Run: pip install platformio
echo   2. Or install VS Code extension: PlatformIO IDE
echo   3. Or install from: https://platformio.org/install/cli
echo.
pause
:end

