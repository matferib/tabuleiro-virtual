[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A9966132-260A-4314-9480-BBCA4BCA5C8F}
AppName=Instalador Tabuleiro Virtual
AppVerName=Tabuleiro Virtual 1.10.1
AppPublisher=Matferib
OutputDir=output
OutputBaseFilename=TabuleiroVirtual-1.10.1
Compression=lzma
SolidCompression=yes
DefaultDirName={pf32}\TabuleiroVirtual
DirExistsWarning=no
DisableDirPage=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Dirs]
Name: {app}\texturas_locais
Name: {app}\tabuleiros_salvos
Name: {localappdata}\TabuleiroVirtual

[Files]
Source: ..\tabvirt.exe; Destdir: {app};
Source: ..\win32\lib\*.dll; Destdir: {app};
Source: ..\texturas\*.png; Destdir: {app}\texturas;
Source: ..\dados\*.asciiproto; Destdir: {app}\dados;

