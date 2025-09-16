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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#include "report-container.h"

#include "report-drone.h"
#include "report-remote.h"
#include "report-zsp.h"

#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/object-factory.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReportContainer");

template <class T>
ReportContainer<T>::ReportContainer(const std::string groupName)
    : m_groupName{groupName}
{
    NS_LOG_FUNCTION(this);
}

template <class T>
typename ReportContainer<T>::Iterator
ReportContainer<T>::Begin() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_entities.begin();
}

template <class T>
typename ReportContainer<T>::Iterator
ReportContainer<T>::End() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_entities.end();
}

template <class T>
uint32_t
ReportContainer<T>::GetN() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_entities.size();
}

template <class T>
Ptr<T>
ReportContainer<T>::Get(const uint32_t i) const
{
    NS_LOG_FUNCTION(i);

    return m_entities[i];
}

template <class T>
void
ReportContainer<T>::Add(Ptr<T> entity)
{
    NS_LOG_FUNCTION(entity);

    m_entities.push_back(entity);
}

template <class T>
void
ReportContainer<T>::Add(uint32_t entityUid)
{
    NS_LOG_FUNCTION(entityUid);
    auto entityReport = CreateObjectWithAttributes<T>("Reference", IntegerValue(entityUid));

    entityReport->Initialize();
    Add(entityReport);
}

template <class T>
void
ReportContainer<T>::Write(xmlTextWriterPtr h) const
{
    NS_LOG_FUNCTION(h);
    if (!h)
    {
        NS_LOG_WARN("Passed handler is not valid: " << h
                                                    << ". "
                                                       "Data will be discarded.");
        return;
    }

    int rc;

    rc = xmlTextWriterStartElement(h, BAD_CAST m_groupName.c_str());
    NS_ASSERT(rc >= 0);

    /* Nested Elements */
    for (auto entity = Begin(); entity != End(); entity++)
        (*entity)->Write(h);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);
}

// Explicit template instantiations
template class ReportContainer<ReportDrone>;
template class ReportContainer<ReportRemote>;
template class ReportContainer<ReportZsp>;

}; // namespace ns3
