#ifndef IOD_SIM_SCENARIO_H
#define IOD_SIM_SCENARIO_H
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
#include "ns3/beam-manager.h"
#include "ns3/bwp-manager-gnb.h"
#include "ns3/bwp-manager-ue.h"
#include "ns3/circular-aperture-antenna-model.h"
#include "ns3/ideal-beamforming-algorithm.h"
#include "ns3/isotropic-antenna-model.h"
#include "ns3/lte-spectrum-phy.h"
#include "ns3/nr-epc-ue-nas.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-gnb-rrc.h"
#include "ns3/nr-net-device.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-rrc.h"
#include "ns3/parabolic-antenna-model.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/uniform-planar-array.h"
#include <ns3/app-statistics-helper.h>
#include <ns3/buildings-helper.h>
#include <ns3/config.h>
#include <ns3/csma-module.h>
#include <ns3/debug-helper.h>
#include <ns3/drone-client-application.h>
#include <ns3/drone-container.h>
#include <ns3/drone-energy-model-helper.h>
#include <ns3/drone-list.h>
#include <ns3/drone-peripheral-container.h>
#include <ns3/drone-peripheral.h>
#include <ns3/drone-server-application.h>
#include <ns3/energy-source.h>
#include <ns3/geocentric-mobility-model.h>
#include <ns3/ideal-beamforming-helper.h>
#include <ns3/input-peripheral.h>
#include <ns3/interest-region-container.h>
#include <ns3/internet-module.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/ipv4-routing-helper.h>
#include <ns3/ipv4-simulation-helper.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/leo-sat-list.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/lte-phy-simulation-helper.h>
#include <ns3/lte-setup-helper.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/mobility-factory-helper.h>
#include <ns3/mobility-helper.h>
#include <ns3/mobility-model-configuration.h>
#include <ns3/names.h>
#include <ns3/nat-application.h>
#include <ns3/net-device-container.h>
#include <ns3/node-list.h>
#include <ns3/none-phy-layer-configuration.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-helper.h>
#include <ns3/nr-netdevice-configuration.h>
#include <ns3/nr-phy-layer-configuration.h>
#include <ns3/nr-phy-rx-trace.h>
#include <ns3/nr-phy-simulation-helper.h>
#include <ns3/nr-qos-flow-configuration.h>
#include <ns3/nr-qos-flow.h>
#include <ns3/nr-qos-rule.h>
#include <ns3/nr-radio-environment-map-helper.h>
#include <ns3/nr-radio-geo-environment-map-helper.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/null-ntn-demo-mac-layer-configuration.h>
#include <ns3/null-ntn-demo-mac-layer-simulation-helper.h>
#include <ns3/object-factory.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/ptr.h>
#include <ns3/radio-environment-map-helper.h>
#include <ns3/remote-list.h>
#include <ns3/report.h>
#include <ns3/rng-seed-manager.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/show-progress.h>
#include <ns3/simple-net-device.h>
#include <ns3/ssid.h>
#include <ns3/string.h>
#include <ns3/three-dimensional-rem-helper.h>
#include <ns3/three-gpp-phy-layer-configuration.h>
#include <ns3/three-gpp-phy-simulation-helper.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/traced-value.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/wifi-netdevice-configuration.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/zsp-list.h>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sys/resource.h>
#include <vector>

// #define SINR_DISTANCE_PRINT_DEBUG

// Set to apply ISL delay only on data (DataDelay of NrGnbMac)
// unset to apply ISL delay to the Point-To-Point backhaul link
// #define APPLY_ISL_DELAY_ONLY_ON_DATA

// #define ENABLE_CLI_COMMANDS
// Set to enable extra feature that requires CLI commands enabled at compile time

namespace ns3
{

constexpr int N_LAYERS = 4;
constexpr int PHY_LAYER = 0;
constexpr int MAC_LAYER = 1;
constexpr int NET_LAYER = 2;
// constexpr int APP_LAYER = 3;
constexpr int PROGRESS_REFRESH_INTERVAL_SECONDS = 1;

class Scenario
{
  public:
    Scenario(int argc, char** argv);
    virtual ~Scenario();

