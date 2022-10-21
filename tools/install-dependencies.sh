#!/bin/bash

if [ -z "$ID" ]; then
  source /etc/os-release
fi

function install_debian_deps() {
  sudo apt update \
  && sudo apt install -y --no-install-recommends \
    cmake         \
    g++           \
    gdb           \
    gcc           \
    git           \
    libgsl-dev    \
    libxml2-dev   \
    make          \
    patch         \
    pkg-config    \
    python3       \
    rapidjson-dev
}

function install_fedora_deps() {
  sudo dnf -y install \
    cmake             \
    gdb               \
    gcc               \
    gcc-c++           \
    git               \
    gsl-devel         \
    libxml2-devel     \
    make              \
    patch             \
    pkgconf           \
    python3           \
    rapidjson-devel
}

case "$ID" in
  debian | ubuntu)
    install_debian_deps
    ;;
  fedora)
    install_fedora_deps
    ;;
  *)
    echo "${NAME} is not supported by this script. Please, install IoD Sim dependencies manually by following the official guide."
    exit 1
    ;;
esac
