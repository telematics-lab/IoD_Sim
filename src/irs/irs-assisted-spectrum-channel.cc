/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
 * Copyright (C) 2018-2024 The IoD_Sim Authors
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
#include "irs-assisted-spectrum-channel.h"

#include "irs-list.h"
#include "irs-patch.h"

#include <ns3/angles.h>
#include <ns3/antenna-model.h>
#include <ns3/boolean.h>
#include <ns3/buildings-channel-condition-model.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/object.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-converter.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-propagation-loss-model.h>
#include <ns3/spectrum-value.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

constexpr double SPEED_OF_LIGHT = 299792458.0;

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IrsAssistedSpectrumChannel");

NS_OBJECT_ENSURE_REGISTERED(IrsAssistedSpectrumChannel);

void
IrsAssistedSpectrumChannel::DoDispose()
{
    NS_LOG_FUNCTION(this);
    MultiModelSpectrumChannel::DoDispose();
}

TypeId
IrsAssistedSpectrumChannel::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::IrsAssistedSpectrumChannel")
            .SetParent<MultiModelSpectrumChannel>()
            .SetGroupName("Spectrum")
            .AddConstructor<IrsAssistedSpectrumChannel>()
            .AddAttribute("OutageProbability",
                          "Outage probability threshold",
                          DoubleValue(1e-2),
                          MakeDoubleAccessor(&IrsAssistedSpectrumChannel::SetEps),
                          MakeDoubleChecker<double>())
            .AddAttribute("KMin",
                          "Minimum k-factor for the environment, in dB",
                          DoubleValue(6.),
                          MakeDoubleAccessor(&IrsAssistedSpectrumChannel::m_kmin),
                          MakeDoubleChecker<double>())
            .AddAttribute("KMax",
                          "Maximum k-factor for the environment, in dB",
                          DoubleValue(10.),
                          MakeDoubleAccessor(&IrsAssistedSpectrumChannel::m_kmax),
                          MakeDoubleChecker<double>())
            .AddAttribute("KNlos",
                          "k-factor used in case of NLoS condition, in dB",
                          DoubleValue(0.),
                          MakeDoubleAccessor(&IrsAssistedSpectrumChannel::m_knlos),
                          MakeDoubleChecker<double>())
            .AddAttribute("AlphaLoss",
                          "Environment attenuation factor",
                          DoubleValue(2.),
                          MakeDoubleAccessor(&IrsAssistedSpectrumChannel::m_alpha),
                          MakeDoubleChecker<double>())
            .AddAttribute("NoDirectLink",
                          "Suppress the direct link with the communicating nodes",
                          BooleanValue(false),
                          MakeBooleanAccessor(&IrsAssistedSpectrumChannel::m_noDirectLink),
                          MakeBooleanChecker())
            .AddAttribute("NoIrsLink",
                          "Suppress the IRS reflected link with the communicating nodes",
                          BooleanValue(false),
                          MakeBooleanAccessor(&IrsAssistedSpectrumChannel::m_noIrsLink),
                          MakeBooleanChecker())
            .AddAttribute(
                "MultipathInterference",
                "Assume a perfect destructive, constructive, or "
                "simulated multipath interference between direct and "
                "reflected links",
                EnumValue(MultipathInterferenceType::SIMULATED),
                MakeEnumAccessor<MultipathInterferenceType>(
                    &IrsAssistedSpectrumChannel::m_multipathType),
                MakeEnumChecker<MultipathInterferenceType>(MultipathInterferenceType::DESTRUCTIVE,
                                                           "DESTRUCTIVE",
                                                           MultipathInterferenceType::SIMULATED,
                                                           "SIMULATED",
                                                           MultipathInterferenceType::CONSTRUCTIVE,
                                                           "CONSTRUCTIVE"));
    return tid;
}

