#!/bin/bash

# Copyright 2024 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# OS=debian-11 TAG=2024.06.1 ARCH=amd64 ./build_server_package.bash
# Currently supported OSs debian-11 debian-12 ubuntu-20.04 ubuntu-22.04 ubuntu-24.04 fedora-39 fedora-40 rockylinux-9
# Remember to add a CMAKE_BACKTRACE_URL below if applicable.

echo "Cloning Overte $TAG …"
git clone --depth 1 --branch $TAG https://github.com/overte-org/overte.git Overte-$TAG-$OS-$ARCH
cd Overte-$TAG-$OS-$ARCH

cat > commands.bash <<EOF
#!/bin/bash
# Automatically generated

echo "Preparing environment …";
export CMAKE_BACKTRACE_URL=""
export CMAKE_BACKTRACE_TOKEN="$TAG"
export PRODUCTION_BUILD=1
export RPMVERSION="$TAG"
export DEBVERSION="$TAG-$OS"
export DEBEMAIL="julian.gro@overte.org"
export DEBFULLNAME="Julian Groß"
if [[ "$OS" == "ubuntu-18.04" || "$OS" == "ubuntu-20.04" ]]; then
    : # Do nothing. Don't set OVERTE_USE_SYSTEM_QT
else
    export OVERTE_USE_SYSTEM_QT=true
fi
if [[ "$OS" =~ "ubuntu" || "$OS" =~ "debian" ]]; then
    # Debian
    apt update
    apt dist-upgrade -y
else
    # RPM
    dnf upgrade -y
fi

cd /overte
rm -rf build && mkdir build
cd build || exit

echo "Configuring …"
if [ "$ARCH" == "amd64" ]; then
    cmake .. -DOVERTE_CPU_ARCHITECTURE=-msse3 -DVCPKG_BUILD_TYPE=release -DSERVER_ONLY=true -DBUILD_TOOLS=true
else
    # aarch64
    VCPKG_FORCE_SYSTEM_BINARIES=1 cmake .. -DOVERTE_CPU_ARCHITECTURE= -DVCPKG_BUILD_TYPE=release -DSERVER_ONLY=true -DBUILD_TOOLS=true
fi

echo "Building …"
make domain-server assignment-client oven -j$(nproc) || exit

echo "Packaging for $OS …"
cd ../pkg-scripts || exit; \
if [[ "$OS" =~ "ubuntu" || "$OS" =~ "debian" ]]; then
    # Debian
    ./make-deb-server || /bin/bash;
else
    # RPM
    ./make-rpm-server || /bin/bash;
fi

echo "Preparing files for Sentry …"
cd .. || exit
tar -cf Senty_upload_$TAG-$OS-$ARCH.tar build || exit



EOF

if [[ "$OS" =~ "ubuntu" || "$OS" =~ "debian" || "$OS" == "rockylinux-9" ]]; then
    docker run -v $(pwd):/overte -it overte/overte-server-build:0.1.3-$OS-$ARCH /bin/bash -c " \
    cd /overte; \
    chmod +x commands.bash; \
    ./commands.bash; \
    "
else
    docker run -v $(pwd):/overte -it overte/overte-server-build:0.1.4-$OS-$ARCH /bin/bash -c " \
    cd /overte; \
    chmod +x commands.bash; \
    ./commands.bash; \
    "
fi

