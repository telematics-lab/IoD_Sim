#!/bin/bash
####
# Clone ns3 using convient git flags.
# Also setup symbolic links automatically to include IoD Sim modules
# run from IoD_Sim/ with ./tools/prepare-ns3-dual.sh
####

git clone                                \
  --branch=ns-3.29                       \
  --depth=1                              \
  https://gitlab.com/nsnam/ns-3-dev.git/ \
  ns-3-29

git clone                                \
  --branch=ns-3.31                       \
  --depth=1                              \
  https://gitlab.com/nsnam/ns-3-dev.git/ \
  ns-3-31

cd ns-3-29/src
ln -s ../../drone drone
ln -s ../../report report
cd ../../ns-3-31/src
ln -s ../../drone drone
ln -s ../../report report
cd ../../
ln -s ns-3-31 ns3
