/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 *
 */

#ifndef IRS_ASSISTED_SPECTRUM_CHANNEL_H
#define IRS_ASSISTED_SPECTRUM_CHANNEL_H

#include "irs.h"

#include <ns3/multi-model-spectrum-channel.h>

namespace ns3
{

enum MultipathInterferenceType
{
    DESTRUCTIVE = -1,
    SIMULATED = 0,
    CONSTRUCTIVE = 1
};

/**
 * \ingroup Irs
 *
 * This SpectrumChannel implementation derived from MultiModelSpectrumChannel
 * also considers the presence of Intelligent Reflective Surface (Irs)
 * that affect the calculation of channel gain.
 *
 * \see MultiModelSpectrumChannel
 */
class IrsAssistedSpectrumChannel : public MultiModelSpectrumChannel
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);

    virtual void StartTx(Ptr<SpectrumSignalParameters> params);

  protected:
    virtual void DoDispose();

  private:
    /*TODO*/
    std::vector<double> GetGain(const double f_c,
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
                                const std::vector<double>& K_BG_sigma);

    /**
     * \brief TODO
     *
     * \param
     */
    void SetInvqfunc(const double epsilon);

    /**
     * \brief TODO
     *
     * \param
     */
    void SetEps(const double e);

    static Vector BackRotate(const Vector& P, const RotoAxisType axis, double angle);
    static Vector BackShift(const Vector& P, Ptr<MobilityModel> MM);
    static void NodeToIrssDistance(Ptr<MobilityModel> Node, std::vector<double>& d_Vector);
    static void NodeToIrssAngles(Ptr<MobilityModel> Node, std::vector<Angles>& a_Vector);
    static Angles NodeToIrsAngles(Ptr<MobilityModel> Node, Ptr<Irs> irs);
    static void IrsBeta(std::vector<double>& beta, const double f_c);
    static double NodeToIrsDistance(Ptr<MobilityModel> Node, Ptr<Irs> irs);
    static void GetServedParam(Ptr<Irs> irs,
                               Ptr<IrsPatch> patch,
                               double& phaseY,
                               double& phaseX,
                               double& distance);
    static double GetElevation(const Vector& destination, const Vector& origin);
    static double GetElevation(const Angles& angles);

    double m_eps;
    double m_invqfunc;
    double m_kmin;
    double m_kmax;
    double m_knlos;
    double m_alpha;
    bool m_noDirectLink;
    bool m_noIrsLink;
    MultipathInterferenceType m_multipathType;
};

} // namespace ns3

#endif /* IRS_ASSISTED_SPECTRUM_CHANNEL_H */
