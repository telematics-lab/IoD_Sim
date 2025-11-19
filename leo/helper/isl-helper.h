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

#ifndef ISL_HELPER_H
#define ISL_HELPER_H

#include <string>

#include <ns3/object-factory.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>

#include <ns3/trace-helper.h>

/**
 * \file
 * \ingroup leo
 * Declares IslNetDevice
 */

namespace ns3 {

class NetDevice;
class Node;

/**
 * \ingroup leo
 * \brief Build a set of IslNetDevice objects
 */
class IslHelper : public PcapHelperForDevice,
	                   public AsciiTraceHelperForDevice
{
public:
  /**
   * Create a IslHelper to make life easier when creating ISL networks.
   */
  IslHelper ();
  virtual ~IslHelper () {}

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

  /**
   * \param c a set of nodes
   * \return a NetDeviceContainer for nodes
   *
   * This method creates a ns3::IslChannel with the
   * attributes configured by IslHelper::SetChannelAttribute,
   * then, for each node in the input container, we create a
   * ns3::IslNetDevice with the requested attributes,
   * a queue for this ns3::NetDevice, and associate the resulting
   * ns3::NetDevice with the ns3::Node and ns3::IslChannel.
   */
  NetDeviceContainer Install (NodeContainer c);

  /**
   * \param nodes Nodes
   * \return a NetDeviceContainer for nodes
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NetDeviceContainer Install (std::vector<Ptr<Node> > &nodes);

  /**
   * \param nodes Names of the nodes
   * \return a NetDeviceContainer for nodes
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NetDeviceContainer Install (std::vector<std::string> &nodes);

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

private:
  ObjectFactory m_queueFactory;         //!< Queue Factory
  ObjectFactory m_channelFactory;       //!< Channel Factory
  ObjectFactory m_deviceFactory;        //!< Device Factory
};

} // namespace ns3

#endif /* ISL_HELPER_H */
