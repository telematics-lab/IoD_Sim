FROM ubuntu:20.04

ENV PYTHONUNBUFFERED=1
ENV DEBIAN_FRONTEND=noninteractive

ARG USERNAME=user
ARG USER_UID=1000
ARG USER_GID=$USER_UID

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
        patch \
        ssh \
        sudo \
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
        libxml2-dev \
    # User setup allows devcontainer to use the same UID as host linux user
    && groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
