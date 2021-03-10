#!/usr/bin/bash
####
# Clone ns3 using convient git flags.
# Also setup symbolic links automatically to include IoD Sim modules
####

git clone                           \
  --branch=ns-3.33                  \
  --depth=1                         \
  git@gitlab.com:nsnam/ns-3-dev.git \
  ns3

ln -s ../../drone ns3/src/drone
ln -s ../../report ns3/src/report
