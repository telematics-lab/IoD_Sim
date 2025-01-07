/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
 * Authors: Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#ifndef PROTOCOL_LAYER_H
#define PROTOCOL_LAYER_H

#include <ns3/object.h>

#include <libxml/xmlwriter.h>

namespace ns3
{

/**
 * \ingroup report
 *
 * \brief A base class to report a protocol layers
 */
class ProtocolLayer : public Object
{
  public:
    /**
     * Register the type using ns-3 TypeId System.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * Write Stack Layer report data to a XML file with a given handler
     *
     * \param handle the XML handler to write data on
     */
    virtual void Write(xmlTextWriterPtr handle);

  protected:
    /**
     * Object internal initialization
     */
    virtual void DoInitialize()
    {
        Object::DoInitialize();
    };

  private:
    std::string m_instancetypeid;
};

} // namespace ns3
#endif /* IPV4_LAYER_H */
