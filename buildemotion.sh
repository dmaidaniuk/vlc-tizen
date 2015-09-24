#!/bin/bash

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")
source ${PROJECTPATH}/buildcommon

#########################
# FETCH EMOTION SOURCES #
#########################

if [ ! -d "${PROJECTPATH}/emotion" ]; then
    echo -e "\e[1m\e[32mEMOTION source not found, cloning\e[0m"
    git clone http://code.videolan.org/videolan/tizen_emotion.git "${PROJECTPATH}/emotion"
    checkfail "emotion source: git clone failed"
fi

###############################
# EMOTION CONFIGURE ARGUMENTS #
###############################
EMOTION_CONFIGURE_ARGS="\
    --enable-libvlc=static \
    --disable-emotion-test \
    --disable-doc \
    --disable-xine \
    --disable-generic \
    --disable-gstreamer \
"

echo -e "\e[1m\e[36mCFLAGS:            ${CFLAGS}\e[0m"
echo -e "\e[1m\e[36mEXTRA_CFLAGS:      ${EXTRA_CFLAGS}\e[0m"

###########################
# Build buildsystem tools #
###########################

cd ${PROJECTPATH}/emotion

#############
# BOOTSTRAP #
#############

if [ ! -f configure ]; then
    echo -e "\e[1m\e[32mBootstraping\e[0m"
    NOCONFIGURE=1 ./autogen.sh
    checkfail "emotion: bootstrap failed"
fi

#####################
# FAKE EMOTION DEPS #
#####################

EMOTION_BUILD_DIR=${PROJECTPATH}/emotion/build-tizen-${TARGET_TUPLE}
EMOTION_PREFIX=${EMOTION_BUILD_DIR}/prefix
EFL_INCLUDES="${PROJECTPATH}/hacks/emotion/include"
EFL_LIB="${EMOTION_PREFIX}/lib"

mkdir -p $EFL_LIB

cd ${PROJECTPATH}/hacks/emotion

for symbols in *.symbols; do
    so_file="`basename $symbols .symbols`"
    c_file="`basename $so_file .so`.c"
    rm -f ${EMOTION_PREFIX}/$c_file ${EFL_LIB}/$so_file*
    for s in `cat $symbols`; do echo "void $s() {}" >> ${EMOTION_PREFIX}/$c_file; done
    ${CC} ${EMOTION_PREFIX}/$c_file -shared -o ${EFL_LIB}/$so_file.1.7.99 -fPIC
    ln -sf $so_file.1.7.99 ${EFL_LIB}/$so_file.1
done

#############
# CONFIGURE #
#############

cd $EMOTION_BUILD_DIR

if [ ! -e ./config.h -o "$RELEASE" = 1 ]; then
CPPFLAGS="$CPPFLAGS" \
CFLAGS="$CFLAGS ${EXTRA_CFLAGS}" \
CXXFLAGS="$CFLAGS ${EXTRA_CXXFLAGS}" \
LDFLAGS="$LDFLAGS" \
CC="${CROSS_COMPILE}gcc -fPIC --sysroot=${SYSROOT}" \
CXX="${CROSS_COMPILE}g++ -fPIC --sysroot=${SYSROOT} -D__cpp_static_assert=200410" \
NM="${CROSS_COMPILE}nm" \
STRIP="${CROSS_COMPILE}strip" \
RANLIB="${CROSS_COMPILE}ranlib" \
AR="${CROSS_COMPILE}ar" \
ECORE_X_CFLAGS="-I${EFL_INCLUDES}/ecore-1" \
ECORE_X_LIBS="-L${EFL_LIB} -lecore_x" \
ECORE_FB_CFLAGS="-I${EFL_INCLUDES}/ecore-1" \
ECORE_FB_LIBS="-L${EFL_LIB} -lecore_fb" \
EDJE_EXTERNAL_CFLAGS="-I${EFL_INCLUDES}/edje-1" \
EDJE_EXTERNAL_LIBS="-L${EFL_LIB} -ledje" \
LIBVLC_CFLAGS="-I${PROJECTPATH}/vlc/include" \
LIBVLC_LIBS="-L${PROJECTPATH}/lib -lvlc" \
EMOTION_CFLAGS="-I${EFL_INCLUDES}/ecore-1 -I${EFL_INCLUDES}/evas-1 \
-I${EFL_INCLUDES}/eet-1 -I${EFL_INCLUDES}/eina-1 -I${EFL_INCLUDES}/eina-1/eina \
${LIBVLC_CFLAGS}" \
EMOTION_LIBS="-L${EFL_LIB} -lecore -levas -leet -leina -lpthread ${LIBVLC_LIBS}" \
sh ../configure --host=$TARGET_TUPLE --build=x86_64-unknown-linux \
                ${EXTRA_PARAMS} ${EMOTION_CONFIGURE_ARGS} ${OPTS} \
                --prefix=${EMOTION_PREFIX}
checkfail "emotion: configure failed"
fi

############
# BUILDING #
############

echo -e "\e[1m\e[32mBuilding libemotion\e[0m"
make $MAKEFLAGS -C src install

checkfail "emotion: make failed"

cp -a ${EMOTION_PREFIX}/lib/libemotion.so* ${PROJECTPATH}/lib

cd ../../

if [ "$RELEASE" = 1 ]; then
    echo -e "\e[1m\e[32mStripping\e[0m"
    ${CROSS_COMPILE}strip ${PROJECTPATH}/lib/libemotion.so.1.7.99
    checkfail "stripping"
fi
