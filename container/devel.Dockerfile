FROM ubuntu:18.04

WORKDIR /drone

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
    && mkdir /opt/third_party    \
    && cd /opt/third_party     \
    && git clone https://github.com/gjcarneiro/pybindgen.git \
    && cd pybindgen         \
    && python3 setup.py install
