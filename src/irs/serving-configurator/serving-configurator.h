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
 */
#ifndef SERVING_CONFIGURATOR_H
#define SERVING_CONFIGURATOR_H

#include <ns3/object.h>

namespace ns3
{

/**
 * \ingroup irs
 * Defines the base object for Serving Configurators. Such a configurator updates Serving Nodes of
 * an Irs Patch during the simulation, in Patch lifetime.
 */
class ServingConfigurator : public Object
{
  public:
    /**
     * \brief This method schedule the updates of nodes to serve over time, according to a certain
     * logic. It must be implemented by child classes
     */
    virtual void ScheduleUpdates() = 0;
    /**
     * \brief It sets a new pair of nodes to be served by the Irs Patch to which the configurator is
     * aggregated
     * \param servingnodes the pair of nodes to be served by the patch
     */
    void UpdateServingNodes(std::pair<std::string, std::string> servingnodes);

  protected:
    virtual void DoDispose(void) = 0;
    virtual void DoInitialize(void) = 0;
};

} // namespace ns3

#endif /* SERVING_CONFIGURATOR_H */
