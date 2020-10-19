/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
 */

#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/point-to-point-module.h>

using namespace ns3;

int main (int argc, char *argv[])
{
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

  NodeContainer enbNodes, ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(2);

  MobilityHelper staticNodeMobility;
  staticNodeMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  staticNodeMobility.Install(enbNodes);
  staticNodeMobility.Install(ueNodes);

  NetDeviceContainer enbDevices = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueDevices = lteHelper->InstallUeDevice(ueNodes);

  lteHelper->Attach(ueDevices, enbDevices.Get(0));

  EpsBearer dataRadioBearer(EpsBearer::GBR_CONV_VIDEO);
  lteHelper->ActivateDataRadioBearer(ueDevices, dataRadioBearer);

  Simulator::Stop(Seconds(0.5));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}