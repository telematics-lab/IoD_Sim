#include "trace-reader.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <fcntl.h>
#include <sstream>
#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TraceReader");

TraceReader* TraceReader::m_instance = nullptr;

TraceReader*
TraceReader::Get()
{
    if (!m_instance)
    {
        m_instance = new TraceReader();
    }
    return m_instance;
}

TraceReader::TraceReader()
{
    NS_LOG_FUNCTION(this);
}

TraceReader::~TraceReader()
{
    for (auto& pair : m_traceFiles)
    {
        if (pair.second.archive_handle)
        {
            archive_read_free(pair.second.archive_handle);
        }
        inflateEnd(&pair.second.strm);
    }
}

bool
TraceReader::Register(const std::string& traceFile,
                      const std::string& deviceId,
                      Vector& initialPosition)
{
    NS_LOG_FUNCTION(this << traceFile << deviceId);

    if (m_traceFiles.find(traceFile) == m_traceFiles.end())
    {
        OpenTraceFile(traceFile);
    }

    TraceFileState& state = m_traceFiles[traceFile];

    // Check if the node exists in the available nodes
    auto it = state.availableNodes.find(deviceId);
    if (it == state.availableNodes.end())
    {
        std::stringstream availableNodesList;
        availableNodesList << "Available nodes: ";
        for (const auto& nodePair : state.availableNodes)
        {
            availableNodesList << "'" << nodePair.first << "' ";
        }
        NS_LOG_ERROR("Device '" << deviceId << "' not found in nodes.csv.gz for trace file "
                                << traceFile << ". " << availableNodesList.str());
        return false;
    }

    // Set the initial position from the node info
    initialPosition = it->second.initialPosition;

    state.registeredDevices.insert(deviceId);
    NS_LOG_INFO("Registered device " << deviceId << " for trace file " << traceFile
                                     << " with initial position (" << initialPosition.x << ", "
                                     << initialPosition.y << ", " << initialPosition.z << ")");
    return true;
}

std::vector<std::string>
TraceReader::GetDeviceIds(const std::string& traceFile)
{
    NS_LOG_FUNCTION(this << traceFile);

    if (m_traceFiles.find(traceFile) == m_traceFiles.end())
    {
        OpenTraceFile(traceFile);
    }

    std::vector<std::string> deviceIds;
    const TraceFileState& state = m_traceFiles[traceFile];

    for (const auto& pair : state.availableNodes)
    {
        deviceIds.push_back(pair.first);
    }

    return deviceIds;
}

