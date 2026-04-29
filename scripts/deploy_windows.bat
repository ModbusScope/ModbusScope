echo on

echo "Deploy ModbusScope ..."

g++ -v

cd release\src\bin\win
IF ERRORLEVEL 1 GOTO errorHandling

set DEPLOY_DIR=deploy
set PATH=C:\Program Files (x86)\Inno Setup 6;%PATH%

mkdir %DEPLOY_DIR%
copy modbusscope.exe %DEPLOY_DIR%
IF ERRORLEVEL 1 GOTO errorHandling

REM Copy adapter binaries into the flat deployment directory
copy %~dp0..\adapters\modbusadapter.exe %DEPLOY_DIR%
IF ERRORLEVEL 1 GOTO errorHandling

copy %~dp0..\adapters\dummymodbusadapter.exe %DEPLOY_DIR%
IF ERRORLEVEL 1 GOTO errorHandling

cd %DEPLOY_DIR%

REM Run windeployqt on all three executables. Because they share the same Qt
REM version and output directory, DLLs are written once and cover all binaries.
windeployqt.exe modbusscope.exe --compiler-runtime -verbose 2
IF ERRORLEVEL 1 GOTO errorHandling

windeployqt.exe modbusadapter.exe -verbose 2
IF ERRORLEVEL 1 GOTO errorHandling

windeployqt.exe dummymodbusadapter.exe -verbose 2
IF ERRORLEVEL 1 GOTO errorHandling

REM Add OpenSSL dll's
echo "OPENSSL_DIR: %OPENSSL_DIR%"

xcopy "%OPENSSL_DIR%\libcrypto-3-x64.dll" .
IF ERRORLEVEL 1 GOTO errorHandling

xcopy "%OPENSSL_DIR%\libssl-3-x64.dll" .
IF ERRORLEVEL 1 GOTO errorHandling

cd ..
7z a ModbusScope.zip ".\%DEPLOY_DIR%\*"
IF ERRORLEVEL 1 GOTO errorHandling

cd ..\..\..\..
REM https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/xcopy
xcopy release\src\bin\win\%DEPLOY_DIR% installer\deploy /e /i /y /s
IF ERRORLEVEL 1 GOTO errorHandling

compil32 /cc "installer\modbusscope_installer.iss"
IF ERRORLEVEL 1 GOTO errorHandling

move release\src\bin\win\ModbusScope.zip ModbusScope.zip
move installer\Output\ModbusScope_setup.exe ModbusScope_setup.exe

EXIT

:errorHandling
EXIT /B 1
