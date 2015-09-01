# ModbusScope Installer for Windows
## Requirements:
* Windows OS
* Inno Setup Compiler (version used: v5.5.6(a))
* Inno Setup Preprocessor (ISPP) add-on

## Current installer features:
* Desktop icon
* Mbs file association

## Create installer:
1. Compile statically linked executable of ModbusScope
2. Make sure executable is named "ModbusScope.exe"
3. Copy executable to same folder as installer script
4. Compile Inno installer script
5. Installer is generated in Output directory
