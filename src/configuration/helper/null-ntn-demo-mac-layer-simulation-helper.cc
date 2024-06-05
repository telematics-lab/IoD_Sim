/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
 * Copyright (c) 2023 SIGNET Lab, Department of Information Engineering,
 * University of Padova
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

#include "null-ntn-demo-mac-layer-simulation-helper.h"

#include "scenario-configuration-helper.h"

#include <ns3/circular-aperture-antenna-model.h>
#include <ns3/geocentric-mobility-model.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/phased-array-model.h>
#include <ns3/simulator.h>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>

#include <fstream>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("NullNtnDemoMacLayerSimulationHelper");

NullNtnDemoMacLayerSimulationHelper::NullNtnDemoMacLayerSimulationHelper(
    Ptr<NullNtnDemoMacLayerConfiguration> config,
    Ptr<ThreeGppPhySimulationHelper> phyHelper)
    : m_configuration(config),
      m_phyHelper(phyHelper)
{
}

NullNtnDemoMacLayerSimulationHelper::~NullNtnDemoMacLayerSimulationHelper()
{
}

void
NullNtnDemoMacLayerSimulationHelper::Setup(const double simDuration) const
{
    NS_LOG_FUNCTION(this);

    const double satEirpDensity = m_configuration->GetSatEirpDensity();
    const double bandwidth = m_configuration->GetBandwidth();
    const double rbBandwidth = m_configuration->GetRbBandwidth();
    const double ueAntennaNoiseFigure = m_configuration->GetUeAntennaNoiseFigure();
    const double frequency = m_phyHelper->GetPropagationLossModel()->GetFrequency();

    // tx is GEO Sat | rx is Ground Node
    NS_ASSERT_MSG(NodeList::GetNNodes() == 2,
                  "NullNtnDemo requires 2 nodes, one GEO Sat and the other the Ground Node");
    auto txNode = NodeList::GetNode(0);
    auto rxNode = NodeList::GetNode(1);

    auto txDev = txNode->GetDevice(0);
    auto rxDev = rxNode->GetDevice(0);

    auto txAntenna = txDev->GetObject<PhasedArrayModel>();
    NS_ASSERT_MSG(txAntenna, "Antenna not found in Node 0");
    auto rxAntenna = rxDev->GetObject<PhasedArrayModel>();
    NS_ASSERT_MSG(txAntenna, "Antenna not found in Node 1");
    auto txMob = txNode->GetObject<MobilityModel>();
    NS_ASSERT_MSG(txMob, "Mobility Model not found in Node 0");
    auto rxMob = rxNode->GetObject<MobilityModel>();
    NS_ASSERT_MSG(rxMob, "Mobility Model not found in Node 1");

    // get tx antenna max gain
    const double txGainDb = [&txAntenna]() -> double {
        // assuming the antenna faces down
        auto antennaElement = StaticCast<const CircularApertureAntennaModel, const AntennaModel>(
            txAntenna->GetAntennaElement());
        return antennaElement->GetMaxGain();
    }();

    const double txPow = (satEirpDensity + 10 * log10(bandwidth / 1e6) - txGainDb) + 30;
    NS_LOG_DEBUG("Transmitting power: " << txPow << "dBm, (" << std::pow(10., (txPow - 30) / 10)
                                        << "W)");

    // set the beamforming vectors
    DoBeamforming(rxDev, rxAntenna, txDev);
    DoBeamforming(txDev, txAntenna, rxDev);

    for (unsigned int i = 0;
         i < (decltype(i))floor(simDuration / m_configuration->GetTimeResolution());
         ++i)
    {
        Simulator::Schedule(Seconds(m_configuration->GetTimeResolution() * i),
                            &NullNtnDemoMacLayerSimulationHelper::ComputeSnr,
                            this,
                            ComputeSnrParams(txMob,
                                             rxMob,
                                             txPow,
                                             ueAntennaNoiseFigure,
                                             txAntenna,
                                             rxAntenna,
                                             frequency,
                                             bandwidth,
                                             rbBandwidth));
    }
}

NullNtnDemoMacLayerSimulationHelper::ComputeSnrParams::ComputeSnrParams(
    Ptr<MobilityModel> pTxMob,
    Ptr<MobilityModel> pRxMob,
    double pTxPow,
    double pNoiseFigure,
    Ptr<PhasedArrayModel> pTxAntenna,
    Ptr<PhasedArrayModel> pRxAntenna,
    double pFrequency,
    double pBandwidth,
    double pResourceBlockBandwidth)
    : txMob(pTxMob),
      rxMob(pRxMob),
      txPow(pTxPow),
      noiseFigure(pNoiseFigure),
      txAntenna(pTxAntenna),
      rxAntenna(pRxAntenna),
      frequency(pFrequency),
      bandwidth(pBandwidth),
      resourceBlockBandwidth(pResourceBlockBandwidth)
{
}

