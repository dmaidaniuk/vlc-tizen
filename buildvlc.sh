#!/bin/bash

VLC_TESTED_HASH=0deb1ed

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")
source ${PROJECTPATH}/buildcommon

# atomics are broken for x86 on 2.3, dirty hacks since it's used only from emulator
if [ ${TIZEN_SDK_VERSION} = "2.3.1" -a "${TIZEN_ABI}" = "x86" ];then
VLC_INCLUDE_HACKS=" -I${PROJECTPATH}/hacks/vlc/2.3-x86-include"
else
VLC_INCLUDE_HACKS=""
fi

#####################
# FETCH VLC SOURCES #
#####################

if [ ! -d "${PROJECTPATH}/vlc" ]; then
    echo -e "\e[1m\e[32mVLC source not found, cloning\e[0m"
    git clone git://git.videolan.org/vlc.git "${PROJECTPATH}/vlc"
    checkfail "vlc source: git clone failed"
else
    echo -e "\e[1m\e[32mVLC source found\e[0m"
    cd "${PROJECTPATH}/vlc"
    if ! git cat-file -e ${VLC_TESTED_HASH}; then
        cat << EOF
***
*** Error: Your vlc checkout does not contain the latest tested commit: ${VLC_TESTED_HASH}
***
EOF
        exit 1
    fi
    cd ..
fi

###########################
# VLC BOOTSTRAP ARGUMENTS #
###########################

VLC_BOOTSTRAP_ARGS="\
    --disable-disc \
    --disable-sout \
    --enable-dvdread \
    --enable-dvdnav \
    --disable-dca \
    --disable-goom \
    --disable-chromaprint \
    --disable-lua \
    --disable-schroedinger \
    --disable-sdl \
    --disable-SDL_image \
    --disable-fontconfig \
    --enable-zvbi \
    --disable-kate \
    --disable-caca \
    --disable-gettext \
    --disable-mpcdec \
    --enable-upnp \
    --disable-gme \
    --disable-tremor \
    --enable-vorbis \
    --disable-sidplay2 \
    --disable-samplerate \
    --disable-faad2 \
    --enable-harfbuzz \
    --enable-iconv \
    --disable-aribb24 \
    --disable-aribb25 \
    --disable-mpg123 \
    --disable-libdsm \
    --enable-libarchive \
    --enable-libtasn1 \
"

###########################
# VLC CONFIGURE ARGUMENTS #
###########################

VLC_CONFIGURE_ARGS="\
    --disable-nls \
    --enable-live555 --enable-realrtsp \
    --enable-avformat \
    --enable-swscale \
    --enable-avcodec \
    --enable-opus \
    --enable-opensles \
    --enable-mkv \
    --enable-taglib \
    --enable-dvbpsi \
    --disable-vlc \
    --disable-shared \
    --disable-update-check \
    --disable-vlm \
    --disable-dbus \
    --disable-lua \
    --disable-vcd \
    --disable-v4l2 \
    --disable-gnomevfs \
    --enable-dvdread \
    --enable-dvdnav \
    --disable-bluray \
    --disable-linsys \
    --disable-decklink \
    --disable-libva \
    --disable-dv1394 \
    --enable-mod \
    --disable-sid \
    --disable-gme \
    --disable-tremor \
    --disable-mad \
    --disable-dca \
    --disable-sdl-image \
    --enable-zvbi \
    --disable-fluidsynth \
    --disable-jack \
    --disable-pulse \
    --disable-alsa \
    --disable-samplerate \
    --disable-sdl \
    --disable-xcb \
    --disable-atmo \
    --disable-qt \
    --disable-skins2 \
    --disable-mtp \
    --disable-notify \
    --enable-libass \
    --disable-svg \
    --disable-udev \
    --enable-libxml2 \
    --disable-caca \
    --disable-glx \
    --disable-egl \
    --disable-gles2 \
    --disable-goom \
    --disable-projectm \
    --disable-sout \
    --enable-vorbis \
    --disable-faad \
    --disable-x264 \
    --disable-schroedinger \
    --enable-evas \
"

########################
# VLC MODULE BLACKLIST #
########################

VLC_MODULE_BLACKLIST="
    addons.*
    stats
    access_(bd|shm|imem)
    oldrc
    real
    hotkeys
    gestures
    sap
    dynamicoverlay
    rss
    ball
    audiobargraph_[av]
    clone
    mosaic
    osdmenu
    puzzle
    mediadirs
    t140
    ripple
    motion
    sharpen
    grain
    posterize
    mirror
    wall
    scene
    blendbench
    psychedelic
    alphamask
    netsync
    audioscrobbler
    motiondetect
    motionblur
    export
    smf
    podcast
    bluescreen
    erase
    stream_filter_record
    speex_resampler
    remoteosd
    magnify
    gradient
    dtstofloat32
    logger
    visual
    fb
    aout_file
    yuv
    .dummy
"

# Release or not?
if [ "$RELEASE" = 1 ]; then
    OPTS=""
else
    OPTS="--enable-debug"
fi

cd ${PROJECTPATH}/vlc

