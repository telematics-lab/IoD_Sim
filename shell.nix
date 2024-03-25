# Nix shells are used to create a confined development environment, with
# everything needed to develop or build the software.
#
# To create the shell, run `nix-shell ./shell.nix`.
#
# This command will place you in a development shell with all the
# following packages at your disposal.
#
# More info:
#  - What is NixOS: https://nixos.org/
#  - What is nix-shell: https://nixos.org/manual/nix/stable/command-ref/nix-shell.html
#

{ pkgs ? import <nixpkgs> {} }:
let
  pythonEnv = pkgs.python3.withPackages(p: with p; [
    #kaleido
    ipykernel
    jstyleson
    matplotlib
    nbformat
    numpy
    pandas
    plotly
    #plyfile
    scipy
  ]);
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs.buildPackages; [
    cmake
    gcc13
    gdb
    git
    gnupatch
    gsl
    libclang
    libxml2
    ninja
    pkg-config
    pythonEnv
    rapidjson
    valgrind
  ];

  shellHook = ''
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/ns3/build/lib
  '';
}