void
IrsAssistedSpectrumChannel::StartTx(Ptr<SpectrumSignalParameters> txParams)
{
    NS_LOG_FUNCTION(this << txParams);

    NS_ASSERT(txParams->txPhy);
    NS_ASSERT(txParams->psd);
    Ptr<SpectrumSignalParameters> txParamsTrace =
        txParams->Copy(); // copy it since traced value cannot be const (because of potential
                          // underlying DynamicCasts)
    m_txSigParamsTrace(txParamsTrace);

    Ptr<MobilityModel> txMobility = txParams->txPhy->GetMobility();
    SpectrumModelUid_t txSpectrumModelUid = txParams->psd->GetSpectrumModelUid();
    NS_LOG_LOGIC("txSpectrumModelUid " << txSpectrumModelUid);

    auto txInfoIt = FindAndEventuallyAddTxSpectrumModel(txParams->psd->GetSpectrumModel());

    NS_LOG_LOGIC("converter map for TX SpectrumModel with Uid " << txInfoIt->first);
    NS_LOG_LOGIC("converter map size: " << txInfoIt->second.m_spectrumConverterMap.size());
    NS_LOG_LOGIC(
        "converter map first element: " << txInfoIt->second.m_spectrumConverterMap.begin()->first);

    for (auto& rxInfo : GetRxSpectrumModelInfoMap())
    {
        SpectrumModelUid_t rxSpectrumModelUid = rxInfo.second.m_rxSpectrumModel->GetUid();
        NS_LOG_LOGIC("rxSpectrumModelUids " << rxSpectrumModelUid);

        Ptr<SpectrumValue> convertedTxPowerSpectrum;
        if (txSpectrumModelUid == rxSpectrumModelUid)
        {
            NS_LOG_LOGIC("no spectrum conversion needed");
            convertedTxPowerSpectrum = txParams->psd;
        }
        else
        {
            NS_LOG_LOGIC("converting txPowerSpectrum SpectrumModelUids "
                         << txSpectrumModelUid << " --> " << rxSpectrumModelUid);
            auto rxConverterIt = txInfoIt->second.m_spectrumConverterMap.find(rxSpectrumModelUid);
            if (rxConverterIt == txInfoIt->second.m_spectrumConverterMap.end())
            {
                // No converter means TX SpectrumModel is orthogonal to RX SpectrumModel
                continue;
            }
            convertedTxPowerSpectrum = rxConverterIt->second.Convert(txParams->psd);
        }

        // Speculative calc. section
        double K_BG, F_BG, F_BRG, Irs2TxGain, Irs2RxGain, Tx2RxGain, Rx2TxGain;
        std::vector<double> d_BR, d_BG, K_BR, K_BG_nu, K_BG_sigma, lambdav, d_RG_temp, etav_tmp,
            K_RG_tmp, beta_BRG;
        std::vector<Angles> a_BR, a_RG_temp;
        std::vector<std::vector<double>> d_RG, etav, K_RG;
        std::vector<std::vector<Angles>> a_RG;
        Vector IrsPosition, TxPosition, RxPosition;
        Ptr<BuildingsChannelConditionModel> condModel =
            CreateObject<BuildingsChannelConditionModel>();

        const auto n_irs = IrsList::GetN();                 // Number of Irs
        const auto n_users = rxInfo.second.m_rxPhys.size(); // Number of receiving Phy layer
        const double Kmin = std::pow(10., m_kmin / 10.);    // A1
        const double Kmax = std::pow(10., m_kmax / 10.);
        const double Knlos = std::pow(10., m_knlos / 10.);
        const double A2 = std::log(std::pow(Kmax / Kmin, 2.)) / M_PI;

        NodeToIrssDistance(txMobility, d_BR);
        NodeToIrssAngles(txMobility, a_BR);

        const auto f_l = convertedTxPowerSpectrum->GetSpectrumModel()->Begin()->fl;
        const auto f_h = (--convertedTxPowerSpectrum->GetSpectrumModel()->End())->fh;
        const double f_c = (f_h + f_l) / 2.0;

        const double beta_BG = std::pow(SPEED_OF_LIGHT / f_c, 2.) / std::pow(4. * M_PI, 2.);
        IrsBeta(beta_BRG, f_c);

        int i = 0;
        for (auto& rxPhy : rxInfo.second.m_rxPhys)
        {
            RxPosition = rxPhy->GetMobility()->GetPosition();
            TxPosition = txMobility->GetPosition();
            auto rxParams = txParams->Copy();
            rxParams->psd = convertedTxPowerSpectrum->Copy();
            F_BG = 1.;

            const auto channelCondBG =
                condModel->GetChannelCondition(txMobility, rxPhy->GetMobility())->GetLosCondition();
            if (channelCondBG == ChannelCondition::LosConditionValue::LOS)
            {
                K_BG = Kmin * exp(A2 * GetElevation(txMobility->GetPosition(),
                                                    rxPhy->GetMobility()->GetPosition()));
            }
            else
            {
                K_BG = Knlos;
            }
            K_BG_nu.push_back(K_BG / (K_BG + 1.));
            K_BG_sigma.push_back(std::sqrt(1. / (K_BG + 1.)));
            d_BG.push_back(txMobility->GetDistanceFrom(rxPhy->GetMobility()));
            NodeToIrssDistance(rxPhy->GetMobility(), d_RG_temp);
            d_RG.push_back(d_RG_temp);
            d_RG_temp.clear();
            NodeToIrssAngles(rxPhy->GetMobility(), a_RG_temp);
            a_RG.push_back(a_RG_temp);
            a_RG_temp.clear();
            etav_tmp.clear();
            K_RG_tmp.clear();

            auto rxAntenna = DynamicCast<AntennaModel>(rxPhy->GetAntenna());

            for (uint32_t j = 0; j < n_irs; ++j)
            {
                F_BRG = 1;
                IrsPosition =
                    IrsList::Get(j)->GetDrone()->GetObject<MobilityModel>()->GetPosition();
                const auto& txNodeIrsInclination =
                    NodeToIrsAngles(txMobility, IrsList::Get(j)).GetInclination();
                const auto& rxNodeIrsInclination =
                    NodeToIrsAngles(rxPhy->GetMobility(), IrsList::Get(j)).GetInclination();
                const auto& irsPowerState = IrsList::Get(j)->GetState();
                const auto& irsDroneMM = IrsList::Get(j)->GetDrone()->GetObject<MobilityModel>();
                const auto& chCondIrsRxNode =
                    condModel->GetChannelCondition(irsDroneMM, rxPhy->GetMobility())
                        ->GetLosCondition();
                const auto& chCondTxNodeIrs =
                    condModel->GetChannelCondition(txMobility, irsDroneMM)->GetLosCondition();

                if (rxParams->txAntenna)
                {
                    Irs2TxGain = rxParams->txAntenna->GetGainDb(Angles(IrsPosition, TxPosition));
                    if (std::isinf(Irs2TxGain))
                        F_BRG *= 0.;
                    else
                        F_BRG *= std::pow(10., Irs2TxGain / 10.);
                }
                else
                {
                    NS_LOG_WARN("TX Antenna not found, isotropic antenna is used by dafualt");
                    F_BRG *= 1.;
                }

                if (rxAntenna)
                {
                    Irs2RxGain = rxAntenna->GetGainDb(Angles(IrsPosition, RxPosition));
                    if (std::isinf(Irs2RxGain))
                        F_BRG *= 0.;
                    else
                        F_BRG *= std::pow(10., Irs2RxGain / 10.);
                }
                else
                {
                    NS_LOG_WARN("RX Antenna not found, isotropic antenna is used by dafualt");
                    F_BRG *= 1.;
                }

                // Check if source and destination nodes are in LOS with the reflective face, if
                // not, F_BRG=0 same logic applies even if the IRS is not ON.
                if (txNodeIrsInclination > M_PI / 2. || rxNodeIrsInclination > M_PI / 2. ||
                    irsPowerState != DronePeripheral::PeripheralState::ON)
                {
                    // Irss doesn't reflect by both faces and doesn't refract
                    // Irs contribution is excluded
                    F_BRG *= 0.;
                }

                if (!m_noIrsLink)
                {
                    etav_tmp.push_back(
                        std::sqrt(beta_BRG[j] * std::pow(d_BR[j] * d_RG[i][j], -2.) * F_BRG));
                }
                else
                {
                    etav_tmp.push_back(0.);
                }

                if (chCondIrsRxNode == ChannelCondition::LosConditionValue::LOS)
                {
                    K_RG_tmp.push_back(Kmin * exp(A2 * (GetElevation(a_RG[i][j]))));
                }
                else
                {
                    K_RG_tmp.push_back(Knlos);
                }

                if (chCondTxNodeIrs == ChannelCondition::LosConditionValue::LOS)
                {
                    K_BR.push_back(Kmin * exp(A2 * (GetElevation(a_BR[j]))));
                }
                else
                {
                    K_BR.push_back(Knlos);
                }
            }
            etav.push_back(etav_tmp);

            if (rxParams->txAntenna)
            {
                Rx2TxGain = rxParams->txAntenna->GetGainDb(Angles(RxPosition, TxPosition));
                if (std::isinf(Rx2TxGain))
                    F_BG *= 0.;
                else
                    F_BG *= std::pow(10., Rx2TxGain / 10.);
            }
            else
            {
                NS_LOG_WARN("TX Antenna not found, isotropic antenna is used by dafualt");
                F_BG *= 1.;
            }

            if (rxAntenna)
            {
                Tx2RxGain = rxAntenna->GetGainDb(Angles(TxPosition, RxPosition));
                if (std::isinf(Tx2RxGain))
                    F_BG *= 0.;
                else
                    F_BG *= std::pow(10., Tx2RxGain / 10.);
            }
            else
            {
                NS_LOG_WARN("RX Antenna not found, isotropic antenna is used by dafualt");
                F_BG *= 1.;
            }

            if (!m_noDirectLink)
            {
                lambdav.push_back(std::sqrt(beta_BG * std::pow(d_BG[i], -m_alpha) * F_BG));
            }
            else
            {
                lambdav.push_back(0.);
            }

            K_RG.push_back(K_RG_tmp);
            ++i;
        }

        const auto gain = GetGain(f_c,
                                  n_users,
                                  n_irs,
                                  d_BG,
                                  K_BR,
                                  K_RG,
                                  etav,
                                  lambdav,
                                  d_BR,
                                  d_RG,
                                  a_BR,
                                  a_RG,
                                  K_BG_nu,
                                  K_BG_sigma);

        //
        int u = 0;
        for (auto& rxPhy : rxInfo.second.m_rxPhys)
        {
            NS_ASSERT_MSG(rxPhy->GetRxSpectrumModel()->GetUid() == rxSpectrumModelUid,
                          "SpectrumModel change was not notified to IrsAssistedSpectrumChannel "
                          "(i.e., AddRx should be called again after model is changed)");

            if (rxPhy != txParams->txPhy) // Check if TX Phy is different from RX Phy
            {
                Ptr<NetDevice> rxNetDevice = rxPhy->GetDevice();
                Ptr<NetDevice> txNetDevice = txParams->txPhy->GetDevice();
                // Ignore communication between device on the same Node.
                if (rxNetDevice && txNetDevice)
                {
                    // we assume that devices are attached to a node
                    if (rxNetDevice->GetNode()->GetId() == txNetDevice->GetNode()->GetId())
                    {
                        NS_LOG_DEBUG("Skipping the pathloss calculation among different antennas "
                                     "of the same "
                                     "node, not supported yet by any pathloss model in ns-3.");
                        continue;
                    }
                }

                NS_LOG_LOGIC("copying signal parameters " << txParams);
                Ptr<SpectrumSignalParameters> rxParams = txParams->Copy();
                rxParams->psd = convertedTxPowerSpectrum->Copy();
                Time delay = MicroSeconds(0);

                Ptr<MobilityModel> receiverMobility = rxPhy->GetMobility();
                // Channel gain calc. section
                if (txMobility && receiverMobility)
                {
                    double pathLossDb = 0.;

                    if (gain[u] > 0)
                        pathLossDb = -10. * std::log10(gain[u]);
                    else
                        pathLossDb = m_maxLossDb;

                    // Pathloss trace
                    m_pathLossTrace(txParams->txPhy, rxPhy, pathLossDb);

                    if (pathLossDb >= m_maxLossDb)
                    {
                        // beyond range
                        continue;
                    }

                    *(rxParams->psd) *= gain[u];

                    if (m_propagationDelay)
                    {
                        delay = m_propagationDelay->GetDelay(txMobility, receiverMobility);
                    }
                }

                if (rxNetDevice)
                {
                    // the receiver has a NetDevice, so we expect that it is attached to a Node
                    const auto& dstNode = rxNetDevice->GetNode()->GetId();
                    Simulator::ScheduleWithContext(dstNode,
                                                   delay,
                                                   &IrsAssistedSpectrumChannel::StartRx,
                                                   this,
                                                   rxParams,
                                                   rxPhy);
                }
                else
                {
                    // the receiver is not attached to a NetDevice, so we cannot assume that it is
                    // attached to a node
                    Simulator::Schedule(delay,
                                        &IrsAssistedSpectrumChannel::StartRx,
                                        this,
                                        rxParams,
                                        rxPhy);
                }
            }
            ++u;
        }
    }
}

