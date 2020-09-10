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

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>
#include <ns3/buildings-helper.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <vector>
#include <ns3/assert.h>
#include <ns3/command-line.h>
#include <ns3/config.h>
#include <ns3/log.h>
#include "ns3/flow-monitor-module.h"
#include <ns3/flow-monitor-helper.h>

#include <ns3/drone-client.h>
#include <ns3/drone-server.h>
#include <ns3/flight-plan.h>
#include <ns3/proto-point.h>
#include <ns3/scenario-configuration-helper.h>
//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Scenario");

using ms = std::chrono::duration<float, std::milli>;


int main (int argc, char *argv[])
{
  std::string configFilePath = "";
  CommandLine cmd;
  cmd.AddValue ("config", "Configuration file path", configFilePath);
  cmd.Parse (argc, argv);

  /*Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (true));*/


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> (); //Create an LteHelper object
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel")); //Pathloss model selection

  // Create Nodes: eNodeB and UE
  CONFIGURATOR->Initialize (argc, argv, "lena-simple-davinci");
  Time simTime = Seconds (CONFIGURATOR->GetDuration()+3);

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (CONFIGURATOR->GetDronesN());   // default: N_Drones = 1

  // Install Mobility Model
  MobilityHelper mobility;

   for (uint32_t i = 0; i < ueNodes.GetN (); i++)
     {
       mobility.SetMobilityModel ("ns3::ConstantAccelerationDroneMobilityModel",
                                  "Acceleration", DoubleValue (CONFIGURATOR->GetDroneAcceleration (i)),
                                  "MaxSpeed", DoubleValue (CONFIGURATOR->GetDroneMaxSpeed (i)),
                                  "FlightPlan", FlightPlanValue (CONFIGURATOR->GetDroneFlightPlan (i)),
                                  "CurveStep", DoubleValue (CONFIGURATOR->GetCurveStep ()));

       mobility.Install (ueNodes.Get (i));
       BuildingsHelper::Install (ueNodes.Get (i));
     }

   MobilityHelper mobilityZsps;
   auto positionAllocatorZsps = CreateObject<ListPositionAllocator> ();

   CONFIGURATOR->GetZspsPosition (positionAllocatorZsps);

   mobilityZsps.SetPositionAllocator (positionAllocatorZsps);
   mobilityZsps.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobilityZsps.Install (enbNodes);
   BuildingsHelper::Install (enbNodes);

 //Finalize the building and mobility model configuration
  //BuildingsHelper::MakeMobilityModelConsistent ();  // DEPRECATED in ns-3-31

 //using the Proportional Fair (PF) scheduler
   lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
   lteHelper->SetSchedulerAttribute ("HarqEnabled",  BooleanValue (true));
 //lteHelper->SetSchedulerAttribute ("CqiTimerThreshold", UintegerValue (1000));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = lteHelper->InstallEnbDevice (enbNodes); //Install an LTE protocol stack on the eNB(s).
  ueDevs = lteHelper->InstallUeDevice (ueNodes);   //Install an LTE protocol stack on the UEs.

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs,enbDevs.Get(0));
  //Attach the UEs to an eNB. This will configure each UE according to the eNB configuration,
  //and create an RRC connection between them.

  // Activate a data radio bearer between each UE and the eNB it is attached to.
  //For constant bit rate (CBR) traffic, it is suggested to set MBR to GBR.
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO   ;  // define Qci type --> Qos class indicator
  GbrQosInformation qos;
  /*qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
  qos.gbrUl = 132;
  qos.mbrDl = qos.gbrDl;
  qos.mbrUl = qos.gbrUl;*/
  qos.gbrDl = 20000000; 	   // Downlink GBR (bit/s) ---> 20 Mbps
  qos.gbrUl = 5000000;	 	  // Uplink GBR ---> 5 Mbps
  qos.mbrDl = 20000000;		 // Downlink MBR
  qos.mbrUl = 5000000; 		// Uplink MBR,
  EpsBearer bearer (q, qos);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();

  std::cout << "> Starting simulation..." << std::endl;

  Simulator::Stop (simTime);

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (0.04)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));
//The time interval duration for RLC KPIs

  std::chrono::high_resolution_clock timer;
  auto start = timer.now();

  Simulator::Run();

  Simulator::Destroy();

  auto stop = timer.now();
  auto deltaTime = std::chrono::duration_cast<ms>(stop - start).count();
  std::cout << "> Simulation terminated. Took: " << deltaTime/1000 << " s." << std::endl;

  return 0;
}
