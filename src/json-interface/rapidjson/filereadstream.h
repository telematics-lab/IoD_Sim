/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDJSON_FILEREADSTREAM_H_
#define RAPIDJSON_FILEREADSTREAM_H_

#include <cstdio>
#include <cstddef>

namespace rapidjson {

class FileReadStream {
public:
    FileReadStream(std::FILE* fp, char* buffer, size_t bufferSize) : m_fp(fp) {}
    std::FILE* GetFP() const { return m_fp; }
private:
    std::FILE* m_fp;
};

} // namespace rapidjson

#endif // RAPIDJSON_FILEREADSTREAM_H_
