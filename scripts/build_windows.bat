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

ctest --output-on-failure
IF ERRORLEVEL 1 GOTO errorHandling

cd ..

EXIT /B 0

:errorHandling
EXIT /B 1
