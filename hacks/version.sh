#!/bin/sh

BASEDIR=$(dirname $0)
REVISION=`git describe --always --dirty --tags`

cat >${BASEDIR}/../inc/version.h <<EOL
#ifndef VERSION_H_
#define VERSION_H_

#define REVISION "${REVISION}"

#endif
EOL
