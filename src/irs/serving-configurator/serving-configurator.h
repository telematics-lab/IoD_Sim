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
#ifndef SERVING_CONFIGURATOR_H
#define SERVING_CONFIGURATOR_H

#include <ns3/object.h>

namespace ns3 {

/**
 * \brief Defines the base object that updates Serving Nodes of an Irs Patch during the simulation, in Patch lifetime.
 */
class ServingConfigurator : public Object
{
public:

  ServingConfigurator ();
  ~ServingConfigurator ();

  virtual void ScheduleUpdates () = 0;
  void UpdateServingNodes (std::pair<std::string,std::string> servingnodes);

protected:
  virtual void DoDispose (void) = 0;
  virtual void DoInitialize (void) = 0;
};

} // namespace ns3

#endif /* SERVING_CONFIGURATOR_H */