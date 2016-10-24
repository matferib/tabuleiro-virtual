[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A9966132-260A-4314-9480-BBCA4BCA5C8F}
AppName=Instalador Tabuleiro Virtual
AppVerName=Tabuleiro Virtual 2.4.0.
AppPublisher=Matferib
OutputDir=output
OutputBaseFilename=TabuleiroVirtual-2.4.0
Compression=lzma
SolidCompression=yes
DefaultDirName={pf32}\TabuleiroVirtual
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
Source: ..\tabvirt.exe; Destdir: {app};
Source: ..\icon.ico; Destdir: {app};
Source: ..\win32\lib\*.dll; Destdir: {app};
Source: ..\texturas\*.png; Destdir: {app}\texturas;
Source: ..\dados\*.asciiproto; Destdir: {app}\dados;
Source: ..\shaders\*.c; Destdir: {app}\shaders;
Source: ..\modelos3d\*.binproto; Destdir: {app}\modelos3d;
Source: ..\tabuleiros_salvos\castelo.binproto; Destdir: {app}\tabuleiros_salvos
Source: ..\tabuleiros_salvos\deserto.binproto; Destdir: {app}\tabuleiros_salvos
Source: ..\tabuleiros_salvos\features.binproto; Destdir: {app}\tabuleiros_salvos

[Icons]
Name: "{group}\Tabuleiro Virtual"; Filename: "{app}\tabvirt.exe"; WorkingDir: "{app}"; IconFilename: "{app}/icon.ico"
Name: "{group}\Desinstalar Tabuleiro Virtual"; Filename: "{uninstallexe}";
Name: "{commondesktop}\Tabuleiro Virtual"; Filename: "{app}\tabvirt.exe"; WorkingDir: "{app}"; IconFilename: "{app}/icon.ico"
