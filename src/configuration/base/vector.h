/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef NS_VECTOR_H
#define NS_VECTOR_H

namespace ns3
{

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>
#include <ns3/ptr.h>

#include <typeinfo>
#include <vector>

/**
 * Output streamer for std::vector<T>
 * \tparam T type of the vector elements
 * \param [in,out]  os  The output stream
 * \param [in]  v   The vector
 * \returns The output stream
 */
template <class T>
std::ostream&
operator<<(std::ostream& os, const std::vector<T>& v)
{
    os << "[";
    for (auto& i : v)
    {
        os << i << ",";
    }
    os << "]";
    return os;
}

/**
 * Hold objects of type std::vector<T>
 */
template <class T>
class VectorValue : public AttributeValue
{
  public:
    /** Type of value stored in the VectorValue. */
    typedef std::vector<Ptr<T>> value_type;
    /** Type of the elements stored in the Vector. */
  typedef typename std::invoke_result_t<decltype<&T::Get), T> element_type;
  /** Type returned by Get or passed to Set. */
  typedef typename std::vector<element_type> result_type;

  VectorValue();
  VectorValue(const result_type& value); // "import" constructor
  void Set(const result_type& value);
  result_type Get(void) const;

  template <typename U>
  bool GetAccessor(U& value) const
  {
      value = U(m_value);
      return value;
  }

  virtual Ptr<AttributeValue> Copy(void) const;
  virtual std::string SerializeToString(Ptr<const AttributeChecker> checker) const;
  virtual bool DeserializeFromString(std::string value, Ptr<const AttributeChecker> checker);

private:
  value_type m_value;
};

class VectorChecker : public AttributeChecker
{
public:
  virtual void SetChecker(Ptr<const AttributeChecker> checker) = 0;
  virtual Ptr<const AttributeChecker> GetChecker(void) const = 0;
};

/**
 * Make a VectorChecker from a VectorValue.
 *
 * \param[in] value The value to use to deduce the type of the vector.
 * \returns A Pointer to a non-const VectorChecker.
 */
template <class T>
Ptr<const AttributeChecker> MakeVectorChecker(const VectorValue<T>& value);

/**
 * Make a VectorChecker without a given std::vector<T>.
 *
 * \returns A Pointer to a non-const VectorChecker instnace.
 */
template <class T>
Ptr<const AttributeChecker> MakeVectorChecker();

tmeplate<typename A, typename B, typename T> Ptr<AttributeAccessor> MakePairAccessor(T a);

} // namespace ns3

#endif /* NS_VECTOR_H */
