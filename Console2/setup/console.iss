[Setup]
OutputDir=setup
SourceDir=..\
OutputBaseFilename=Console 2.0
VersionInfoVersion=2.00
MinVersion=0,5.0.2195sp3
AppName=Console
AppVerName=2.00
DefaultDirName={pf}\Console
AllowNoIcons=true
ShowLanguageDialog=no
AppID={{99537A70-81BE-46EA-AA30-81C8F074AF45}
Compression=lzma
[Files]
Source: bin\release\Console.exe; DestDir: {app}
Source: bin\release\ConsoleHook.dll; DestDir: {app}
Source: bin\release\FreeImage.dll; DestDir: {app}
Source: bin\release\FreeImagePlus.dll; DestDir: {app}
[Registry]
Root: HKCU; Subkey: Console\Console2 command window; ValueType: dword; ValueName: FontSize; ValueData: $000a0000; Flags: createvalueifdoesntexist uninsclearvalue uninsdeletekeyifempty
Root: HKCU; Subkey: Console\Console2 command window; ValueType: dword; ValueName: FontFamily; ValueData: $00000036; Flags: createvalueifdoesntexist uninsclearvalue uninsdeletekeyifempty
Root: HKCU; Subkey: Console\Console2 command window; ValueType: dword; ValueName: FontWeight; ValueData: $00000190; Flags: createvalueifdoesntexist uninsclearvalue uninsdeletekeyifempty
Root: HKCU; Subkey: Console\Console2 command window; ValueType: string; ValueName: FaceName; ValueData: Lucida Console; Flags: createvalueifdoesntexist uninsclearvalue uninsdeletekeyifempty
