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
#ifndef DEFINED_SERVING_CONFIGURATOR_H
#define DEFINED_SERVING_CONFIGURATOR_H

#include <ns3/object.h>

#include <ns3/double-vector.h>
#include <ns3/serving-configurator.h>
#include <ns3/str-vec.h>

namespace ns3 {

/**
 * \brief Defines the base object that updates Serving Nodes of an Irs Patch with defined periods.
 */
class DefinedServingConfigurator : public ServingConfigurator
{
public:

  static TypeId GetTypeId (void);

  DefinedServingConfigurator ();
  ~DefinedServingConfigurator ();

 void ScheduleUpdates ();
void SetPeriods(const DoubleVector &a);

  void SetServingPairs(const StrVec& pairs);

protected:
  void DoDispose (void);
  void DoInitialize (void);
private:
  std::vector<std::pair<std::string,std::string>> m_servingpairs;
  std::vector<double> m_periods;
};

} // namespace ns3

#endif /* DEFINED_SERVING_CONFIGURATOR_H */