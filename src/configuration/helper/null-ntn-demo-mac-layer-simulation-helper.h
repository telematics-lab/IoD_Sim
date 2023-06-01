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
#ifndef NULL_NTN_DEMO_MAC_LAYER_HELPER_H
#define NULL_NTN_DEMO_MAC_LAYER_HELPER_H

#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/null-ntn-demo-mac-layer-configuration.h>
#include <ns3/object.h>
#include <ns3/phased-array-model.h>
#include <ns3/spectrum-value.h>
#include <ns3/three-gpp-phy-simulation-helper.h>

namespace ns3
{

/**
 * Helper simulate a NTN scenario without a real-world MAC.
 */
class NullNtnDemoMacLayerSimulationHelper : public Object
{
  public:
    /**
     * \brief A structure that holds the parameters for the
     * ComputeSnr function. In this way the problem with the limited
     * number of parameters of method Schedule is avoided.
     */
    struct ComputeSnrParams
    {
        Ptr<MobilityModel> txMob;        /// the tx mobility model
        Ptr<MobilityModel> rxMob;        /// the rx mobility model
        double txPow;                    /// the tx power in dBm
        double noiseFigure;              /// the noise figure in dB
        Ptr<PhasedArrayModel> txAntenna; /// the tx antenna array
        Ptr<PhasedArrayModel> rxAntenna; /// the rx antenna array
        double frequency;                /// the carrier frequency in Hz
        double bandwidth;                /// the total bandwidth in Hz
        double resourceBlockBandwidth;   /// the Resource Block bandwidth in Hz

        /**
         * \brief Constructor
         * \param pTxMob the tx mobility model
         * \param pRxMob the rx mobility model
         * \param pTxPow the tx power in dBm
         * \param pNoiseFigure the noise figure in dB
         * \param pTxAntenna the tx antenna array
         * \param pRxAntenna the rx antenna array
         * \param pFrequency the carrier frequency in Hz
         * \param pBandwidth the total bandwidth in Hz
         * \param pResourceBlockBandwidth the Resource Block bandwidth in Hz
         */
        ComputeSnrParams(Ptr<MobilityModel> pTxMob,
                         Ptr<MobilityModel> pRxMob,
                         double pTxPow,
                         double pNoiseFigure,
                         Ptr<PhasedArrayModel> pTxAntenna,
                         Ptr<PhasedArrayModel> pRxAntenna,
                         double pFrequency,
                         double pBandwidth,
                         double pResourceBlockBandwidth);
    };

    NullNtnDemoMacLayerSimulationHelper(Ptr<NullNtnDemoMacLayerConfiguration> configuration,
                                        Ptr<ThreeGppPhySimulationHelper> phyHelper);
    ~NullNtnDemoMacLayerSimulationHelper();

    void Setup(const double simDuration) const;

  private:
    /**
     * Compute the average SNR
     * \param params A structure that holds the parameters that are needed to perform calculations
     * in ComputeSnr
     */
    void ComputeSnr(ComputeSnrParams& params) const;

    static Ptr<SpectrumValue> CreateTxPowerSpectralDensity(double fc,
                                                           double pwr,
                                                           double BW,
                                                           double RB_WIDTH);
    static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity(double fc,
                                                              double noiseFigure,
                                                              double BW,
                                                              double RB_WIDTH);
    /**
     * Perform the beamforming using the DFT beamforming method
     * \param thisDevice the device performing the beamforming
     * \param thisAntenna the antenna object associated to thisDevice
     * \param otherDevice the device towards which point the beam
     */
    static void DoBeamforming(Ptr<NetDevice> thisDevice,
                              Ptr<PhasedArrayModel> thisAntenna,
                              Ptr<NetDevice> otherDevice);

    Ptr<NullNtnDemoMacLayerConfiguration> m_configuration;
    Ptr<ThreeGppPhySimulationHelper> m_phyHelper;
};

} // namespace ns3

#endif /* NULL_NTN_DEMO_MAC_LAYER_HELPER_H */