std::vector<double>
IrsAssistedSpectrumChannel::GetGain(const double f_c,
                                    const int n_users,
                                    const int n_irs,
                                    const std::vector<double>& d_BG,
                                    const std::vector<double>& K_BR,
                                    const std::vector<std::vector<double>>& K_RG,
                                    const std::vector<std::vector<double>>& etav,
                                    const std::vector<double>& lambdav,
                                    const std::vector<double>& d_BR,
                                    const std::vector<std::vector<double>>& d_RG,
                                    const std::vector<Angles>& a_BR,
                                    const std::vector<std::vector<Angles>>& a_RG,
                                    const std::vector<double>& K_BG_nu,
                                    const std::vector<double>& K_BG_sigma)
{
    double nu_BRG, sig_BRG, K, sigma;
    std::vector<double> phases_tmp, modules_tmp, gain;
    std::vector<std::vector<double>> phases, modules;

    double cos_BR = 0.;
    double sin_BR = 0.;
    double cos_RG = 0.;
    double sin_RG = 0.;

    for (int u = 0; u < n_users; ++u)
    {
        nu_BRG = 0.;
        sig_BRG = 0.;

        phases.clear();
        modules.clear();
        for (int d = 0; d < n_irs; ++d)
        {
            const auto K_BRG_nu =
                std::sqrt(K_BR[d] * K_RG[u][d] / (K_BR[d] + 1.) / (K_RG[u][d] + 1.));
            const auto K_BRG_sigma =
                std::sqrt((K_BR[d] + K_RG[u][d]) / (K_BR[d] + 1) / (K_RG[u][d] + 1.));
            // For each patch
            const auto irs = IrsList::Get(d);
            const auto d_r = irs->GetPruY();
            const auto d_c = irs->GetPruX();
            const auto patches = irs->GetPatchVector();

            phases_tmp.clear();
            modules_tmp.clear();
            const auto P = patches.size();
            for (std::size_t p = 0; p < P; ++p)
            {
                modules_tmp.push_back(etav[u][d] * K_BRG_nu);
                double phaseY, phaseX, distance;
                if (patches[p]->IsServing())
                {
                    GetServedParam(irs, patches[p], phaseX, phaseY, distance);
                }
                else
                {
                    double theta_r, phi_r, z;
                    const auto theta_o = a_BR[d].GetInclination();
                    const auto phi_o = a_BR[d].GetAzimuth();
                    phaseX = patches[p]->GetPhaseX();
                    phaseY = patches[p]->GetPhaseY();
                    // Computation of Irs-Ground distance following the angle of reflection
                    if (phaseX == 0)
                    {
                        theta_r = theta_o;
                    }
                    else
                    {
                        if (!std::isnan(phi_o))
                        {
                            phi_r = std::atan(phaseY / phaseX - std::tan(phi_o));
                            theta_r = std::asin((phaseY - std::sin(theta_o) * std::sin(phi_o)) /
                                                std::sin(phi_r));
                        }
                        else
                        {
                            phi_r = std::atan(phaseY / phaseX);
                            theta_r = std::asin(phaseY - std::sin(theta_o) / std::sin(phi_r));
                        }
                    }
                    if (!std::isnan(theta_r))
                    {
                        z = irs->GetDrone()->GetObject<MobilityModel>()->GetPosition().z;
                        distance =
                            std::sqrt(std::pow(z, 2.) + std::pow(z * std::tan(theta_r), 2.)) +
                            d_BR[d];
                    }
                    else
                    {
                        NS_ABORT_MSG("The values of phaseX and phaseY are not valid.");
                    }
                }
                cos_BR = 0.;
                sin_BR = 0.;
                cos_RG = 0.;
                sin_RG = 0.;

                if (!std::isnan(a_BR[d].GetAzimuth()))
                {
                    cos_BR = std::cos(a_BR[d].GetAzimuth());
                    sin_BR = std::sin(a_BR[d].GetAzimuth());
                }

                if (!std::isnan(a_RG[u][d].GetAzimuth()))
                {
                    cos_RG = std::cos(a_RG[u][d].GetAzimuth());
                    sin_RG = std::sin(a_RG[u][d].GetAzimuth());
                }

                const auto phi_x = d_c * (std::sin(a_BR[d].GetInclination()) * cos_BR +
                                          std::sin(a_RG[u][d].GetInclination()) * cos_RG - phaseX);
                const auto phi_y = d_r * (std::sin(a_BR[d].GetInclination()) * sin_BR +
                                          std::sin(a_RG[u][d].GetInclination()) * sin_RG - phaseY);

                auto pigr = 2. * M_PI;
                if (modf(phi_x * M_PI * f_c / SPEED_OF_LIGHT, &pigr) ==
                    0.) // if it is multiple of pi denominator is 0
                    modules_tmp[p] = modules_tmp[p] *
                                     patches[p]->GetSize().GetColSize(); // Paper: Formula 19 (Chi)
                else
                    modules_tmp[p] =
                        modules_tmp[p] *
                        std::sin(patches[p]->GetSize().GetColSize() * phi_x * M_PI * f_c /
                                 SPEED_OF_LIGHT) /
                        std::sin(phi_x * M_PI * f_c / SPEED_OF_LIGHT); // Paper: Formula 19 (Chi)

                if (modf(phi_y * M_PI * f_c / SPEED_OF_LIGHT, &pigr) ==
                    0) // if it is multiple of pi denominator is 0
                {
                    modules_tmp[p] = modules_tmp[p] *
                                     patches[p]->GetSize().GetRowSize(); // Paper: Formula 19 (Chi)
                }
                else
                    modules_tmp[p] =
                        modules_tmp[p] *
                        std::sin(patches[p]->GetSize().GetRowSize() * phi_y * M_PI * f_c /
                                 SPEED_OF_LIGHT) /
                        std::sin(phi_y * M_PI * f_c / SPEED_OF_LIGHT); // Paper: Formula 19 (Chi)

                phases_tmp.push_back(
                    -2. * M_PI * f_c / SPEED_OF_LIGHT *
                    (d_BR[d] + d_RG[u][d] - distance)); // Paper: Formula 19 (Omega)
            }
            modules.push_back(modules_tmp);
            phases.push_back(phases_tmp);

            for (std::size_t p = 0; p < P; ++p)
            {
                for (int d1 = 0; d1 <= d; ++d1)
                {
                    for (std::size_t p1 = 0; p1 < P; ++p1)
                    {
                        if (d == d1)
                        {
                            if (p1 <= p)
                                p1 = p + 1;
                            if (p == P - 1)
                                break;
                        }
                        nu_BRG += 2. * std::abs(modules[d][p]) * std::abs(modules[d1][p1]) *
                                  std::cos(phases[d][p] - phases[d1][p1]);
                    }
                }
                nu_BRG += std::pow(modules[d][p], 2.);

                switch (m_multipathType)
                {
                case MultipathInterferenceType::SIMULATED:
                    nu_BRG += 2. * std::abs(modules[d][p]) * lambdav[u] * K_BG_nu[u] *
                              std::cos(phases[d][p] + 2. * M_PI * f_c / SPEED_OF_LIGHT * d_BG[u]);

                    break;
                case MultipathInterferenceType::CONSTRUCTIVE:
                    nu_BRG += 2. * std::abs(modules[d][p]) * lambdav[u] * K_BG_nu[u];
                    break;
                case MultipathInterferenceType::DESTRUCTIVE:
                    nu_BRG -= 2. * std::abs(modules[d][p]) * lambdav[u] * K_BG_nu[u];
                    break;
                }

                sig_BRG += std::pow(etav[u][d], 2.) * std::pow(K_BRG_sigma, 2.) *
                           patches[p]->GetSize().GetRowSize() * patches[p]->GetSize().GetColSize();
            }
        }
        nu_BRG += std::pow(lambdav[u] * K_BG_nu[u], 2.);
        sig_BRG += std::pow(lambdav[u] * K_BG_sigma[u], 2.);

        if (sig_BRG == 0. || nu_BRG == 0.)
        {
            gain.push_back(0.);
        }
        else
        {
            K = nu_BRG / sig_BRG;

            sigma = nu_BRG + sig_BRG;
            if (std::sqrt(2. * K) > 2.5848)
                gain.push_back(std::pow(std::sqrt(2. * K) +
                                            0.5 / m_invqfunc *
                                                std::log(std::sqrt(2. * K) /
                                                         (std::sqrt(2. * K) - m_invqfunc)) -
                                            m_invqfunc,
                                        2.) /
                               (2. * (K + 1.) / sigma));
            else
                gain.push_back(std::pow(std::sqrt(-2. * std::log(1. - m_eps)) * exp(K / 2.), 2.) /
                               (2. * (K + 1.) / sigma));
        }
    }
    return gain;
}

