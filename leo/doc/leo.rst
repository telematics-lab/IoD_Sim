Low Earth Orbit Mobility
------------------------

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

This a maintained version of https://gitlab.ibr.cs.tu-bs.de/tschuber/ns-3-leo .
It was originally developed as a student master project and published as part of `ns-3-leo: Evaluation Tool for Satellite Swarm Communication Protocols <https://ieeexplore.ieee.org/document/9693958>`_.
You may also want to check out `Hypatia <https://github.com/snkas/hypatia>`_ which provides similar functionality, but is more mature software.

Model Description
*****************

The source code for the module lives in the directory ``contrib/leo``.

This module provides a mobility model for LEO satellites, propagation loss models for satellite to satellite and satellite to ground transmission.
It also includes a simplistic model of the inter-satellite and satellite-ground channels and the associated network devices.
It contains helpers to build a LEO satellite network based on configurable parameters or import mobility data from TLE files.


References
==========

See the project thesis report for more details.

Usage
*****

Interaction with the mobility model is mostly at configuration time using the
associated helpers. At simulation time, various parts of |ns3| may interact
with the mobility model and propagation loss models to obtain the position,
heading, speed of nodes and the path loss, including if a link can be
established between two nodes. 

Topologies may be constructed using ``MockDevice`` and ``MockChannel``.

Helpers
=======

Since the configuration of these large networks can be complicated, the helpers try to collect common
configutation steps, such as setting up the LEO and ISL channels and adding
devices with the correct mobility patterns based on the constellation.

Ground stations may be installed using the ``LeoGndStationHelper``.
It provides methods for installing ground station by their polar coordinates, a uniform grid of polar coordinates or it can also load a CSV file containing a list of such coordinates.
The following exaple configures a 20 by 20 grid and places a source inside Middle Europe and a sink on the east cost of North America.

.. sourcecode:: cpp

  LeoGndNodeHelper ground;
  NodeContainer stations = ground.Install (20, 20);

  LeoLatLong source (51.399, 10.536);
  LeoLatLong destination (40.76, -73.96);
  NodeContainer users = ground.Install (source, destination);
  stations.Add (users);

The satellite orbits can be configured using the ``LeoOrbitNodeHellper``.
It sets the positions of the satellites according to their orbit definitions, which can either be provided using a CSV file or directly.

.. sourcecode:: cpp

  LeoOrbitNodeHelper orbit;
  NodeContainer satellites;

  // using CSV file
  satellites = orbit.Install ("orits.csv");

  // defining orbits in code (height, inclination, number of planes, satellites per plane)
  satellites = orbit.Install ({ LeoOrbit (1200, 20, 32, 16),
                                LeoOrbit (1180, 30, 12, 10) });

Afterwards, the channels between the satellites and betweeen the ground stations and the satellites need to be configured.
This can be acchieved using the ``LeoChannelHelper`` and the ``IslChannelHelper``.

.. sourcecode:: cpp

  LeoChannelHelper utCh;
  utCh.SetConstellation ("StarlinkUser");
  NetDeviceContainer utNet = utCh.Install (satellites, stations);

  IslHelper islCh;
  islCh.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  NetDeviceContainer islNet = islCh.Install (satellites);

Afterwards, the ground stations should be connected to the satellites using a ``LeoMockChannel`` and the satellites should be connected to each other using ``IslMockChnnel``.
Please see their documentation to find additional parameters that can be configured using the helpers.

Output
======

The mobility of individual satellites can be traced using the respective trace sources of ``MobilityModel``.
To see what is going on inside the network, you may choose to configure trace sinks for the trace sources of network ``MockNetDevice``, ``IslPropagationLossModel``, ``LeoPropagationLossModel`` and ``MockChannel``.
Following example shows how to set up a trace sink to log dropped transmissions.

.. sourcecode:: cpp

  uint64_t countBytes = 0;
  static void
  TracePacket (std::string context, Ptr<const Packet> packet)
  {
    Ptr<Packet> p = packet->Copy ();
    std::cout << Simulator::Now () << ":" << context << ":" << p->GetUid () << ":" << (countBytes += p->GetSerializedSize ()) << std::endl;
  }

  int main (int argc, char *argv[])
  {
    // [...]
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/MacTxDrop",
                     MakeCallback (&TracePacket));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/PhyTxDrop",
      	           MakeCallback (&TracePacket));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/MacRxDrop",
      	           MakeCallback (&TracePacket));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/PhyRxDrop",
    		   MakeCallback (&TracePacket));
    // [...]
  }

It can also be quite useful to explore the network traffic using external tools like Wireshark.
PCAP output can be enabled on all network devices using the ``PcapHelper``

.. sourcecode:: cpp

  utCh.EnablePcapAll ("my-user-netdev", false);
  islCh.EnablePcapAll ("my-isl-netdev", false);

Examples
========

leo-circlular-orbit
###################

The program configures a ``LeoCircularOrbitMobilityModel`` and traces the positions of the satellites during the simulation.

.. sourcecode:: bash
  
  $ ./waf --run "leo-orbit 
  --orbitFile=contrib/leo/data/orbits/starlink.csv \
  --duration=1000 \
  --traceFile=os.log"

leo-delay
#########

The delay tracing example uses ``UdpServer`` and ``UdpClient`` to measure the delay and packet loss on between two nodes.
The source and destination locations are given as pairs of longitude and latitude.

.. sourcecode:: bash
  
  $ ./waf --run "leo-delay \
  --destOnly=true \
  --orbitFile=contrib/leo/data/orbits/starlink.csv \
  --constellation=StarlinkGateway \
  --traceFile=dsai.log \
  --islRate=2Gbps \
  --islEnabled=true \
  --duration=1000"

leo-throughput
##############

The throughput tracing example uses ``BulkSendHelper`` and ``PacketSinkHelper``
to measure the throughput inbetween two nodes. The throughput is logged to the `traceFile`.

.. sourcecode:: bash
  
  $ ./waf --run "leo-bulk-send \
  --destOnly=true \
  --orbitFile=contrib/leo/data/orbits/telesat.csv \
  --constellation=TelesatGateway \
  --traceFile=btai.log \
  --islRate=2Gbps \
  --islEnabled=true \
  --duration=1000"

Validation
**********

Much of the module is covered using tests. The evalutation of the module in
part of the project thesis.
