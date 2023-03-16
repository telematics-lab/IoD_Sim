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
#include "vector.h"

#include <sstream>

namespace ns3 {

// This internal namespace is used to implement the templated methods of VectorChecker
// that is instanciated in MakePairChecker. The non-templated base ns3::VectorChecker
// is returned in that function. This is the same pattern as ObjectPtrContainer.
namespace internal {

/**
 * Internal checker class templated to each AttributeChecker
 * for each entry in the pair.
 */
template <class T>
class VectorChecker : public AttributeChecker
{
public:
  /** Default Ctor */
  VectorChecker ();
  /**
   * Construct from an AttributeChecker
   * \param [in] checker The AttributeChecker that will be copied in.
   */
  VectorChecker (const AttributeChecker &checker);
  void SetChecker (Ptr<const AttributeChecker> checker);
  Ptr<const AttributeChecker> GetChecker () const;

private:
  /** The checker for each entry in the vector */
  Ptr<const AttributeChecker> m_checker;
};

template <class T>
VectorChecker<T>::VectorChecker () :
  m_checker {0}
{}

template <class T>
VectorChecker<T>::VectorChecker (Ptr<const AttributeChecker> &checker) :
  m_checker {checker}
{}

template <class T>
void
VectorChecker<T>::SetChecker (Ptr<const AttributeChecker> checker)
{
  m_checker = checker;
}

template <class T>
Ptr<const AttributeChecker>
VectorChecker<T>::GetChecker () const
{
  return m_checker;
}

} // namespace internal

template <class T>
Ptr<const AttributeChecker>
MakeVectorChecker (const VectorValue<T>& value)
{
  return MakeVectorChecker<T> ();
}

template <class T>
Ptr<const AttributeChecker>
MakeVectorChecker ()
{
  std::string pairName;
  std::string underlyingType;
  std::string typeName = typeid (T).name ();

  {
    std::ostringstream oss;
    oss << "ns3::VectorValue<" << typeName << ">";
    pairName = oss.str ();
  }

  {
    std::ostringstream oss;

  }

}

} // namespace ns3

template <typename T>
Ptr<const AttributeChecker>
MakeVectorChecker (void)
{
  return MakeSimpleAttributeChecker<VectorValue<T>, VectorChecker<T>> ("VectorValue", "std::vector");
}
