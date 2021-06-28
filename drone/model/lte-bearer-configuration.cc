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
#include "lte-bearer-configuration.h"

namespace ns3 {

class LteBearerConfigurationPriv
{
public:
  static const EpsBearer::Qci ParseBearerType (const std::string t)
  {
    if (t.compare("GBR_CONV_VOICE") == 0)
      return EpsBearer::Qci::GBR_CONV_VOICE;
    else if (t.compare("GBR_CONV_VIDEO") == 0)
      return EpsBearer::Qci::GBR_CONV_VIDEO;
    else if (t.compare("GBR_GAMING") == 0)
      return EpsBearer::Qci::GBR_GAMING;
    else if (t.compare("GBR_NON_CONV_VIDEO") == 0)
      return EpsBearer::Qci::GBR_NON_CONV_VIDEO;
    else if (t.compare("GBR_MC_PUSH_TO_TALK") == 0)
      return EpsBearer::Qci::GBR_MC_PUSH_TO_TALK;
    else if (t.compare("GBR_NMC_PUSH_TO_TALK") == 0)
      return EpsBearer::Qci::GBR_NMC_PUSH_TO_TALK;
    else if (t.compare("GBR_MC_VIDEO") == 0)
      return EpsBearer::Qci::GBR_MC_VIDEO;
    else if (t.compare("GBR_V2X") == 0)
      return EpsBearer::Qci::GBR_V2X;
    else if (t.compare("NGBR_IMS") == 0)
      return EpsBearer::Qci::NGBR_IMS;
    else if (t.compare("NGBR_VIDEO_TCP_OPERATOR") == 0)
      return EpsBearer::Qci::NGBR_VIDEO_TCP_OPERATOR;
    else if (t.compare("NGBR_VOICE_VIDEO_GAMING") == 0)
      return EpsBearer::Qci::NGBR_VOICE_VIDEO_GAMING;
    else if (t.compare("NGBR_VIDEO_TCP_PREMIUM") == 0)
      return EpsBearer::Qci::NGBR_VIDEO_TCP_PREMIUM;
    else if (t.compare("NGBR_VIDEO_TCP_DEFAULT") == 0)
      return EpsBearer::Qci::NGBR_VIDEO_TCP_DEFAULT;
    else if (t.compare("NGBR_MC_DELAY_SIGNAL") == 0)
      return EpsBearer::Qci::NGBR_MC_DELAY_SIGNAL;
    else if (t.compare("NGBR_MC_DATA") == 0)
      return EpsBearer::Qci::NGBR_MC_DATA;
    else if (t.compare("NGBR_V2X") == 0)
      return EpsBearer::Qci::NGBR_V2X;
    else if (t.compare("NGBR_LOW_LAT_EMBB") == 0)
      return EpsBearer::Qci::NGBR_LOW_LAT_EMBB;
    else if (t.compare("DGBR_DISCRETE_AUT_SMALL") == 0)
      return EpsBearer::Qci::DGBR_DISCRETE_AUT_SMALL;
    else if (t.compare("DGBR_DISCRETE_AUT_LARGE") == 0)
      return EpsBearer::Qci::DGBR_DISCRETE_AUT_LARGE;
    else if (t.compare("DGBR_ITS") == 0)
      return EpsBearer::Qci::DGBR_ITS;
    else if (t.compare("DGBR_ELECTRICITY") == 0)
      return EpsBearer::Qci::DGBR_ELECTRICITY;
    else
      NS_FATAL_ERROR ("Unsupported LTE Bearer QCI of type " << t);
  }

  static const GbrQosInformation BuildQosInformation (const uint64_t gbrDl,
                                                      const uint64_t gbrUl,
                                                      const uint64_t mbrDl,
                                                      const uint64_t mbrUl)
  {
    auto qosInfo = GbrQosInformation ();
    qosInfo.gbrDl = gbrDl;
    qosInfo.gbrUl = gbrUl;
    qosInfo.mbrDl = mbrDl;
    qosInfo.mbrUl = mbrUl;

    return qosInfo;
  }
};

LteBearerConfiguration::LteBearerConfiguration (const std::string type,
                                                const uint64_t gbrDl,
                                                const uint64_t gbrUl,
                                                const uint64_t mbrDl,
                                                const uint64_t mbrUl) :
  m_type {LteBearerConfigurationPriv::ParseBearerType (type)},
  m_qos {LteBearerConfigurationPriv::BuildQosInformation (gbrDl, gbrUl, mbrDl, mbrUl)}
{

}

LteBearerConfiguration::~LteBearerConfiguration ()
{

}

const EpsBearer::Qci
LteBearerConfiguration::GetType () const
{
  return m_type;
}

const GbrQosInformation
LteBearerConfiguration::GetQos () const
{
  return m_qos;
}

} // namespace ns3