    void operator()();

  private:
    void ApplyStaticConfig();
    void ConfigureWorld();
    void ConfigurePhy();
    void ConfigureMac();
    void ConfigureNetwork();
    void EnableIpv4RoutingTableReporting();
    void ConfigureEntities(const std::string& entityKey, NodeContainer& nodes);
    void ConfigureEntityMobility(const std::string& entityKey,
                                 Ptr<EntityConfiguration> entityConf,
                                 const uint32_t entityId);
    NetDeviceContainer ConfigureEntityWifiStack(std::string entityKey,
                                                Ptr<NetdeviceConfiguration> entityNetDev,
                                                Ptr<Node> entityNode,
                                                const uint32_t entityId,
                                                const uint32_t deviceId,
                                                const uint32_t netId);
    Ptr<NetDevice> ConfigureLteEnb(Ptr<Node> entityNode,
                                   const uint32_t netId,
                                   const std::vector<AntennaModelConfiguration> antennaModels,
                                   const std::optional<ModelConfiguration> phyConf);
    Ptr<NetDevice> ConfigureLteUe(Ptr<Node> entityNode,
                                  const std::vector<LteBearerConfiguration> bearers,
                                  const uint32_t netId,
                                  const std::vector<AntennaModelConfiguration> antennaModels,
                                  const std::optional<ModelConfiguration> phyConf);

    Ptr<NetDevice> ConfigureNrGnb(Ptr<Node> entityNode,
                                  const uint32_t netId,
                                  const std::vector<AntennaModelConfiguration> antennaModels,
                                  const std::vector<ns3::NrPhyProperty> phyConf,
                                  const std::vector<ns3::NrPhyProperty> rrcConf,
                                  const std::vector<OutputLinkConfiguration> outputLinks,
                                  const std::vector<X2NeighborConfiguration> x2Neighbors,
                                  const uint32_t channelId,
                                  const std::vector<ChannelBandFilter> channelBands);

    Ptr<NetDevice> ConfigureNrUe(Ptr<Node> entityNode,
                                 const std::vector<NrQosFlowConfiguration> qosFlows,
                                 const uint32_t netId,
                                 const std::vector<AntennaModelConfiguration> antennaModels,
                                 const std::vector<ns3::NrPhyProperty> phyConf,
                                 const std::vector<ns3::NrPhyProperty> rrcConf,
                                 const std::vector<OutputLinkConfiguration> outputLinks,
                                 const uint32_t channelId,
                                 const std::vector<ChannelBandFilter> channelBands);

