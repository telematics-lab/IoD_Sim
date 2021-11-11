#!/bin/bash

source /etc/os-release

function install_debian_deps() {
  sudo apt install -y --no-install-recommends \
    g++           \
    gdb           \
    gcc           \
    libgsl-dev    \
    libxml2-dev   \
    patch         \
    pkg-config    \
    python3       \
    rapidjson-dev
}

function install_fedora_deps() {
  sudo dnf -y install \
    gdb               \
    gcc               \
    gcc-c++           \
    gsl-devel         \
    libxml2-devel     \
    patch             \
    pkgconf           \
    python3           \
    rapidjson-devel
}

case "$ID" in
  debian)
    install_debian_deps
    ;;
  ubuntu)
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