###########################
# Build buildsystem tools #
###########################

export PATH=${PROJECTPATH}/vlc/extras/tools/build/bin:$PATH
echo -e "\e[1m\e[32mBuilding tools\e[0m"
cd extras/tools
./bootstrap
checkfail "buildsystem tools: bootstrap failed"
make $MAKEFLAGS
checkfail "buildsystem tools: make"
cd ../..

#############
# BOOTSTRAP #
#############

if [ ! -f configure ]; then
    echo -e "\e[1m\e[32mBootstraping\e[0m"
    ./bootstrap
    checkfail "vlc: bootstrap failed"
fi

############
# Contribs #
############

echo -e "\e[1m\e[32mBuilding the contribs\e[0m"
mkdir -p contrib/contrib-tizen-${TARGET_TUPLE}

gen_pc_file() {
    echo "Generating $1 pkg-config file"
    echo "Name: $1
Description: $1
Version: $2
Libs: -l$1
Cflags:" > contrib/${TARGET_TUPLE}/lib/pkgconfig/`echo $1|tr 'A-Z' 'a-z'`.pc
}

mkdir -p contrib/${TARGET_TUPLE}/lib/pkgconfig
gen_pc_file EGL 1.1
gen_pc_file GLESv2 2

cd contrib/contrib-tizen-${TARGET_TUPLE}

TIZEN_ABI=${TIZEN_ABI} TIZEN_API=${TIZEN_API} \
    ../bootstrap --host=${TARGET_TUPLE} --build=x86_64-linux-gnu ${VLC_BOOTSTRAP_ARGS}
checkfail "contribs: bootstrap failed"

# TODO: mpeg2, theora

# Some libraries have arm assembly which won't build in thumb mode
# We append -marm to the CFLAGS of these libs to disable thumb mode
[ ${TIZEN_ABI} = "armv7l" ] && echo "NOTHUMB := -marm" >> config.mak

echo "EXTRA_CFLAGS= -g ${EXTRA_CFLAGS}" >> config.mak
echo "EXTRA_LDFLAGS= ${EXTRA_LDFLAGS}" >> config.mak

make fetch
checkfail "contribs: make fetch failed"

# gettext
which autopoint >/dev/null || make $MAKEFLAGS .gettext
# export the PATH
export PATH="$PATH:$PWD/../$TARGET_TUPLE/bin"
# Make
make $MAKEFLAGS
checkfail "contribs: make failed"

cd ../../

###################
# BUILD DIRECTORY #
###################

VLC_BUILD_DIR=build-tizen-${TARGET_TUPLE}
mkdir -p $VLC_BUILD_DIR && cd $VLC_BUILD_DIR

#############
# CONFIGURE #
#############

if [ ! -e ./config.h -o "$RELEASE" = 1 ]; then
CPPFLAGS="$CPPFLAGS" \
CFLAGS="$CFLAGS ${EXTRA_CFLAGS}" \
CXXFLAGS="$CFLAGS ${EXTRA_CXXFLAGS}" \
LDFLAGS="$LDFLAGS" \
CC="${CROSS_COMPILE}gcc -fPIC --sysroot=${SYSROOT}${VLC_INCLUDE_HACKS}" \
CXX="${CROSS_COMPILE}g++ -fPIC --sysroot=${SYSROOT} -D__cpp_static_assert=200410" \
NM="${CROSS_COMPILE}nm" \
STRIP="${CROSS_COMPILE}strip" \
RANLIB="${CROSS_COMPILE}ranlib" \
AR="${CROSS_COMPILE}ar" \
PKG_CONFIG_LIBDIR=../contrib/$TARGET_TUPLE/lib/pkgconfig \
EVAS_CFLAGS="-I${TIZEN_INCLUDES} -I${TIZEN_INCLUDES}/evas-1 \
             -I${TIZEN_INCLUDES}/ecore-1 \
             -I${TIZEN_INCLUDES}/eina-1 -I${TIZEN_INCLUDES}/eina-1/eina" \
EVAS_LIBS="-L${TIZEN_LIBS}" \
sh ../configure --host=$TARGET_TUPLE --build=x86_64-unknown-linux \
                ${EXTRA_PARAMS} ${VLC_CONFIGURE_ARGS} ${OPTS}
checkfail "vlc: configure failed"
fi

############
# BUILDING #
############

echo -e "\e[1m\e[32mBuilding libvlc\e[0m"
make $MAKEFLAGS
checkfail "vlc: make failed"

cd ../../

##################
# libVLC modules #
##################

REDEFINED_VLC_MODULES_DIR=${PROJECTPATH}/vlc/.modules/${VLC_BUILD_DIR}
rm -rf ${REDEFINED_VLC_MODULES_DIR}
mkdir -p ${REDEFINED_VLC_MODULES_DIR}

echo -e "\e[1m\e[32mGenerating static module list\e[0m"
blacklist_regexp=
for i in ${VLC_MODULE_BLACKLIST}
do
    if [ -z "${blacklist_regexp}" ]
    then
        blacklist_regexp="${i}"
    else
        blacklist_regexp="${blacklist_regexp}|${i}"
    fi
