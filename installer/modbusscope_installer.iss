; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "ModbusScope"
#define MyAppExtension "mbs"
#define MyAppExeName "ModbusScope.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{E2D28A15-1D34-4D76-AF4E-49A17C19A3A2}
AppName={#MyAppName}
AppVersion=1.3.0
AppPublisher="jgeudens"
AppPublisherURL=http://jgeudens.github.io/
AppSupportURL=http://jgeudens.github.io/
AppUpdatesURL=http://jgeudens.github.io/
DefaultDirName={pf32}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename="{#MyAppName}_setup"
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes
UninstallDisplayName={#MyAppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "mbsfileextension"; Description: "Associate ""mbs"" extension"; GroupDescription: "File extensions:"; Flags: unchecked

[Files]
Source: "ModbusScope.exe"; DestDir: "{app}"; Flags: ignoreversion
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