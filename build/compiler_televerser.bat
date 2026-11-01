@echo off
REM ============================================================
REM  Script de compilation / televersement - NUCLEO-L476RG
REM  Double-cliquer dessus, carte branchee en USB.
REM ============================================================
set PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe
cd /d "%~dp0"

echo === Compilation ===
"%PIO%" run
if errorlevel 1 goto echec

echo.
echo === Televersement ===
"%PIO%" run --target upload
if errorlevel 1 goto echec

echo.
echo === Console serie 115200 bauds (Ctrl+C pour quitter) ===
"%PIO%" device monitor
exit /b 0

:echec
echo.
echo ECHEC : voir le message ci-dessus.
pause
exit /b 1
