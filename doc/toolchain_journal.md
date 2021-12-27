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

## Build doc

### System wide install
sudo apt install python3-sphinx
sudo apt install python3-recommonmark
sudo apt install python3-sphinx-rtd-theme

### in repo: using venv and requirements file

#### Create/start venv

python3 create_venv.py

[Windows] venv/bin/activate
[Linux]   source venv/bin/activate

#### html

make html

#### pdf

sudo apt install latexmk
sudo apt install texlive-latex-recommended
sudo apt install texlive-latex-extra

make latexpdf

## Build/deploy on Windows

* Add tool directories to `PATH`
  * Build

```
set PATH=C:\Qt\Tools\mingw810_64\bin;%PATH%
set PATH=C:\Qt\Tools\Ninja;%PATH%
set PATH=C:\Qt\Tools\CMake_64\bin;%PATH%
```

* Deploy

  ```
  set PATH=C:\Qt\5.15.2\mingw81_64\bin;%PATH%
  set PATH=C:\Program Files\7-Zip;%PATH%
  ```

* Run `scripts/build_windows.h` to build

* Run `scripts/deploy_windows.h` to create deploy files