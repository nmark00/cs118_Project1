#!/bin/bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
WINDOWSPATH=$(echo $SCRIPTPATH | sed 's/^\///' | sed 's/\//\\/g' | sed 's/^./\0:/')

echo "Starting in $WINDOWSPATH"

winpty docker run --interactive --privileged -e DISPLAY \
             -p 8080:8080 \
             --tmpfs=/curr/${USER}:exec,mode=755,uid=$(id -u),gid=$(id -g) \
             --tty \
             --volume=$WINDOWSPATH/setup-windows.sh:/opt/docker-cs118/setup.sh:ro \
             --volume=$WINDOWSPATH:/project:rw \
             ubuntu:20.04
