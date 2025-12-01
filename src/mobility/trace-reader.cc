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
        if (pair.second.tar_handle)
        {
            tar_close(pair.second.tar_handle);
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

    TAR* t;
    if (tar_open(&t, const_cast<char*>(traceFile.c_str()), nullptr, O_RDONLY, 0, 0) == -1)
    {
        NS_FATAL_ERROR("Failed to open trace file: " << traceFile);
    }

    TraceFileState state;
    state.tar_handle = t;
    state.finished = false;
    state.lastReadTime = Seconds(0);
    state.fileFound = false;
    state.fileRemainingBytes = 0;

    // Don't initialize zlib yet - we'll do it after finding traces.csv.gz
    // to avoid having an idle stream that needs resetting

    // Find and read nodes.csv.gz first
    bool nodesFound = false;
    while (th_read(t) == 0)
    {
        std::string filename = th_get_pathname(t);
        if (filename.find("nodes.csv.gz") != std::string::npos)
        {
            nodesFound = true;
            unsigned long nodesFileSize = th_get_size(t);
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
            unsigned long nodesRemaining = nodesFileSize;
            unsigned char nodesInBuf[16384];
            unsigned char nodesOutBuf[16384];

            while (nodesRemaining > 0)
            {
                char tar_block[T_BLOCKSIZE];
                if (tar_block_read(t, tar_block) == -1)
                {
                    NS_FATAL_ERROR("Error reading tar block for nodes.csv.gz");
                }

                size_t validBytes = std::min((size_t)T_BLOCKSIZE, (size_t)nodesRemaining);
                memcpy(nodesInBuf, tar_block, validBytes);
                nodesStrm.avail_in = validBytes;
                nodesStrm.next_in = (Bytef*)nodesInBuf;
                nodesRemaining -= validBytes;

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

                if (ret == Z_STREAM_END)
                {
                    break;
                }
            }

            inflateEnd(&nodesStrm);

            // Parse nodes.csv.gz content
            std::stringstream ss(nodesBuffer);
            std::string line;
            while (std::getline(ss, line))
            {
                if (line.empty())
                {
                    continue;
                }

                std::stringstream lineStream(line);
                std::string segment;
                std::vector<std::string> parts;

                while (std::getline(lineStream, segment, ';'))
                {
                    parts.push_back(segment);
                }

                if (parts.size() < 5)
                {
                    continue;
                }

                // Format: node;first_relative_time;latitude;longitude;altitude
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
            break;
        }

        // Skip this file
        size_t size = th_get_size(t);
        size_t blocks = (size + T_BLOCKSIZE - 1) / T_BLOCKSIZE;
        for (size_t i = 0; i < blocks; ++i)
        {
            char buf[T_BLOCKSIZE];
            if (tar_block_read(t, buf) == -1)
            {
                NS_FATAL_ERROR("Error reading tar block while skipping");
            }
        }
    }

    if (!nodesFound)
    {
        NS_FATAL_ERROR("nodes.csv.gz not found in " << traceFile);
    }

    // Now find traces.csv.gz
    while (th_read(t) == 0)
    {
        std::string filename = th_get_pathname(t);
        if (filename.find("traces.csv.gz") != std::string::npos)
        {
            state.fileFound = true;
            state.fileRemainingBytes = th_get_size(t);
            NS_LOG_INFO("Found traces.csv.gz in " << traceFile
                                                  << ", size: " << state.fileRemainingBytes);
            break;
        }

        // Skip this file
        size_t size = th_get_size(t);
        size_t blocks = (size + T_BLOCKSIZE - 1) / T_BLOCKSIZE;
        for (size_t i = 0; i < blocks; ++i)
        {
            char buf[T_BLOCKSIZE];
            if (tar_block_read(t, buf) == -1)
            {
                NS_FATAL_ERROR("Error reading tar block while skipping");
            }
        }
    }

    if (!state.fileFound)
    {
        NS_FATAL_ERROR("traces.csv.gz not found in " << traceFile);
    }

    // First, insert the state into the map
    m_traceFiles[traceFile] = state;

    // NOW initialize zlib for traces.csv.gz decompression
    // IMPORTANT: Initialize AFTER inserting into map because z_stream
    // contains internal pointers that must not be copied after initialization
    TraceFileState& finalState = m_traceFiles[traceFile];
    finalState.strm.zalloc = Z_NULL;
    finalState.strm.zfree = Z_NULL;
    finalState.strm.opaque = Z_NULL;
    finalState.strm.avail_in = 0;
    finalState.strm.next_in = Z_NULL;
    if (inflateInit2(&finalState.strm, 16 + MAX_WBITS) != Z_OK) // 16+MAX_WBITS for gzip
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

            // Read from tar
            // We must read in 512 byte blocks for tar.
            // tar_block_read reads T_BLOCKSIZE (512) bytes.

            NS_LOG_DEBUG("Reading tar block, fileRemainingBytes=" << state.fileRemainingBytes);

            char tar_block_buf[T_BLOCKSIZE];
            if (tar_block_read(state.tar_handle, tar_block_buf) == -1)
            {
                NS_LOG_ERROR("Error reading tar block for " << traceFile);
                state.finished = true;
                break;
            }

            // The file size might not be a multiple of 512.
            // The last block is padded.
            size_t validBytesInBlock =
                std::min((size_t)T_BLOCKSIZE, (size_t)state.fileRemainingBytes);

            NS_LOG_DEBUG("Read tar block, validBytesInBlock=" << validBytesInBlock);

            // Copy valid bytes from the tar block into zlib's input buffer
            memcpy(state.inBuffer, tar_block_buf, validBytesInBlock);
            state.strm.avail_in = validBytesInBlock;
            state.strm.next_in = (Bytef*)state.inBuffer;

            state.fileRemainingBytes -= validBytesInBlock;
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
