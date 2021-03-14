[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A9966132-260A-4314-9480-BBCA4BCA5C8F}
AppName=Instalador Tabuleiro Virtual
AppVerName=Tabuleiro Virtual 5.5.0
AppPublisher=Matferib
OutputDir=output
OutputBaseFilename=TabuleiroVirtual-5.5.0
Compression=lzma
SolidCompression=yes
DefaultDirName={commonpf64}\TabuleiroVirtual
DirExistsWarning=no
DisableDirPage=no
DefaultGroupName=TabuleiroVirtual

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Dirs]
Name: {app}\texturas_locais
Name: {app}\tabuleiros_salvos
Name: {localappdata}\TabuleiroVirtual

[Files]
Source: ..\tabvirt.exe; Destdir: {app}; Flags: ignoreversion;
Source: ..\icon.ico; Destdir: {app}; Flags: ignoreversion;
Source: ..\libprotobuf.dll; Destdir: {app}; Flags: ignoreversion;
Source: ..\libprotobuf-lite.dll; Destdir: {app}; Flags: ignoreversion;
Source: ..\Qt5*.dll; Destdir: {app}; Flags: ignoreversion;
Source: ..\texturas\*.png; Destdir: {app}\texturas; Flags: ignoreversion;
Source: ..\sons\*.wav; Destdir: {app}\sons; Flags: ignoreversion;
Source: ..\dados\*.asciiproto; Destdir: {app}\dados; Flags: ignoreversion;
Source: ..\shaders\*.c; Destdir: {app}\shaders; Flags: ignoreversion;
Source: ..\modelos3d\*.binproto; Destdir: {app}\modelos3d; Flags: ignoreversion;
Source: ..\tabuleiros_salvos\castelo.binproto; Destdir: {app}\tabuleiros_salvos; Flags: ignoreversion;
Source: ..\tabuleiros_salvos\deserto.binproto; Destdir: {app}\tabuleiros_salvos; Flags: ignoreversion;
Source: ..\tabuleiros_salvos\features.binproto; Destdir: {app}\tabuleiros_salvos; Flags: ignoreversion;
Source: ..\platforms\*.dll; Destdir: {app}\platforms; Flags: ignoreversion;
Source: ..\audio\*.dll; Destdir: {app}\audio; Flags: ignoreversion;
Source: ..\styles\*.dll; Destdir: {app}\styles; Flags: ignoreversion;
Source: ..\mediaservice\*.dll; Destdir: {app}\mediaservice; Flags: ignoreversion;
Source: ..\playlistformats\*.dll; Destdir: {app}\playlistformats; Flags: ignoreversion;
Source: ..\iconengines\*.dll; Destdir: {app}\iconengines; Flags: ignoreversion;
Source: ..\imageformats\*.dll; Destdir: {app}\imageformats; Flags: ignoreversion;
Source: ..\bearer\*.dll; Destdir: {app}\bearer; Flags: ignoreversion;

[Icons]
Name: "{group}\Tabuleiro Virtual"; Filename: "{app}\tabvirt.exe"; WorkingDir: "{app}"; IconFilename: "{app}/icon.ico"
Name: "{group}\Desinstalar Tabuleiro Virtual"; Filename: "{uninstallexe}";
Name: "{commondesktop}\Tabuleiro Virtual"; Filename: "{app}\tabvirt.exe"; WorkingDir: "{app}"; IconFilename: "{app}/icon.ico"
