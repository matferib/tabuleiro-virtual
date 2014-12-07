#!/bin/bash

BASEDIR=`dirname $0`
cd ${BASEDIR}
export DYLD_FALLBACK_FRAMEWORK_PATH="${BASEDIR}:${DYLD_FALLBACK_FRAMEWORK_PATH}"
export DYLD_FALLBACK_LIBRARY_PATH="${BASEDIR}/lib:${DYLD_FALLBACK_LIBRARY_PATH}"
echo "lp:$DYLD_FALLBACK_LIBRARY_PATH fp:$DYLD_FALLBACK_FRAMEWORK_PATH"
./tabvirt &
# kill terminal window
#kill $(ps -p $(ps -p $PPID -o ppid=) -o ppid=)
osascript -e 'tell application "Terminal" to close (every window whose name contains "tabvirt.command")' &
