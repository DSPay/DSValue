#!/bin/sh
set -e
srcdir="$(dirname $0)"
cd "$srcdir"
autoreconf --install --force

CPPFLAGS="-I/c/deps/boost_1_55_0 \
-I/c/deps/db-4.8.30.NC/build_unix \
-I/c/deps/openssl-1.0.1g/include \
-I/c/deps \
-I/c/deps/protobuf-2.5.0/src \
-I/c/deps/libpng-1.6.9 \
-I/c/deps/qrencode-3.4.3" \
LDFLAGS="-L/c/deps/boost_1_55_0/stage/lib \
-L/c/deps/db-4.8.30.NC/build_unix \
-L/c/deps/openssl-1.0.1g \
-L/c/deps/miniupnpc \
-L/c/deps/protobuf-2.5.0/src/.libs \
-L/c/deps/libpng-1.6.9/.libs \
-L/c/deps/qrencode-3.4.3/.libs" \
./configure \
--disable-upnp-default \
--disable-tests \
--without-gui \
--without-cli \
--with-daemon \
--with-qt-incdir=/c/Qt/5.2.1/include \
--with-qt-libdir=/c/Qt/5.2.1/lib \
--with-qt-bindir=/c/Qt/5.2.1/bin \
--with-qt-plugindir=/c/Qt/5.2.1/plugins \
--with-boost-libdir=/c/deps/boost_1_55_0/stage/lib \
--with-boost-system=mgw48-mt-s-1_55 \
--with-boost-filesystem=mgw48-mt-s-1_55 \
--with-boost-program-options=mgw48-mt-s-1_55 \
--with-boost-thread=mgw48-mt-s-1_55 \
--with-boost-chrono=mgw48-mt-s-1_55 \
--with-protoc-bindir=/c/deps/protobuf-2.5.0/src
