#!/bin/sh
export CFLAGS+='-nostdlib'
export CPPFLAGS+=-I$KERNEL_INC_DIR
export LDFLAGS+=-L$KERNEL_LIB_DIR
export LIBS+='-lkernel'
./configure CFLAGS="-g" --without-imagemagick --without-qt --host=arm-linux --prefix=`pwd`/_install --without-gtk --without-x   --without-python --enable-codes=qrcode  --without-xshm  --without-xv --without-jpeg --disable-video --disable-pthread --without-libiconv-prefix --enable-static=yes
#./configure --without-imagemagick --without-qt --host=arm-linux --prefix=`pwd`/_install --without-gtk --without-x   --without-python --enable-codes=qrcode  --without-xshm  --without-xv --without-jpeg --disable-video --disable-pthread --without-libiconv-prefix --enable-static=yes
