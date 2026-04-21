#include "scenario.h"

#include <ns3/nr-helper.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScenarioMobility");

void
Scenario::UpdateAntennaDirectivity(Ptr<NetDevice> dev,
                                   DirectivityConfiguration config,
                                   std::string deviceType,
                                   std::optional<uint32_t> netId,
                                   Ptr<const MobilityModel> model)
{
    uint32_t nodeId = model->GetObject<Node>()->GetId();

    Vector currentPos = model->GetPosition();
    Vector targetPos;
    bool targetFound = false;

    if (config.mode == "serving-gnb" || config.mode == "nearest-gnb")
    {
        NS_ASSERT_MSG(deviceType == "nr",
                      "Only NR devices can use serving-gnb/nearest-gnb directivity mode");

        bool useNearestGnb = (config.mode == "nearest-gnb");

        // Try serving-gnb logic first if requested
        if (config.mode == "serving-gnb")
        {
            if (Ptr<NrUeNetDevice> ueDevice = DynamicCast<NrUeNetDevice>(dev))
            {
                auto rrcState = ueDevice->GetRrc()->GetState();

                if (rrcState == NrUeRrc::CONNECTED_NORMALLY ||
                    rrcState == NrUeRrc::CONNECTED_HANDOVER)
                {
                    uint16_t cellId = ueDevice->GetRrc()->GetCellId();

                    // Find the gNB with this CellID
                    if (netId.has_value())
                    {
                        auto it = m_nrGnbDevices.find(*netId);
                        if (it != m_nrGnbDevices.end())
                        {
                            for (const auto& devContainer : it->second)
                            {
                                for (uint32_t i = 0; i < devContainer.GetN(); ++i)
                                {
                                    Ptr<NetDevice> gnbNetDev = devContainer.Get(i);
                                    if (auto gnbDev = DynamicCast<NrGnbNetDevice>(gnbNetDev))
                                    {
                                        if (gnbDev->GetCellId() == cellId)
                                        {
                                            Ptr<Node> gnbNode = gnbDev->GetNode();
                                            if (gnbNode)
                                            {
                                                Ptr<MobilityModel> gnbMob =
                                                    gnbNode->GetObject<MobilityModel>();
                                                if (gnbMob)
                                                {
                                                    targetPos = gnbMob->GetPosition();
                                                    targetFound = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!targetFound)
            {
#ifdef SINR_DISTANCE_PRINT_DEBUG
                std::cout << "UE Falling back to nearest-gnb" << std::endl;
#endif
                // Fallback to nearest-gnb if not connected or gNB not found
                useNearestGnb = true;
            }
        }

        if (useNearestGnb)
        {
            double minDist = std::numeric_limits<double>::max();

            if (netId.has_value())
            {
                auto it = m_nrGnbDevices.find(*netId);
                if (it != m_nrGnbDevices.end())
                {
                    // Iterate only over gNBs for this netId
                    for (const auto& devContainer : it->second)
                    {
                        for (uint32_t i = 0; i < devContainer.GetN(); ++i)
                        {
                            Ptr<NetDevice> gnbDev = devContainer.Get(i);
                            if (!gnbDev)
                            {
                                continue;
                            }
                            Ptr<Node> gnbNode = gnbDev->GetNode();
                            if (!gnbNode || gnbNode->GetId() == nodeId)
                            {
                                continue;
                            }

                            Ptr<MobilityModel> gnbMob = gnbNode->GetObject<MobilityModel>();
                            if (gnbMob)
                            {
                                double dist = gnbMob->GetDistanceFrom(model);
                                if (dist < minDist)
                                {
                                    minDist = dist;
                                    targetPos = gnbMob->GetPosition();
                                    targetFound = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (config.mode == "nearest-ue")
    {
        // TARGET_UE
        NS_ASSERT_MSG(deviceType == "nr", "Only NR devices can use nearest-ue directivity mode");
        double minDist = std::numeric_limits<double>::max();

        if (netId.has_value())
        {
            auto it = m_nrUeDevices.find(*netId);
            if (it != m_nrUeDevices.end())
            {
                // Iterate only over UEs for this netId
                // m_nrUeDevices is map<uint32_t, std::vector<Ptr<NetDevice>>>
                for (const auto& ueDev : it->second)
                {
                    if (!ueDev)
                    {
                        continue;
                    }
                    Ptr<Node> ueNode = ueDev->GetNode();
                    if (!ueNode || ueNode->GetId() == nodeId)
                    {
                        continue;
                    }

                    Ptr<MobilityModel> ueMob = ueNode->GetObject<MobilityModel>();
                    if (ueMob)
                    {
                        double dist = ueMob->GetDistanceFrom(model);
                        if (dist < minDist)
                        {
                            minDist = dist;
                            targetPos = ueMob->GetPosition();
                            targetFound = true;
                        }
                    }
                }
            }
        }
    }
    else if (config.mode == "earth-centered")
    {
        targetPos = Vector(0, 0, 0);
        targetFound = true;
    }
    else if (config.mode == "point")
    {
        // Assuming config.position is in the simulation coordinate system
        if (config.coordinates == "geocentric")
        {
            targetPos = config.position;
        }
        else if (config.coordinates == "geographic")
        {
            targetPos =
                GeographicPositions::GeographicToCartesianCoordinates(config.position.x,
                                                                      config.position.y,
                                                                      config.position.z,
                                                                      GeographicPositions::SPHERE);
        }
        else
        {
            NS_ASSERT_MSG(
                false,
                "Unknown coordinate system for directivity point: " << config.coordinates);
        }
        targetFound = true;
    }
    else if (config.mode == "node")
    {
        Ptr<Node> targetNode = GetNodeByKey(config.key, config.index);

        if (targetNode)
        {
            Ptr<MobilityModel> targetMob = targetNode->GetObject<MobilityModel>();
            if (targetMob)
            {
                targetPos = targetMob->GetPosition();
                targetFound = true;
            }
        }
    }

    if (!targetFound)
    {
        return;
    }

    Vector dir = targetPos - currentPos;
    // Calculate angles in radians
    // Azimuth: Angle in XY plane from X axis
    double azimuth = std::atan2(dir.y, dir.x);
    // Elevation: Angle from XY plane towards Z axis
    double dist2d = std::hypot(dir.x, dir.y);
    double elevation = std::atan2(dir.z, dist2d);

    if (config.bearingOffset.has_value())
    {
        azimuth += config.bearingOffset.value() * M_PI / 180.0;
    }
    if (config.downtiltOffset.has_value())
    {
        elevation -= config.downtiltOffset.value() * M_PI / 180.0;
    }

    // Update Antenna based on Device Type
    std::vector<Ptr<Object>> upaAntennas;

    if (deviceType == "nr")
    {
        uint32_t startBwp = 0;
        uint32_t endBwp = 1;
        uint32_t numBwp = ns3::NrHelper::GetNumberBwp(dev);
        if (config.bwpId.has_value())
        {
            if (config.bwpId.value() < numBwp)
            {
                startBwp = config.bwpId.value();
                endBwp = startBwp + 1;
            }
            else
            {
                startBwp = numBwp; // no-op if out of bounds
                endBwp = numBwp;
            }
        }
        else
        {
            endBwp = numBwp;
        }

        for (uint32_t i = startBwp; i < endBwp; ++i)
        {
            Ptr<NrPhy> phy = nullptr;
            if (Ptr<NrUeNetDevice> nrUe = DynamicCast<NrUeNetDevice>(dev))
            {
                phy = nrUe->GetPhy(i);
            }
            else if (Ptr<NrGnbNetDevice> nrGnb = DynamicCast<NrGnbNetDevice>(dev))
            {
                phy = nrGnb->GetPhy(i);
            }

            if (phy)
            {
                auto spectrumPhy = phy->GetSpectrumPhy();
                if (spectrumPhy)
                {
                    upaAntennas.emplace_back(spectrumPhy->GetAntenna());
                }
            }
        }
    }
    else if (deviceType == "lte")
    {
        // TODO test and verify
        if (Ptr<LteNetDevice> lteDev = DynamicCast<LteNetDevice>(dev))
        {
            if (Ptr<LteEnbNetDevice> lteEnb = DynamicCast<LteEnbNetDevice>(dev))
            {
                auto phy = lteEnb->GetPhy();
                if (phy)
                {
                    // LteEnbPhy has GetDlSpectrumPhy
                    auto specPhy = phy->GetDlSpectrumPhy();
                    if (specPhy)
                    {
                        upaAntennas.emplace_back(specPhy->GetAntenna());
                    }
                }
            }
            else if (Ptr<LteUeNetDevice> lteUe = DynamicCast<LteUeNetDevice>(dev))
            {
                auto phy = lteUe->GetPhy();
                if (phy)
                {
                    // LteUePhy has GetDlSpectrumPhy
                    auto specPhy = phy->GetDlSpectrumPhy();
                    if (specPhy)
                    {
                        upaAntennas.emplace_back(specPhy->GetAntenna());
                    }
                }
            }
        }
    }
    else if (deviceType == "wifi")
    {
        // TODO test and verify
        if (Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(dev))
        {
            auto phy = wifiDev->GetPhy();
            if (phy)
            {
                Ptr<SpectrumWifiPhy> specPhy = DynamicCast<SpectrumWifiPhy>(phy);
                if (specPhy)
                {
                    upaAntennas.emplace_back(specPhy->GetAntenna());
                }
            }
        }
    }

    for (const auto& antennaObj : upaAntennas)
    {
        if (antennaObj)
        {
            bool steerArrays = deviceType != "nr";
            RecursiveUpdateAntennaDirectivity(antennaObj, azimuth, elevation, steerArrays);
        }
    }

    // Schedule next update
    Simulator::Schedule(config.precision,
                        &Scenario::UpdateAntennaDirectivity,
                        this,
                        dev,
                        config,
                        deviceType,
                        netId,
                        model);
}

void
Scenario::RecursiveUpdateAntennaDirectivity(Ptr<Object> antennaObj,
                                            double azimuth,
                                            double elevation,
                                            bool steerArrays)
{
    if (!antennaObj)
    {
        return;
    }

    // Try Parabolic
    if (Ptr<ParabolicAntennaModel> parabolic = DynamicCast<ParabolicAntennaModel>(antennaObj))
    {
        parabolic->SetOrientation(azimuth * 180.0 / M_PI);
        parabolic->SetElevation(elevation * 180.0 / M_PI);
    }
    else if (Ptr<CircularApertureAntennaModel> circular =
                 DynamicCast<CircularApertureAntennaModel>(antennaObj))
    {
        circular->SetBoresightAzimuth(azimuth);
        circular->SetBoresightInclination(M_PI_2 - elevation);
    }
    else if (Ptr<UniformPlanarArray> upa = DynamicCast<UniformPlanarArray>(antennaObj))
    {
        if (steerArrays)
        {
            // TODO verify is correct to set azimuth and elevation here in this way
            upa->SetAlpha(azimuth);
            upa->SetBeta(-elevation);
        }

        // Recurse on element
        Ptr<const AntennaModel> elem = upa->GetAntennaElement();
        if (elem)
        {
            auto elemFactory = NrRadioGeoEnvironmentMapHelper::ConfigureObjectFactory(
                ns3::ConstCast<ns3::AntennaModel>(elem));
            // ConstCast is needed because GetAntennaElement returns const pointer
            Ptr<AntennaModel> mutableElem = DynamicCast<AntennaModel>(elemFactory.Create());
            RecursiveUpdateAntennaDirectivity(mutableElem, azimuth, elevation, steerArrays);
            upa->SetAntennaElement(mutableElem);
        }
    }
    else if (Ptr<IsotropicAntennaModel> array = DynamicCast<IsotropicAntennaModel>(antennaObj))
    {
        // Do nothing
    }
}

void
Scenario::ConfigureEntityMobility(const std::string& entityKey,
                                  Ptr<EntityConfiguration> entityConf,
                                  const uint32_t entityId)
{
    NS_LOG_FUNCTION(entityKey << entityConf << entityId);

    MobilityHelper mobility;
    const auto mobilityConf = entityConf->GetMobilityModel();
    const auto mobilityType = mobilityConf.GetName(); // Configure Entity Mobility
    MobilityFactoryHelper::SetMobilityModel(mobility, mobilityConf);

    if (entityKey == "drones")
    {
        mobility.Install(m_drones.Get(entityId));
        MobilityFactoryHelper::ApplyExtraAttributes(m_drones.Get(entityId), mobilityConf);
        std::ostringstream oss;
        oss << "/DroneList/" << entityId << "/$ns3::MobilityModel/CourseChange";
        auto mob = m_drones.Get(entityId)->GetObject<MobilityModel>();
        mob->TraceConnect("CourseChange",
                          oss.str(),
                          MakeCallback(&Scenario::DroneCourseChange, this));
    }
    else if (entityKey == "ZSPs")
    {
        mobility.Install(m_zsps.Get(entityId));
        MobilityFactoryHelper::ApplyExtraAttributes(m_zsps.Get(entityId), mobilityConf);
    }
    else if (entityKey == "nodes")
    {
        mobility.Install(m_plainNodes.Get(entityId));
        MobilityFactoryHelper::ApplyExtraAttributes(m_plainNodes.Get(entityId), mobilityConf);
    }
    else if (entityKey == "leo-sats")
    {
        auto node = m_leoSats.Get(entityId);
        mobility.Install(node);
        MobilityFactoryHelper::ApplyExtraAttributes(node, mobilityConf);
        std::ostringstream oss;
        oss << "/LeoSatList/" << entityId << "/$ns3::MobilityModel/CourseChange";

        auto mob = node->GetObject<MobilityModel>();
        mob->TraceConnect("CourseChange",
                          oss.str(),
                          MakeCallback(&Scenario::LeoSatCourseChange, this));
    }
    else if (entityKey == "vehicles")
    {
        auto vehicle = m_vehicles.Get(entityId);
        mobility.Install(vehicle);
        MobilityFactoryHelper::ApplyExtraAttributes(vehicle, mobilityConf);
        std::ostringstream oss;
        oss << "/VehicleList/" << entityId << "/$ns3::MobilityModel/CourseChange";
        auto mob = vehicle->GetObject<MobilityModel>();
        mob->TraceConnect("CourseChange",
                          oss.str(),
                          MakeCallback(&Scenario::VehicleCourseChange, this));
    }
    else
    {
        NS_FATAL_ERROR("Unsupported Entity Key: " << entityKey);
    }
}

void
Scenario::LeoSatCourseChange(std::string context, Ptr<const MobilityModel> model)
{
    auto mobility = DynamicCast<const GeocentricMobilityModel>(model);
    if (mobility)
    {
        auto pos = mobility->GetPosition(ns3::PositionType::GEOCENTRIC);
        auto geo = mobility->GetPosition(ns3::PositionType::GEOGRAPHIC);
        Ptr<const Node> node = model->GetObject<Node>();
        // Write to CSV file: Time,Node,X,Y,Z,Latitude,Longitude,Altitude
        if (m_leoSatTraceStream)
        {
            *m_leoSatTraceStream->GetStream()
                << Simulator::Now().GetSeconds() << "," << node->GetId() << "," << pos.x << ","
                << pos.y << "," << pos.z << "," << geo.x << "," << geo.y << "," << geo.z
                << std::endl;
        }
    }
}

void
Scenario::VehicleCourseChange(std::string context, Ptr<const MobilityModel> model)
{
    auto mobility = DynamicCast<GeocentricMobilityModel>(ConstCast<MobilityModel>(model));
    if (mobility)
    {
        auto pos = mobility->GetPosition(ns3::PositionType::GEOCENTRIC);
        auto geo = mobility->GetPosition(ns3::PositionType::GEOGRAPHIC);
        Ptr<const Node> node = model->GetObject<Node>();
        // Write to CSV file: Time,Node,X,Y,Z,Latitude,Longitude,Altitude,ElevationAngle
        double minDistance = std::numeric_limits<double>::max();
        Ptr<const GeocentricMobilityModel> nearestSat = nullptr;
        int32_t nearestSatId = -1;

        for (uint32_t i = 0; i < m_leoSats.GetN(); ++i)
        {
            Ptr<Node> satNode = m_leoSats.Get(i);
            Ptr<const GeocentricMobilityModel> satMobility =
                satNode->GetObject<GeocentricMobilityModel>();
            if (satMobility)
            {
                double dist = mobility->GetDistanceFrom(satMobility);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    nearestSat = satMobility;
                    nearestSatId = satNode->GetId();
                }
            }
        }

        double elevationAngle = 0.0;
        if (nearestSat)
        {
            elevationAngle = mobility->GetElevationAngle(nearestSat);
        }

        if (m_vehicleTraceStream)
        {
            *m_vehicleTraceStream->GetStream()
                << Simulator::Now().GetSeconds() << "," << node->GetId() << "," << pos.x << ","
                << pos.y << "," << pos.z << "," << geo.x << "," << geo.y << "," << geo.z << ","
                << nearestSatId << "," << elevationAngle << std::endl;
        }
    }
}

void
Scenario::DroneCourseChange(std::string context, Ptr<const MobilityModel> model)
{
    Vector position = model->GetPosition();
    std::string start = "/DroneList/";
    std::string end = "/$ns3::MobilityModel/CourseChange";
    std::string id = context.substr(context.find(start) + start.length(),
                                    context.length() - end.length() - start.length());
    auto dronePeripheralsContainer = m_drones.Get(std::stoi(id))->GetPeripherals();
    Ptr<DronePeripheral> peripheral;
    std::vector<int> regionindex;
    for (DronePeripheralContainer::Iterator i = dronePeripheralsContainer->Begin();
         i != dronePeripheralsContainer->End();
         i++)
    {
        peripheral = *i;
        regionindex = peripheral->GetRegionsOfInterest();
        int status = irc->IsInRegions(regionindex, position);
        if (regionindex.empty())
        {
            continue;
        }
        if (status >= 0 || status == -2)
        {
            if (peripheral->GetState() != DronePeripheral::PeripheralState::ON)
            {
                peripheral->SetState(DronePeripheral::PeripheralState::ON);
            }
        }
        else
        {
            if (peripheral->GetState() == DronePeripheral::PeripheralState::ON)
            {
                peripheral->SetState(DronePeripheral::PeripheralState::IDLE);
            }
        }
    }
}

} // namespace ns3
