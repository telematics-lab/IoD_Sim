/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDYYJSON_FILEREADSTREAM_H_
#define RAPIDYYJSON_FILEREADSTREAM_H_

#include <cstdio>
#include <cstddef>

namespace rapidyyjson {

class FileReadStream {
public:
    FileReadStream(std::FILE* fp, char* buffer, size_t bufferSize) : m_fp(fp) {}
    std::FILE* GetFP() const { return m_fp; }
private:
    std::FILE* m_fp;
};

} // namespace rapidyyjson

#endif // RAPIDYYJSON_FILEREADSTREAM_H_
