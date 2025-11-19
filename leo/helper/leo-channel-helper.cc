/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include "ns3/log.h"
#include "ns3/config.h"
#include "leo-channel-helper.h"
#include "ns3/enum.h"
#include "ns3/queue.h"
#include "ns3/names.h"
#include "ns3/assert.h"
#include "ns3/string.h"
#include "ns3/data-rate.h"
#include "ns3/net-device-queue-interface.h"

#include "../model/leo-mock-channel.h"
#include "../model/leo-mock-net-device.h"
#include "../model/leo-starlink-constants.h"
#include "../model/leo-telesat-constants.h"
#include "../model/leo-propagation-loss-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("LeoChannelHelper");

LeoChannelHelper::LeoChannelHelper ()
{
  m_gndQueueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
 // m_gndQueueFactory.Set ("MaxSize", QueueSizeValue (QueueSize ("10000p")));
  m_satQueueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
  //m_satQueueFactory.Set ("MaxSize", QueueSizeValue (QueueSize ("10000p")));

  m_gndDeviceFactory.SetTypeId ("ns3::LeoMockNetDevice");
  m_gndDeviceFactory.Set ("DeviceType", EnumValue (LeoMockNetDevice::GND));

  m_satDeviceFactory.SetTypeId ("ns3::LeoMockNetDevice");
  m_satDeviceFactory.Set ("DeviceType", EnumValue (LeoMockNetDevice::SAT));

  m_channelFactory.SetTypeId ("ns3::LeoMockChannel");
  m_channelFactory.Set ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));

  m_propagationLossFactory.SetTypeId ("ns3::LeoPropagationLossModel");
  m_propagationDelayFactory.SetTypeId ("ns3::ConstantSpeedPropagationDelayModel");
  m_propagationDelayFactory.Set ("Speed", DoubleValue (LEO_SPEED_OF_LIGHT_IN_AIR));
}

LeoChannelHelper::LeoChannelHelper (std::string constellation) :
  LeoChannelHelper ()
{
  SetConstellation (constellation);
}

void
LeoChannelHelper::SetConstellation (std::string constellation)
{
  if (constellation == "StarlinkGateway")
    {
      SetConstellationAttributes (LEO_STARLINK_GATEWAY_EIRP,
      				  LEO_STARLINK_GATEWAY_ELEVATION_ANGLE,
      				  LEO_STARLINK_GATEWAY_FSPL,
      				  LEO_STARLINK_GATEWAY_ATMOSPHERIC_LOSS,
      				  LEO_STARLINK_GATEWAY_LINK_MARGIN,
      				  LEO_STARLINK_GATEWAY_DATA_RATE,
      				  LEO_STARLINK_GATEWAY_RX_ANTENNA_GAIN,
      				  0.0 // TODO
      );
    }
  else if (constellation == "StarlinkUser")
    {
      SetConstellationAttributes (LEO_STARLINK_USER_EIRP,
      				  LEO_STARLINK_USER_ELEVATION_ANGLE,
      				  LEO_STARLINK_USER_FSPL,
      				  LEO_STARLINK_USER_ATMOSPHERIC_LOSS,
      				  LEO_STARLINK_USER_LINK_MARGIN,
      				  LEO_STARLINK_USER_DATA_RATE,
      				  LEO_STARLINK_USER_RX_ANTENNA_GAIN,
      				  0.0
      );
    }
  else if (constellation == "TelesatGateway")
    {
      SetConstellationAttributes (LEO_TELESAT_GATEWAY_EIRP,
      				  LEO_TELESAT_GATEWAY_ELEVATION_ANGLE,
      				  LEO_TELESAT_GATEWAY_FSPL,
      				  LEO_TELESAT_GATEWAY_ATMOSPHERIC_LOSS,
      				  LEO_TELESAT_GATEWAY_LINK_MARGIN,
      				  LEO_TELESAT_GATEWAY_DATA_RATE,
      				  LEO_TELESAT_GATEWAY_RX_ANTENNA_GAIN,
      				  0.0
      );
    }
  else if (constellation == "TelesatUser")
    {
      SetConstellationAttributes (LEO_TELESAT_USER_EIRP,
      				  LEO_TELESAT_USER_ELEVATION_ANGLE,
      				  LEO_TELESAT_USER_FSPL,
      				  LEO_TELESAT_USER_ATMOSPHERIC_LOSS,
      				  LEO_TELESAT_USER_LINK_MARGIN,
      				  LEO_TELESAT_USER_DATA_RATE,
      				  LEO_TELESAT_USER_RX_ANTENNA_GAIN,
      				  0.0
      );
    }
  else
    {
      NS_ASSERT_MSG (false, "Invalid constellation");
    }
}

