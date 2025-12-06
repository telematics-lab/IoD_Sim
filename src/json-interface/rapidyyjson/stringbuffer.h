/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDYYJSON_STRINGBUFFER_H_
#define RAPIDYYJSON_STRINGBUFFER_H_

#include <string>

namespace rapidyyjson {

class StringBuffer {
public:
    typedef char Ch;

    StringBuffer() {}

    const char* GetString() const { return m_str.c_str(); }
    size_t GetSize() const { return m_str.size(); }
    void Clear() { m_str.clear(); }

    void Put(Ch c) { m_str += c; }
    void Flush() {}

    char* Push(size_t count) {
        size_t oldSize = m_str.size();
        m_str.resize(oldSize + count);
        return &m_str[oldSize];
    }
    void Pop(size_t count) {
        if (count <= m_str.size()) {
            m_str.resize(m_str.size() - count);
        }
    }

    // Internal helper for writer
    void Append(const char* s) { m_str += s; }
    void Append(const char* s, size_t len) { m_str.append(s, len); }

private:
    std::string m_str;
};

} // namespace rapidyyjson

#endif // RAPIDYYJSON_STRINGBUFFER_H_
