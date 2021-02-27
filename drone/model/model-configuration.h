/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
#ifndef MODEL_CONFIGURATION_H
#define MODEL_CONFIGURATION_H

#include <string>
#include <vector>

#include <ns3/attribute.h>

namespace ns3 {

class ModelConfiguration
{
public:
  ModelConfiguration (const std::string name,
                      const std::vector<std::pair<std::string, Ptr<AttributeValue>>> attributes);
  ~ModelConfiguration ();

  const std::string GetName ();
  const std::vector<std::pair<std::string, Ptr<AttributeValue>>> GetAttributes ();

private:
  const std::string m_name;
  const std::vector<std::pair<std::string, Ptr<AttributeValue>>> m_attributes;
};

} // namespace ns3

#endif /* MODEL_CONFIGURATION_H */
