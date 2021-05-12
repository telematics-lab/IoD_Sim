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
#include <ns3/simulator.h>
#include <ns3/rectangle.h>
#include <ns3/box.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/constant-velocity-mobility-model.h>

//#include "ns3/gtk-config-store.h"

using namespace ns3;


int main (int argc, char *argv[])
{
  double speed = 10;       // m/s
  double simTime = 100;
  double enbTxPowerDbm = 46.0;
  bool useCa = false;

  CommandLine cmd;
  cmd.AddValue ("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)", simTime);
  cmd.AddValue ("speed", "Speed of the UE ", speed);
  cmd.AddValue ("enbTxPowerDbm", "TX power [dBm] used by HeNBs", enbTxPowerDbm);

  cmd.Parse (argc, argv);

 // Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (10));
  //Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (7));

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

 /* lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100));
    lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (100 + 18000));
    lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (100));
    lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (25));*/


 // lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  // Uncomment to enable logging
  //lteHelper->EnableLogComponents ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);

  // Install Mobility Model for Drones

               MobilityHelper mobility;
               mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                               "MinX", DoubleValue (1.0),
                                               "MinY", DoubleValue (1.0),
                                               "DeltaX", DoubleValue (5.0),
                                               "DeltaY", DoubleValue (5.0),
                                               "GridWidth", UintegerValue (20),
                                               "LayoutType", StringValue ("RowFirst"));

               mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
               mobility.Install (ueNodes);

               for (uint n=0 ; n < ueNodes.GetN() ; n++)
                {
                   Ptr<ConstantVelocityMobilityModel> mob = ueNodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
                   mob->SetVelocity(Vector(speed, speed, speed));
                }


  // Mobility Configuration for APs.

     MobilityHelper mobilityAp;
     Ptr<ListPositionAllocator> positionAllocatorAp = CreateObject<ListPositionAllocator>();

     positionAllocatorAp = CreateObject<ListPositionAllocator>();
     positionAllocatorAp->Add(Vector(0.0, 0.0, 0.0));
     mobilityAp.SetPositionAllocator(positionAllocatorAp);
     mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
     mobilityAp.Install(enbNodes);



  // Create Devices and install them in the Nodes (eNB and UE)
 // Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;

  lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

 Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  enbPhy->SetAttribute ("TxPower", DoubleValue (enbTxPowerDbm));

  lteHelper->EnablePhyTraces();
  lteHelper->EnableMacTraces();
  lteHelper->EnableRlcTraces();
  lteHelper->EnablePdcpTraces();

  Simulator::Stop (Seconds (simTime));

  Simulator::Run ();

  for (NodeContainer::Iterator j = ueNodes.Begin ();
          j != ueNodes.End (); ++j)
       {
         Ptr<Node> object = *j;
         Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
         NS_ASSERT (position != 0);
         Vector pos = position->GetPosition ();
         std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  return 0;
}
