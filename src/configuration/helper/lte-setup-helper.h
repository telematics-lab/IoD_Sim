/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#ifndef LTE_SETUP_HELPER_H
#define LTE_SETUP_HELPER_H

#include <ns3/lte-helper.h>

namespace ns3
{

/**
 * Helper to enhance ns-3 LteHelper functionalities with additional
 * features, without modifying objects external to IoD_Sim.
 */
class LteSetupHelper
{
  public:
    /**
     * Create an eNodeB device (LteEnbNetDevice) on the given node.
     * \param helper A pre-inizialized instance of the Lte Helper.
     * \param n the node where the device is to be installed
     * \return pointer to the created device
     */
    static Ptr<NetDevice> InstallSingleEnbDevice(Ptr<LteHelper> helper, Ptr<Node> n);

    /**
     * Create a User Equipment device (LteUeNetDevice) on the given node.
     * \param helper A pre-inizialized instance of the Lte Helper.
     * \param n Target node to install the LTE device.
     * \return pointer to the created device.
     */
    static Ptr<NetDevice> InstallSingleUeDevice(Ptr<LteHelper> helper, Ptr<Node> n);

  private:
    LteSetupHelper();
};

} // namespace ns3

#endif /* LTE_SETUP_HELPER_H */
