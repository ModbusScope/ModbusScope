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
```bash
sudo apt install python3-sphinx
sudo apt install python3-recommonmark
sudo apt install python3-sphinx-rtd-theme
```

### In repo: using venv and requirements file

#### Create/start venv

```
python3 create_venv.py

[Windows] venv/bin/activate
[Linux]   source venv/bin/activate
```

#### html

```
make html
```

#### pdf

```
sudo apt install latexmk
sudo apt install texlive-latex-recommended
sudo apt install texlive-latex-extra

make latexpdf
```

## Build/deploy scripts

### aqtinstall

* Sources:
  * https://github.com/miurahr/aqtinstall
  * https://aqtinstall.readthedocs.io/en/stable/

* List Qt versions (mingw)

  * ```
    aqt list-qt windows desktop
    ```

* List Qt arch for specific Qt versions

  * ```
    aqt list-qt windows desktop --arch 6.2.0
    ```

* List tools

  * ```
    aqt list-tool windows desktop
    ```

* List compilers

  * ```
    aqt list-tool windows desktop tools_mingw
    ```

### Linux (Docker env)

Track down missing dependencies when error shows libqxcb.so

```
ldd /opt/qt/5.15.2/gcc_64/plugins/platforms/libqxcb.so  | grep "not found"
```

### Windows

* Dependencies for aqtinstall
  * Visual Studio C++ 14: Download from https://visualstudio.microsoft.com/visual-cpp-build-tools/
  * pip needs to be upgraded to latest version
* Install aqtinstall: `pip3 install aqtinstall`
