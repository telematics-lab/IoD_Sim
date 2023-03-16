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
#ifndef MODEL_CONFIGURATION_MATRIX_H
#define MODEL_CONFIGURATION_MATRIX_H

#include <istream>
#include <ostream>
#include <vector>

#include <ns3/model-configuration-vector.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

namespace ns3 {

/**
 * \brief Represent a container of Model Configurations.
 */
class ModelConfigurationMatrix
{
public:
  typedef std::vector<ModelConfigurationVector>::const_iterator Iterator;
  typedef std::vector<ModelConfigurationVector>::iterator MutableIterator;

  /** Default constructor. Create an empty vector of IRS Patches. */
  ModelConfigurationMatrix ();
  /**
   * Create a new ModelConfigurationMatrix with exactly one Patch which has been
   * previously istantiated. This single Patch is specified by a smart pointer.
   *
   * \param patch The ModelConfiguration to add to the container.
   */
  ModelConfigurationMatrix (ModelConfigurationVector patch);
  /**
   * Create a new ModelConfigurationMatrix from a standard vector of ModelConfiguration
   */
  ModelConfigurationMatrix (const std::vector<ModelConfigurationVector>& patches);
  /**
   * Create a new ModelConfigurationMatrix from a concatenation of two ModelConfigurationMatrixs.
   *
   * \param v1 The first ModelConfigurationMatrix.
   * \param v2 The second ModelConfigurationMatrix.
   */
  ModelConfigurationMatrix (const ModelConfigurationMatrix &v1,
                            const ModelConfigurationMatrix &v2);
  /**
   * Create a new ModelConfigurationMatrix from a concatenation of three ModelConfigurationMatrixs.
   *
   * \param v1 The first ModelConfigurationMatrix.
   * \param v2 The second ModelConfigurationMatrix.
   * \param v3 The third ModelConfigurationMatrix.
   */
  ModelConfigurationMatrix (const ModelConfigurationMatrix &v1,
                            const ModelConfigurationMatrix &v2,
                            const ModelConfigurationMatrix &v3);
  /**
   * Create a new ModelConfigurationMatrix from a concatenation of four ModelConfigurationMatrixs.
   *
   * \param v1 The first ModelConfigurationMatrix.
   * \param v2 The second ModelConfigurationMatrix.
   * \param v3 The third ModelConfigurationMatrix.
   * \param v4 The fourth ModelConfigurationMatrix.
   */
  ModelConfigurationMatrix (const ModelConfigurationMatrix &v1,
                            const ModelConfigurationMatrix &v2,
                            const ModelConfigurationMatrix &v3,
                            const ModelConfigurationMatrix &v4);
  /**
   * Create a new ModelConfigurationMatrix from a concatenation of five ModelConfigurationMatrixs.
   *
   * \param v1 The first ModelConfigurationMatrix.
   * \param v2 The second ModelConfigurationMatrix.
   * \param v3 The third ModelConfigurationMatrix.
   * \param v4 The fourth ModelConfigurationMatrix.
   * \param v5 The fifth ModelConfigurationMatrix.
   */
  ModelConfigurationMatrix (const ModelConfigurationMatrix &v1,
                            const ModelConfigurationMatrix &v2,
                            const ModelConfigurationMatrix &v3,
                            const ModelConfigurationMatrix &v4,
                            const ModelConfigurationMatrix &v5);
  /** Get an iterator to the first element of the vector. */
  Iterator Begin () const;
  /** Get an iterator to the first element of the vector. */
  Iterator begin () const;
  /** Get an iterator to the first element of the vector. */
  MutableIterator MutableBegin ();
  /** Get an iterator to the last element of the vector. */
  Iterator End () const;
  /** Get an iterator to the last element of the vector. */
  Iterator end () const;
  /** Get an iterator to the last element of the vector. */
  MutableIterator MutableEnd ();
  /** Get the number of elements in the vector. */
  uint32_t GetN () const;
  /** Get the ModelConfiguration at the given index. */
  ModelConfigurationVector operator[] (uint32_t i) const;
  /** Get the ModelConfiguration at the given index. */
  ModelConfigurationVector Get (uint32_t i) const;
  /** Get the first ModelConfiguration in the vector. */
  ModelConfigurationVector GetFront () const;
  /** Get the last ModelConfiguration in the vector. */
  ModelConfigurationVector GetBack () const;
  /** Append an existing ModelConfigurationMatrix to the end of this one. */
  void Add (const ModelConfigurationMatrix &v);
  /** Append a new ModelConfiguration to the vector. */
  void Add (ModelConfigurationVector patch);

private:
  std::vector<ModelConfigurationVector> m_confs;
};

ATTRIBUTE_HELPER_HEADER (ModelConfigurationMatrix);

std::ostream& operator<< (std::ostream &os, const ModelConfigurationMatrix &vector);
std::istream& operator>> (std::istream &is, ModelConfigurationMatrix &vector);

} // namespace ns3

#endif /* MODEL_CONFIGURATION_MATRIX_H */
