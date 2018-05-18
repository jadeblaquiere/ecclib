#!/bin/bash

case "$OSTYPE" in
  linux*)
    LINUX_FLAVOR=`cat /etc/issue | cut -d " " -f 1 | head -1`
    if [ "$LINUX_FLAVOR" = "Ubuntu" ]
    then
        echo "Configuring build for Ubuntu"
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
    ;;
  darwin*)
    echo "Configuring build for Mac OSX"
    brew update
    brew install check
    brew install libtasn1
    brew install popt
    mkdir libb64-build
    cd libb64-build
    git clone https://github.com/transmission/libb64.git
    cd libb64
    make clean
    make
    sudo mkdir -p -m 755 /usr/local/include/b64
    sudo chown root.admin /usr/local/include/b64
    sudo install -m 644 -o root -g admin include/cdecode.h /usr/local/include/b64/cdecode.h
    sudo install -m 644 -o root -g admin include/cencode.h /usr/local/include/b64/cencode.h
    sudo install -m 644 -o root -g admin include/decode.h /usr/local/include/b64/decode.h
    sudo install -m 644 -o root -g admin include/encode.h /usr/local/include/b64/encode.h
    sudo install -m 644 -o root -g admin src/libb64.a /usr/local/lib/libb64.a
    cd ../..
    ;;
  *)
    echo "Unknown OSTYPE = $OSTYPE"
    exit 1
    ;;
esac

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
