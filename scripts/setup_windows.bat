echo on

echo Argument1: %~1
echo Argument2: %~2

if "%~1"=="'true'" (set "CACHE_HIT=1") else (set "CACHE_HIT=0")
if "%~2"=="" (set "QT_INSTALL_DIR=D:\aqt_Qt\Qt") else (set "QT_INSTALL_DIR=%~2")
echo QT_INSTALL_DIR: %QT_INSTALL_DIR%
echo CACHE_HIT: %CACHE_HIT%

REM Set configuration
set QT=6.8.3
set QT_MODULES=qtserialbus qtserialport
set QT_HOST=windows
set QT_TARGET=desktop
set QT_ARCH=win64_mingw

set QT_ARCH_PATH=mingw_64

if %CACHE_HIT% EQU 1 (
    echo "Cache hit!"
    goto cache_hit
)

echo "Cache miss: installing Qt"

REM Install Qt
aqt install-qt --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% %QT% %QT_ARCH%  -m %QT_MODULES%

REM Install Tools
REM Qt 6.8.x uses mingw 13.1.0 (GCC 13.1.0) (https://wiki.qt.io/MinGW)
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_mingw1310
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_cmake
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_ninja
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_opensslv3_x64

:cache_hit

REM Debug info
echo Debug info
dir %QT_INSTALL_DIR%\Tools

REM Set env variables with path
set "PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\bin;%PATH%"
set "PATH=%QT_INSTALL_DIR%\Tools\mingw1310_64\bin;%PATH%"
set "PATH=%QT_INSTALL_DIR%\Tools\CMake_64\bin;%PATH%"
set "PATH=%QT_INSTALL_DIR%\Tools\Ninja;%PATH%"

set "QT_PLUGIN_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\plugins\"
set "QML_IMPORT_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\qml\"
set "QML2_IMPORT_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\qml\"
set "CMAKE_PREFIX_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\lib\cmake\Qt6"

set "OPENSSL_DIR=%QT_INSTALL_DIR%\Tools\OpenSSLv3\Win_x64\bin"
