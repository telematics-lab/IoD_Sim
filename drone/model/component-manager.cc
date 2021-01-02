/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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

#include "component-manager.h"
#include <ns3/log.h>
#include <ns3/assert.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("ComponentManager");

void
ComponentManager::RegisterComponent(uintptr_t object, std::string comp)
{
  NS_LOG_FUNCTION(object << comp);

  if (m_components.find(object) == m_components.end())
  {
    NS_LOG_LOGIC("Object " << object << " not found, creating new map key.");
    m_components.emplace(object, std::unordered_set<std::string>());
  }

  std::unordered_set<std::string> *components = &m_components[object];

  NS_ASSERT_MSG(
      components->find(comp) == components->end(),
      "The component '" << comp << "' has already been registered!");


  components->insert(comp);
  NS_LOG_INFO("Component '" << comp << "' registered.");
}

bool
ComponentManager::CheckComponent(uintptr_t object, std::string comp)
{
  NS_LOG_FUNCTION(object << comp);

  if (m_components.find(object) == m_components.end())
  {
    NS_LOG_INFO("Object " << object << " has never registered a component");
    return false;
  }

  std::unordered_set<std::string> *components = &m_components[object];

  if (components->find(comp) == components->end())
  {
    NS_LOG_INFO("Object " << object << " has no '" << comp << "' component registered");
    return false;
  }

  NS_LOG_INFO("Object " << object << " has registered the '" << comp << "' component");
  return true;
}

void
ComponentManager::RequireComponent(uintptr_t object, std::string caller, std::string comp)
{
  NS_LOG_FUNCTION(object << caller << comp);

  NS_ASSERT_MSG(
      m_components.find(object) != m_components.end(),
      "No component has been registered for object " << object
      << " but the component '" << comp << "' is required before '" << caller << "' can be used!");

  std::unordered_set<std::string> *components = &m_components[object];

  NS_ASSERT_MSG(
      components->find(comp) != components->end(),
      "For the object " << object << " the component '" << comp
      << "' is required before '" << caller << "' can be used!");
  NS_LOG_INFO("Component '" << comp << "' is present");
}

} // namespace ns3