#ifndef TRACE_READER_H
#define TRACE_READER_H

#include "ns3/nstime.h"
#include "ns3/vector.h"

#include <cstdio>
#include <deque>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <zlib.h>

namespace ns3
{

struct NodeInfo
{
    std::string nodeId;
    Vector initialPosition; // lat, lon, alt from nodes.csv.gz
};

struct TracePoint
{
    std::string nodeId;
    Time time;
    Vector position;
};

class TraceReader
{
  public:
    static TraceReader* Get();

    // Delete copy constructor and assignment operator
    TraceReader(const TraceReader&) = delete;
    TraceReader& operator=(const TraceReader&) = delete;

    bool Register(const std::string& traceFile,
                  const std::string& deviceId,
                  Vector& initialPosition);

    /**
     * \brief Get the list of device IDs available in a trace file.
     * \param traceFile The path to the trace file (tar.gz)
     * \return A vector of device IDs found in nodes.csv.gz
     */
    std::vector<std::string> GetDeviceIds(const std::string& traceFile);

    // Returns true if a point was found (interpolated or exact).
    // If false, it means end of trace or no data.
    bool GetNextPoint(const std::string& traceFile,
                      const std::string& deviceId,
                      Time currentTime,
                      Vector& position);

  private:
    TraceReader();
    ~TraceReader();

    void OpenTraceFile(const std::string& traceFile);
    /**
     * @brief Reads the trace file until the specified time is reached
     * @param traceFile The trace file to read from
     * @param deviceId The device ID to read for
     * @param reqTime The time to read until
     */
    void ReadUntil(const std::string& traceFile, const std::string& deviceId, Time reqTime);
    void ParseLine(const std::string& traceFile, const std::string& line);

    static TraceReader* m_instance;

    struct TraceFileState
    {
        struct archive* archive_handle;
        z_stream strm;
        bool finished;
        Time lastReadTime;
        std::string lineBuffer; // Buffer for incomplete lines
        bool fileFound;
        unsigned long fileRemainingBytes;
        unsigned char inBuffer[16384];  // Input buffer for zlib
        unsigned char outBuffer[16384]; // Output buffer for zlib

        // Available nodes from nodes.csv.gz
        std::unordered_map<std::string, NodeInfo> availableNodes;
        // Queue of points for each device in this file
        std::unordered_map<std::string, std::deque<TracePoint>> buffers;
        // Registered devices for this file
        std::set<std::string> registeredDevices;
    };

    std::unordered_map<std::string, TraceFileState> m_traceFiles;
};

} // namespace ns3

#endif // TRACE_READER_H
