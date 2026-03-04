// Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2013 Budiarto Herman
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Original work authors (from lte-enb-rrc.cc):
//   Nicola Baldo <nbaldo@cttc.es>
//   Marco Miozzo <mmiozzo@cttc.es>
//   Manuel Requena <manuel.requena@cttc.es>
//
// Converted to handover algorithm interface by:
//   Budiarto Herman <budiarto.herman@magister.fi>

#ifndef NR_SINR_DISTANCE_HANDOVER_ALGORITHM_H
#define NR_SINR_DISTANCE_HANDOVER_ALGORITHM_H

#include "ns3/net-device.h"
#include "ns3/nr-channel-helper.h"
#include "ns3/nr-handover-algorithm.h"
#include "ns3/nr-handover-management-sap.h"
#include "ns3/nr-phy-layer-configuration.h"
#include "ns3/nr-rrc-sap.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/string.h"

#include <map>

namespace ns3
{

/**
 * @brief Handover algorithm implementation based on SINR and Distance, Event
 *        A2 and Event A4.
 */
class NrSinrDistanceHandoverAlgorithm : public NrHandoverAlgorithm
{
  public:
    /// Creates a SINR-Distance handover algorithm instance.
    NrSinrDistanceHandoverAlgorithm();

    ~NrSinrDistanceHandoverAlgorithm() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    // inherited from NrHandoverAlgorithm
    void SetNrHandoverManagementSapUser(NrHandoverManagementSapUser* s) override;
    NrHandoverManagementSapProvider* GetNrHandoverManagementSapProvider() override;

    /**
     * @brief Set the Evaluation Precision
     */
    void SetEvaluationPrecision(Time precision);

    /**
     * @brief Set the SINR-Distance attachment Table
     */
    void SetSinrDistanceTable(const std::vector<SinrDistanceTableEntry>& table);

    /**
     * @brief Parses and sets the SINR-Distance attachment Table from a string
     */
    void SetSinrDistanceTableString(std::string tableStr);

    /**
     * @brief Get the SINR-Distance attachment Table string representation
     * needed for ns-3 attributes.
     */
    std::string GetSinrDistanceTableString() const;

    /**
     * @brief Set the Handover Margin Threshold
     */
    void SetHandoverMargin(double threshold);

    /// let the forwarder class access the protected and private members
    friend class MemberNrHandoverManagementSapProvider<NrSinrDistanceHandoverAlgorithm>;

  protected:
    // inherited from Object
    void DoInitialize() override;
    void DoDispose() override;

    // inherited from NrHandoverAlgorithm as a Handover Management SAP implementation
    void DoReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults) override;

  private:
    /**
     * Called when Event A2 is detected, then trigger a handover if needed.
     *
     * @param rnti The RNTI of the UE who reported the event.
     * @param servingCellRsrq The RSRQ of this cell as reported by the UE.
     */
    void EvaluateHandover(uint16_t rnti, uint8_t servingCellRsrq);

    /**
     * Determines if a neighbour cell is a valid destination for handover.
     * Currently always return true.
     *
     * @param cellId The cell ID of the neighbour cell.
     * @return True if the cell is a valid destination for handover.
     */
    bool IsValidNeighbour(uint16_t cellId);

    /**
     * Called when Event A4 is reported, then update the measurements table.
     * If the RNTI and/or cell ID is not found in the table, a corresponding
     * entry will be created. Only the latest measurements are stored in the
     * table.
     *
     * @param rnti The RNTI of the UE who reported the event.
     * @param cellId The cell ID of the measured cell.
     * @param rsrq The RSRQ of the cell as measured by the UE.
     */
    void UpdateNeighbourMeasurements(uint16_t rnti, uint16_t cellId, uint8_t rsrq);

    /// The expected measurement identities for A2 measurements.
    std::vector<uint8_t> m_a2MeasIds;
    /// The expected measurement identities for A4 measurements.
    std::vector<uint8_t> m_a4MeasIds;

    class UeMeasure : public SimpleRefCount<UeMeasure>
    {
      public:
        uint16_t m_cellId; ///< Cell ID.
        uint8_t m_rsrp;    ///< RSRP in quantized format.
        uint8_t m_rsrq;    ///< RSRQ in quantized format.
    };

    typedef std::map<uint16_t, Ptr<UeMeasure>> MeasurementRow_t;
    typedef std::map<uint16_t, MeasurementRow_t> MeasurementTable_t;

    // Table of measurement reports from all UEs (kept for potentially storing SINR)
    MeasurementTable_t m_neighbourCellMeasures;

    Time m_evaluationPrecision;
    std::vector<SinrDistanceTableEntry> m_sinrDistanceTable;
    double m_handoverMargin;

    /// Interface to the eNodeB RRC instance.
    NrHandoverManagementSapUser* m_handoverManagementSapUser;
    /// Receive API calls from the eNodeB RRC instance.
    NrHandoverManagementSapProvider* m_handoverManagementSapProvider;

    EventId m_evaluationEvent; // Event for periodic evaluation
    void EvaluateSinrDistanceAttachment();

}; // end of class NrSinrDistanceHandoverAlgorithm

} // end of namespace ns3

#endif /* NR_SINR_DISTANCE_HANDOVER_ALGORITHM_H */
