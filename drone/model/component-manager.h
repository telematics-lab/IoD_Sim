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

#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <ns3/singleton.h>


/// Use this as the only interface for ComponentManager to register a component with a custom name at the end of a method before returning.
#define NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME(comp_name) ComponentManager::Get ()->RegisterComponent ((uintptr_t)(void*)this, comp_name)
/// Use this as the only interface for ComponentManager to register a component with the name of the method at the end of it before returning.
#define NS_COMPMAN_REGISTER_COMPONENT() NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME (__FUNCTION__)
/// Use this as the only interface for ComponentManager to check if a component has been registered
#define NS_COMPMAN_CHECK_COMPONENT(param) ComponentManager::Get ()->CheckComponent ((uintptr_t)(void*)this, param)
/// Use this as the only interface for ComponentManager to check if the calling method has been registered
#define NS_COMPMAN_CHECK_THIS_COMPONENT() NS_COMPMAN_CHECK_COMPONENT (__FUNCTION__)
///Use this as the only interface for ComponentManager to check for multiple component within a range (extremes included)
#define NS_COMPMAN_CHECK_MULTI_COMPONENT_RANGE(param, start, stop) ComponentManager::Get ()->CheckMultiComponent ((uintptr_t)(void*)this, param, start, stop)
/// Use this as the only interface for ComponentManager to check for multiple component with same name (range 0 to num-1)
#define NS_COMPMAN_CHECK_MULTI_COMPONENT(param, num) NS_COMPMAN_CHECK_MULTI_COMPONENT_RANGE (param, 0, num - 1)
/// Use this as the only interface for ComponentManager to require a component at the beginning of a method.
#define NS_COMPMAN_REQUIRE_COMPONENT(param) ComponentManager::Get ()->RequireComponent ((uintptr_t)(void*)this, __FUNCTION__, param)
/// Use this as the only interface for ComponentManager to exclude a component at the beginning of a method.
#define NS_COMPMAN_EXCLUDE_COMPONENT(param) ComponentManager::Get ()->ExcludeComponent ((uintptr_t)(void*)this, __FUNCTION__, param)
/// Use this as the only interface for ComponentManager to avoid a component to be repeated (at the beginning of a method) if it is going to be registered.
#define NS_COMPMAN_ENSURE_UNIQUE() NS_COMPMAN_EXCLUDE_COMPONENT (__FUNCTION__)

namespace ns3
{

/**
 * \brief Allows methods of an object to require other methods to be called before
 *        by requiring the needed components and then registering itself as such for
 *        other methods. Each object has its own components and the ComponentManager
 *        automatically keeps records of them for each object indipendently.
 */
class ComponentManager : public Singleton<ComponentManager>
{
public:
  /**
   * \brief Adds the caller function to the set of components of the
   *        proprietary object.
   *        Use with `NS_COMPMAN_REGISTER_COMPONENT()` or `NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("component_name")`
   * \param object The pointer to the caller object. The component will be added to this object's set.
   *               The macro uses `(uintptr_t)(void*)this` for that.
   * \param comp The component name to register.
   *             The macro without name uses builtin `__FUNCTION__` for that
   */
  void RegisterComponent (uintptr_t object, std::string comp);

  /**
   * \brief Returns `true` if the component exists between the registered components else `false`.
   *        Use with `NS_COMPMAN_CHECK_COMPONENT("component_name")`
   * \param object The pointer to the caller object. The component will be searched in this object's set.
   *               The macro uses `(uintptr_t)(void*)this` for that.
   * \param comp The component name to check.
   *             The macro without name uses builtin `__FUNCTION__` for that
   */
  bool CheckComponent (uintptr_t object, std::string comp);

  /**
   * \brief Returns `true` if all the components exist between the registered components else `false`.
   *        Use with `NS_COMPMAN_CHECK_MUKTI_COMPONENT("component_name", number)`
   * \param object The pointer to the caller object. The components will be searched in this object's set.
   *               The macro uses `(uintptr_t)(void*)this` for that.
   * \param comp The component base name to check.
   * \param start The starting number for the range of multi components to check (included)
   * \param stop The ending number for the range of multi components to check (included)
   */
  bool CheckMultiComponent (uintptr_t object, std::string comp, uint32_t start, uint32_t stop);

  /**
   * \brief Asks for a method of the same object to be called before the caller method.
   *        If it's not been called stops the program. (Inverse of `ExcludeComponent`)
   *        Use with `NS_COMPMAN_REGISTER_COMPONENT("RequiredMethodName")`
   * \param object The pointer to the caller object. The component will be searched inside this object's set.
   *               The macro uses `(uintptr_t)(void*)this` for that.
   * \param caller The name of the caller function that is requiring the component.
   *               The macro uses builtin `__FUNCTION__` for that.
   * \param comp The component name to search.
   *             This should be explicitly passed even inside the macro.
   */
  void RequireComponent (uintptr_t object, std::string caller, std::string comp);

  /**
   * \brief Asks for a method of the same object to NOT be called before the caller method.
   *        If it's been called already stops the program. (Inverse of `RequireComponent`)
   * \param object The pointer to the caller object. The component will be searched inside this object's set.
   *               The macro uses `(uintptr_t)(void*)this` for that.
   * \param caller The name of the caller function that is excluding the component.
   *               The macro uses builtin `__FUNCTION__` for that.
   * \param comp The component name to search.
   *             This should be explicitly passed even inside the macro.
   */
  void ExcludeComponent (uintptr_t object, std::string caller, std::string comp);

private:
  std::unordered_map<uintptr_t, std::unordered_set<std::string> > m_components;
};

} // namespace ns3

#endif // COMPONENT_MANAGER_H
