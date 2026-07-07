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

#ifndef NR_SINR_DISTANCE_HANDOVER_ALGORITHM_H
#define NR_SINR_DISTANCE_HANDOVER_ALGORITHM_H

#include "ns3/nr-handover-algorithm.h"
#include "ns3/nr-handover-management-sap.h"
#include "ns3/nr-rrc-sap.h"
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include <vector>
#include <string>

namespace ns3 {

/**
 * \ingroup handover
 * \brief Handover algorithm based on a distance-dependent SINR threshold table.
 *
 * This algorithm triggers handover to the best neighbor gNB if its estimated SNR
 * is higher than the serving gNB's SNR by at least `Threshold`, AND if the neighbor
 * gNB satisfies the minimum SINR requirements for its distance according to the
 * `DistanceSinrTable`.
 */
class NrSinrDistanceHandoverAlgorithm : public NrHandoverAlgorithm
{
public:
    struct TableEntry {
        double maxDistance;
        double minSinr;
    };

    NrSinrDistanceHandoverAlgorithm();
    virtual ~NrSinrDistanceHandoverAlgorithm();

    static TypeId GetTypeId();

    // Inherited from NrHandoverAlgorithm
    virtual void SetNrHandoverManagementSapUser(NrHandoverManagementSapUser* s) override;
    virtual NrHandoverManagementSapProvider* GetNrHandoverManagementSapProvider() override;

    /**
     * \brief Initialize the handover algorithm, configuring UE measurements.
     */
    virtual void DoInitialize() override;

protected:
    virtual void DoDispose() override;

    // Inherited from NrHandoverManagementSapProvider
    virtual void DoReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults) override;

private:
    /**
     * \brief Stored measurement IDs.
     */
    std::vector<uint8_t> m_measIds;
    class Provider : public NrHandoverManagementSapProvider
    {
    public:
        Provider(NrSinrDistanceHandoverAlgorithm* algorithm);
        virtual ~Provider() = default;
        virtual void ReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults) override;
    private:
        NrSinrDistanceHandoverAlgorithm* m_algorithm;
    };

    friend class Provider;

    void SetDistanceSinrTable(std::string tableStr);
    std::string GetDistanceSinrTable() const;

    void ParseTableString(const std::string& tableStr);

    NrHandoverManagementSapUser* m_handoverManagementSapUser;
    Provider* m_handoverManagementSapProvider;

    double m_threshold;
    Time m_timeToTrigger;
    Time m_handoverDelay;
    
    std::string m_tableStr;
    
    std::vector<TableEntry> m_table;
};

} // namespace ns3

#endif // NR_SINR_DISTANCE_HANDOVER_ALGORITHM_H