done

find_modules()
{
    echo "`find $1 -name 'lib*plugin.a' | grep -vE "lib(${blacklist_regexp})_plugin.a" | tr '\n' ' '`"
}

get_symbol()
{
    echo "$1" | grep vlc_entry_$2|cut -d" " -f 3
}

VLC_MODULES=$(find_modules vlc/$VLC_BUILD_DIR/modules)
DEFINITION="";
BUILTINS="const void *vlc_static_modules[] = {\n";
for file in $VLC_MODULES; do
    outfile=${REDEFINED_VLC_MODULES_DIR}/`basename $file`
    name=`echo $file | sed 's/.*\.libs\/lib//' | sed 's/_plugin\.a//'`;
    symbols=$("${CROSS_COMPILE}nm" -g $file)

    # assure that all modules have differents symbol names
    entry=$(get_symbol "$symbols" _)
    copyright=$(get_symbol "$symbols" copyright)
    license=$(get_symbol "$symbols" license)
    cat <<EOF > ${REDEFINED_VLC_MODULES_DIR}/syms
AccessOpen AccessOpen__$name
AccessClose AccessClose__$name
StreamOpen StreamOpen__$name
StreamClose StreamClose__$name
DemuxOpen DemuxOpen__$name
DemuxClose DemuxClose__$name
OpenFilter OpenFilter__$name
CloseFilter CloseFilter__$name
Open Open__$name
Close Close__$name
$entry vlc_entry__$name
$copyright vlc_entry_copyright__$name
$license vlc_entry_license__$name
EOF
    ${CROSS_COMPILE}objcopy --redefine-syms ${REDEFINED_VLC_MODULES_DIR}/syms $file $outfile
    checkfail "objcopy failed"

    DEFINITION=$DEFINITION"int vlc_entry__$name (int (*)(void *, void *, int, ...), void *);\n";
    BUILTINS="$BUILTINS vlc_entry__$name,\n";
done;
BUILTINS="$BUILTINS 0\n};\n"; \
printf "/* Autogenerated from the list of modules */\n$DEFINITION\n$BUILTINS\n" > ${PROJECTPATH}/vlc/.modules/libvlc-modules.c
rm ${REDEFINED_VLC_MODULES_DIR}/syms

# Generating the .ver file like libvlc.so upstream
VER_FILE="vlc/$VLC_BUILD_DIR/lib/.libs/libvlc.ver"
echo "{ global:" > $VER_FILE
cat vlc/lib/libvlc.sym | sed -e "s/\(.*\)/\1;/" >> $VER_FILE
echo "local: *; };" >> $VER_FILE

##################
#  Linking VLC   #
##################

echo -e "\e[1m\e[32mLinking\e[0m"

${CC} -fPIC -rdynamic -shared \
    -Lvlc/contrib/${TARGET_TUPLE}/lib \
    -Wl,-soname -Wl,libvlc.so.5 -Wl,-version-script \
    -Wl,${PROJECTPATH}/$VER_FILE \
    -o ${PROJECTPATH}/lib/libvlc.so.5 \
    ${PROJECTPATH}/vlc/.modules/libvlc-modules.c \
    -Wl,--whole-archive \
    ${PROJECTPATH}/vlc/build-tizen-${TARGET_TUPLE}/lib/.libs/libvlc.a \
    -Wl,--no-whole-archive \
    ${PROJECTPATH}/vlc/.modules/build-tizen-${TARGET_TUPLE}/*.a \
    ${PROJECTPATH}/vlc/build-tizen-${TARGET_TUPLE}/src/.libs/libvlccore.a \
    ${PROJECTPATH}/vlc/build-tizen-${TARGET_TUPLE}/compat/.libs/libcompat.a \
    -Wl,--whole-archive \
    ${PROJECTPATH}/vlc/contrib/${TARGET_TUPLE}/lib/libiconv.a \
    -Wl,--no-whole-archive \
    -ldl -lz -lm \
    -ldvbpsi -lmatroska -lebml -ltag \
    -logg -lFLAC -ltheora -lvorbis \
    -lmpeg2 -la52 \
    -lavformat -lavcodec -lswscale -lavutil -lpostproc -lgsm -lopenjpeg \
    -lliveMedia -lUsageEnvironment -lBasicUsageEnvironment -lgroupsock \
    -lspeex -lspeexdsp \
    -lxml2 -lpng -lgnutls -lgcrypt -lgpg-error \
    -lnettle -lhogweed -lgmp \
    -lfreetype -liconv -lass -lfribidi -lopus -lharfbuzz \
    -ljpeg \
    -ldvdnav -ldvdread -ldvdcss \
    -ltasn1 \
    -lzvbi \
    -lssh2 \
    -lmodplug \
    -lupnp -lthreadutil -lixml \
    -larchive \
    -levas -lecore \
    ${EXTRA_LDFLAGS}

# Missing:f
# -lEGL -lGLESv2 -ldsm

checkfail "linker: libvlc.so"

cd ${PROJECTPATH}/lib/ && ln -sf libvlc.so.5 libvlc.so && cd -
