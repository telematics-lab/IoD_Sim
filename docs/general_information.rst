General Information
-------------------

IoD_Sim is an open source and system-level simulator for the IoD (Internet of
Drones).

Developed on top of the well-known `ns-3 <https://www.nsnam.org/>`_ (Network
Simulator 3), it implements the key networking elements (drones, network access
points, and Zone Service Provider), a standard-compliant communication stack
based on the IEEE 802.11 technology, and scenarios with various mobility
models.

The source code presents the implementation of models and examples scripts,
also known as *scenarios*, that use such models to simulate a variable number
of drones that: 

* Move according to the mobility model associated to the reference scenario. 
* Exchange messages with network access points deployed on a generic and flat 
  3D space.


System Compatibility
~~~~~~~~~~~~~~~~~~~~

IoD_Sim is primarily compatible with **Linux**-based operating systems. You can
test it on macOS, although it is not planned a first-class support for it. For
macOS, Windows, or any other operating system, you are encouraged to use
`Docker <https://www.docker.com/>`_, `WSL
<https://docs.microsoft.com/en-us/windows/wsl/about>`_ or any other
(pseudo)virtualization platform that can provide you a stable Linux-based work
environment. You are welcome to provide new compatibility solutions any time.
