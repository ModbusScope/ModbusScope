; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "ModbusScope"
#define MyAppExtension "mbs"
#define MyAppExeName "ModbusScope.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B5B201A8-51DE-418F-8BCF-F46686ED638C}
AppName={#MyAppName}
AppVersion=3.1.0
AppPublisher="jgeudens"
AppPublisherURL=https://modbusscope.readthedocs.io
AppSupportURL=https://github.com/jgeudens/ModbusScope
AppUpdatesURL=https://modbusscope.readthedocs.io
DefaultDirName={commonpf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename="{#MyAppName}_setup"
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes
UninstallDisplayName={#MyAppName}
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "mbsfileextension"; Description: "Associate ""mbs"" extension"; GroupDescription: "File extensions:"; Flags: unchecked

[Files]
Source: "deploy\*"; DestDir: "{app}\"; Flags: ignoreversion recursesubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,ModbusScope}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: ".{#MyAppExtension}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue; Tasks: mbsfileextension
Root: HKCR; Subkey: "{#MyAppName}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName} project"; Flags: uninsdeletekey; Tasks: mbsfileextension
Root: HKCR; Subkey: "{#MyAppName}DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: mbsfileextension
Root: HKCR; Subkey: "{#MyAppName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: mbsfileextension
