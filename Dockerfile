FROM ubuntu:22.04

ENV PYTHONUNBUFFERED=1
ENV DEBIAN_FRONTEND=noninteractive

####
# Machine setup and dependecies installation
####
RUN apt-get update \
    && apt-get install --no-install-recommends --quiet --assume-yes \
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
    rapidjson-dev     \
    doxygen           \
    graphviz

# Use local versione of the simulator

#RUN mkdir IoD_Sim
#COPY . ./IoD_Sim/

# Use remote version of the simulator
RUN git clone -b v4.0.2 https://github.com/telematics-lab/IoD_Sim.git

WORKDIR /IoD_Sim
RUN ./tools/prepare-ns3-docker.sh
RUN ./tools/configure-iodsim.sh
WORKDIR ./ns3
RUN ./ns3 build -j$(grep -c ^processor /proc/cpuinfo)
WORKDIR ..

ENTRYPOINT ["/bin/bash","./tools/simulate.sh"]