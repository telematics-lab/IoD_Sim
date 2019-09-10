Compile using Containers
========================

This page will guide you through building the source code using a container.
Containers are easy to share and can be considered as a reliable reference to
work on.

.. note::
    This guide refers on using `Docker <https://docs.docker.com/>`_ as the
    installed container platform. The following instructions are applicable to
    other platforms, as long as they are `OCI
    <https://www.opencontainers.org/>`_ compatible, such as `podman
    <https://podman.io/>`_ or `cri-o <https://github.com/cri-o/cri-o>`_ for
    Kubernetes.


Prerequisites
-------------
This software does not request any further prerequisites than the ones
suggested by your container platform of reference. Please refer to their
documentation for such information.


Build the image
---------------

Be sure that Docker service is running. By placing yourself in ``container/``
directory, you can start building::

    $  ./Make-Tarball.sh   # if you are using Bash
    >  ./Make-Tarball.ps1  # if you are using PowerShell 

    $  docker build -t iodsim:latest .

``Make-Tarball`` is a script that compresses local source code in order to be
deployed and compiled on the container. This source code will be compiled
against the most recent ns-3 revision by cloning it from git repository. This
process is done automatically.


Container usage
---------------

``waf`` has been set as the container entrypoint. This means that a ``docker
run iodsim`` will automatically call ``waf``, not the shell. For this reason,
you are subject to *waf* command line interface, on which you can call your
scenario with ``docker run iodsim:devel --run my-test-scenario`` or invoke
*waf* shell using ``docker run iodsim shell``.