    void InstallEntityIpv4(Ptr<Node> entityNode,
                           NetDeviceContainer netDevices,
                           const uint32_t netId);
    void InstallEntityIpv4(Ptr<Node> entityNode, Ptr<NetDevice> netDevice, const uint32_t netId);
    void ConfigureEntityIpv4(Ptr<Node> entityNode,
                             NetDeviceContainer devContainer,
                             const uint32_t deviceId,
                             const uint32_t netId);
    void ConfigureEntityApplications(const std::string& entityKey,
                                     const Ptr<EntityConfiguration>& conf,
                                     const uint32_t& entityId);
    void ConfigureEntityMechanics(const std::string& entityKey,
                                  Ptr<EntityConfiguration> entityConf,
                                  const uint32_t entityId);
    Ptr<energy::EnergySource> ConfigureEntityBattery(const std::string& entityKey,
                                                     Ptr<EntityConfiguration> entityConf,
                                                     const uint32_t entityId);
    void ConfigureEntityPeripherals(const std::string& entityKey,
                                    const Ptr<EntityConfiguration>& conf,
                                    const uint32_t& entityId);
    void ConfigureInternetRemotes();
    void ConfigureInternetBackbone();
    void EnablePhyLteTraces();
    void EnablePhyNrTraces();
    void ConfigureRegionsOfInterest();
    void DroneCourseChange(std::string context, Ptr<const MobilityModel> model);
    void LeoSatCourseChange(std::string context, Ptr<const MobilityModel> model);
    void VehicleCourseChange(std::string context, Ptr<const MobilityModel> model);
    void ConfigureSimulator();
    void AttachAllNrUesToGnbs();
    void InitializeIslDelayMode();
    void UpdateIslDelay(uint32_t netId, Ptr<NrPhyLayerConfiguration> config);
    double GetISLMinimumDistance(Ptr<Node> n1, Ptr<Node> n2, uint32_t netId);
    double GetISLMinimumDistance(Ptr<Node> n, uint32_t gsIndex, uint32_t netId);
    // Returns {minDist, gsIndex}. If no ground station is reachable, returns
    // {std::numeric_limits<double>::infinity(), 0}
    std::pair<double, uint32_t> GetISLMinimumDistance(Ptr<Node> n, uint32_t netId);
    void ConfigureScheduling();
    void ConfigureFullMeshX2Links();
    void EvaluateSinrDistanceAttachment(const uint32_t netId);
    void ExecuteHandoverRequest(Ptr<NrUeNetDevice> ueDevice,
                                Ptr<NrGnbNetDevice> targetGnb,
                                uint16_t targetCellId,
                                uint32_t ueNetId);
    void UpdateAntennaDirectivity(Ptr<NetDevice> dev,
                                  DirectivityConfiguration config,
                                  std::string deviceType,
                                  std::optional<uint32_t> netId,
                                  Ptr<const MobilityModel> model);
    void RecursiveUpdateAntennaDirectivity(Ptr<Object> antennaObj,
                                           double azimuth,
                                           double elevation,
                                           bool isNr);
    Ptr<Node> GetNodeByKey(std::string key, uint32_t index);

    struct IslGraphCache
    {
        Time lastUpdate = Time::Min();
        std::map<Ptr<Node>, size_t> nodeToIndex;
        std::vector<Ptr<Node>> indexToNode;
        std::vector<std::vector<double>> adjMatrix;
        std::vector<std::vector<double>> shortestPaths; // APSP matrix
        std::vector<std::vector<size_t>> parents;       // APSP parents
        std::vector<std::vector<double>> nodeToGsDirectDist; // [satIndex][gsIndex]
        std::vector<double> minEarthDist; // from virtual earth node to each sat
        std::vector<size_t> earthParent;  // parent pointer to virtual earth node (numSats)
        std::vector<uint32_t> closestGsIdx; // closest GS index for each sat
        bool valid = false;

        void Clear()
        {
            nodeToIndex.clear();
            indexToNode.clear();
            adjMatrix.clear();
            shortestPaths.clear();
            parents.clear();
            nodeToGsDirectDist.clear();
            minEarthDist.clear();
            earthParent.clear();
            closestGsIdx.clear();
            valid = false;
        }
    };

    void BuildIslGraph(uint32_t netId);

    std::map<uint32_t, IslGraphCache> m_islCaches;
    NodeContainer m_plainNodes;
    DroneContainer m_drones;
    NodeContainer m_zsps;
    NodeContainer m_remoteNodes;
    NodeContainer m_backbone;
    NodeContainer m_leoSats;
    NodeContainer m_vehicles;

    std::array<std::vector<Ptr<Object>>, N_LAYERS> m_protocolStacks;
    Ptr<OutputStreamWrapper> m_leoSatTraceStream;
    Ptr<OutputStreamWrapper> m_vehicleTraceStream;

    // NR gNB and UE tracking for proper attachment
    std::map<uint32_t, std::vector<NetDeviceContainer>> m_nrGnbDevices;
    std::map<uint32_t, std::vector<Ptr<NetDevice>>> m_nrUeDevices;

    // Application statistics helper
    AppStatisticsHelper m_appStatsHelper;

    // Persistent containers for NR attachment workaround
    std::list<std::shared_ptr<NetDeviceContainer>> m_persistentContainers;

    // Track active SINR attachment loops to avoid duplicates
    std::set<uint32_t> m_sinrAttachmentRunning;
};

} // namespace ns3
#endif
