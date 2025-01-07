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
 * \ingroup irs
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
    /**
     * \brief Class to calculate the channel gain.
     *
     * \param f_c carrier frequency.
     * \param n_users number of users.
     * \param n_irs number of IRS.
     * \param d_BG distances between Base Station and Ground Users.
     * \param K_BR k-factors between Base Station and IRSs.
     * \param K_RG k-factors between Ground Users and IRSs.
     * \param etav eta component of the overall gain between Base Station and Ground Users.
     * \param lambdav lambda component of the overall gain between Base Station and Ground Users.
     * \param d_BR distances between Base Station and IRSs.
     * \param d_RG distances between IRSs and Ground Users.
     * \param a_BR angles between Base Station and IRSs.
     * \param a_RG angles between IRSs and Ground Users.
     * \param K_BG_nu nu component of the overall gain between Base Station and Ground Users.
     * \param K_BG_sigma sigma component of the overall gain between Base Station and Ground Users.
     * \return the channel gain.
     */
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
     * \brief Class to set the Inverse Q function.
     *
     * \param epsilon defined out-of-service threshold.
     */
    void SetInvqfunc(const double epsilon);

    /**
     * \brief Class to set the out-of-service threshold.
     *
     * \param e out-of-service threshold to be set.
     */
    void SetEps(const double e);

    /**
     * \brief Class to apply a rotation on a node position.
     *
     * \param P node position to be back-rotated.
     * \param axis axis of rotation.
     * \param angle angle of rotation.
     * \return the back-rotated node position.
     */
    static Vector BackRotate(const Vector& P, const RotoAxisType axis, double angle);

    /**
     * \brief Class to apply a shift in space to a node position.
     *
     * \param P position with respect to which to be shifted.
     * \param MM node position mobility model.
     * \return the back-shifted vector.
     */
    static Vector BackShift(const Vector& P, Ptr<MobilityModel> MM);

    /**
     * \brief Class to calculate the distance between a node and IRSs.
     *
     * \param Node mobility model of the node.
     * \param d_Vector vector of distances between the node and IRSs.
     */
    static void NodeToIrssDistance(Ptr<MobilityModel> Node, std::vector<double>& d_Vector);

    /**
     * \brief Class to calculate the angles between a node and IRSs.
     *
     * \param Node mobility model of the node.
     * \param a_Vector vector of angles between the node and IRSs.
     */
    static void NodeToIrssAngles(Ptr<MobilityModel> Node, std::vector<Angles>& a_Vector);

    /**
     * \brief Class to calculate the angles between a node and IRS.
     *
     * \param Node mobility model of the node.
     * \param irs IRS.
     * \return the angles between the node and IRS.
     */
    static Angles NodeToIrsAngles(Ptr<MobilityModel> Node, Ptr<Irs> irs);

    /**
     * \brief Class to calculate the beta of IRSs.
     *
     * \param beta pointer to the vector of betas to be set.
     * \param f_c carrier frequency.
     */
    static void IrsBeta(std::vector<double>& beta, const double f_c);

    /**
     * \brief Class to calculate the distance between a node and IRS.
     *
     * \param Node mobility model of the node.
     * \param irs IRS.
     * \return the distance between the node and IRS.
     */
    static double NodeToIrsDistance(Ptr<MobilityModel> Node, Ptr<Irs> irs);

    /**
     * \brief Class that returns the parameters of the IRS.
     *
     * \param irs IRS.
     * \param patch IRS patch.
     * \param phaseY phase Y.
     * \param phaseX phase X.
     * \param distance distance.
     */
    static void GetServedParam(Ptr<Irs> irs,
                               Ptr<IrsPatch> patch,
                               double& phaseY,
                               double& phaseX,
                               double& distance);

    /**
     * \brief Class that returns the elevation between two nodes.
     *
     * \param destination destination node.
     * \param origin origin node.
     * \return the elevation between the two nodes.
     */
    static double GetElevation(const Vector& destination, const Vector& origin);

    /**
     * \brief Class that returns the elevation angle from the inclination.
     *
     * \param angles inclination angle.
     * \return the elevation angle.
     */
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
