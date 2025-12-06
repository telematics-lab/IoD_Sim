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
    libboost-all-dev  \
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
    libarchive-dev    \
    doxygen           \
    graphviz          \
    libc6-dev         \
    libsqlite3-dev    \
    libeigen3-dev     \
    pybind11-dev      \
    liblzma-dev       \
    xz-utils          \
    nettle-dev        \
    libbz2-dev        \
    liblz4-dev        \
    libzstd-dev       \
    libacl1-dev       \
    libxxhash-dev     \
    wget
}

function install_fedora_deps() {
  sudo dnf -y install          \
    clang-tools-extra          \
    cmake                      \
    gdb                        \
    gcc                        \
    gcc-c++                    \
    git                        \
    make                       \
    boost-devel                \
    gsl-devel                  \
    libxml2-devel              \
    patch                      \
    pkgconf                    \
    python-unversioned-command \
    python3                    \
    python3-devel              \
    python3-pip                \
    yyjson-devel               \
    fmt-devel                  \
    libarchive-devel           \
    doxygen                    \
    graphviz                   \
    sqlite-devel               \
    eigen3-devel               \
    pybind11-devel             \
    xz-devel                   \
    xz                         \
    nettle-devel               \
    bzip2-devel                \
    lz4-devel                  \
    libzstd-devel              \
    libacl-devel               \
    xxhash-devel               \
    wget
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
    make                  \
    boost                 \
    gsl                   \
    libxml2               \
    patch                 \
    pkgconf               \
    python                \
    python-pip            \
    yyjson                \
    fmt                   \
    libarchive            \
    doxygen               \
    graphviz              \
    sqlite                \
    eigen                 \
    pybind11              \
    xz                    \
    nettle                \
    bzip2                 \
    lz4                   \
    zstd                  \
    acl                   \
    xxhash                \
    wget
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
