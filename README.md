# IoD_Sim

[IoD_Sim](https://telematics.poliba.it/iod-sim) is an open source and
system-level simulator for the IoD ([Internet of
Drones](https://ieeexplore.ieee.org/document/7423671)).

Developed on top of the well-known [ns-3](https://www.nsnam.org/) (Network
Simulator 3), it implements a flexible simulation platform constisting on key
networking elements, i.e., drones, network access points, and
Zone Service Providers, peripherals, buildings, and an extensive configuration
interface for high-level and low-code scenario design.

The IoD_Sim repository is where this
software is developed and there are many ways in which you can participate in
the project, for example:
* [Submit bugs and feature requests](issues) and help us verify as they are
  checked in.
* Review [source code changes](pulls).
* Review the [documentation](doc) and make pull requests for anything, from
  typos to new content.

IoD_Sim is primarily compatible with **Linux**-based operating systems. You can
test it on macOS and Windows, although we are not focused on providing first-class support
for it.
For Windows you are encouraged to use [WSL2](https://docs.microsoft.com/en-us/windows/wsl/about).
For macOS it is sufficient to install [UTM](https://mac.getutm.app/), a
(pseudo)virtualization platform that can provide you a stable Linux-based work
environment. Please remember that on Apple processors you have to install a
64-bit ARM version of Linux distribution.
Alternatively, if you are used to container technology, [Docker](https://www.docker.com/)
can be used to create an image for IoD Sim.
You are welcome to provide new compatibility solutions any time.

For more details, a dedicated publication that describes the entire simulation
platform is available on [IEEE Internet of Things Journal](https://doi.org/10.1109/JIOT.2022.3207324), with its preprint freely downloadable from [arXiv](https://doi.org/10.48550/arXiv.2203.13710).

Want to build scenarios through a GUI? [Airflow](https://github.com/GiovanniGrieco/IoD_Sim-airflow) is a Visual Programming Editor ad-hoc for IoD_Sim! It relies on [splash](https://github.com/GiovanniGrieco/IoD_Sim-splash) to transpile C++ models in Python visual blocks.

## Getting Started

This project can be quick started if opened as folder on Visual Studio Code.
Tasks are already provided in sequence to:
* Install dependencies
* Download and integrate IoD Sim with third-party dependencies
* Build IoD Sim
* Execute IoD Sim scenarios

> Please note that the instructions provided hereby use the following notation. If a command line
> starts with a $, that means that it should be executed as a normal user.
> Otherwise, if a command is preceded by a #, that means that you should run
> the command under elevated privileges, e.g. root and/or sudo.

Given a Linux-based shell, ensure you have installed `git`. Then clone this
repository through the following command:
```
$ git clone https://github.com/telematics-lab/IoD_Sim.git
$ cd IoD_Sim/
```
If you have Visual Studio Code installed, just execute `code .` and follow the
aforementioned tasks. Otherwise, execute the following sequence of commands to
configure and build the project:
```
$ ./tools/install-dependencies.sh
$ ./tools/prepare-ns3.sh
$ cd ns3/
$ ./ns3 configure --enable-examples \
                  --disable-mpi     \
                  --disable-python  \
                  --enable-modules=drone
$ ./ns3 build
```
If build was successful, you can run a scenario through the following command:
```
$ ./ns3 run "iod-sim --config=../drone/config/simple_wifi.json"
```
You can replace the config file path with any other JSON that describes a scenario.
All the scenario data will be available under the `results/` directory.

## Cite as
Please cite this work in the following manner:
> G. Grieco, G. Iacovelli, P. Boccadoro and L. A. Grieco, "Internet of Drones Simulator: Design, Implementation, and Performance Evaluation," in IEEE Internet of Things Journal, vol. 10, no. 2, pp. 1476-1498, 15 Jan.15, 2023, doi: 10.1109/JIOT.2022.3207324.

or through BibTeX:
```
@ARTICLE{9893879,
  author={Grieco, Giovanni and Iacovelli, Giovanni and Boccadoro, Pietro and Grieco, Luigi Alfredo},
  journal={IEEE Internet of Things Journal}, 
  title={Internet of Drones Simulator: Design, Implementation, and Performance Evaluation}, 
  year={2023},
  volume={10},
  number={2},
  pages={1476-1498},
  doi={10.1109/JIOT.2022.3207324}
}
```

## License

Copyright (C) 2019-2022 [The IoD_Sim Authors](AUTHORS).

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
