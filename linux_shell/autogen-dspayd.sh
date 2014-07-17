#!/bin/sh
echo copy the libmemenv.a and libleveldb.a to leveldb/
cp -rf ./libmemenv.a ../src/leveldb
cp -rf ./libleveldb.a ../src/leveldb

echo go to "$PWD"
srcdir=".."
cd "$srcdir"

set -e
autoreconf --install --force

CPPFLAGS="-I/c/deps/boost_1_55_0 \
-I/c/deps/db-4.8.30.NC/build_unix \
-I/c/deps/openssl-1.0.1g/include \
-I/c/deps \
-I/c/deps/protobuf-2.5.0/src \
-I/c/deps/libpng-1.6.9 \
-fPIC \
-s -static -static-libgcc -static-libstdc++ \
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
--enable-debug \
--without-gui \
--without-cli \
--with-daemon \
--with-incompatible-bdb

make