void
LeoChannelHelper::SetConstellationAttributes (double eirp,
					      double elevationAngle,
					      double fspl,
					      double atmosphericLoss,
					      double linkMargin,
					      std::string dataRate,
					      double rxGain,
					      double rxLoss)
{
  m_gndDeviceFactory.Set ("TxPower", DoubleValue (eirp));
  m_satDeviceFactory.Set ("TxPower", DoubleValue (eirp));

  m_gndDeviceFactory.Set ("RxLoss", DoubleValue (rxLoss));
  m_gndDeviceFactory.Set ("RxGain", DoubleValue (rxGain));

  m_satDeviceFactory.Set ("RxLoss", DoubleValue (rxLoss));
  m_satDeviceFactory.Set ("RxGain", DoubleValue (rxGain));

  m_gndDeviceFactory.Set ("DataRate", StringValue (dataRate));
  m_satDeviceFactory.Set ("DataRate", StringValue (dataRate));

  m_propagationLossFactory.Set ("ElevationAngle", DoubleValue (elevationAngle));
  m_propagationLossFactory.Set ("FreeSpacePathLoss", DoubleValue (fspl));
  m_propagationLossFactory.Set ("AtmosphericLoss", DoubleValue (atmosphericLoss));
  m_propagationLossFactory.Set ("LinkMargin", DoubleValue (linkMargin));
}

void
LeoChannelHelper::SetQueue(ObjectFactory &factory,
			   std::string type,
             		   std::string n1, const AttributeValue &v1,
             		   std::string n2, const AttributeValue &v2,
             		   std::string n3, const AttributeValue &v3,
             		   std::string n4, const AttributeValue &v4)
{
  QueueBase::AppendItemTypeIfNotPresent (type, "Packet");

  factory.SetTypeId (type);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
}

void
LeoChannelHelper::SetGndQueue (std::string type,
             		       std::string n1, const AttributeValue &v1,
             		       std::string n2, const AttributeValue &v2,
             		       std::string n3, const AttributeValue &v3,
             		       std::string n4, const AttributeValue &v4)
{
  SetQueue (m_gndQueueFactory, type, n1, v1, n2, v2, n3, v3, n4, v4);
}

void
LeoChannelHelper::SetSatQueue (std::string type,
             std::string n1, const AttributeValue &v1,
             std::string n2, const AttributeValue &v2,
             std::string n3, const AttributeValue &v3,
             std::string n4, const AttributeValue &v4)
{

  SetQueue (m_satQueueFactory, type, n1, v1, n2, v2, n3, v3, n4, v4);
}


void
LeoChannelHelper::SetGndDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_gndDeviceFactory.Set (name, value);
}

void
LeoChannelHelper::SetSatDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_satDeviceFactory.Set (name, value);
}

void
LeoChannelHelper::SetChannelAttribute (std::string name, const AttributeValue &value)
{
  m_channelFactory.Set (name, value);
}