void
IrsAssistedSpectrumChannel::SetEps(const double e)
{
    m_eps = e;
    SetInvqfunc(e);
}

void
IrsAssistedSpectrumChannel::SetInvqfunc(const double eps)
{
    if (eps <= 0. || eps >= 1.)
        NS_FATAL_ERROR("epsilon must be between 0 and 1. Given: " << eps);

    constexpr double c[] = {2.515517, 0.802853, 0.010328};
    constexpr double d[] = {1.432788, 0.189269, 0.001308};
    double t = 0.;

    if (eps < 0.5)
        t = std::sqrt(-2. * std::log(eps));
    else
        t = std::sqrt(-2. * std::log(1 - eps));

    m_invqfunc = (t - ((c[2] * t + c[1]) * t + c[0]) / (((d[2] * t + d[1]) * t + d[0]) * t + 1.));
}

Vector
IrsAssistedSpectrumChannel::BackRotate(const Vector& P, const RotoAxisType axis, double angle)
{
    angle = -angle;
    double x = 0., y = 0., z = 0.;
    switch (axis)
    {
    case RotoAxisType::X_AXIS: // X=[1 0 0; 0 cos(angle) -sin(angle); 0 sin(angle) cos(angle)];
        x = P.x;
        y = P.y * std::cos(angle) - P.z * std::sin(angle);
        z = P.y * std::sin(angle) + P.z * std::cos(angle);
        break;
    case RotoAxisType::Y_AXIS: // Y=[cos(angle) 0 sin(angle); 0 1 0; -sin(angle) 0 cos(angle)];
        x = P.x * std::cos(angle) + P.z * std::sin(angle);
        y = P.y;
        z = P.z * std::cos(angle) - P.x * std::sin(angle);
        break;
    case RotoAxisType::Z_AXIS: // Z=[cos(angle) -sin(angle) 0; sin(angle) cos(angle) 0; 0 0 1];
        x = P.x * std::cos(angle) - P.y * std::sin(angle);
        y = P.x * std::sin(angle) + P.y * std::cos(angle);
        z = P.z;
        break;
    }
    return {x, y, z};
}

