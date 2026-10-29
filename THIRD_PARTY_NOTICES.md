# Third-Party Notices

This file lists the software shipped in the ModbusScope release packages (Windows zip, Windows
installer and Linux AppImage), with its license. The license texts are in the `licenses/` folder
next to this file. In the AppImage, they are in `usr/share/doc/modbusscope/`.

## ModbusScope

- License: GNU General Public License v3.0, see `LICENSE`
- Source: <https://github.com/ModbusScope/ModbusScope>, the tag matching the release version

## Qt

- Version: 6.8.3
- Copyright: The Qt Company Ltd. and other contributors
- License: GNU Lesser General Public License v3.0, see `licenses/LGPL-3.0.txt` (supplements the
  GPL-3.0 text in `LICENSE`)
- Source: <https://download.qt.io/archive/qt/6.8/6.8.3/>
- Qt is linked dynamically. You may replace the bundled Qt libraries with a modified version.

## QCustomPlot

- Version: 2.1.1, ported to Qt 6.8 and modified with `libraries/patches/axis_deselect_modifier.patch`
- Copyright: Copyright (C) 2011-2022 Emanuel Eichhammer
- License: GNU General Public License v3.0, see `licenses/qcustomplot.txt`
- Source: <https://github.com/jgeudens/qcustomplot-release> (branch `v2.1.1_Qt6.8_port`), and the
  patched copy in the ModbusScope source tree under `libraries/qcustomplot`

## muparser

- Version: 2.3.6 (development snapshot)
- Copyright: Copyright 2020 Ingo Berg
- License: BSD 2-Clause, see `licenses/muparser.txt`
- Source: <https://github.com/beltoforion/muparser>

## Lucide icons

- Version: 0.469.0
- Copyright: Cole Bemis (Feather, 2013-2022) and Lucide Contributors (2022)
- License: ISC, see `licenses/lucide.txt`
- Source: <https://github.com/lucide-icons/lucide>

## ModbusAdapter

- Binary: `modbusadapter`
- Copyright: Copyright (C) 2026 Synvis
- License: proprietary, see `licenses/modbusadapter/LICENSE.md`. ModbusAdapter is a separate
  program and is not covered by the ModbusScope GPL-3.0 license.
- Includes libsodium (Frank Denis, ISC), see `licenses/modbusadapter/THIRD_PARTY_LICENSES.md`

## Windows only

### OpenSSL

- Version: 3.x (`libcrypto-3-x64.dll`, `libssl-3-x64.dll`)
- Copyright: The OpenSSL Project Authors
- License: Apache License 2.0, see `licenses/Apache-2.0.txt`
- Source: <https://github.com/openssl/openssl>

### MinGW-w64 runtime

- Files: `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`
- `libgcc` and `libstdc++`: GPL-3.0 with the GCC Runtime Library Exception, which places no
  conditions on distributing them with ModbusScope
- `libwinpthread`: Copyright (c) 2011 mingw-w64 project, MIT-style license, see
  `licenses/winpthreads.txt`
- Source: <https://www.mingw-w64.org/>

## Linux AppImage only

The AppImage also bundles system libraries from the build image (for example ICU, libxcb and
fontconfig). Each is distributed under its own license, as provided by its upstream project and
the Linux distribution it was taken from.