/**
 * @brief Create the PSD for the TX
 *
 * @param fcHz the carrier frequency in Hz
 * @param pwrDbm the transmission power in dBm
 * @param bwHz the bandwidth in Hz
 * @param rbWidthHz the Resource Block (RB) width in Hz
 *
 * @return the pointer to the PSD
 */
Ptr<SpectrumValue>
NullNtnDemoMacLayerSimulationHelper::CreateTxPowerSpectralDensity(double fcHz,
                                                                  double pwrDbm,
                                                                  double bwHz,
                                                                  double rbWidthHz)
{
    unsigned int numRbs = std::floor(bwHz / rbWidthHz);
    double f = fcHz - (numRbs * rbWidthHz / 2.0);
    double powerTx = pwrDbm; // dBm power

    Bands rbs; // A vector representing each resource block
    for (uint32_t numrb = 0; numrb < numRbs; ++numrb)
    {
        BandInfo rb;
        rb.fl = f;
        f += rbWidthHz / 2;
        rb.fc = f;
        f += rbWidthHz / 2;
        rb.fh = f;

        rbs.push_back(rb);
    }
    Ptr<SpectrumModel> model = Create<SpectrumModel>(rbs);
    Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

    double powerTxW = std::pow(10., (powerTx - 30) / 10); // Get Tx power in Watts
    double txPowerDensity = (powerTxW / bwHz);

    for (auto psd = txPsd->ValuesBegin(); psd != txPsd->ValuesEnd(); ++psd)
    {
        *psd = txPowerDensity;
    }

    return txPsd; // [W/Hz]
}

/**
 * @brief Create the noise PSD for the
 *
 * @param fcHz the carrier frequency in Hz
 * @param noiseFigureDb the noise figure in dB
 * @param bwHz the bandwidth in Hz
 * @param rbWidthHz the Resource Block (RB) width in Hz
 *
 * @return the pointer to the noise PSD
 */
Ptr<SpectrumValue>
NullNtnDemoMacLayerSimulationHelper::CreateNoisePowerSpectralDensity(double fcHz,
                                                                     double noiseFigureDb,
                                                                     double bwHz,
                                                                     double rbWidthHz)
{
    unsigned int numRbs = std::floor(bwHz / rbWidthHz);
    double f = fcHz - (numRbs * rbWidthHz / 2.0);

    Bands rbs;              // A vector representing each resource block
    std::vector<int> rbsId; // A vector representing the resource block IDs
    for (uint32_t numrb = 0; numrb < numRbs; ++numrb)
    {
        BandInfo rb;
        rb.fl = f;
        f += rbWidthHz / 2;
        rb.fc = f;
        f += rbWidthHz / 2;
        rb.fh = f;

        rbs.push_back(rb);
        rbsId.push_back(numrb);
    }
    Ptr<SpectrumModel> model = Create<SpectrumModel>(rbs);
    Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

    // see "LTE - From theory to practice"
    // Section 22.4.4.2 Thermal Noise and Receiver Noise Figure
    const double ktDbmHz = -174.0;                        // dBm/Hz
    double ktWHz = std::pow(10.0, (ktDbmHz - 30) / 10.0); // W/Hz
    double noiseFigureLinear = std::pow(10.0, noiseFigureDb / 10.0);

    double noisePowerSpectralDensity = ktWHz * noiseFigureLinear;

    for (auto rbId : rbsId)
    {
        (*txPsd)[rbId] = noisePowerSpectralDensity;
    }

    return txPsd; // W/Hz
}

/**
 * Perform the beamforming using the DFT beamforming method
 * \param thisDevice the device performing the beamforming
 * \param thisAntenna the antenna object associated to thisDevice
 * \param otherDevice the device towards which point the beam
 */
void
NullNtnDemoMacLayerSimulationHelper::DoBeamforming(Ptr<NetDevice> thisDevice,
                                                   Ptr<PhasedArrayModel> thisAntenna,
                                                   Ptr<NetDevice> otherDevice)
{
    // retrieve the position of the two devices
    Vector aPos = thisDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();
    Vector bPos = otherDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();

    // compute the azimuth and the elevation angles
    Angles completeAngle(bPos, aPos);
    double hAngleRadian = completeAngle.GetAzimuth();
    double vAngleRadian = completeAngle.GetInclination(); // the elevation angle

    // retrieve the number of antenna elements and resize the vector
    uint64_t totNoArrayElements = thisAntenna->GetNumElems();
    PhasedArrayModel::ComplexVector antennaWeights(totNoArrayElements);

    // the total power is divided equally among the antenna elements
    double power = 1.0 / sqrt(totNoArrayElements);

    // compute the antenna weights
    const double sinVAngleRadian = sin(vAngleRadian);
    const double cosVAngleRadian = cos(vAngleRadian);
    const double sinHAngleRadian = sin(hAngleRadian);
    const double cosHAngleRadian = cos(hAngleRadian);

    for (uint64_t ind = 0; ind < totNoArrayElements; ind++)
    {
        Vector loc = thisAntenna->GetElementLocation(ind);
        double phase = -2 * M_PI *
                       (sinVAngleRadian * cosHAngleRadian * loc.x +
                        sinVAngleRadian * sinHAngleRadian * loc.y + cosVAngleRadian * loc.z);
        antennaWeights[ind] = exp(std::complex<double>(0, phase)) * power;
    }

    // store the antenna weights
    thisAntenna->SetBeamformingVector(antennaWeights);
}

