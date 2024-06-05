/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef MODEL_CONFIGURATION_VECTOR_H
#define MODEL_CONFIGURATION_VECTOR_H

#include "model-configuration.h"

#include <ns3/object.h>
#include <ns3/ptr.h>

#include <istream>
#include <ostream>
#include <vector>

namespace ns3
{

/**
 * \brief Represent a container of Model Configurations.
 */
class ModelConfigurationVector
{
  public:
    typedef std::vector<ModelConfiguration>::const_iterator Iterator;
    typedef std::vector<ModelConfiguration>::iterator MutableIterator;

    /** Default constructor. Create an empty vector of IRS Patches. */
    ModelConfigurationVector();
    /**
     * Create a new ModelConfigurationVector with exactly one Patch which has been
     * previously istantiated. This single Patch is specified by a smart pointer.
     *
     * \param patch The ModelConfiguration to add to the container.
     */
    ModelConfigurationVector(ModelConfiguration patch);
    /**
     * Create a new ModelConfigurationVector from a standard vector of ModelConfiguration
     */
    ModelConfigurationVector(const std::vector<ModelConfiguration>& patches);
    /**
     * Create a new ModelConfigurationVector from a concatenation of two ModelConfigurationVectors.
     *
     * \param v1 The first ModelConfigurationVector.
     * \param v2 The second ModelConfigurationVector.
     */
    ModelConfigurationVector(const ModelConfigurationVector& v1,
                             const ModelConfigurationVector& v2);
    /**
     * Create a new ModelConfigurationVector from a concatenation of three
     * ModelConfigurationVectors.
     *
     * \param v1 The first ModelConfigurationVector.
     * \param v2 The second ModelConfigurationVector.
     * \param v3 The third ModelConfigurationVector.
     */
    ModelConfigurationVector(const ModelConfigurationVector& v1,
                             const ModelConfigurationVector& v2,
                             const ModelConfigurationVector& v3);
    /**
     * Create a new ModelConfigurationVector from a concatenation of four ModelConfigurationVectors.
     *
     * \param v1 The first ModelConfigurationVector.
     * \param v2 The second ModelConfigurationVector.
     * \param v3 The third ModelConfigurationVector.
     * \param v4 The fourth ModelConfigurationVector.
     */
    ModelConfigurationVector(const ModelConfigurationVector& v1,
                             const ModelConfigurationVector& v2,
                             const ModelConfigurationVector& v3,
                             const ModelConfigurationVector& v4);
    /**
     * Create a new ModelConfigurationVector from a concatenation of five ModelConfigurationVectors.
     *
     * \param v1 The first ModelConfigurationVector.
     * \param v2 The second ModelConfigurationVector.
     * \param v3 The third ModelConfigurationVector.
     * \param v4 The fourth ModelConfigurationVector.
     * \param v5 The fifth ModelConfigurationVector.
     */
    ModelConfigurationVector(const ModelConfigurationVector& v1,
                             const ModelConfigurationVector& v2,
                             const ModelConfigurationVector& v3,
                             const ModelConfigurationVector& v4,
                             const ModelConfigurationVector& v5);
    /** Get an iterator to the first element of the vector. */
    Iterator Begin() const;
    /** Get an iterator to the first element of the vector. */
    Iterator begin() const;
    /** Get an iterator to the first element of the vector. */
    MutableIterator MutableBegin();
    /** Get an iterator to the last element of the vector. */
    Iterator End() const;
    /** Get an iterator to the last element of the vector. */
    Iterator end() const;
    /** Get an iterator to the last element of the vector. */
    MutableIterator MutableEnd();
    /** Get the number of elements in the vector. */
    uint32_t GetN() const;
    /** Get the ModelConfiguration at the given index. */
    ModelConfiguration operator[](uint32_t i) const;
    /** Get the ModelConfiguration at the given index. */
    ModelConfiguration Get(uint32_t i) const;
    /** Get the first ModelConfiguration in the vector. */
    ModelConfiguration GetFront() const;
    /** Get the last ModelConfiguration in the vector. */
    ModelConfiguration GetBack() const;
    /** Append an existing ModelConfigurationVector to the end of this one. */
    void Add(const ModelConfigurationVector& v);
    /** Append a new ModelConfiguration to the vector. */
    void Add(ModelConfiguration patch);

  private:
    std::vector<ModelConfiguration> m_confs;
};

ATTRIBUTE_HELPER_HEADER(ModelConfigurationVector);

std::ostream& operator<<(std::ostream& os, const ModelConfigurationVector& vector);
std::istream& operator>>(std::istream& is, ModelConfigurationVector& vector);

} // namespace ns3

#endif /* MODEL_CONFIGURATION_VECTOR_H */
