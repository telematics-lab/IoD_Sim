FROM ubuntu:25.04

ENV PYTHONUNBUFFERED=1
ENV DEBIAN_FRONTEND=noninteractive

####
# Machine setup and dependecies installation
####
RUN apt-get update \
    && apt-get install --no-install-recommends --quiet --assume-yes \
    git \
    ca-certificates \
    sudo

# Use remote version of the simulator
RUN git clone -b v4.1.0 https://github.com/telematics-lab/IoD_Sim.git

WORKDIR /IoD_Sim
RUN ./tools/install-dependencies.sh
RUN ./tools/prepare-ns3.sh
RUN ./tools/configure-iodsim.sh
WORKDIR ./ns3
RUN ./ns3 build -j$(grep -c ^processor /proc/cpuinfo)
WORKDIR ..

ENTRYPOINT ["/bin/bash","./tools/simulate.sh"]