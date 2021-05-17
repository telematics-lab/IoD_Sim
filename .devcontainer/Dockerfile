FROM ubuntu:20.04

ENV PYTHONUNBUFFERED=1
ENV DEBIAN_FRONTEND=noninteractive

####
# Machine Setup
####
RUN apt-get update \
    && apt-get install --no-install-recommends --quiet --assume-yes \
    git \
    gcc \
    g++ \
    gdb \
    valgrind \
    # ns3 dependencies
    python3 \
    python-is-python3 \
    python3-dev \
    python3-pip \
    python3-pybindgen \
    python3-pygccxml \
    pkg-config \
    sqlite3 \
    #libgtk-3-dev \
    #libsqlite3-dev \
    # pybindgen dependencies
    #libboost-dev \
    #castxml \
    # IoD_Sim dependencies
    rapidjson-dev \
    libgsl-dev \
    libxml2-dev
