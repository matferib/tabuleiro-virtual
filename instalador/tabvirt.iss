[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A9966132-260A-4314-9480-BBCA4BCA5C8F}
AppName=Instalador Tabuleiro Virtual
AppVerName=Tabuleiro Virtual 1.0
AppPublisher=Matferib
OutputDir=output
OutputBaseFilename=TabVirt
Compression=lzma
SolidCompression=yes
DefaultDirName=basedir
DirExistsWarning=no
DisableDirPage=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Files]
Source: ..\tabvirt.exe; Destdir: {app};
Source: ..\win32\lib\*.dll; Destdir: {app};