void
LeoChannelHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type LeoMockNetDevice.
  //
  Ptr<LeoMockNetDevice> device = nd->GetObject<LeoMockNetDevice> ();
  if (device == nullptr)
    {
      NS_LOG_INFO ("LeoChannelHelper::EnablePcapInternal(): Device " << device << " not of type ns3::LeoMockNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_EN10MB);
  pcapHelper.HookDefaultSink<LeoMockNetDevice> (device, "PromiscSniffer", file);
}

void
LeoChannelHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type LeoMockNetDevice.
  //
  Ptr<LeoMockNetDevice> device = nd->GetObject<LeoMockNetDevice> ();
  if (device == nullptr)
    {
      NS_LOG_INFO ("LeoChannelHelper::EnableAsciiInternal(): Device " << device <<
                   " not of type ns3::LeoMockNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == nullptr)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      //
      // The MacRx trace source provides our "r" event.
      //
      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<LeoMockNetDevice> (device, "MacRx", theStream);

      //
      // The "+", '-', and 'd' events are driven by trace sources actually in the
      // transmit queue.
      //
      Ptr<Queue<Packet> > queue = device->GetQueue ();
      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue<Packet> > (queue, "Enqueue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue<Packet> > (queue, "Drop", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue<Packet> > (queue, "Dequeue", theStream);

      // PhyRxDrop trace source for "d" event
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<MockNetDevice> (device, "PhyRxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to providd a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static
  // functions that are always there waiting for just such a case.
  //
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  oss << "/NodeList/" << nd->GetNode ()->GetId () << "/DeviceList/" << deviceid << "/$ns3::LeoMockNetDevice/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LeoMockNetDevice/TxQueue/Enqueue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LeoMockNetDevice/TxQueue/Dequeue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LeoMockNetDevice/TxQueue/Drop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LeoMockNetDevice/PhyRxDrop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer
LeoChannelHelper::Install (std::vector<Ptr<Node> > &satellites, std::vector<Ptr<Node> > &stations)
{
  NS_LOG_FUNCTION (this);

  Ptr<LeoMockChannel> channel = m_channelFactory.Create<LeoMockChannel> ();
  channel->SetPropagationLoss (m_propagationLossFactory.Create<LeoPropagationLossModel> ());
  channel->SetPropagationDelay (m_propagationDelayFactory.Create<ConstantSpeedPropagationDelayModel> ());

  NetDeviceContainer container;

  for (Ptr<Node> node : satellites)
  {
    Ptr<LeoMockNetDevice> dev = m_satDeviceFactory.Create<LeoMockNetDevice> ();
    dev->SetAddress (Mac48Address::Allocate ());
    node->AddDevice (dev);
    Ptr<Queue<Packet> > queue = m_satQueueFactory.Create<Queue<Packet> > ();
    dev->SetQueue (queue);

    // Aggregate NetDeviceQueueInterface objects
    Ptr<NetDeviceQueueInterface> ndqi = CreateObject<NetDeviceQueueInterface> ();
    ndqi->GetTxQueue (0)->ConnectQueueTraces (queue);
    dev->AggregateObject (ndqi);

    dev->Attach (channel);
    container.Add (dev);

    NS_LOG_DEBUG ("Added device for node " << node->GetId ());
  }

  for (Ptr<Node> node : stations)
  {
    Ptr<LeoMockNetDevice> dev = m_gndDeviceFactory.Create<LeoMockNetDevice> ();
    dev->SetAddress (Mac48Address::Allocate ());
    node->AddDevice (dev);
    Ptr<Queue<Packet> > queue = m_gndQueueFactory.Create<Queue<Packet> > ();
    dev->SetQueue (queue);

    // Aggregate NetDeviceQueueInterface objects
    Ptr<NetDeviceQueueInterface> ndqi = CreateObject<NetDeviceQueueInterface> ();
    ndqi->GetTxQueue (0)->ConnectQueueTraces (queue);
    dev->AggregateObject (ndqi);

    dev->Attach (channel);
    container.Add (dev);

    NS_LOG_DEBUG ("Added device for node " << node->GetId ());
  }

  return container;
}

NetDeviceContainer
LeoChannelHelper::Install (NodeContainer &satellites, NodeContainer &stations)
{
  std::vector<Ptr<Node> > satelliteNodes = std::vector<Ptr<Node> >(satellites.Begin(), satellites.End());
  std::vector<Ptr<Node> > stationNodes = std::vector<Ptr<Node> >(stations.Begin(), stations.End());
  return Install (satelliteNodes, stationNodes);
}

NetDeviceContainer
LeoChannelHelper::Install (std::vector<std::string> &satellites, std::vector<std::string> &stations)
{
  NS_LOG_FUNCTION (this);

  std::vector<Ptr<Node> > sats;
  std::vector<Ptr<Node> > stats;
  for (std::string name : satellites)
  {
    NS_LOG_DEBUG ("Adding node " << name);
    Ptr<Node> node = Names::Find<Node>(name);
    sats.push_back (node);
  }
  for (std::string name : stations)
  {
    NS_LOG_DEBUG ("Adding node " << name);
    Ptr<Node> node = Names::Find<Node>(name);
    stats.push_back (node);
  }

  return Install (sats, stats);
}

}; /* namespace ns3 */
