#!/bin/bash


[ "" != "${TIZEN_SDK}" ] || TIZEN_SDK="${HOME}/tizen-sdk"
[ "" != "${TIZEN_ABI}" ] || TIZEN_ABI="armv7l"


SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")
. ${PROJECTPATH}/buildcommon

${PROJECTPATH}/buildvlc.sh $*
checkfail "buildvlc.sh failed"

${PROJECTPATH}/buildemotion.sh $*
checkfail "buildemotion.sh failed"

${PROJECTPATH}/buildmedialibrary.sh $*
checkfail "buildmedialibrary.sh failed"

if [ "$RELEASE" = 1 -o "$STRIP" = 1 ]; then
	echo -e "\e[1m\e[32mStripping\e[0m"
    ${CROSS_COMPILE}strip -v ${PROJECTPATH}/lib/*so
    checkfail "stripping"
fi

cp ${PROJECTPATH}/.cproject-${TIZEN_ABI} ${PROJECTPATH}/.cproject

hacks/version.sh

echo -e "\e[1m\e[32mDone! You can now build the Tizen application.\e[0m"
