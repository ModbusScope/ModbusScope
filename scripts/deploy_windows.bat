echo on

echo "Deploy ModbusScope ..."

cd release\src\bin\win
IF ERRORLEVEL 1 GOTO errorHandling

set DEPLOY_DIR=deploy
set PATH=C:\Program Files (x86)\Inno Setup 6;%PATH%

mkdir %DEPLOY_DIR%
copy modbusscope.exe %DEPLOY_DIR%
IF ERRORLEVEL 1 GOTO errorHandling

cd %DEPLOY_DIR%
windeployqt.exe modbusscope.exe --compiler-runtime -verbose 2
IF ERRORLEVEL 1 GOTO errorHandling

REM Add OpenSSL dll's
echo "IQTA_TOOLS: %IQTA_TOOLS%"

IF DEFINED IQTA_TOOLS (
    set "OPENSSL_DIR=%IQTA_TOOLS%\OpenSSL\Win_x64\bin"
) ELSE (
    set "OPENSSL_DIR=C:\Qt\Tools\OpenSSL\Win_x64\bin"
)

echo "OPENSSL_DIR: %OPENSSL_DIR%"

xcopy "%OPENSSL_DIR%\libcrypto-1_1-x64.dll" .
IF ERRORLEVEL 1 GOTO errorHandling

xcopy "%OPENSSL_DIR%\libssl-1_1-x64.dll" .
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

EXIT

:errorHandling
EXIT /B 1