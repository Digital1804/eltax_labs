#!/bin/bash

# Параметры
ARCH=arm
GNUE=arm-linux-gnueabihf
PREFIX=~/build/_install
OPENSSH_VERSION=9.8p1
OPENSSL_VERSION=1.1.1w
OPENSSL_VERSION_URL=1_1_1w
ZLIB_VERSION=1.3.1

# Установите пути к кросс-компилятору и утилитам
export CC=${GNUE}-gcc
export AR=${GNUE}-ar
export RANLIB=${GNUE}-ranlib

# Создайте временные каталоги для исходников и сборки
# rm -rf ~/build
# mkdir -p ~/build/{openssl,zlib,openssh,_install}

# Загрузите и распакуйте OpenSSL
cd ~/build/openssl
wget https://github.com/openssl/openssl/releases/download/OpenSSL_${OPENSSL_VERSION_URL}/openssl-${OPENSSL_VERSION}.tar.gz
tar -xzf openssl-${OPENSSL_VERSION}.tar.gz
cd openssl-${OPENSSL_VERSION}

# Кросс-компиляция OpenSSL
./Configure linux-armv4 --prefix=${PREFIX}/openssl
export CROSS_COMPILE=${GNUE}-
export CC=${GNUE}-gcc
make -j 6
make install -j 6

# Сборка zlib
cd ~/build/zlib
wget http://zlib.net/zlib-${ZLIB_VERSION}.tar.gz
tar -xzf zlib-${ZLIB_VERSION}.tar.gz
cd zlib-${ZLIB_VERSION}

# Кросс-компиляция zlib
./configure --prefix=${PREFIX}/zlib
export CC=${GNUE}-gcc
make -j 6
make install -j 6

# Сборка OpenSSH
cd ~/build/openssh
wget https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-${OPENSSH_VERSION}.tar.gz
tar -xzf openssh-${OPENSSH_VERSION}.tar.gz
cd openssh-${OPENSSH_VERSION}

# Настройка окружения для кросс-компиляции
export CFLAGS="-I${PREFIX}/openssl/include -I${PREFIX}/zlib/include"
export LDFLAGS="-L${PREFIX}/openssl/lib -L${PREFIX}/zlib/lib"
export PKG_CONFIG_PATH="${PREFIX}/openssl/lib/pkgconfig:${PREFIX}/zlib/lib/pkgconfig"
export LD_LIBRARY_PATH=${PREFIX}/openssl/lib:$LD_LIBRARY_PATH
# Конфигурация OpenSSH
autoreconf
./configure --host=${GNUE} --with-ssl-dir=${PREFIX}/openssl --with-zlib=${PREFIX}/zlib --prefix=${PREFIX}/openssh --disable-strip

# Сборка и установка OpenSSH
make
make install

echo "Кросс-компиляция OpenSSH завершена!"
