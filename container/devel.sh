#!/usr/bin/bash

CONTAINER_NAME="iodsim:devel"

function delete_all_containers {
    for container in $(podman ps -a | grep ${CONTAINER_NAME} | awk '{print $1}'); do
        podman rm $container
    done

    podman image rm ${CONTAINER_NAME}
}

function build {
    DOCKERFILE="devel.Dockerfile"
    if [ "$NEXT" == "1" ]; then
        DOCKERFILE="next.Dockerfile"
        echo -e " ##############################\n" \
                "#    Using next containter   #\n" \
                "# CUTTING-EDGE TECH INCOMING #\n" \
                "#        WEAR GLOVES!        #\n" \
                "##############################\n"
        sleep 1
    fi

    podman build -t $CONTAINER_NAME -f $DOCKERFILE .
}

function run {
    wd=$(realpath ${PWD}/../)
    # ":Z" is used for container access to the volume, granted by SELinux
    podman run -it -v $wd:/drone:Z $CONTAINER_NAME bash
}

case "$1" in
    "build")
    delete_all_containers
    build
    ;;
    "run")
    run
    ;;
    *)
    echo "Do you need help?"
    ;;
esac
