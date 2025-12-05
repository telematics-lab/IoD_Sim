/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDJSON_POINTER_H_
#define RAPIDJSON_POINTER_H_

#include "rapidjson.h"
#include "document.h"
#include <string>
#include <vector>

namespace rapidjson {

class Pointer {
public:
    Pointer(const char* path) : m_path(path) {}

    // Returns Value by value.
    // If path is invalid or not found, returns Null Value.
    // Usage in legacy code: `const Value* v = ptr.Get(doc)`
    // We will update legacy code.
    Value* Get(Value& root) const {
       Value* current = &root;
       if (m_path.empty()) return current;
       if (m_path == "/") return current;

       if (m_path[0] != '/') return nullptr;

       size_t pos = 1;
       while (pos < m_path.size()) {
           size_t nextSlash = m_path.find('/', pos);
           std::string token;
           if (nextSlash == std::string::npos) {
               token = m_path.substr(pos);
               pos = m_path.size();
           } else {
               token = m_path.substr(pos, nextSlash - pos);
               pos = nextSlash + 1;
           }

           std::string key;
           key.reserve(token.size());
           for (size_t i=0; i<token.size(); ++i) {
               if (token[i] == '~') {
                   if (i+1 < token.size()) {
                       if (token[i+1] == '0') key += '~';
                       else if (token[i+1] == '1') key += '/';
                       i++;
                   } else {
                       key += '~';
                   }
               } else {
                   key += token[i];
               }
           }

           if (current->IsArray()) {
               if (key == "-") return nullptr;
               if (key.size() > 1 && key[0] == '0') return nullptr;
               char* end;
               long idx = std::strtol(key.c_str(), &end, 10);
               if (end != key.c_str() + key.size()) return nullptr;
               if (idx < 0 || (size_t)idx >= current->Size()) return nullptr;
               current = &((*current)[(SizeType)idx]);
           } else if (current->IsObject()) {
               if (!current->HasMember(key.c_str())) return nullptr;
               current = &((*current)[key.c_str()]);
           } else {
               return nullptr;
           }
       }
       return current;
    }

    const Value* Get(const Value& root) const {
        return const_cast<Pointer*>(this)->Get(const_cast<Value&>(root));
    }

private:
    std::string m_path;
};

} // namespace rapidjson

#endif // RAPIDJSON_POINTER_H_
