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

#ifndef APPLICATION_CONFIGURATION_H
#define APPLICATION_CONFIGURATION_H

#include <string>

#include <ns3/object.h>

namespace ns3 {

class ApplicationConfiguration : public Object
{
  public:
    ApplicationConfiguration ();
    ~ApplicationConfiguration ();

    const std::string GetType () const;
    const double GetStartTime () const;
    const double GetStopTime () const;

  private:
    std::string m_type;
    double m_startTime;
    double m_stopTime;

};

} // namespace ns3

#endif /* APPLICATION_CONFIGURATION_H */
