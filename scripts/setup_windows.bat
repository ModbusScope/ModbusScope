echo on

if "%~1"=="" (set "QT_INSTALL_DIR=D:\aqt_Qt\Qt") else (set "QT_INSTALL_DIR=%~1\Qt")
echo QT_INSTALL_DIR: %QT_INSTALL_DIR%

REM Set configuration
set QT=5.15.2
set QT_MODULES=
set QT_HOST=windows
set QT_TARGET=desktop
set QT_ARCH=win64_mingw81

set QT_ARCH_PATH=mingw81_64

REM Install Qt
aqt install-qt --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% %QT% %QT_ARCH%  -m %QT_MODULES%

REM Install Tools
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_mingw qt.tools.win64_mingw810
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_cmake
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_ninja
aqt install-tool --outputdir %QT_INSTALL_DIR% %QT_HOST% %QT_TARGET% tools_openssl_x64

REM Set env variables with path
set "PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\bin;%PATH%"
set "PATH=%QT_INSTALL_DIR%\Tools\mingw810_64\bin;%PATH%"
set "PATH=%QT_INSTALL_DIR%\Tools\CMake_64\bin;%PATH%"
set "PATH=%QT_INSTALL_DIR%\Tools\Ninja;%PATH%"

set "QT_PLUGIN_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\plugins\"
set "QML_IMPORT_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\qml\"
set "QML2_IMPORT_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\qml\"
set "CMAKE_PREFIX_PATH=%QT_INSTALL_DIR%\%QT%\%QT_ARCH_PATH%\lib\cmake\Qt5"

set "OPENSSL_DIR=%QT_INSTALL_DIR%\Tools\OpenSSL\Win_x64\bin"
