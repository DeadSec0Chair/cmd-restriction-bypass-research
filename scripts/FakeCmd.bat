
@echo off
TITLE "Command Prompt"
REM The quotes here are intentional. This is the "Command Prompt", not the actual CMD.

if "%~1" neq "" (
	if exist "%~1\" cd /d "%~1"
)

:start
set "cmd2exec="
set /p cmd2exec= %cd%^>
if "%cmd2exec%" neq "" call %cmd2exec%
echo.
goto start


