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
#ifndef DEFINED_PATCH_CONFIGURATOR_H
#define DEFINED_PATCH_CONFIGURATOR_H

#include <ns3/object.h>
#include <ns3/pointer.h>

#include <ns3/double-vector.h>
#include <ns3/model-configuration-matrix.h>
#include <ns3/patch-configurator.h>

namespace ns3 {

/**
 * \brief Defines the base object that updates the patch configuration of an Irs during the simulation with defined periods.
 */
class DefinedPatchConfigurator : public PatchConfigurator
{
public:
  static TypeId GetTypeId (void);

  DefinedPatchConfigurator ();
  ~DefinedPatchConfigurator ();

  void ScheduleUpdates ();
  void SetPeriods(const DoubleVector &a);
  void SetLifeTimes();

protected:
  void DoDispose (void);
  void DoInitialize (void);

private:
  ModelConfigurationMatrix m_configurations;
  std::vector<double> m_periods;
};

} // namespace ns3

#endif /* DEFINED_PATCH_CONFIGURATOR_H */