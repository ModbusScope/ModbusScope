echo on

echo Building ModbusScope using MinGW ...
mkdir release
cd release

echo %PATH%

g++ -v

cmake -G "Ninja" ..
ninja

ctest

cd ..
