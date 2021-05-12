FROM ubuntu:18.04

ARG nproc=2

WORKDIR /opt/drone

ENV ARCHIVE_NAME SNAPSHOT.tar

####
# Machine Setup
####
RUN apt-get update        \ 
    && apt-get install -y \
    # general
    xz-utils              \
    # ns3 dependencies
    git                   \
    python3               \
    python3-pip           \
    pkg-config            \
    libgtk-3-dev          \
    libsqlite3-dev        \
    python3-gi-cairo      \
    python3-pygraphviz    \
    gir1.2-goocanvas-2.0  \
    valgrind              \
    gdb                   \
    libgsl-dev            \
    doxygen               \
    libopenmpi-dev        \
    # pybindgen dependencies
    castxml               \
    libboost-dev          \
    # drone module dependencies
    rapidjson-dev         \
    && pip3 install -U pip\
    && pip install -U     \
           pygccxml       \
    && mkdir third_party && cd ./third_party \
    && git clone https://github.com/gjcarneiro/pybindgen.git \
    && cd pybindgen       \
    && python3 setup.py install \
    && cd ../../       
   
####
# Simulator Setup
####
COPY ${ARCHIVE_NAME} .

RUN mkdir results \
    && mkdir -p src && cd src \
    && tar xf ../${ARCHIVE_NAME} \
    && cd ../ \
    && git clone https://gitlab.com/nsnam/ns-3-dev.git ns3 -b ns-3.29 \
    && cd ./ns3 \
    # patch ns3
    && ln -s ../../src/drone ./src/drone \
    # we will force the use of python3 because we won't install python 2.7
    && sed -i 's/python/python3/' ./waf \
    && ./waf configure --enable-examples --enable-mpi --disable-python \
    && ./waf build -j${nproc}

VOLUME /opt/drone

WORKDIR /opt/drone/ns3
ENTRYPOINT ["/opt/drone/ns3/waf"]
