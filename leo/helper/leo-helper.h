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

#ifndef LEO_HELPER_H
#define LEO_HELPER_H

#include "ns3/leo.h"

#include <string>

#include <ns3/object-factory.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/trace-helper.h>
#include "ns3/internet-stack-helper.h"

#include "leo-channel-helper.h"
#include "isl-helper.h"

namespace ns3 {

/**
 * \brief Builds a LEO network with user terminals, gateways and satellites.
 */
class LeoHelper
{
public:
  /**
   * Creates a LeoHelper
   */
  LeoHelper ();
  virtual ~LeoHelper () {}

  /**
   * \param satellites satellites
   * \param gateways gateways
   * \param terminals terminals
   * \return a NetDeviceContainer for nodes
   *
   * This method creates
   * - an ns3::IslChannel between the satellite nodes,
   * - an ns3::MockLeoChannel between the satellite nodes and gateway nodes,
   * - and an ns3::MockLeoChannel between the satellite nodes and terminal nodes
   * with the attributes configured by LeoHelper::SetChannelAttribute,
   * LeoHelper::SetIslChannelAttribute, LeoHelper::SetGatewayChannelAttribute,
   * LeoHelper::SetTerminalChannelAttribute.
   *
   * Then, for each satellite node in the input containers, we create a
   * ns3::IslNetDevice with the attributes requested, a queue for this
   * ns3::NetDevice, and associate the resulting ns3::NetDevice with the
   * ns3::Node and ns3::IslChannel.
   *
   * Then, for each satellite node and gateway node, we create a
   * ns3::LeoMockNetDevice using the configured attributes and add them to a
   * ns3::LeoMockChannel.
   *
   * Same is done with each satellite and terminal node.
   */
  NetDeviceContainer Install (NodeContainer &satellites,
  			      NodeContainer &gateways,
  			      NodeContainer &terminals);

  /**
   * Each point to point net device must have a queue to pass packets through.
   * This method allows one to set the type of the queue that is automatically
   * created when the device is created and attached to a node.
   *
   * \param type the type of queue
   * \param n1 the name of the attribute to set on the queue
   * \param v1 the value of the attribute to set on the queue
   * \param n2 the name of the attribute to set on the queue
   * \param v2 the value of the attribute to set on the queue
   * \param n3 the name of the attribute to set on the queue
   * \param v3 the value of the attribute to set on the queue
   * \param n4 the name of the attribute to set on the queue
   * \param v4 the value of the attribute to set on the queue
   *
   * Set the type of queue to create and associated to each
   * IslNetDevice created through IslHelper::Install.
   */
  void SetQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  void SetIslQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  void SetGndGwQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  void SetSatGwQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  void SetGndUtQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  void SetSatUtQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());
  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::IslNetDevice created
   * by IslHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  void SetIslDeviceAttribute (std::string name, const AttributeValue &value);
  void SetGndUtDeviceAttribute (std::string name, const AttributeValue &value);
  void SetGndGwDeviceAttribute (std::string name, const AttributeValue &value);
  void SetSatUtDeviceAttribute (std::string name, const AttributeValue &value);
  void SetSatGwDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Channel created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::IslChannel created
   * by IslHelper::Install
   */
  void SetChannelAttribute (std::string name, const AttributeValue &value);
  void SetIslChannelAttribute (std::string name, const AttributeValue &value);
  void SetUtChannelAttribute (std::string name, const AttributeValue &value);
  void SetGwChannelAttribute (std::string name, const AttributeValue &value);

  void SetInternetStackAttribute (std::string name, const AttributeValue &value);

  void SetRoutingHelper (const Ipv4RoutingHelper &routing);
  void SetRoutingHelper (const Ipv6RoutingHelper &routing);
private:
  /**
   * \brief Enable pcap output the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (
    Ptr<OutputStreamWrapper> stream,
    std::string prefix,
    Ptr<NetDevice> nd,
    bool explicitFilename);

  InternetStackHelper m_stackHelper;
  IslHelper m_islChannelHelper;
  LeoChannelHelper m_utChannelHelper;
  LeoChannelHelper m_gwChannelHelper;
};

}

#endif /* LEO_HELPER_H */
