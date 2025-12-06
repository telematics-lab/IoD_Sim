/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDJSON_WRITER_H_
#define RAPIDJSON_WRITER_H_

#include "rapidjson.h"
#include <yyjson.h>
#include <vector>

namespace rapidjson {

template<typename OutputStream>
class Writer {
public:
    Writer(OutputStream& os) : m_os(os), m_doc(yyjson_mut_doc_new(nullptr)), m_current_key(nullptr) {}
    ~Writer() { if (m_doc) yyjson_mut_doc_free(m_doc); }

    bool StartObject() {
        yyjson_mut_val* val = yyjson_mut_obj(m_doc);
        AddToParent(val);
        m_stack.push_back(val);
        return true;
    }
    bool EndObject(SizeType memberCount = 0) {
        if (m_stack.empty()) return false;
        m_stack.pop_back();
        CheckRoot();
        return true;
    }
    bool StartArray() {
        yyjson_mut_val* val = yyjson_mut_arr(m_doc);
        AddToParent(val);
        m_stack.push_back(val);
        return true;
    }
    bool EndArray(SizeType elementCount = 0) {
        if (m_stack.empty()) return false;
        m_stack.pop_back();
        CheckRoot();
        return true;
    }

    bool Key(const char* str) {
        m_current_key = yyjson_mut_str(m_doc, str);
        return true;
    }
    bool Key(const char* str, SizeType len, bool copy = false) {
        m_current_key = yyjson_mut_strn(m_doc, str, len);
        return true;
    }
    bool String(const char* str) {
        AddToParent(yyjson_mut_str(m_doc, str));
        return true;
    }
    bool String(const char* str, SizeType len, bool copy = false) {
        AddToParent(yyjson_mut_strn(m_doc, str, len));
        return true;
    }

    bool Int(int i) { AddToParent(yyjson_mut_int(m_doc, i)); return true; }
    bool Uint(unsigned u) { AddToParent(yyjson_mut_uint(m_doc, u)); return true; }
    bool Int64(int64_t i) { AddToParent(yyjson_mut_sint(m_doc, i)); return true; }
    bool Uint64(uint64_t u) { AddToParent(yyjson_mut_uint(m_doc, u)); return true; }
    bool Double(double d) { AddToParent(yyjson_mut_real(m_doc, d)); return true; }
    bool Bool(bool b) { AddToParent(yyjson_mut_bool(m_doc, b)); return true; }
    bool Null() { AddToParent(yyjson_mut_null(m_doc)); return true; }

private:
   void AddToParent(yyjson_mut_val* val) {
       if (m_stack.empty()) {
           // Root
           yyjson_mut_doc_set_root(m_doc, val);
           // Only check root if it's a primitive. Containers will be pushed to stack.
           if (!yyjson_mut_is_obj(val) && !yyjson_mut_is_arr(val)) {
                CheckRoot();
           }
           return;
       }
       yyjson_mut_val* parent = m_stack.back();
       if (yyjson_mut_is_arr(parent)) {
           yyjson_mut_arr_append(parent, val);
       } else if (yyjson_mut_is_obj(parent)) {
           if (m_current_key) {
               yyjson_mut_obj_add(parent, m_current_key, val);
               m_current_key = nullptr;
           }
       }
   }

   void CheckRoot() {
       if (m_stack.empty()) {
           // Root is complete, serialize (0 = compact)
           char* json = yyjson_mut_write(m_doc, 0, nullptr);
           if (json) {
               m_os.Append(json);
               free(json);
           }
       }
   }

    OutputStream& m_os;
    yyjson_mut_doc* m_doc;
    std::vector<yyjson_mut_val*> m_stack;
    yyjson_mut_val* m_current_key;
};
} // namespace rapidjson

#endif // RAPIDJSON_WRITER_H_
