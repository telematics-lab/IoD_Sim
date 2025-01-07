#ifndef SIMULATION_DURATION_H
#define SIMULATION_DURATION_H

#include <ns3/nstime.h>

#include <libxml/xmlwriter.h>

namespace ns3
{

/**
 * \ingroup report
 *
 * \brief Store real and virtual time, useful when it is required to calculate the time taken in
 * respect to the computation (real) and to the simulation (virtual).
 */
class SimulationDuration
{
  public:
    /**
     * Construct the object with the given values.
     *
     * \params realDuration the real time
     * \param virtualDuration the virtual time
     */
    SimulationDuration(const Time& realDuration, const Time& virtualDuration);
    SimulationDuration();

    /**
     * Write SimulationDuration report data to a XML file with a given handler
     *
     * \param handle the handler to communicate data to the opened XML file
     */
    void Write(xmlTextWriterPtr handle) const;

  private:
    /**
     * Get current real time
     */
    static Time GetRealTime();

    /**
     * Get current virtual time
     */
    static Time GetVirtualTime();

    Time m_realDuration;    /// the duration measured by the computer
    Time m_virtualDuration; /// the duration relative to the simulator time
};

} // namespace ns3

#endif /* SIMULATION_DURATION_H */
