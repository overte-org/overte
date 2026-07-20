# Copyright 2020-2021 Vircadia contributors.
# Copyright 2022-2026 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

#OVERTE=~/Overte rpmbuild --target x86_64 -bb overte-server.spec
%define version %{lua:print(os.getenv("VERSION"))}
%define depends %{lua:print(os.getenv("DEPENDS"))}

Name:           overte-server
Version:        %{version}
Release:        1%{?dist}
Summary:        Overte platform, based on the High Fidelity Engine.

License:        ASL 2.0
URL:            https://overte.org
Source0:        https://github.com/overte-org/overte

#BuildRequires:  systemd-rpm-macros
BuildRequires:  chrpath
Requires:       %{depends}

BuildArch:      %_target_cpu


AutoReq:        no
AutoProv:       no

%description
Overte allows creation and sharing of VR experiences.
  The Overte provides built-in social features, including avatar interactions, spatialized audio and interactive physics. Additionally, you have the ability to import any 3D object into your virtual environment.


%prep


%build


%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT/opt/overte
install -m 0755 -t $RPM_BUILD_ROOT/opt/overte $OVERTE/build/assignment-client/assignment-client
install -m 0755 -t $RPM_BUILD_ROOT/opt/overte $OVERTE/build/domain-server/domain-server
install -m 0755 -t $RPM_BUILD_ROOT/opt/overte $OVERTE/build/tools/oven/oven
#install -m 0755 -t $RPM_BUILD_ROOT/opt/overte $OVERTE/build/ice-server/ice-server
strip --strip-all $RPM_BUILD_ROOT/opt/overte/*
chrpath -d $RPM_BUILD_ROOT/opt/overte/*
install -m 0755 -t $RPM_BUILD_ROOT/opt/overte $OVERTE/pkg-scripts/new-server
install -d $RPM_BUILD_ROOT/opt/overte/lib
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/build/libraries/*/*.so
strip --strip-all $RPM_BUILD_ROOT/opt/overte/lib/*
chrpath -d $RPM_BUILD_ROOT/opt/overte/lib/*
# hack: we get libnode.so.108 from conan-libs folder, because we don't know if it is available on the system.
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/build/conanlibs/Release/libnode.so*
# hack: we get libttb.so.12 from conan-libs folder, because we don't know if it is available on the system.
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/build/conanlibs/Release/libtbb.so*
%if "$OVERTE_USE_SYSTEM_QT" == ""
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5Network.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5Core.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5Widgets.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5Gui.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5WebSockets.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5Qml.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $QT5_LIBS/libQt5Quick.so.*.*.*
%endif
install -d $RPM_BUILD_ROOT/usr/lib/systemd/system
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-agents@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-asset-server@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-audio-mixer@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-avatar-mixer@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-domain-server@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-entity-script-server@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-entity-server@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-message-mixer@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-domain-server@.service
#install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-ice-server@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/pkg-scripts/systemd/overte-server@.target
cp -a $OVERTE/domain-server/resources $RPM_BUILD_ROOT/opt/overte
cp -a $OVERTE/build/assignment-client/plugins $RPM_BUILD_ROOT/opt/overte
chrpath -d $RPM_BUILD_ROOT/opt/overte/plugins/*.so
chrpath -d $RPM_BUILD_ROOT/opt/overte/plugins/*/*.so
strip --strip-all $RPM_BUILD_ROOT/opt/overte/plugins/*.so
strip --strip-all $RPM_BUILD_ROOT/opt/overte/plugins/*/*.so
install -d $RPM_BUILD_ROOT/usr/share/licenses/overte-server
cp $OVERTE/LICENSE $RPM_BUILD_ROOT/usr/share/licenses/overte-server/LICENSE
find $RPM_BUILD_ROOT/opt/overte/resources -name ".gitignore" -delete


%files
%license /usr/share/licenses/overte-server/LICENSE
/opt/overte
/usr/lib/systemd/system


%changelog


%post
# create users
getent passwd overte >/dev/null 2>&1 || useradd -r -c "Overte" -d /var/lib/overte -U -M overte
#getent group overte >/dev/null 2>&1 || groupadd -r overte

# create data folder
mkdir -p /etc/opt/overte
mkdir -p /var/lib/overte && chown overte:overte /var/lib/overte && chmod 775 /var/lib/overte

ldconfig -n /opt/overte/lib

%systemd_post overte-agents@.service
%systemd_post overte-asset-server@.service
%systemd_post overte-audio-mixer@.service
%systemd_post overte-avatar-mixer@.service
%systemd_post overte-domain-server@.service
%systemd_post overte-entity-script-server@.service
%systemd_post overte-entity-server@.service
%systemd_post overte-message-mixer@.service

%systemd_post overte-domain-server@.service
#%systemd_post overte-ice-server@.service
%systemd_post overte-server@.target

if [ ! -d "/var/lib/overte/default" ]; then
	/opt/overte/new-server default 40100
	systemctl enable overte-server@default.target
	systemctl start overte-server@default.target
else
	systemctl list-units \
		| grep -P -o "(overte-assignment-client|overte-domain-server|overte-server)\S+" \
		| xargs systemctl restart
fi


%preun

if [ "$1" -eq 0 ]; then
	systemctl list-units \
		| grep -P -o "(overte-assignment-client|overte-domain-server|overte-server)\S+" \
		| xargs systemctl stop
fi

%systemd_preun overte-server@.target
%systemd_preun overte-agents@.service
%systemd_preun overte-asset-server@.service
%systemd_preun overte-audio-mixer@.service
%systemd_preun overte-avatar-mixer@.service
%systemd_preun overte-domain-server@.service
%systemd_preun overte-entity-script-server@.service
%systemd_preun overte-entity-server@.service
%systemd_preun overte-message-mixer@.service
%systemd_preun overte-domain-server@.service
#%systemd_preun overte-ice-server@.service


%postun
%systemd_postun_with_restart overte-server@.target
%systemd_postun_with_restart overte-agents@.service
%systemd_postun_with_restart overte-asset-server@.service
%systemd_postun_with_restart overte-audio-mixer@.service
%systemd_postun_with_restart overte-avatar-mixer@.service
%systemd_postun_with_restart overte-domain-server@.service
%systemd_postun_with_restart overte-entity-script-server@.service
%systemd_postun_with_restart overte-entity-server@.service
%systemd_postun_with_restart overte-message-mixer@.service
%systemd_postun_with_restart overte-domain-server@.service
#%systemd_postun_with_restart overte-ice-server@.service
