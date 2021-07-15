************************
Installation Walkthrough
************************

Before you begin
================
IoD_Sim is only compatible with **Linux**-based operating systems. To this end, ensure you have a Linux-based
working environment. This guide assumes you are using Ubuntu. Dependencies, software packages, and the overall
install methodology may differ between Linux distributions. If you already have a working Linux environment available,
you can skip this section.

WSL2
----
In case you are new to the Linux world and you are on Windows, it is highly recommended that you install Windows
Subsystem for Linux 2 (WSL2). WSL2 can be downloaded from `here <https://docs.microsoft.com/en-us/windows/wsl/about>`_.

Useful recommended software to interact with WSL2 is Windows Terminal, which can be downloaded from the
`Microsoft Store <https://www.microsoft.com/en-us/p/windows-terminal/9n0dx20hk701>`_.

VSCode Integration
------------------
Finally, this guide assumes that you will work on IoD Sim using Visual Studio Code, available
`here <https://code.visualstudio.com/>`_. IoD Sim integrates with VSCode to ease some repetitive and mechanical tasks,
such as source code compilation and scenario execution.

Docker Integration
------------------
IoD Sim also comes with Docker integration for development purposes. If you already know Docker, you can easily build
a safe working environment by following this mini-guide:

1. Install Docker and VSCode.
2. Clone IoD Sim source code with git on your Linux environment.
3. Open the IoD Sim directory with VSCode.
4. Install VSCode ``Remote - Containers`` extension.
5. Run ``Remote-Containers: Rebuild And Reopen In Container`` from VSCode Command Palette.

After this set of operations, you can skip this guide to the configuration and build of IoD Sim.


Required Software Packages
==========================
The following packages must be installed in order to compile and run IoD Sim. If you see the bash command prefixed with
a ``$``, it means that the command can be run as a user. In case of a ``#``, it means that you have to be the ``root`` user.
You can prefix the command with ``sudo`` to request root privileges temporarily. Furthermore, if a command line ends with
a ``\``, please continue the command by following the next line.

::

  # apt update
  # apt install --no-install-recommends -y \
      git gcc g++ gdb patch python3 python-is-python3 pkg-config rapidjson-dev libgsl-dev libxml2-dev


Clone IoD Sim
=============
The cloning process is a specific process of ``git`` software that allows distribute code sharing among engineers.
Once you have the *git clone URL* at hand, you can clone the remote repository on your Linux environment with the
following command. Then, you can start VSCode to jump to the next step in the guide::

  $ git clone <Paste_IoD_Sim_git_clone_URL_here>
  $ code IoD_Sim/


Configuration and Compilation
=============================
Once VSCode is started and the IoD Sim source code is visible in the project explorer column of its user interface, is
it possible that VSCode suggests some extensions to install. Please do so in order to allow VSCode integration with
IoD Sim.

When extension install has finished, you can proceed with configuration and compilation of IoD Sim.
In order to configure IoD Sim project, from VSCode Command Palette select ``Task: Run Task`` and then
``Integrate IoD Sim with ns3``.

After the integration completes, run the task ``configure``. This task emits an colored output. You can check yellow and
red text to evaluate source code configuration. Most yellow and red messages can be ignored, as they are not relevant
for IoD Sim. Please ensure that the output is green for the following checks:

* ``Checking for 'gcc' (C compiler)``
* ``Checking for 'g++' (C++ compiler)``
* ``Checking for program 'pkg-config'``
* ``Checking for 'libxml-2.0 >= 2.7'``
* ``Checking for 'gsl'``
* ``Examples``
* ``GNU Scientific Library (GSL)``

If the ``configure`` task was executed successfully, it is now time to run ``build`` task.

Run a Scenario to Verify the Compilation
========================================
To check if the compilation went without problems, you can execute the task ``run IoD Sim``. Simulation output files will
be stored in the ``results/`` directory.

Conclusions
===========
You now have a working environment. Enjoy IoD Sim!
