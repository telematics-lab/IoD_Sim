/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#ifndef NAT_APPLICATION_H
#define NAT_APPLICATION_H

#include <unordered_map>

#include <ns3/application.h>
#include <ns3/inet-socket-address.h>
#include <ns3/net-device.h>
#include <ns3/ptr.h>

namespace ns3 {

class NatApplication : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  // virtual void DoDispose ();

  NatApplication ();
  virtual ~NatApplication ();

  bool RecvPktFromIntNetDev (Ptr<NetDevice> netdev,
                             Ptr<const Packet> pkt,
                             uint16_t protocol,
                             const Address& sender,
                             const Address& receiver,
                             enum NetDevice::PacketType pktType);

  bool RecvPktFromExtNetDev (Ptr<NetDevice> netdev,
                             Ptr<const Packet> pkt,
                             uint16_t protocol,
                             const Address& sender);

  bool RecvPktFromNetDev (Ptr<NetDevice> netdev,
                          Ptr<const Packet> pkt,
                          uint16_t protocol,
                          const Address& sender,
                          const Address& receiver,
                          enum NetDevice::PacketType pktType);

  typedef struct __NatEntry {
    Ipv4Address ipv4Addr;
    uint32_t port;
    Address macAddr;
  } NatEntry;

protected:
  virtual void DoInitialize (void);

private:
  class NatEntryHash
  {
  public:
    size_t operator() (NatEntry const &x) const;
  };

  typedef std::unordered_map<NatEntry, uint16_t, NatEntryHash> NatTable;
  typedef std::unordered_map<NatEntry, uint16_t, NatEntryHash>::iterator NatTableI;

  NatTableI InverseLookup (uint16_t port);

  uint32_t m_intNetDevId;
  uint32_t m_extNetDevId;
  Ptr<NetDevice> m_intNetDev;
  Ptr<NetDevice> m_extNetDev;
  NatTable m_natTable;
  uint16_t m_curNatPort;
};

bool operator== (const NatApplication::NatEntry x, const NatApplication::NatEntry y);

} // namespace ns3

#endif /* NAT_APPLICATION_H */
