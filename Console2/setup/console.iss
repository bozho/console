[Setup]
OutputDir=setup
SourceDir=..\
OutputBaseFilename=Console 2.00 Beta b125
VersionInfoVersion=2.00
MinVersion=0,5.0.2195sp3
AppName=Console
AppVerName=Console 2.00 Beta build 125
DefaultDirName={pf}\Console
AllowNoIcons=true
ShowLanguageDialog=no
AppID={{99537A70-81BE-46EA-AA30-81C8F074AF45}
Compression=lzma
DefaultGroupName=Console
[Files]
; Binary files
Source: bin\release\Console.exe; DestDir: {app}; Components: main
Source: bin\release\ConsoleHook.dll; DestDir: {app}; Components: main
Source: help\console.chm; DestDir: {app}; Components: main
Source: setup\dlls\FreeImage.dll; DestDir: {app}; Components: main
Source: setup\dlls\FreeImagePlus.dll; DestDir: {app}; Components: main
Source: setup\dlls\msvcp71.dll; DestDir: {app}; Components: main
Source: setup\dlls\msvcr71.dll; DestDir: {app}; Components: main
; Config files
Source: setup\config\console.xml; DestDir: {app}; Components: main
; Fonts
Source: setup\fonts\FixedMedium5x7.fon; DestDir: {fonts}; FontInstall: FixedMedium5x7; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium5x8.fon; DestDir: {fonts}; FontInstall: FixedMedium5x8; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x9.fon; DestDir: {fonts}; FontInstall: FixedMedium6x9; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x10.fon; DestDir: {fonts}; FontInstall: FixedMedium6x10; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x12.fon; DestDir: {fonts}; FontInstall: FixedMedium6x12; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x13.fon; DestDir: {fonts}; FontInstall: FixedMedium6x13; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium7x13.fon; DestDir: {fonts}; FontInstall: FixedMedium7x13; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium7x14.fon; DestDir: {fonts}; FontInstall: FixedMedium7x14; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium8x13.fon; DestDir: {fonts}; FontInstall: FixedMedium8x13; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium9x15.fon; DestDir: {fonts}; FontInstall: FixedMedium9x15; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium9x18.fon; DestDir: {fonts}; FontInstall: FixedMedium9x18; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium10x20.fon; DestDir: {fonts}; FontInstall: FixedMedium10x20; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\TerminalMedium14.fon; DestDir: {fonts}; FontInstall: TerminalMedium14; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
[Registry]
Root: HKCU; Subkey: Console\Console2 command window; ValueType: dword; ValueName: FontSize; ValueData: $000a0000; Flags: createvalueifdoesntexist uninsdeletekey; Components: main
Root: HKCU; Subkey: Console\Console2 command window; ValueType: dword; ValueName: FontFamily; ValueData: $00000036; Flags: createvalueifdoesntexist uninsclearvalue; Components: main
Root: HKCU; Subkey: Console\Console2 command window; ValueType: dword; ValueName: FontWeight; ValueData: $00000190; Flags: createvalueifdoesntexist uninsclearvalue; Components: main
Root: HKCU; Subkey: Console\Console2 command window; ValueType: string; ValueName: FaceName; ValueData: Lucida Console; Flags: createvalueifdoesntexist uninsclearvalue; Components: main
Root: HKLM; Subkey: SOFTWARE\Classes\Directory\shell\ConsoleHere; Tasks: console_here
Root: HKLM; Subkey: SOFTWARE\Classes\Drive\shell\ConsoleHere; Tasks: console_here
Root: HKCR; Subkey: Directory\Shell\ConsoleHere; ValueType: string; ValueData: Co&nsole Here; Tasks: console_here
Root: HKCR; Subkey: Drive\Shell\ConsoleHere; ValueType: string; ValueData: Co&nsole Here; Tasks: console_here
Root: HKCR; Subkey: Directory\Shell\ConsoleHere\command; ValueType: string; ValueData: "{app}\console.exe -c ""/k cd /d """"%1"""""""; Tasks: console_here
Root: HKCR; Subkey: Drive\Shell\ConsoleHere\command; ValueType: string; ValueData: "{app}\console.exe -c ""/k cd /d """"%1"""""""; Tasks: console_here
[Icons]
Name: {group}\Console; Filename: {app}\Console.exe; WorkingDir: {app}; IconFilename: {app}\Console.exe; Comment: Console application; IconIndex: 0; Components: main
Name: {userdesktop}\Console; Filename: {app}\Console.exe; WorkingDir: {app}; IconFilename: {app}\Console.exe; Comment: Console application; IconIndex: 0; Components: main; Tasks: desktop_icon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Console; Filename: {app}\Console.exe; WorkingDir: {app}; IconFilename: {app}\Console.exe; Comment: Console application; IconIndex: 0; Components: main; Tasks: quicklaunch_icon
[Components]
Name: main; Description: Program files; Types: custom compact full; Flags: fixed
Name: fonts; Description: Install additional console fonts; Types: custom full
[Tasks]
Name: console_here; Description: "Install ""Console Here"" shell extension"
Name: desktop_icon; Description: Create desktop icon
Name: quicklaunch_icon; Description: Create Quick launch icon
