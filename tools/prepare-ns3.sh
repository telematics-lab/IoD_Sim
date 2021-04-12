#!/usr/bin/bash
####
# Clone ns3 using convient git flags.
# Also setup symbolic links automatically to include IoD Sim modules
####

git clone                                \
  --branch=ns-3.33                       \
  --depth=1                              \
  https://gitlab.com/nsnam/ns-3-dev.git/ \
  ns-3-33

ln -s ns-3-33 ns3
cd ns3/src
ln -s ../../drone drone
ln -s ../../report report
cd ../../
