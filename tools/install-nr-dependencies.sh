#!/usr/bin/env bash

if [ -z "$ID" ]; then
  if [ -r /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
  elif command -v lsb_release >/dev/null 2>&1; then
    NAME="$(lsb_release -ds 2>/dev/null | tr -d '"')"
    ID="$(lsb_release -is 2>/dev/null | tr '[:upper:]' '[:lower:]')"
  else
    NAME="$(uname -s)"
    ID="$(uname -s | tr '[:upper:]' '[:lower:]')"
    echo "Warning: /etc/os-release not found and lsb_release unavailable; assuming NAME=${NAME}, ID=${ID}"
  fi
fi

function install_debian_deps() {
  sudo apt update \
  && sudo apt install -y --no-install-recommends \
    libc6-dev      \
    sqlite         \
    sqlite3        \
    libsqlite3-dev \
    libeigen3-dev  \
    pybind11-dev
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
