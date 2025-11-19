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

#ifndef LEO_CANNEL_HELPER_H
#define LEO_CANNEL_HELPER_H

#include <string>

#include <ns3/object-factory.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>

#include <ns3/trace-helper.h>

/**
 * \file
 * \ingroup leo
 * Declares LeoChannelHelper
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Build a channel for transmissions between ns3::LeoMockNetDevice s
 */
class LeoChannelHelper : public PcapHelperForDevice,
	                 public AsciiTraceHelperForDevice
{
public:
  /// constructor
  LeoChannelHelper ();

  /**
   * constructor
   *
   * \param constellation name of the link parameter preset
   */
  LeoChannelHelper (std::string constellation);

  /// destructor
  virtual ~LeoChannelHelper ()
    {};

  /**
   * \brief Install the satellites and stations into the channel
   * \param satellites satellites
   * \param stations ground stations
   * \return container of network devices attached to the channel
   */
  NetDeviceContainer Install (NodeContainer &satellites, NodeContainer &stations);

  /**
   * \brief Install the satellites and stations into the channel
   * \param satellites satellites
   * \param stations ground stations
   * \return container of network devices attached to the channel
   */
  NetDeviceContainer Install (std::vector<Ptr<Node> > &satellites, std::vector<Ptr<Node> > &stations);

  /**
   * \brief Install the satellites and stations into the channel
   * \param satellites satellites
   * \param stations ground stations
   * \return container of network devices attached to the channel
   */
  NetDeviceContainer Install (std::vector<std::string> &satellites, std::vector<std::string> &stations);

  /**
   * \brief Set the type and attributes of the queues of the ground stations
   * \param type type of the queue
   * \param n1 name of an attribute of the queue
   * \param v1 value of an attribute of the queue
   * \param n2 name of an attribute of the queue
   * \param v2 value of an attribute of the queue
   * \param n3 name of an attribute of the queue
   * \param v3 value of an attribute of the queue
   * \param n4 name of an attribute of the queue
   * \param v4 value of an attribute of the queue
   */
  void SetGndQueue (std::string type,
                    std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                    std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                    std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                    std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  /**
   * \brief Set the type and attributes of the queues of the satellite devices
   * \param type type of the queue
   * \param n1 name of an attribute of the queue
   * \param v1 value of an attribute of the queue
   * \param n2 name of an attribute of the queue
   * \param v2 value of an attribute of the queue
   * \param n3 name of an attribute of the queue
   * \param v3 value of an attribute of the queue
   * \param n4 name of an attribute of the queue
   * \param v4 value of an attribute of the queue
   */
  void SetSatQueue (std::string type,
                    std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                    std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                    std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                    std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  /**
   * \brief Set the type and attributes of the devices of the ground devices
   * \param type type of the device
   * \param n1 name of an attribute of the device
   * \param v1 value of an attribute of the device
   * \param n2 name of an attribute of the device
   * \param v2 value of an attribute of the device
   * \param n3 name of an attribute of the device
   * \param v3 value of an attribute of the device
   * \param n4 name of an attribute of the device
   * \param v4 value of an attribute of the device
   */
  void SetGndDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Set the type and attributes of the devices of the satellite devices
   * \param type type of the device
   * \param n1 name of an attribute of the device
   * \param v1 value of an attribute of the device
   * \param n2 name of an attribute of the device
   * \param v2 value of an attribute of the device
   * \param n3 name of an attribute of the device
   * \param v3 value of an attribute of the device
   * \param n4 name of an attribute of the device
   * \param v4 value of an attribute of the device
   */
  void SetSatDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Set the type and attributes of the channel
   * \param type type of the channel
   * \param n1 name of an attribute of the channel
   * \param v1 value of an attribute of the channel
   * \param n2 name of an attribute of the channel
   * \param v2 value of an attribute of the channel
   * \param n3 name of an attribute of the channel
   * \param v3 value of an attribute of the channel
   * \param n4 name of an attribute of the channel
   * \param v4 value of an attribute of the channel
   */
  void SetChannelAttribute (std::string name, const AttributeValue &value);

  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
  				    std::string prefix,
  				    Ptr<NetDevice> nd,
  				    bool explicitFilename);

  /**
   * \brief Set the link parameter preset
   * \param constellation name of the link parameter preset
   */
  void SetConstellation (std::string constellation);

private:
  /// Satellite queues
  ObjectFactory m_satQueueFactory;
  /// Ground station queues
  ObjectFactory m_gndDeviceFactory;

  /// Satellite devices
  ObjectFactory m_satDeviceFactory;
  /// Ground station devices
  ObjectFactory m_gndQueueFactory;

  /// Channel
  ObjectFactory m_channelFactory;

  /// Propagation loss models
  ObjectFactory m_propagationLossFactory;
  //
  /// Propagation delay models
  ObjectFactory m_propagationDelayFactory;

  /**
   * \brief Set the factory and attributes of the queue
   * \param factory queue factory
   * \param n1 name of an attribute of the factory
   * \param v1 value of an attribute of the factory
   * \param n2 name of an attribute of the factory
   * \param v2 value of an attribute of the factory
   * \param n3 name of an attribute of the factory
   * \param v3 value of an attribute of the factory
   * \param n4 name of an attribute of the factory
   * \param v4 value of an attribute of the factory
   */
  void SetQueue (ObjectFactory &factory,
	   	 std::string type,
           	 std::string n1, const AttributeValue &v1,
           	 std::string n2, const AttributeValue &v2,
           	 std::string n3, const AttributeValue &v3,
           	 std::string n4, const AttributeValue &v4);


  /**
   * \brief Set the attributes of the link and propagation loss model
   * \param eirp EIRP
   * \param elevationAngle elevation angle of satellite beam
   * \param fspl free space loss
   * \param atmosphericLoss atmospheric loss
   * \param linkMargin link margin
   * \param dataRate data rate
   * \param rxGain receiver gain
   * \param rxLoss receiver loss
   */
  void SetConstellationAttributes (double eirp,
				   double elevationAngle,
				   double fspl,
				   double atmosphericLoss,
				   double linkMargin,
				   std::string dataRate,
				   double rxGain,
				   double rxLoss);
};

};

#endif /* LEO_CHANNEL_HELPER_H */
