/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include "ns3/simulator.h"
#include "ns3/rectangle.h"
#include "ns3/box.h"
#include "ns3/log.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/simulator.h"



//#include "ns3/gtk-config-store.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  Time simTime = Seconds (50);
  bool useCa = false;

  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.Parse (argc, argv);

  // to save a template default attribute file run it like this:
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-simple
  //
  // to load a previously created default attribute file
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-simple

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  if (useCa)
   {
     Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (useCa));
     Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (2));
     Config::SetDefault ("ns3::LteHelper::EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
   }

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Uncomment to enable logging
  //lteHelper->EnableLogComponents ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (3);

  // Install Mobility Model

  /*
      * Mobility Configuration for Drones.
      */
     MobilityHelper mobilityDrones;
     ObjectFactory position;
     int64_t streamIndex = 0; // used to get consistent random numbers across scenarios

     // Note that with FixedRssLossModel, the positions below are not
     // used for received signal strength.
     position.SetTypeId("ns3::RandomBoxPositionAllocator");
     position.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100000.0]"));
     position.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100000.0]"));
     position.Set("Z", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100000.0]"));

     Ptr<PositionAllocator> positionAllocatorDrones = position.Create()->GetObject<PositionAllocator>();
     streamIndex += positionAllocatorDrones->AssignStreams(streamIndex);

     std::string speed("ns3::ConstantRandomVariable[Constant=1000.0]");
     std::string pause("ns3::ConstantRandomVariable[Constant=1.0]");

     mobilityDrones.SetMobilityModel("ns3::ConstantVelocityMobilityModel",
                                    "Speed", StringValue(speed),
                                    "Pause", StringValue(pause),
                                    "PositionAllocator", PointerValue(positionAllocatorDrones));

     mobilityDrones.Install(ueNodes);

     /*
      * Mobility Configuration for APs.
      */
     MobilityHelper mobilityAp;
     Ptr<ListPositionAllocator> positionAllocatorAp = CreateObject<ListPositionAllocator>();

     positionAllocatorAp = CreateObject<ListPositionAllocator>();
     positionAllocatorAp->Add(Vector(0.0, 0.0, 0.0));
     mobilityAp.SetPositionAllocator(positionAllocatorAp);
     mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
     mobilityAp.Install(enbNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  // Default scheduler is PF, uncomment to use RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);
  lteHelper->EnableTraces ();

  Simulator::Stop (simTime);
  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  return 0;
}
