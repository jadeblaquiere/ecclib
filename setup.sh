#!/bin/bash

OS_FLAVOR=$OSTYPE
if [ "$OSTYPE" = "linux-gnu" ]
then
    LINUX_FLAVOR=`cat /etc/issue | cut -d " " -f 1 | head -1`
    if [ "$LINUX_FLAVOR" = "Ubuntu" ]
    then
        echo "Configuring build for Ubuntu"
        # ...
        sudo apt-get update
        sudo apt-get -y install check
        # the following are needed for building examples (core library only depends on GMP)
        #####
        # alternatively git clone git@gitlab.com:gnutls/libtasn1.git
        sudo apt-get -y install libtasn1-6-dev libtasn1-bin
        # alternatively brew install popt
        sudo apt-get -y install libpopt-dev
        # alternatively https://github.com/transmission/libb64.git
        sudo apt-get -y install libb64-dev
    else
        echo "Unimplemented Linux Variant"
        exit 1
    fi
elif [ "$OSTYPE" = "darwin"* ]
then
    echo "Configuring build for Mac OSX"
    echo "Error, not yet implemented"
    exit 1
        # Mac OSX
elif [ "$OSTYPE" = "cygwin" ]
then
    echo "Configuring build for Cygwin"
    echo "Error, not yet implemented"
    exit 1
        # POSIX compatibility layer and Linux environment emulation for Windows
else
    echo "Unknown OSTYPE = $OSTYPE"
    exit 1
fi

# need to build libsodium from source - expect v1.0.16
# alternatively https://github.com/jedisct1/libsodium.git
mkdir sodium-build
cd sodium-build
wget http://archive.ubuntu.com/ubuntu/pool/main/libs/libsodium/libsodium_1.0.16.orig.tar.gz
tar xvf libsodium_1.0.16.orig.tar.gz
cd libsodium-1.0.16
autoreconf --install
./configure --prefix=/usr
make
sudo make install
cd ../..
#####
