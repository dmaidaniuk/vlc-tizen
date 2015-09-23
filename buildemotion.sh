#!/bin/bash

#############
#  DEFINES  #
#############

GCCVER=4.8
GCCVERFULL=4.8.3
source ${TIZEN_SDK}/sdk.version

#############
# FUNCTIONS #
#############

checkfail()
{
    if [ ! $? -eq 0 ];then
        echo -e "\e[1m\e[31m$1\e[0m"
        exit 1
    fi
}

#############
# ARGUMENTS #
#############

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")

RELEASE=0
while [ $# -gt 0 ]; do
    case $1 in
        help|--help)
            echo "Use -a to set the ARCH"
            echo "Use --release to build in release mode"
            exit 1
            ;;
        a|-a)
            TIZEN_ABI=$2
            shift
            ;;
        release|--release)
            RELEASE=1
            ;;
    esac
    shift
done

if [ -z "$TIZEN_SDK" ]; then
    echo "Please set the TIZEN_SDK environment variable with its path."
    exit 1
fi

if [ -z "$TIZEN_ABI" ]; then
    echo "Please pass the TIZEN_ABI to the correct architecture, using
                ${SCRIPT} -a ARCH
    ARM:     armv7l
    X86:     x86_64"
    exit 1
fi

#########################
# FETCH EMOTION SOURCES #
#########################

if [ ! -d "${PROJECTPATH}/emotion" ]; then
    echo -e "\e[1m\e[32mEMOTION source not found, cloning\e[0m"
    git clone https://github.com/tguillem/tizen_emotion.git "${PROJECTPATH}/emotion"
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

#########
# FLAGS #
#########

# Set up ABI variables
if [ "${TIZEN_ABI}" = "x86" ] ; then
    TARGET_TUPLE="i386-linux-gnueabi"
    PATH_HOST="x86"
    PLATFORM_SHORT_ARCH="x86"
elif [ "${TIZEN_ABI}" = "armv7l" ] ; then
    TARGET_TUPLE="arm-linux-gnueabi"
    PATH_HOST=$TARGET_TUPLE
    HAVE_ARM=1
    PLATFORM_SHORT_ARCH="arm"
else
    echo -e "\e[1m\e[31mUnknown ABI: '${TIZEN_ABI}'\e[0m"
    exit 2
fi

SYSROOT=${TIZEN_SDK}/tools/${TARGET_TUPLE}-gcc-${GCCVER}
LIBEXEC=${TIZEN_SDK}/tools/${TARGET_TUPLE}-gcc-${GCCVER}/libexec/gcc/${TARGET_TUPLE}/${GCCVERFULL}
TIZEN_TOOLCHAIN_PATH=${SYSROOT}/bin

export PATH=${TIZEN_TOOLCHAIN_PATH}:${LIBEXEC}:${PATH}


###############
# DISPLAY ABI #
###############
echo -e "\e[1m\e[36mABI:        $TIZEN_ABI\e[0m"
echo -e "\e[1m\e[36mTIZEN SDK:  $TIZEN_SDK\e[0m"
echo -e "\e[1m\e[36mSYSROOT:    $SYSROOT\e[0m"
echo -e "\e[1m\e[36mPATH:       $PATH\e[0m"

# Make in //
if [ -z "$MAKEFLAGS" ]; then
    UNAMES=$(uname -s)
    MAKEFLAGS=
    if which nproc >/dev/null; then
        MAKEFLAGS=-j`nproc`
    elif [ "$UNAMES" == "Darwin" ] && which sysctl >/dev/null; then
        MAKEFLAGS=-j`sysctl -n machdep.cpu.thread_count`
    fi
fi

###########
# FIX SDK #
###########

LIBSTDCPP=${TIZEN_SDK}/tools/arm-linux-gnueabi-gcc-4.8/arm-linux-gnueabi/lib/libstdc++.la

if [ -f "${LIBSTDCPP}" ] ; then
    mv -f ${LIBSTDCPP} ${LIBSTDCPP}.bak
fi

##########
# CFLAGS #
##########

CFLAGS="${CFLAGS} -fstrict-aliasing -funsafe-math-optimizations"

EXTRA_CFLAGS="-fPIC"
# Setup CFLAGS per ABI
if [ "${TIZEN_ABI}" = "armv7l" ] ; then
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -march=armv7-a -mtune=cortex-a8 -mfpu=vfpv3-d16 -mthumb -mfloat-abi=softfp"
elif [ "${TIZEN_ABI}" = "x86" ] ; then
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -mtune=atom -msse3 -mfpmath=sse -m32"
fi

EXTRA_CFLAGS="${EXTRA_CFLAGS} -I${SYSROOT}/include/c++/${GCCVERFULL}"
EXTRA_CFLAGS="${EXTRA_CFLAGS} -I${SYSROOT}/include/c++/${GCCVERFULL}/${TARGET_TUPLE}"
EXTRA_CFLAGS="${EXTRA_CFLAGS} -I${TIZEN_SDK}/platforms/mobile-${TIZEN_SDK_VERSION}/rootstraps/mobile-${TIZEN_SDK_VERSION}-device.core/usr/include"

EXTRA_CXXFLAGS="-D__STDC_FORMAT_MACROS=1 -D__STDC_CONSTANT_MACROS=1 -D__STDC_LIMIT_MACROS=1"

CPPFLAGS="-I${SYSROOT}/include/c++/${GCCVERFULL} -I${SYSROOT}/include/c++/${GCCVERFULL}/${TARGET_TUPLE} -I${TIZEN_SDK}/platforms/mobile-${TIZEN_SDK_VERSION}/rootstraps/mobile-${TIZEN_SDK_VERSION}-device.core/usr/include"

#################
# Setup LDFLAGS #
#################

TIZEN_TOOLS=${TIZEN_SDK}/tools/arm-linux-gnueabi-gcc-${GCCVER}/bin/arm-linux-gnueabi-
CROSS_COMPILE=${TIZEN_TOOLS}
TIZEN_FLAGS="--sysroot=${TIZEN_SDK}/platforms/mobile-${TIZEN_SDK_VERSION}/rootstraps/mobile-${TIZEN_SDK_VERSION}-device.core"
export CC="${TIZEN_TOOLS}gcc ${TIZEN_FLAGS}"
export CXX="${TIZEN_TOOLS}g++ ${TIZEN_FLAGS}"
export AR="${TIZEN_TOOLS}ar"
export LD="${TIZEN_TOOLS}ld ${TIZEN_FLAGS}"

EXTRA_LDFLAGS="-L${TIZEN_SDK}/platforms/mobile-${TIZEN_SDK_VERSION}/rootstraps/mobile-${TIZEN_SDK_VERSION}-device.core/usr/lib"

LDFLAGS="${TIZEN_FLAGS} -Wl,-Bdynamic -Wl,--no-undefined -lrt"

if [ -n "$HAVE_ARM" ]; then
        LDFLAGS="${LDFLAGS} -Wl,--fix-cortex-a8"
fi

LDFLAGS="${LDFLAGS} -L${SYSROOT}/lib"

# Release or not?
if [ "$RELEASE" = 1 ]; then
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -DNDEBUG "
fi

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
EFL_INCLUDES="${PROJECTPATH}/emotion_utils/include"
EFL_LIB="${EMOTION_PREFIX}/prefix/lib"

mkdir -p $EFL_LIB

cd ${PROJECTPATH}/emotion_utils

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

echo -e "\e[1m\e[32mDone! You can now build the Tizen application.\e[0m"
