#OVERTE=~/Overte rpmbuild --target x86_64 -bb overte-server.spec
%define version %{lua:print(os.getenv("VERSION"))}
%define depends %{lua:print(os.getenv("DEPENDS"))}

Name:           overte-server
Version:        %{version}
Release:        1%{?dist}
Summary:        Overte platform, based on the High Fidelity Engine.

License:        ASL 2.0
URL:            https://overte.org
Source0:        https://github.com/overte-org/overte-builder/blob/master/vircadia-builder

#BuildRequires:  systemd-rpm-macros
BuildRequires:  chrpath
Requires:       %{depends}
BuildArch:      x86_64
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
install -m 0755 -t $RPM_BUILD_ROOT/opt/overte $OVERTE/source/pkg-scripts/new-server
install -d $RPM_BUILD_ROOT/opt/overte/lib
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/build/libraries/*/*.so
strip --strip-all $RPM_BUILD_ROOT/opt/overte/lib/*
chrpath -d $RPM_BUILD_ROOT/opt/overte/lib/*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Network.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Core.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Widgets.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Gui.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Script.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5WebSockets.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Qml.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5Quick.so.*.*.*
install -m 0644 -t $RPM_BUILD_ROOT/opt/overte/lib $OVERTE/qt5-install/lib/libQt5ScriptTools.so.*.*.*
install -d $RPM_BUILD_ROOT/usr/lib/systemd/system
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-assignment-client.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-assignment-client@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-domain-server.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-domain-server@.service
#install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-ice-server.service
#install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-ice-server@.service
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-server.target
install -m 0644 -t $RPM_BUILD_ROOT/usr/lib/systemd/system $OVERTE/source/pkg-scripts/overte-server@.target
cp -a $OVERTE/source/domain-server/resources $RPM_BUILD_ROOT/opt/overte
cp -a $OVERTE/build/assignment-client/plugins $RPM_BUILD_ROOT/opt/overte
chrpath -d $RPM_BUILD_ROOT/opt/overte/plugins/*.so
chrpath -d $RPM_BUILD_ROOT/opt/overte/plugins/*/*.so
strip --strip-all $RPM_BUILD_ROOT/opt/overte/plugins/*.so
strip --strip-all $RPM_BUILD_ROOT/opt/overte/plugins/*/*.so
find $RPM_BUILD_ROOT/opt/overte/resources -name ".gitignore" -delete


%files
%license $OVERTE/source/LICENSE
/opt/overte
/usr/lib/systemd/system


%changelog


%post
# create users
getent passwd overte >/dev/null 2>&1 || useradd -r -c "overte" -d /var/lib/overte -U -M overte
#getent group overte >/dev/null 2>&1 || groupadd -r overte

# create data folder
mkdir -p /etc/opt/overte
mkdir -p /var/lib/overte && chown overte:overte /var/lib/overte && chmod 775 /var/lib/overte

ldconfig -n /opt/overte/lib

%systemd_post overte-assignment-client.service
%systemd_post overte-assignment-client@.service
%systemd_post overte-domain-server.service
%systemd_post overte-domain-server@.service
#%systemd_post overte-ice-server.service
#%systemd_post overte-ice-server@.service
%systemd_post overte-server.target
%systemd_post overte-server@.target

if [ ! -d "/var/lib/overte/default" ]; then
	if [ -d "/var/lib/athena" ]; then
		ATHENA_ACTIVE=`systemctl list-units \
			| grep -P -o "(athena-assignment-client|athena-domain-server|athena-server)\S+" \
			| paste -s -d'|' \
			| head -c -1`
		ATHENA_ENABLED=`systemctl list-units --state=loaded \
			| grep -P -o "(athena-assignment-client|athena-domain-server|athena-server)\S+" \
			| xargs -I {} sh -c 'if systemctl is-enabled {} >/dev/null ; then echo {} ; fi' \
			| paste -s -d'|' \
			| head -c -1`

		# shutdown athena servers
		echo -n $ATHENA_ACTIVE | xargs -d'|' systemctl stop

		# copy the server files over
		cp /etc/opt/athena/* /etc/opt/overte
		cp -R /var/lib/athena/* /var/lib/overte
		chown -R overte:overte /var/lib/overte/*
		find /var/lib/overte -maxdepth 3 -path "*\.local/share" -execdir sh -c 'cd share; ln -s ../.. "overte - dev"' ';'
		find /var/lib/overte -maxdepth 3 -path "*\.local/share" -execdir sh -c 'cd share; ln -s ../.. overte' ';'

		OVERTE_ACTIVE=`echo -n $ATHENA_ACTIVE | sed 's/athena/overte/g'`
		OVERTE_ENABLED=`echo -n $ATHENA_ENABLED | sed 's/athena/overte/g'`

		echo -n $ATHENA_ENABLED | xargs -d'|' systemctl disable
		echo -n $OVERTE_ENABLED | xargs -d'|' systemctl enable
		echo -n $OVERTE_ACTIVE | xargs -d'|' systemctl start
	else
		/opt/overte/new-server default 40100
		systemctl enable overte-server@default.target
		systemctl start overte-server@default.target
	fi
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

%systemd_preun overte-server.target
%systemd_preun overte-server@.target
%systemd_preun overte-assignment-client.service
%systemd_preun overte-assignment-client@.service
%systemd_preun overte-domain-server.service
%systemd_preun overte-domain-server@.service
#%systemd_preun overte-ice-server.service
#%systemd_preun overte-ice-server@.service


%postun
%systemd_postun_with_restart overte-server.target
%systemd_postun_with_restart overte-server@.target
%systemd_postun_with_restart overte-assignment-client.service
%systemd_postun_with_restart overte-assignment-client@.service
%systemd_postun_with_restart overte-domain-server.service
%systemd_postun_with_restart overte-domain-server@.service
#%systemd_postun_with_restart overte-ice-server.service
#%systemd_postun_with_restart overte-ice-server@.service
