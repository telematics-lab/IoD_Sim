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
#ifndef PATCH_CONFIGURATOR_H
#define PATCH_CONFIGURATOR_H

#include <ns3/model-configuration-vector.h>
#include <ns3/object.h>

namespace ns3
{

/**
 * \ingroup irs
 *
 * \brief Base class for Patch Configurators.
 *
 * Defines the base object for Patch Configurators. Such a configurator updates the patch
 * organisation of an Irs during the simulation
 */
class PatchConfigurator : public Object
{
  public:
    /**
     * \brief This method schedule patch updates over time according to a certain logic. It must be
     * implemented by child classes
     */
    virtual void ScheduleUpdates() = 0;
    /**
     * \brief It sets a new patch configuration to the Irs to which the configurator is aggregated
     */
    void UpdateConfiguration(const ModelConfigurationVector& c);

  protected:
    virtual void DoDispose(void) = 0;
    virtual void DoInitialize(void) = 0;
};

} // namespace ns3

#endif /* PATCH_CONFIGURATOR_H */
