@echo off
setlocal
set "PY="
set "PYARGS="

py -3 -c "import sys" >nul 2>&1 && (set "PY=py" & set "PYARGS=-3")
if not defined PY python -c "import sys" >nul 2>&1 && set "PY=python"

if not defined PY (
    where winget >nul 2>&1 || (
        echo [vital] Python not found and winget unavailable. Install Python 3 from https://www.python.org/downloads/ and re-run.
        exit /b 1
    )
    echo [vital] Python not found, installing via winget ...
    winget install -e --id Python.Python.3.12 --silent --accept-package-agreements --accept-source-agreements
    for /d %%D in ("%LOCALAPPDATA%\Programs\Python\Python3*") do if exist "%%D\python.exe" set "PY=%%D\python.exe"
    for /d %%D in ("%ProgramFiles%\Python3*") do if exist "%%D\python.exe" set "PY=%%D\python.exe"
)
if not defined PY (
    echo [vital] Python install failed. Install Python 3 manually and re-run.
    exit /b 1
)

where git >nul 2>&1 || (
    where winget >nul 2>&1 && (
        echo [vital] Git not found, installing via winget ...
        winget install -e --id Git.Git --silent --accept-package-agreements --accept-source-agreements
        set "PATH=%ProgramFiles%\Git\cmd;%PATH%"
    )
)

"%PY%" %PYARGS% "%~dp0build.py" %*
exit /b %errorlevel%
