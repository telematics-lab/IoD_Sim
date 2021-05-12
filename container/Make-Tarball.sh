#!/bin/bash

function check_cwd {
    stat Dockerfile &> /dev/null  # stable unique reference on container/
    if [ ! $? -eq 0 ]; then
        echo "Please run this script in container/"
        exit 1
    fi
}

function check_tar {
    which tar &> /dev/null
    if [ ! $? -eq 0 ]; then
        echo "tar is not available on your operating system. Please install it."
        exit 1
    fi
}

function check_snapshot {
    stat SNAPSHOT.tar &> /dev/null
    if [ $? -eq 0 ]; then
        rm SNAPSHOT.tar
    fi
}

function make_snapshot {
    cd ../
    tar -cf ../SNAPSHOT.tar *
    mv ../SNAPSHOT.tar container/
    cd container/
}

check_cwd
check_tar
check_snapshot
make_snapshot