Vector
IrsAssistedSpectrumChannel::BackShift(const Vector& P, Ptr<MobilityModel> MM)
{
    const auto& pos = MM->GetPosition();
    return {P.x - pos.x, P.y - pos.y, P.z - pos.z};
}

void
IrsAssistedSpectrumChannel::NodeToIrssDistance(Ptr<MobilityModel> Node,
                                               std::vector<double>& d_Vector)
{
    for (auto& irs : IrsList())
    {
        const auto mm = irs->GetDrone()->GetObject<MobilityModel>();
        d_Vector.push_back(mm->GetDistanceFrom(Node));
    }
}

void
IrsAssistedSpectrumChannel::NodeToIrssAngles(Ptr<MobilityModel> node, std::vector<Angles>& a_Vector)
{
    const Vector irsPos{0., 0., 0.};
    const Vector& nGlobalPos = node->GetPosition();
    // Is normalized the position of the node as if the res were centered
    // in the origin of the axes and were parallel to the XY plane
    for (auto& irs : IrsList())
    {
        const auto& rotoAxis = irs->GetRotoAxis();
        const auto& rotoAngles = irs->GetRotoAngles();
        const auto& irsMm = irs->GetDrone()->GetObject<MobilityModel>();
        auto nRelPos = BackShift(nGlobalPos, irsMm);

        for (auto i = rotoAxis.size(); i > 0; --i)
            nRelPos = BackRotate(nRelPos, rotoAxis[i - 1], rotoAngles[i - 1]);

        a_Vector.push_back({nRelPos, irsPos});
    }
}

