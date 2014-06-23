#
# spec file for package 
#
# Copyright (c) 2014 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Name: tabvirt
Version: 1.0
Release: 1
License: GPL
Summary: An RPG board.
Url: http://code.google.com/p/tabuleiro-virtual/
Group: Applications/Games
Source: https://mfribeiro@code.google.com/p/tabuleiro-virtual/tabvirt-1.0.tgz
#BuildRequires:
#PreReq:
#Provides:
BuildRoot: %{_tmppath}/%{name}-%{version}-build

%description
An RPG board to play games over network. Features two modes: master and player. Accepts textures on players and board.

%prep
%setup -q

%build
make

%install
mkdir -p ${RPM_BUILD_ROOT}/opt/tabvirt
mkdir -p ${RPM_BUILD_ROOT}/opt/tabvirt/texturas
mkdir -p ${RPM_BUILD_ROOT}/opt/tabvirt/texturas_locais
mkdir -p ${RPM_BUILD_ROOT}/opt/tabvirt/tabuleiros_salvos
cp ${RPM_BUILD_DIR}/tabvirt-1.0/tabvirt ${RPM_BUILD_ROOT}/opt/tabvirt
cp -R ${RPM_BUILD_DIR}/tabvirt-1.0/dados ${RPM_BUILD_ROOT}/opt/tabvirt

%post

%postun

%files
%defattr(-,root,root)
/opt/tabvirt/tabvirt
/opt/tabvirt/tabuleiros_salvos
/opt/tabvirt/texturas
/opt/tabvirt/texturas_locais
/opt/tabvirt/dados/acoes.asciiproto
/opt/tabvirt/dados/modelos.asciiproto



%changelog

