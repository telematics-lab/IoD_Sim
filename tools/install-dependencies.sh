#!/usr/bin/env bash

# Cd on the root IoD_Sim directory
cd $(dirname "$0")/..

if [ -z "$ID" ]; then
  source /etc/os-release
fi

function install_debian_deps() {
  sudo apt update \
  && sudo apt install -y --no-install-recommends \
    clang-format      \
    cmake             \
    g++               \
    gdb               \
    gcc               \
    git               \
    make              \
    libgsl-dev        \
    libxml2-dev       \
    patch             \
    pkg-config        \
    python3           \
    python3-venv      \
    python3-pip       \
    python-is-python3 \
    libyyjson-dev     \
    libfmt-dev        \
    libtar-dev        \
    doxygen           \
    graphviz          \
    libtar-dev
}

function install_fedora_deps() {
  sudo dnf -y install          \
    clang-tools-extra          \
    cmake                      \
    gdb                        \
    gcc                        \
    gcc-c++                    \
    git                        \
    gsl-devel                  \
    libxml2-devel              \
    make                       \
    patch                      \
    pkgconf                    \
    python-unversioned-command \
    python3                    \
    yyjson-devel               \
    fmt-devel                  \
    libtar-devel               \
    doxygen                    \
    graphviz                   \
    libtar-devel
}

function install_arch_deps() {
  sudo pacman --noconfirm \
              -S --needed \
              -yu         \
    clang                 \
    cmake                 \
    gdb                   \
    gcc                   \
    git                   \
    gsl                   \
    make                  \
    libxml2               \
    patch                 \
    pkgconf               \
    python                \
    yyjson                \
    fmt                   \
    libtar                \
    doxygen-docs          \
    graphviz              \
    libtar
}

case "$ID" in
  debian | ubuntu | linuxmint)
    install_debian_deps
    ;;
  fedora)
    install_fedora_deps
    ;;
  arch)
    install_arch_deps
    ;;
  *)
    echo "${NAME} is not supported by this script. Please, install IoD Sim dependencies manually by following the official guide."
    exit 1
    ;;
esac