Angles
IrsAssistedSpectrumChannel::NodeToIrsAngles(Ptr<MobilityModel> node, Ptr<Irs> irs)
{
    const Vector irsPos{0., 0., 0.};
    // Is normalized the position of the node as if the res were centered
    // in the origin of the axes and were parallel to the XY plane
    const auto& irsMm = irs->GetDrone()->GetObject<MobilityModel>();
    const auto& rotoAxis = irs->GetRotoAxis();
    const auto& rotoAngles = irs->GetRotoAngles();
    Vector nPos = node->GetPosition();

    nPos = BackShift(nPos, irsMm);
    for (int i = rotoAxis.size(); i > 0; --i)
        nPos = BackRotate(nPos, rotoAxis[i - 1], rotoAngles[i - 1]);

    return {nPos, irsPos};
}

void
IrsAssistedSpectrumChannel::IrsBeta(std::vector<double>& beta, const double f_c)
{
    for (auto& irs : IrsList())
    {
        const double s = irs->GetPruX() * irs->GetPruY();
        beta.push_back(std::pow(SPEED_OF_LIGHT / f_c, 2) * s / 64. / std::pow(M_PI, 3));
    }
}

double
IrsAssistedSpectrumChannel::NodeToIrsDistance(Ptr<MobilityModel> node, Ptr<Irs> irs)
{
    const auto& irsMm = irs->GetDrone()->GetObject<MobilityModel>();
    return irsMm->GetDistanceFrom(node);
}

