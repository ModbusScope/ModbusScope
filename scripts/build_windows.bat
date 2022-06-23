echo on

echo Building ModbusScope using MinGW ...
mkdir release
cd release

echo %PATH%

g++ -v

cmake -G "Ninja" ..
IF ERRORLEVEL 1 GOTO errorHandling

ninja
IF ERRORLEVEL 1 GOTO errorHandling

ctest
IF ERRORLEVEL 1 GOTO errorHandling

cd ..

EXIT

:errorHandling
EXIT /B 1
