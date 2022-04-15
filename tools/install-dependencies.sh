#!/bin/bash

source /etc/os-release

function install_debian_deps() {
  sudo apt update \
  && sudo apt install -y --no-install-recommends \
    g++           \
    gdb           \
    gcc           \
    git           \
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
    git               \
    gsl-devel         \
    libxml2-devel     \
    patch             \
    pkgconf           \
    python3           \
    rapidjson-devel
}

if [ ! -z "$(which dnf)" ]
then
install_fedora_deps
elif [ ! -z "$(which apt)" ]
then
install_debian_deps
else
echo "${NAME} is not supported by this script. Please, install IoD Sim dependencies manually by following the official guide."
exit 1
fi
