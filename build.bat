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

where git >nul 2>&1 && goto git_ok
if exist "%ProgramFiles%\Git\cmd\git.exe" set "PATH=%ProgramFiles%\Git\cmd;%PATH%" & goto git_ok
if exist "%LOCALAPPDATA%\Programs\Git\cmd\git.exe" set "PATH=%LOCALAPPDATA%\Programs\Git\cmd;%PATH%" & goto git_ok
where winget >nul 2>&1 || goto git_ok
echo [vital] Git not found, installing via winget ...
winget install -e --id Git.Git --silent --accept-package-agreements --accept-source-agreements
set "PATH=%ProgramFiles%\Git\cmd;%PATH%"
:git_ok

where cmake >nul 2>&1 && goto cmake_ok
if exist "%ProgramFiles%\CMake\bin\cmake.exe" set "PATH=%ProgramFiles%\CMake\bin;%PATH%" & goto cmake_ok
where winget >nul 2>&1 || goto cmake_ok
echo [vital] CMake not found, installing via winget ...
winget install -e --id Kitware.CMake --silent --accept-package-agreements --accept-source-agreements
set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
:cmake_ok

"%PY%" %PYARGS% "%~dp0build.py" %*
exit /b %errorlevel%
