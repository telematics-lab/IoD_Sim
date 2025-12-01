## Building IoD_Sim

If you want to build IoD_Sim for using it as a library, you can use the following commands in the root folder of the project:

```bash
# First install all the required dependencies
./tools/install-dependencies.sh && ./tools/install-nr-dependencies.sh
# These commands works with fedora, debian based distro and arch linux
# You can compile IoD_Sim also using mac-os but you need to skip
# this command and install dependencies manually

# Then we need to clone ns3 into IoD_Sim folder and apply the IoD_Sim patches
./tools/prepare-ns3.sh

# Now we can configure IoD_Sim in debug mode with:
./tools/configure-iodsim.sh

# now you can go on ns3 folder and build IoD_Sim
cd ns3 && ./ns3 build
```

After the execution of this command, we built all the IoD_Sim project and can find the binaries in the `ns3/build` folder.

In particular, the binary that allow us to run json scenarios is the `ns3/build/ns3.45-scenario-debug` executable.

Pay attention to the fact that the `ns3/build/ns3.45-scenario-default` executable is a dynamically built executable so the system in witch has to have all the dependencies installed.

If you wanto to use IoD_Sim compiled in default mode use these command instead:

```bash
./tools/install-dependencies.sh && ./tools/install-nr-dependencies.sh
./tools/prepare-ns3.sh
./tools/configure-iodsim.sh --mode default
cd ns3 && ./ns3 build
```

A particular note is that `release` mode is not supported yet.


In this case you can run any json scenario executing the `ns3/build/ns3.45-scenario-default` executable in this way:

```bash
./ns3/build/ns3.45-scenario-default --config=/path/to/scenario.json
# You can also use ./ns3 script using:
./ns3 run "scenario --config=/path/to/scenario.json"
# This will require python since ./ns3 is a python script
```

The result are described in the json scenario in the "resultsPath" key:
In the example you will file usually the path to "../results/", in this folder there will be a folder for every execution with this name format: "<scenario_name>-<date>.<time>" (e.g. test-trace-2025-11-27.20-34-52).
You can customize the results path in the json scenario in the "resultsPath" key.

If you want a static compiled binary usable on any system you can use the docker builder for this purpose:

```bash
cd tools/compile && docker compose up --build && docker compose rm -f -s -v
#After this command you will find the static binary in tools/compile/bin folder
```

You can use the static binary like with the binary before, but you will not need to install any dependencies on the system where you want to run the binary.