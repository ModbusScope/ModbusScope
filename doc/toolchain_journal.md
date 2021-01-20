# Toolchain journal

## Mocking in Qt Test

#### Sources

http://rodolfotech.blogspot.com/2017/01/qtest-google-mock.html

https://chromium.googlesource.com/external/github.com/google/googletest/+/release-1.8.0/googletest/docs/AdvancedGuide.md#extending-google-test-by-handling-test-events

https://github.com/google/googletest/tree/master/googlemock/docs

https://stackoverflow.com/questions/55384422/how-to-use-google-mock-with-cppunittestframework

## Build on Linux without QtCreator

```bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=/opt/Qt/5.15.1/gcc_64/lib/cmake/Qt5 -GNinja ..
ninja
```