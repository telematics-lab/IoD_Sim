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

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/simulator.h"
#ifdef NS3_MPI
#include "ns3/point-to-point-remote-channel.h"
#endif
#include "../model/isl-mock-channel.h"
#include "../model/mock-net-device.h"
#include "isl-helper.h"

#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/string.h"
#include "ns3/trace-helper.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IslHelper");

IslHelper::IslHelper()
{
    m_queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
    m_deviceFactory.SetTypeId("ns3::MockNetDevice");
    m_channelFactory.SetTypeId("ns3::IslMockChannel");
    m_channelFactory.Set("PropagationDelay",
                         StringValue("ns3::ConstantSpeedPropagationDelayModel"));
    m_channelFactory.Set("PropagationLoss", StringValue("ns3::IslPropagationLossModel"));
}

void
IslHelper::SetQueue(std::string type,
                    std::string n1,
                    const AttributeValue& v1,
                    std::string n2,
                    const AttributeValue& v2,
                    std::string n3,
                    const AttributeValue& v3,
                    std::string n4,
                    const AttributeValue& v4)
{
    QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

    m_queueFactory.SetTypeId(type);
    m_queueFactory.Set(n1, v1);
    m_queueFactory.Set(n2, v2);
    m_queueFactory.Set(n3, v3);
    m_queueFactory.Set(n4, v4);
}

void
IslHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    m_deviceFactory.Set(n1, v1);
}

void
IslHelper::SetChannelAttribute(std::string n1, const AttributeValue& v1)
{
    m_channelFactory.Set(n1, v1);
}

void
IslHelper::EnablePcapInternal(std::string prefix,
                              Ptr<NetDevice> nd,
                              bool promiscuous,
                              bool explicitFilename)
{
    //
    // All of the Pcap enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type MockNetDevice.
    //
    Ptr<MockNetDevice> device = nd->GetObject<MockNetDevice>();
    if (device == nullptr)
    {
        NS_LOG_INFO("IslHelper::EnablePcapInternal(): Device "
                    << device << " not of type ns3::MockNetDevice");
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
        filename = pcapHelper.GetFilenameFromDevice(prefix, device);
    }

    Ptr<PcapFileWrapper> file =
        pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_EN10MB);
    pcapHelper.HookDefaultSink<MockNetDevice>(device, "PromiscSniffer", file);
}

void
IslHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                               std::string prefix,
                               Ptr<NetDevice> nd,
                               bool explicitFilename)
{
    //
    // All of the ascii enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type MockNetDevice.
    //
    Ptr<MockNetDevice> device = nd->GetObject<MockNetDevice>();
    if (device == nullptr)
    {
        NS_LOG_INFO("IslHelper::EnableAsciiInternal(): Device "
                    << device << " not of type ns3::MockNetDevice");
        return;
    }

    //
    // Our default trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Packet::EnablePrinting();

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
            filename = asciiTraceHelper.GetFilenameFromDevice(prefix, device);
        }

        Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream(filename);

        //
        // The MacRx trace source provides our "r" event.
        //
        asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<MockNetDevice>(device,
                                                                             "MacRx",
                                                                             theStream);

        //
        // The "+", '-', and 'd' events are driven by trace sources actually in the
        // transmit queue.
        //
        Ptr<Queue<Packet>> queue = device->GetQueue();
        asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue<Packet>>(queue,
                                                                             "Enqueue",
                                                                             theStream);
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue<Packet>>(queue, "Drop", theStream);
        asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue<Packet>>(queue,
                                                                             "Dequeue",
                                                                             theStream);

        // PhyRxDrop trace source for "d" event
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<MockNetDevice>(device,
                                                                          "PhyRxDrop",
                                                                          theStream);

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
    uint32_t nodeid = nd->GetNode()->GetId();
    uint32_t deviceid = nd->GetIfIndex();
    std::ostringstream oss;

    oss << "/NodeList/" << nd->GetNode()->GetId() << "/DeviceList/" << deviceid
        << "/$ns3::MockNetDevice/MacRx";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::MockNetDevice/TxQueue/Enqueue";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::MockNetDevice/TxQueue/Dequeue";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::MockNetDevice/TxQueue/Drop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::MockNetDevice/PhyRxDrop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer
IslHelper::Install(NodeContainer c)
{
    std::vector<Ptr<Node>> nodes = std::vector<Ptr<Node>>(c.Begin(), c.End());
    return Install(nodes);
}

NetDeviceContainer
IslHelper::Install(std::vector<Ptr<Node>>& nodes)
{
    NS_LOG_FUNCTION(this);

    Ptr<MockChannel> channel = m_channelFactory.Create<MockChannel>();

    NetDeviceContainer container;

    for (Ptr<Node> node : nodes)
    {
        NS_LOG_DEBUG("Adding device for node " << node->GetId());
        Ptr<MockNetDevice> dev = m_deviceFactory.Create<MockNetDevice>();
        dev->SetAddress(Mac48Address::Allocate());
        node->AddDevice(dev);
        Ptr<Queue<Packet>> queue = m_queueFactory.Create<Queue<Packet>>();
        dev->SetQueue(queue);
        dev->Attach(channel);
        container.Add(dev);
    }

    return container;
}

NetDeviceContainer
IslHelper::Install(std::vector<std::string>& names)
{
    std::vector<Ptr<Node>> nodes;
    for (std::string name : names)
    {
        Ptr<Node> node = Names::Find<Node>(name);
        nodes.push_back(node);
    }

    return Install(nodes);
}

} // namespace ns3
