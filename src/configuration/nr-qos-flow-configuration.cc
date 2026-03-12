/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#include "nr-qos-flow-configuration.h"

namespace ns3
{
class NrQosFlowConfigurationPriv
{
  public:
    static const NrQosFlow::FiveQi ParseBearerType(const std::string t)
    {
        if (t == "GBR_CONV_VOICE")
        {
            return NrQosFlow::FiveQi::GBR_CONV_VOICE;
        }
        else if (t == "GBR_CONV_VIDEO")
        {
            return NrQosFlow::FiveQi::GBR_CONV_VIDEO;
        }
        else if (t == "GBR_GAMING")
        {
            return NrQosFlow::FiveQi::GBR_GAMING;
        }
        else if (t == "GBR_NON_CONV_VIDEO")
        {
            return NrQosFlow::FiveQi::GBR_NON_CONV_VIDEO;
        }
        else if (t == "GBR_MC_PUSH_TO_TALK")
        {
            return NrQosFlow::FiveQi::GBR_MC_PUSH_TO_TALK;
        }
        else if (t == "GBR_NMC_PUSH_TO_TALK")
        {
            return NrQosFlow::FiveQi::GBR_NMC_PUSH_TO_TALK;
        }
        else if (t == "GBR_MC_VIDEO")
        {
            return NrQosFlow::FiveQi::GBR_MC_VIDEO;
        }
        else if (t == "GBR_V2X")
        {
            return NrQosFlow::FiveQi::GBR_V2X;
        }
        else if (t == "GBR_LIVE_UL_71")
        {
            return NrQosFlow::FiveQi::GBR_LIVE_UL_71;
        }
        else if (t == "GBR_LIVE_UL_72")
        {
            return NrQosFlow::FiveQi::GBR_LIVE_UL_72;
        }
        else if (t == "GBR_LIVE_UL_73")
        {
            return NrQosFlow::FiveQi::GBR_LIVE_UL_73;
        }
        else if (t == "GBR_LIVE_UL_74")
        {
            return NrQosFlow::FiveQi::GBR_LIVE_UL_74;
        }
        else if (t == "GBR_LIVE_UL_76")
        {
            return NrQosFlow::FiveQi::GBR_LIVE_UL_76;
        }
        else if (t == "NGBR_IMS")
        {
            return NrQosFlow::FiveQi::NGBR_IMS;
        }
        else if (t == "NGBR_VIDEO_TCP_OPERATOR")
        {
            return NrQosFlow::FiveQi::NGBR_VIDEO_TCP_OPERATOR;
        }
        else if (t == "NGBR_VOICE_VIDEO_GAMING")
        {
            return NrQosFlow::FiveQi::NGBR_VOICE_VIDEO_GAMING;
        }
        else if (t == "NGBR_VIDEO_TCP_PREMIUM")
        {
            return NrQosFlow::FiveQi::NGBR_VIDEO_TCP_PREMIUM;
        }
        else if (t == "NGBR_VIDEO_TCP_DEFAULT")
        {
            return NrQosFlow::FiveQi::NGBR_VIDEO_TCP_DEFAULT;
        }
        else if (t == "NGBR_MC_DELAY_SIGNAL")
        {
            return NrQosFlow::FiveQi::NGBR_MC_DELAY_SIGNAL;
        }
        else if (t == "NGBR_MC_DATA")
        {
            return NrQosFlow::FiveQi::NGBR_MC_DATA;
        }
        else if (t == "NGBR_V2X")
        {
            return NrQosFlow::FiveQi::NGBR_V2X;
        }
        else if (t == "NGBR_LOW_LAT_EMBB")
        {
            return NrQosFlow::FiveQi::NGBR_LOW_LAT_EMBB;
        }
        else if (t == "DGBR_DISCRETE_AUT_SMALL")
        {
            return NrQosFlow::FiveQi::DGBR_DISCRETE_AUT_SMALL;
        }
        else if (t == "DGBR_DISCRETE_AUT_LARGE")
        {
            return NrQosFlow::FiveQi::DGBR_DISCRETE_AUT_LARGE;
        }
        else if (t == "DGBR_ITS")
        {
            return NrQosFlow::FiveQi::DGBR_ITS;
        }
        else if (t == "DGBR_ELECTRICITY")
        {
            return NrQosFlow::FiveQi::DGBR_ELECTRICITY;
        }
        else if (t == "DGBR_V2X")
        {
            return NrQosFlow::FiveQi::DGBR_V2X;
        }
        else if (t == "DGBR_INTER_SERV_87")
        {
            return NrQosFlow::FiveQi::DGBR_INTER_SERV_87;
        }
        else if (t == "DGBR_INTER_SERV_88")
        {
            return NrQosFlow::FiveQi::DGBR_INTER_SERV_88;
        }
        else if (t == "DGBR_VISUAL_CONTENT_89")
        {
            return NrQosFlow::FiveQi::DGBR_VISUAL_CONTENT_89;
        }
        else if (t == "DGBR_VISUAL_CONTENT_90")
        {
            return NrQosFlow::FiveQi::DGBR_VISUAL_CONTENT_90;
        }
        else
        {
            NS_FATAL_ERROR("Unsupported LTE Bearer QCI of type " << t);
        }
    }

    static const NrGbrQosInformation BuildQosInformation(const uint64_t gbrDl,
                                                         const uint64_t gbrUl,
                                                         const uint64_t mbrDl,
                                                         const uint64_t mbrUl)
    {
        auto qosInfo = NrGbrQosInformation();
        qosInfo.gbrDl = gbrDl;
        qosInfo.gbrUl = gbrUl;
        qosInfo.mbrDl = mbrDl;
        qosInfo.mbrUl = mbrUl;

        return qosInfo;
    }
};

NS_OBJECT_ENSURE_REGISTERED(NrQosFlowConfiguration);

TypeId
NrQosFlowConfiguration::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrQosFlowConfiguration")
                            .SetParent<Object>()
                            .SetGroupName("IoD_Sim");
    return tid;
}

NrQosFlowConfiguration::NrQosFlowConfiguration(const std::string type,
                                             const uint64_t gbrDl,
                                             const uint64_t gbrUl,
                                             const uint64_t mbrDl,
                                             const uint64_t mbrUl)
    : m_type{NrQosFlowConfigurationPriv::ParseBearerType(type)},
      m_qos{NrQosFlowConfigurationPriv::BuildQosInformation(gbrDl, gbrUl, mbrDl, mbrUl)}
{
}

NrQosFlowConfiguration::NrQosFlowConfiguration(const std::string type)
    : m_type{NrQosFlowConfigurationPriv::ParseBearerType(type)}
{
}

const NrQosFlow::FiveQi
NrQosFlowConfiguration::GetType() const
{
    return m_type;
}

const NrGbrQosInformation
NrQosFlowConfiguration::GetQos() const
{
    return m_qos;
}

} // namespace ns3
