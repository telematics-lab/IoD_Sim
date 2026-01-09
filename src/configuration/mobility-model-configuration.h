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
#ifndef MOBILITY_MODEL_CONFIGURATION_H
#define MOBILITY_MODEL_CONFIGURATION_H

#include <ns3/model-configuration.h>
#include <ns3/vector.h>

#include <optional>

namespace ns3
{

/**
 * \brief Data class specific for Mobility Models configuration, which may set nodes to an initial
 * position through a position allocator.
 */
class MobilityModelConfiguration : public ModelConfiguration
{
  public:
    MobilityModelConfiguration(const std::string modelName,
                               const std::vector<ModelConfiguration::Attribute> modelAttributes,
                               const std::optional<Vector> initialPosition);
    const std::optional<Vector> GetInitialPosition() const;

  private:
    const std::optional<Vector> m_initialPosition;
};

} // namespace ns3

#endif /* MOBILITY_MODEL_CONFIGURATION_H */