void
IrsAssistedSpectrumChannel::GetServedParam(Ptr<Irs> irs,
                                           Ptr<IrsPatch> patch,
                                           double& phaseY,
                                           double& phaseX,
                                           double& distance)
{
    const auto servedNodes = patch->GetServedNodes();
    const auto& node1Mm = servedNodes.first->GetObject<MobilityModel>();
    const auto& node2Mm = servedNodes.second->GetObject<MobilityModel>();
    const auto a_BR = NodeToIrsAngles(node1Mm, irs);
    const auto a_RG = NodeToIrsAngles(node2Mm, irs);

    phaseY = std::sin(a_BR.GetInclination()) * std::cos(a_BR.GetAzimuth()) +
             std::sin(a_RG.GetInclination()) * std::cos(a_RG.GetAzimuth());
    phaseX = std::sin(a_BR.GetInclination()) * std::sin(a_BR.GetAzimuth()) +
             std::sin(a_RG.GetInclination()) * std::sin(a_RG.GetAzimuth());
    distance = NodeToIrsDistance(node1Mm, irs) + NodeToIrsDistance(node2Mm, irs);
}

double
IrsAssistedSpectrumChannel::GetElevation(const Vector& dest, const Vector& orig)
{
    return fabs(asin((dest.z - orig.z) / CalculateDistance(dest, orig)));
}

double
IrsAssistedSpectrumChannel::GetElevation(const Angles& angles)
{
    double inclination = angles.GetInclination();
    double elevation = M_PI / 2. - inclination;

    if (inclination > M_PI / 2.)
    {
        // if the inclination is greater than 90 degrees, it needs to be
        // normalized in the range [0,90]
        elevation -= M_PI;
    }

    return elevation;
}

} // namespace ns3
