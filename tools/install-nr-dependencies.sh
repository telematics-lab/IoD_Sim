#!/usr/bin/env bash

if [ -z "$ID" ]; then
  source /etc/os-release
fi

function install_debian_deps() {
  sudo apt update \
  && sudo apt install -y --no-install-recommends \
    libc6-dev      \
    sqlite         \
    sqlite3        \
    libsqlite3-dev \
    libeigen3-dev
}


case "$ID" in
  debian | ubuntu | linuxmint)
    install_debian_deps
    ;;
  *)
    echo "${NAME} is not supported by this script. Please, install IoD Sim dependencies manually by following the official guide."
    exit 1
    ;;
esac