void
TraceReader::OpenTraceFile(const std::string& traceFile)
{
    NS_LOG_FUNCTION(this << traceFile);

    struct archive* a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_tar(a);

    if (archive_read_open_filename(a, traceFile.c_str(), 10240) != ARCHIVE_OK)
    {
        NS_FATAL_ERROR("Failed to open trace file: " << traceFile << " (" << archive_error_string(a) << ")");
    }

    TraceFileState state;
    state.archive_handle = a;
    state.finished = false;
    state.lastReadTime = Seconds(0);
    state.fileFound = false;
    state.fileRemainingBytes = 0;

    struct archive_entry* entry;
    bool nodesFound = false;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        std::string filename = archive_entry_pathname(entry);

        if (filename.find("nodes.csv.gz") != std::string::npos)
        {
            nodesFound = true;
            unsigned long nodesFileSize = archive_entry_size(entry);
            NS_LOG_INFO("Found nodes.csv.gz in " << traceFile << ", size: " << nodesFileSize);

            // Read and decompress nodes.csv.gz
            z_stream nodesStrm;
            nodesStrm.zalloc = Z_NULL;
            nodesStrm.zfree = Z_NULL;
            nodesStrm.opaque = Z_NULL;
            nodesStrm.avail_in = 0;
            nodesStrm.next_in = Z_NULL;
            if (inflateInit2(&nodesStrm, 16 + MAX_WBITS) != Z_OK)
            {
                NS_FATAL_ERROR("Failed to initialize zlib for nodes.csv.gz");
            }

            std::string nodesBuffer;
            unsigned char nodesInBuf[16384];
            unsigned char nodesOutBuf[16384];

            while (true)
            {
                ssize_t len = archive_read_data(a, nodesInBuf, sizeof(nodesInBuf));
                if (len < 0) {
                     NS_FATAL_ERROR("Error reading nodes.csv.gz: " << archive_error_string(a));
                }
                if (len == 0) break; // EOF

                nodesStrm.avail_in = len;
                nodesStrm.next_in = (Bytef*)nodesInBuf;

                int ret;
                do
                {
                    nodesStrm.avail_out = sizeof(nodesOutBuf);
                    nodesStrm.next_out = (Bytef*)nodesOutBuf;
                    ret = inflate(&nodesStrm, Z_NO_FLUSH);
                    if (ret != Z_OK && ret != Z_STREAM_END)
                    {
                        NS_FATAL_ERROR("Zlib inflate error for nodes.csv.gz: " << ret);
                    }
                    size_t have = sizeof(nodesOutBuf) - nodesStrm.avail_out;
                    nodesBuffer.append((char*)nodesOutBuf, have);
                } while (nodesStrm.avail_out == 0);

                if (ret == Z_STREAM_END) break;
            }

            inflateEnd(&nodesStrm);

            // Parse nodes.csv.gz content
            std::stringstream ss(nodesBuffer);
            std::string line;
            while (std::getline(ss, line))
            {
                if (line.empty()) continue;

                std::stringstream lineStream(line);
                std::string segment;
                std::vector<std::string> parts;

                while (std::getline(lineStream, segment, ';'))
                {
                    parts.push_back(segment);
                }

                if (parts.size() < 5) continue;

                NodeInfo nodeInfo;
                nodeInfo.nodeId = parts[0];
                nodeInfo.initialPosition.x = std::stod(parts[2]); // latitude
                nodeInfo.initialPosition.y = std::stod(parts[3]); // longitude
                nodeInfo.initialPosition.z = std::stod(parts[4]); // altitude

                state.availableNodes[nodeInfo.nodeId] = nodeInfo;
                NS_LOG_DEBUG("Loaded node " << nodeInfo.nodeId << " with position ("
                                            << nodeInfo.initialPosition.x << ", "
                                            << nodeInfo.initialPosition.y << ", "
                                            << nodeInfo.initialPosition.z << ")");
            }
            NS_LOG_INFO("Loaded " << state.availableNodes.size() << " nodes from nodes.csv.gz");
            continue; // Continue to search for traces.csv.gz
        }

        if (filename.find("traces.csv.gz") != std::string::npos)
        {
            state.fileFound = true;
            state.fileRemainingBytes = archive_entry_size(entry);
            NS_LOG_INFO("Found traces.csv.gz in " << traceFile
                                                  << ", size: " << state.fileRemainingBytes);
            break; // Stop iteration, state.archive_handle is now positioned at traces.csv.gz data
        }
    }

    if (!nodesFound)
    {
        NS_FATAL_ERROR("nodes.csv.gz not found in " << traceFile);
    }
    if (!state.fileFound)
    {
        NS_FATAL_ERROR("traces.csv.gz not found in " << traceFile);
    }

    // Insert state map
    m_traceFiles[traceFile] = state;

    // Initialize zlib for traces.csv.gz
    TraceFileState& finalState = m_traceFiles[traceFile];
    finalState.strm.zalloc = Z_NULL;
    finalState.strm.zfree = Z_NULL;
    finalState.strm.opaque = Z_NULL;
    finalState.strm.avail_in = 0;
    finalState.strm.next_in = Z_NULL;
    if (inflateInit2(&finalState.strm, 16 + MAX_WBITS) != Z_OK)
    {
        NS_FATAL_ERROR("Failed to initialize zlib for traces.csv.gz");
    }
}

