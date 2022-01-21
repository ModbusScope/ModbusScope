echo on

echo Building ModbusScope using MinGW ...
mkdir release
cd release

set PATH="%IQTA_TOOLS%\mingw810_64\bin";%PATH%

echo %PATH%

g++ -v

cmake -G "Ninja" ..
ninja

ctest

