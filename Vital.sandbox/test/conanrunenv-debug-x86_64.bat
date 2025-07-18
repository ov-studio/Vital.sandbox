@echo off
chcp 65001 > nul
setlocal
echo @echo off > "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
echo echo Restoring environment >> "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
for %%v in (OPENSSL_MODULES) do (
    set foundenvvar=
    for /f "delims== tokens=1,2" %%a in ('set') do (
        if /I "%%a" == "%%v" (
            echo set "%%a=%%b">> "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
            set foundenvvar=1
        )
    )
    if not defined foundenvvar (
        echo set %%v=>> "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
    )
)
endlocal


set "OPENSSL_MODULES=C:\Users\Tron\.conan2\p\b\opensf6410d345e0fa\p\lib\ossl-modules"