FROM ubuntu:18.04

ARG nproc=2

WORKDIR /opt/drone

####
# Machine Setup
####
RUN apt-get update          \ 
    && apt-get install -y   \
    # general
    xz-utils                \
    # ns3 dependencies
    git                     \
    python3                 \
    python3-pip             \
    pkg-config              \
    libgtk-3-dev            \
    libsqlite3-dev          \
    python3-gi-cairo        \
    python3-pygraphviz      \
    gir1.2-goocanvas-2.0    \
    valgrind                \
    gdb                     \
    libgsl-dev              \
    doxygen                 \
    libopenmpi-dev          \
    # pybindgen dependencies
    castxml                 \
    libboost-dev            \
    # IoD_Sim dependencies
    rapidjson-dev           \
    && pip3 install -U pip  \
    && pip install -U       \
           pygccxml         \
           sphinx           \
           sphinx_rtd_theme \
    && mkdir third_party    \
    && cd ./third_party     \
    && git clone https://github.com/gjcarneiro/pybindgen.git \
    && cd pybindgen         \
    && python3 setup.py install      

RUN git clone https://gitlab.com/nsnam/ns-3-dev.git ns3 \
    && cd ./ns3 \
    # link directories
    && ln -s /workspaces/IoD_Sim/drone /opt/drone/ns3/src/drone \
    && ln -s /workspaces/IoD_Sim/results /opt/drone/results \
    # force the use of python3 so to not install python 2.7
    && sed -i 's/python/python3/' ./waf
