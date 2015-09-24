#!/bin/bash

SCRIPT=$(readlink -f "$0")
PROJECTPATH=$(dirname "$SCRIPT")
source ${PROJECTPATH}/buildcommon

${PROJECTPATH}/buildvlc.sh $*
checkfail "buildvlc.sh failed"

${PROJECTPATH}/buildemotion.sh $*
checkfail "buildemotion.sh failed"

echo -e "\e[1m\e[32mDone! You can now build the Tizen application.\e[0m"
