#!/bin/bash
basedir="$(dirname "$(realpath "$0")")"
docker run --interactive --privileged -e DISPLAY \
             -p 8080:8080 \
             --tmpfs=/curr/${USER}:exec,mode=755,uid=$(id -u),gid=$(id -g) \
             --tty \
             --volume=${basedir}/setup.sh:/opt/docker-cs118/setup.sh:ro \
             --volume=${basedir}/:/project:rw \
             ubuntu:20.04
