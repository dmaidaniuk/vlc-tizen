#!/bin/bash

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")

if [ "$BUILDCOMMONDONE" != 1 ]; then
    source ${PROJECTPATH}/buildcommon
fi

##############################
# FETCH MEDIALIBRARY SOURCES #
##############################

if [ ! -d "${PROJECTPATH}/medialibrary" ]; then
    echo -e "\e[1m\e[32mmedialibrary source not found, cloning\e[0m"
    git clone http://code.videolan.org/videolan/medialibrary.git "${PROJECTPATH}/medialibrary"
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

if [ "$RELEASE" = 1 ]; then
    MEDIALIBRARY_BUILD_DIR=${PROJECTPATH}/medialibrary/build-tizen-${TARGET_TUPLE}-release
else
    MEDIALIBRARY_BUILD_DIR=${PROJECTPATH}/medialibrary/build-tizen-${TARGET_TUPLE}
fi


mkdir -p ${MEDIALIBRARY_BUILD_DIR}

#############
# CONFIGURE #
#############

cd ${MEDIALIBRARY_BUILD_DIR}

CMAKE_OPTS="-DCMAKE_BUILD_TYPE=Debug"
if [ "$RELEASE" = 1 ]; then
    CMAKE_OPTS=""
fi

if [ ${TIZEN_SDK_VERSION} = "2.3.1" ];then
    PKG_CONFIG_PATH="${TIZEN_SDK}/tools/efl-tools/lib/pkgconfig/"
else
    PKG_CONFIG_PATH="${TIZEN_SDK}/platforms/tizen-${TIZEN_SDK_SHORT_VERSION}/common/efl-tool/efl-tools/lib/pkgconfig/"
fi

# PKG_CONFIG_PATH is required to have a proper evas detection.
# however, since the .pc the SDK provides are broken, we override with
# our own values
if [ ! -e ./Makefile -o "$RELEASE" = 1 -o ../CMakeLists.txt -nt ./Makefile ]; then
CPPFLAGS="$CPPFLAGS" \
CFLAGS="$CFLAGS ${EXTRA_CFLAGS}" \
CXXFLAGS="$CFLAGS ${EXTRA_CXXFLAGS} -pthread" \
LDFLAGS="$LDFLAGS -static-libstdc++" \
CC="${CROSS_COMPILE}gcc -fPIC --sysroot=${SYSROOT}" \
CXX="${CROSS_COMPILE}g++ -fPIC --sysroot=${SYSROOT} -D__cpp_static_assert=200410 -DTIZEN" \
NM="${CROSS_COMPILE}nm" \
STRIP="${CROSS_COMPILE}strip" \
RANLIB="${CROSS_COMPILE}ranlib" \
PKG_CONFIG_PATH="${PKG_CONFIG_PATH}" \
AR="${CROSS_COMPILE}ar" \
cmake \
    ${CMAKE_OPTS} \
    -DBUILD_TESTS=OFF \
    -DLIBVLCPP_DIR="${PROJECTPATH}/libvlcpp" \
    -DLIBVLC_INCLUDE_DIR="${PROJECTPATH}/vlc/include;${PROJECTPATH}/vlc/${VLC_BUILD_DIR}/include" \
    -DLIBVLC_LIBRARY="${PROJECTPATH}/lib/libvlc.so" \
    -DSQLITE3_INCLUDE_DIR="${TIZEN_INCLUDES}/" \
    -DSQLITE3_LIBRARY_DEBUG="${TIZEN_LIBS}/libsqlite3.so" \
    -DSQLITE3_LIBRARY_RELEASE="${TIZEN_LIBS}/libsqlite3.so" \
    -DEXTRA_LIBS="${SYSROOT}/${TARGET_TUPLE}/lib/libatomic.a" \
    -DEVAS_INCLUDE_DIRS="${TIZEN_INCLUDES}/ecore-evas-1;${TIZEN_INCLUDES}/evas-1;${TIZEN_INCLUDES}/efl-1;${TIZEN_INCLUDES}/eina-1;${TIZEN_INCLUDES}/eina-1/eina;${TIZEN_INCLUDES}/eo-1;${TIZEN_INCLUDES}/emile-1" \
    -DEVAS_LIBRARIES="-L${TIZEN_LIBS} -levas -lecore_evas" \
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
