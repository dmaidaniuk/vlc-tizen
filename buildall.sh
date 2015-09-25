#!/bin/bash

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")
source ${PROJECTPATH}/buildcommon

${PROJECTPATH}/buildvlc.sh $*
checkfail "buildvlc.sh failed"

${PROJECTPATH}/buildemotion.sh $*
checkfail "buildemotion.sh failed"

cp ${PROJECTPATH}/.cproject-${TIZEN_ABI} ${PROJECTPATH}/.cproject

echo -e "\e[1m\e[32mDone! You can now build the Tizen application.\e[0m"
