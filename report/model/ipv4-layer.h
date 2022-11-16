/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#ifndef IPV4_LAYER_H
#define IPV4_LAYER_H
#include <libxml/xmlwriter.h>

#include "protocol-layer.h"

namespace ns3 {

class Ipv4Layer : public ProtocolLayer
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Default constructor
   */
  Ipv4Layer ();
  /**
   * Default destructor
   */
  ~Ipv4Layer ();

  /**
   * Write IPv4 Layer report data to a XML file with a given handler
   *
   * \param handle the XML handler to write data on
   */
  virtual void Write (xmlTextWriterPtr handle);
private:
    std::string m_hostAddr;      /// IPv4 Address used
    std::string m_broadcastAddr; /// Network broadcast address
};

} // namespace ns3

#endif /* IPV4_LAYER_H */
