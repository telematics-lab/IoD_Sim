# IoD_Sim

[IoD_Sim](https://telematics.poliba.it/iod-sim) is an open source and
system-level simulator for the IoD ([Internet of
Drones](https://uwspace.uwaterloo.ca/bitstream/handle/10012/12663/07423671.pdf?sequence=1&isAllowed=y)).

Developed on top of the well-known [ns-3](https://www.nsnam.org/) (Network
Simulator 3), it implements the key networking elements (drones, network access
points, and Zone Service Provider), a standard-compliant communication stack
based on the IEEE 802.11 technology, and scenarios with various mobility
models.

The source code presents the implementation of models and examples scripts,
also known as _scenarios_, that use such models to simulate a variable number
of drones that:
* Move according to the mobility model associated to the reference scenario.
* Exchange messages with network access points deployed on a generic and flat
  3D space.

The [IoD_Sim](https://telematics.poliba.it/iod-sim) repository is where this
software is developed and there are many ways in which you can participate in
the project, for example:
* [Submit bugs and feature requests](issues) and help us verify as they are
  checked in.
* Review [source code changes](pulls).
* Review the [documentation](doc) and make pull requests for anything, from
  typos to new content.

IoD_Sim is primarily compatible with **Linux**-based operating systems. You can
test it on macOS, altough we are not focused on providing first-class support
for it. For macOS, Windows, or any other operating system, you are encouraged
to use [Docker](https://www.docker.com/),
[WSL](https://docs.microsoft.com/en-us/windows/wsl/about) or any other
(pseudo)virtualization platform that can provide you a stable Linux-based work
environment. You are welcome to provide new compatibility solutions any time.


## Getting Started

This project is a module of the ns-3 software. To be able to compile, develop,
and use it, please [build](docs/) or
[read](https://telematics.poliba.it/iod-sim) the documentation.


## License

Copyright (C) 2019-2020 [The IoD_Sim Authors](AUTHORS).

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[GNU General Public License](LICENSE) for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
