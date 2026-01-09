/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#ifndef TCP_STUB_CLIENT_APPLICATION_H
#define TCP_STUB_CLIENT_APPLICATION_H

#include "tcp-storage-client-application.h"

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/storage-peripheral.h>
#include <ns3/traced-callback.h>

namespace ns3
{

/**
 * \ingroup applications
 * \brief Application that sends random packets with Seq.Num. and Timestamp to a remote server.
 */
class TcpStubClientApplication : public TcpStorageClientApplication
{
  public:
    static TypeId GetTypeId();

  protected:
    virtual void DoInitialize();
    virtual void StartApplication();

  private:
    /// \brief Schedule intial handshake with remote server
    bool Connect();
    /// \brief Send a random packet of a given size
    void SendPacket();

    double m_txFrequency;   /// Transmission frequency
    ns3::Time m_txInterval; /// Frequency inverse
};

} // namespace ns3

#endif /* TCP_STUB_CLIENT_APPLICATION_H */
