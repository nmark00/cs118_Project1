#!/bin/bash
set -e

export DEBIAN_FRONTEND=noninteractive
apt-get update 
apt-get install --yes --no-install-recommends \
  autoconf \
  build-essential \
  sudo \
  wget \
  curl \
  git-all