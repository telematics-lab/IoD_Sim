#ifndef TRACE_BASED_MOBILITY_MODEL_H
#define TRACE_BASED_MOBILITY_MODEL_H

#include "ns3/event-id.h"
#include "ns3/geocentric-mobility-model.h"
#include "ns3/nstime.h"

#include <string>

namespace ns3
{

class TraceBasedMobilityModel : public GeocentricMobilityModel
{
  public:
    static TypeId GetTypeId();

    TraceBasedMobilityModel();
    ~TraceBasedMobilityModel();

    void SetTraceFile(std::string file);
    void SetDeviceId(std::string deviceId);

    void SetPrecision(Time precision);
    Time GetPrecision() const;

  private:
    Vector DoGetPosition(PositionType type) const override;
    void DoSetPosition(const Vector& position, PositionType type) override;
    Vector DoGetVelocity() const override;
    int64_t DoAssignStreams(int64_t) override;

    void Update();
    Vector CalcPosition(Time t) const;

    std::string m_traceFile;
    std::string m_deviceId;

    Time m_precision;
    EventId m_updateEvent;
    Vector m_position; // Cached geographic position (Lat, Lon, Alt)
    Vector m_velocity; // Calculated velocity
    Time m_lastTime;   // Last update time
    bool m_firstUpdate;
};

} // namespace ns3

#endif // TRACE_BASED_MOBILITY_MODEL_H
