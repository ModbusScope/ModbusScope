#!/bin/bash
set -ex

# Copy assets
cp resources/ModbusScope.desktop release/src/bin/linux/
cp resources/icon/icon-256x256.png release/src/bin/linux/ModbusScope.png

# Copy modbusadapter so linuxdeploy bundles it alongside the main executable
cp adapters/modbusadapter release/src/bin/linux/

cd release/src/bin/linux/

# download linuxdeploy and its Qt plugin if not already cached
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
  wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
  chmod +x linuxdeploy-x86_64.AppImage
fi
if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
  wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
  chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi

# Generate AppImage
# The adapter binaries use $ORIGIN RPATH; linuxdeploy patches their RPATH and
# deploys their Qt dependencies alongside the main application.
export APPIMAGE_EXTRACT_AND_RUN=1 # Workaround because FUSE isn't available in Docker
./linuxdeploy-x86_64.AppImage --appdir AppDir \
    -e modbusscope \
    -e modbusadapter \
    -i ModbusScope.png \
    -d ModbusScope.desktop \
    --plugin qt \
    --output appimage

ls

cp ModbusScope*.AppImage ../../../..