void
TraceReader::ReadUntil(const std::string& traceFile, const std::string& deviceId, Time reqTime)
{
    TraceFileState& state = m_traceFiles[traceFile];
    if (state.finished)
    {
        return;
    }

    while (!state.finished)
    {
        // Check if we have enough data for the specific device
        auto& buffer = state.buffers[deviceId];
        if (!buffer.empty() && buffer.back().time > reqTime)
        {
            return;
        }

        // Check if we have a full line in lineBuffer
        size_t pos = state.lineBuffer.find('\n');
        if (pos != std::string::npos)
        {
            std::string line = state.lineBuffer.substr(0, pos);
            state.lineBuffer.erase(0, pos + 1);
            if (!line.empty())
            {
                ParseLine(traceFile, line);
            }
            continue;
        }

        // Need more data
        if (state.strm.avail_in == 0)
        {
            if (state.fileRemainingBytes == 0)
            {
                state.finished = true;
                break;
            }

            // Read from archive
            NS_LOG_DEBUG("Reading archive data...");

            ssize_t len = archive_read_data(state.archive_handle, state.inBuffer, sizeof(state.inBuffer));

            if (len < 0)
            {
                NS_LOG_ERROR("Error reading archive data for " << traceFile << ": " << archive_error_string(state.archive_handle));
                state.finished = true;
                break;
            }
            if (len == 0)
            {
                state.finished = true;
                break;
            }

            NS_LOG_DEBUG("Read " << len << " bytes from archive");

            state.strm.avail_in = len;
            state.strm.next_in = (Bytef*)state.inBuffer;

            // Update remaining bytes just for info, though libarchive handles EOF
            if (state.fileRemainingBytes >= (unsigned long)len)
                 state.fileRemainingBytes -= len;
            else state.fileRemainingBytes = 0;
        }

        // Decompress
        state.strm.avail_out = sizeof(state.outBuffer);
        state.strm.next_out = (Bytef*)state.outBuffer;

        NS_LOG_DEBUG("About to inflate: avail_in=" << state.strm.avail_in
                                                   << ", next_in=" << (void*)state.strm.next_in
                                                   << ", avail_out=" << state.strm.avail_out);

        int ret = inflate(&state.strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END)
        {
            NS_LOG_ERROR("Zlib inflate error: " << ret << " for " << traceFile);
            state.finished = true;
            break;
        }

        size_t have = sizeof(state.outBuffer) - state.strm.avail_out;
        state.lineBuffer.append((char*)state.outBuffer, have);

        if (ret == Z_STREAM_END)
        {
            state.finished = true; // End of compressed stream
            // Process remaining buffer
            while ((pos = state.lineBuffer.find('\n')) != std::string::npos)
            {
                std::string line = state.lineBuffer.substr(0, pos);
                state.lineBuffer.erase(0, pos + 1);
                if (!line.empty())
                {
                    ParseLine(traceFile, line);
                }
            }
            if (!state.lineBuffer.empty())
            {
                ParseLine(traceFile, state.lineBuffer);
                state.lineBuffer.clear();
            }
            break;
        }
    }
}

void
TraceReader::ParseLine(const std::string& traceFile, const std::string& line)
{
    std::stringstream ss(line);
    std::string segment;
    std::vector<std::string> parts;

    while (std::getline(ss, segment, ';'))
    {
        parts.push_back(segment);
    }

    if (parts.size() < 5)
    {
        return;
    }

    // Format: node;relative_time;latitude;longitude;altitude
    std::string nodeId = parts[0];

    TraceFileState& state = m_traceFiles[traceFile];

    // Check if we care about this node in this file
    if (state.registeredDevices.find(nodeId) == state.registeredDevices.end())
    {
        return;
    }

    double relTimeMs = std::stod(parts[1]);
    double lat = std::stod(parts[2]);
    double lon = std::stod(parts[3]);
    double alt = std::stod(parts[4]);

    TracePoint p;
    p.nodeId = nodeId;
    p.time = MilliSeconds(relTimeMs);
    p.position = Vector(lat, lon, alt); // Storing geo coords in Vector for now

    state.buffers[nodeId].push_back(p);

    if (p.time > state.lastReadTime)
    {
        state.lastReadTime = p.time;
    }
}

bool
TraceReader::GetNextPoint(const std::string& traceFile,
                          const std::string& deviceId,
                          Time currentTime,
                          Vector& position)
{
    auto it = m_traceFiles.find(traceFile);
    if (it == m_traceFiles.end())
    {
        NS_LOG_WARN("Trace file " << traceFile << " not opened");
        return false;
    }

    TraceFileState& state = it->second;

    // Ensure we have enough data
    ReadUntil(traceFile, deviceId, currentTime);

    auto& buffer = state.buffers[deviceId];

    if (buffer.empty())
    {
        return false;
    }

    // Remove old points, but keep at least one before currentTime if possible
    while (buffer.size() >= 2 && buffer[1].time <= currentTime)
    {
        buffer.pop_front();
    }

    if (buffer.empty())
        return false;

    TracePoint& p1 = buffer.front();

    if (buffer.size() == 1)
    {
        // Only one point available
        position = p1.position;
        return true;
    }

    TracePoint& p2 = buffer[1];

    // If we are before the first point
    if (currentTime < p1.time)
    {
        position = p1.position;
        return true;
    }

    // If we are exactly at p2 or beyond (shouldn't happen if ReadUntil works and we popped
    // correctly, unless end of trace)
    if (currentTime >= p2.time)
    {
        position = p2.position;
        return true;
    }

    // Interpolate
    double t1 = p1.time.GetSeconds();
    double t2 = p2.time.GetSeconds();
    double t = currentTime.GetSeconds();

    double alpha = (t - t1) / (t2 - t1);

    position.x = p1.position.x + alpha * (p2.position.x - p1.position.x);
    position.y = p1.position.y + alpha * (p2.position.y - p1.position.y);
    position.z = p1.position.z + alpha * (p2.position.z - p1.position.z);

    return true;
}

} // namespace ns3