/**
 * Compute the average SNR
 * \param params A structure that holds the parameters that are needed to perform calculations in
 * ComputeSnr
 */
void
NullNtnDemoMacLayerSimulationHelper::ComputeSnr(ComputeSnrParams& params) const
{
    Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity(params.frequency,
                                                            params.txPow,
                                                            params.bandwidth,
                                                            params.resourceBlockBandwidth);
    Ptr<SpectrumValue> rxPsd = txPsd->Copy();
    NS_LOG_DEBUG("Average tx power " << 10 * log10(Sum(*txPsd) * params.resourceBlockBandwidth)
                                     << " dB");

    // create the noise PSD
    Ptr<SpectrumValue> noisePsd = CreateNoisePowerSpectralDensity(params.frequency,
                                                                  params.noiseFigure,
                                                                  params.bandwidth,
                                                                  params.resourceBlockBandwidth);
    NS_LOG_DEBUG("Average noise power "
                 << 10 * log10(Sum(*noisePsd) * params.resourceBlockBandwidth) << " dB");

    // apply the pathloss
    double propagationGainDb =
        m_phyHelper->GetPropagationLossModel()->CalcRxPower(0, params.txMob, params.rxMob);
    NS_LOG_DEBUG("Pathloss " << -propagationGainDb << " dB");
    double propagationGainLinear = std::pow(10.0, (propagationGainDb) / 10.0);
    *(rxPsd) *= propagationGainLinear;

    NS_ASSERT_MSG(params.txAntenna, "params.txAntenna is nullptr!");
    NS_ASSERT_MSG(params.rxAntenna, "params.rxAntenna is nullptr!");

    Ptr<SpectrumSignalParameters> rxSsp = Create<SpectrumSignalParameters>();
    rxSsp->psd = rxPsd;
    rxSsp->txAntenna =
        ConstCast<AntennaModel, const AntennaModel>(params.txAntenna->GetAntennaElement());

    // apply the fast fading and the beamforming gain
    rxSsp = m_phyHelper->GetSpectrumPropagationLossModel()->CalcRxPowerSpectralDensity(
        rxSsp,
        params.txMob,
        params.rxMob,
        params.txAntenna,
        params.rxAntenna);
    NS_LOG_DEBUG("Average rx power " << 10 * log10(Sum(*rxSsp->psd) * params.bandwidth) << " dB");

    // compute the SNR
    NS_LOG_DEBUG("Average SNR " << 10 * log10(Sum(*rxSsp->psd) / Sum(*noisePsd)) << " dB");

    auto txMobPtr = DynamicCast<GeocentricMobilityModel, MobilityModel>(params.txMob);
    auto rxMobPtr = DynamicCast<GeocentricMobilityModel, MobilityModel>(params.rxMob);

    // compute geocentric distance between nodes
    auto txMobGeocentric = txMobPtr->GetPosition(PositionType::GEOCENTRIC);
    auto rxMobGeocentric = rxMobPtr->GetPosition(PositionType::GEOCENTRIC);
    double geocentricDistance = CalculateDistance(txMobGeocentric, rxMobGeocentric);

    auto getProjectedPositionFailsafe = [](Ptr<GeocentricMobilityModel> mm) -> Vector {
        if (mm->GetEarthSpheroidType() == GeographicPositions::EarthSpheroidType::WGS84)
        {
            return mm->GetPosition(PositionType::PROJECTED);
        }
        else
        {
            // fallback to Topocentric
            // TODO: need a threshold to understand if we can simplify to
            //       Topocentric, or it is better to use a projection
            return mm->GetPosition(PositionType::TOPOCENTRIC);
        }
    };

    auto txMobProjected = getProjectedPositionFailsafe(txMobPtr);
    auto rxMobProjected = getProjectedPositionFailsafe(rxMobPtr);
    double projectedDistance = CalculateDistance(txMobProjected, rxMobProjected);

    // print the SNR and pathloss values in the ntn-snr-trace.txt file
    std::ofstream f;
    std::ostringstream snrFilePath;
    snrFilePath << CONFIGURATOR->GetResultsPath() << "ntn-snr-trace.txt";
    f.open(snrFilePath.str(), std::ios::out | std::ios::app);
    f << Simulator::Now().GetSeconds() << " " << 10 * log10(Sum(*rxPsd) / Sum(*noisePsd)) << " "
      << propagationGainDb << " " << geocentricDistance << " " << projectedDistance << std::endl;
    f.close();
}

} // namespace ns3
