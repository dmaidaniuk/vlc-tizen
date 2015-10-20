#!/bin/bash

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")
source ${PROJECTPATH}/buildcommon

##############################
# FETCH MEDIALIBRARY SOURCES #
##############################

if [ ! -d "${PROJECTPATH}/medialibrary" ]; then
    echo -e "\e[1m\e[32mmedialibrary source not found, cloning\e[0m"
    git clone http://github.com/chouquette/medialibrary.git "${PROJECTPATH}/medialibrary"
    checkfail "medialibrary source: git clone failed"
#else
#    ( cd ${PROJECTPATH}/medialibrary && git pull --rebase ) || echo "Failed to update medialibrary"
fi

if [ ! -d "${PROJECTPATH}/libvlcpp" ]; then
    echo -e "\e[1m\e[32mlibvlcpp source not found, cloning\e[0m"
    git clone http://code.videolan.org/videolan/libvlcpp.git "${PROJECTPATH}/libvlcpp"
    checkfail "libvlcpp source: git clone failed"
fi

echo -e "\e[1m\e[36mCFLAGS:            ${CFLAGS}\e[0m"
echo -e "\e[1m\e[36mEXTRA_CFLAGS:      ${EXTRA_CFLAGS}\e[0m"

#################
# Setup folders #
#################

MEDIALIBRARY_BUILD_DIR=${PROJECTPATH}/medialibrary/build-tizen-${TARGET_TUPLE}
mkdir -p ${MEDIALIBRARY_BUILD_DIR}

#############
# CONFIGURE #
#############

cd ${MEDIALIBRARY_BUILD_DIR}

CMAKE_OPTS="-DCMAKE_BUILD_TYPE=Debug"
if [ "$RELEASE" = 1 ]; then
    CMAKE_OPTS=""
fi

if [ ! -e ./Makefile -o "$RELEASE" = 1 -o ../CMakeLists.txt -nt ./Makefile ]; then
CPPFLAGS="$CPPFLAGS" \
CFLAGS="$CFLAGS ${EXTRA_CFLAGS}" \
CXXFLAGS="$CFLAGS ${EXTRA_CXXFLAGS} -pthread" \
LDFLAGS="$LDFLAGS -static-libstdc++" \
CC="${CROSS_COMPILE}gcc -fPIC --sysroot=${SYSROOT}" \
CXX="${CROSS_COMPILE}g++ -fPIC --sysroot=${SYSROOT} -D__cpp_static_assert=200410" \
NM="${CROSS_COMPILE}nm" \
STRIP="${CROSS_COMPILE}strip" \
RANLIB="${CROSS_COMPILE}ranlib" \
AR="${CROSS_COMPILE}ar" \
cmake \
    ${CMAKE_OPTS} \
    -DBUILD_TESTS=OFF \
    -DLIBVLCPP_DIR="${PROJECTPATH}/libvlcpp" \
    -DLIBVLC_INCLUDE_DIR="${PROJECTPATH}/vlc/include;${PROJECTPATH}/vlc/build-tizen-${TARGET_TUPLE}/include" \
    -DLIBVLC_LIBRARY="${PROJECTPATH}/lib/libvlc.so" \
    -DSQLITE3_INCLUDE_DIR="${TIZEN_INCLUDES}/" \
    -DSQLITE3_LIBRARY_DEBUG="${TIZEN_LIBS}/libsqlite3.so" \
    -DSQLITE3_LIBRARY_RELEASE="${TIZEN_LIBS}/libsqlite3.so" \
    -DJPEG_INCLUDE_DIR="${PROJECTPATH}/vlc/contrib/${TARGET_TUPLE}/include" \
    -DJPEG_LIBRARY="${PROJECTPATH}/vlc/contrib/${TARGET_TUPLE}/lib/libjpeg.a" \
    -DEXTRA_LIBS="${SYSROOT}/${TARGET_TUPLE}/lib/libatomic.a" \
    ..
checkfail "medialibrary: cmake failed"
fi

############
# BUILDING #
############

echo -e "\e[1m\e[32mBuilding medialibrary\e[0m"
make $MAKEFLAGS 

checkfail "medialibrary: make failed"

cp -a ${MEDIALIBRARY_BUILD_DIR}/src/libmedialibrary.so* ${PROJECTPATH}/lib/

cd ../../

if [ "$RELEASE" = 1 ]; then
    echo -e "\e[1m\e[32mStripping\e[0m"
    ${CROSS_COMPILE}strip ${PROJECTPATH}/lib/libmedialibrary.so*
    checkfail "stripping"
fi
