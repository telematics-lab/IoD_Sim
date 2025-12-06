/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDJSON_DOCUMENT_H_
#define RAPIDJSON_DOCUMENT_H_

#include "rapidjson.h"
#include <yyjson.h>
#include <string>
#include <vector>
#include <variant>
#include <cstring>
#include <iostream>
#include <memory>
#include <algorithm>
#include <map>
#include <optional>
#include <limits>



namespace rapidjson {

// Forward declarations
struct GenericMember;
class GenericMemberIterator;
class GenericArrayIterator;
class GenericArrayIterator;
class Value;
class Document;

struct ChildrenCache {
    std::map<std::string, std::unique_ptr<Value>> stringMap;
    std::map<size_t, std::unique_ptr<Value>> intMap;
};

// Allocator wrapper
class Allocator {
public:
    Allocator(yyjson_mut_doc* doc = nullptr) : m_doc(doc) {}
    yyjson_mut_doc* GetDoc() const { return m_doc; }
    void SetDoc(yyjson_mut_doc* doc) { m_doc = doc; }
private:
    yyjson_mut_doc* m_doc;
};

// Generic type for GetAllocator logic
typedef Allocator DocumentAllocator;

// GenericArray wrapper declaration
template <bool Const, typename ValueT>
class GenericArray {
public:
    typedef GenericArrayIterator ValueIterator;
    typedef GenericArrayIterator iterator;

    GenericArray(const ValueT& value) : m_value(const_cast<ValueT&>(value)) {}

    template <bool SourceConst>
    GenericArray(const GenericArray<SourceConst, ValueT>& source) : m_value((ValueT&)source) {}

    operator ValueT&() const { return m_value; }

    ValueIterator Begin() const;
    ValueIterator End() const;
    ValueIterator begin() const;
    ValueIterator end() const;

    SizeType Size() const;
    const ValueT& operator[](SizeType index) const;

private:
    ValueT& m_value;
};

// GenericObject wrapper declaration
template <bool Const, typename ValueT>
class GenericObject {
public:
    typedef GenericMemberIterator MemberIterator;
    typedef GenericMemberIterator iterator;

    GenericObject(const ValueT& value) : m_value(const_cast<ValueT&>(value)) {}

    template <bool SourceConst>
    GenericObject(const GenericObject<SourceConst, ValueT>& source) : m_value((ValueT&)source) {}

    operator ValueT&() const { return m_value; }

    MemberIterator MemberBegin() const;
    MemberIterator MemberEnd() const;
    MemberIterator begin() const;
    MemberIterator end() const;

    SizeType MemberCount() const;
    bool HasMember(const char* name) const;
    const ValueT& operator[](const char* name) const;

private:
    ValueT& m_value;
};

// Value class
class Value {
public:
    typedef GenericMember Member;
    typedef GenericMemberIterator MemberIterator;
    typedef GenericMemberIterator ConstMemberIterator;
    typedef GenericArrayIterator ValueIterator;
    typedef GenericArrayIterator ConstValueIterator;
    typedef rapidjson::Allocator AllocatorType;

    typedef GenericArray<true, Value> ConstArray;
    typedef GenericObject<true, Value> ConstObject;

    Value() : m_type(kNullType), m_val(nullptr) {}
    Value(Type type) : m_type(type), m_val(nullptr) {
        if (type == kObjectType) m_detached = ObjectData{};
        else if (type == kArrayType) m_detached = ArrayData{};
        else if (type == kStringType) m_detached = std::string{};
        else if (type == kTrueType) m_detached = true;
        else if (type == kFalseType) m_detached = false;
    }

    Value(bool b) : m_type(b ? kTrueType : kFalseType), m_val(nullptr), m_detached(b) {}
    Value(int i) : m_type(kNumberType), m_val(nullptr), m_detached((int64_t)i) {}
    Value(unsigned u) : m_type(kNumberType), m_val(nullptr), m_detached((int64_t)u) {}
    Value(int64_t i) : m_type(kNumberType), m_val(nullptr), m_detached(i) {}
    Value(uint64_t u) : m_type(kNumberType), m_val(nullptr), m_detached(u) {}
    Value(double d) : m_type(kNumberType), m_val(nullptr), m_detached(d) {}
    Value(const char* s) : m_type(kStringType), m_val(nullptr), m_detached(std::string(s)) {}
    Value(const std::string& s) : m_type(kStringType), m_val(nullptr), m_detached(s) {}
    Value(const char* s, Allocator&) : m_type(kStringType), m_val(nullptr), m_detached(std::string(s)) {}

    Value(yyjson_mut_val* val) : m_val(val) {
        UpdateTypeFromVal();
    }
    Value(const Value& rhs, Allocator& allocator);
    Value(const Value& rhs) : m_type(rhs.m_type), m_val(rhs.m_val), m_detached(rhs.m_detached) {
        m_children.reset();
    }

    Value& operator=(const Value& rhs) {
        if (this != &rhs) {
            m_type = rhs.m_type;
            m_val = rhs.m_val;
            m_detached = rhs.m_detached;
            m_children.reset(); // Invalidate cache
        }
        return *this;
    }

    void Attach(yyjson_mut_val* val) { m_val = val; UpdateTypeFromVal(); }

    Type GetType() const { return m_type; }
    bool IsNull() const { return m_type == kNullType; }
    bool IsFalse() const { return m_type == kFalseType; }
    bool IsTrue() const { return m_type == kTrueType; }
    bool IsBool() const { return m_type == kFalseType || m_type == kTrueType; }
    bool IsObject() const { return m_type == kObjectType; }
    bool IsArray() const { return m_type == kArrayType; }
    bool IsNumber() const { return m_type == kNumberType; }
    bool IsString() const { return m_type == kStringType; }

    bool IsInt() const {
        if (!IsNumber()) return false;
        if (m_val) {
            if (yyjson_mut_is_int(m_val)) {
                int64_t v = yyjson_mut_get_sint(m_val);
                return v >= std::numeric_limits<int32_t>::min() && v <= std::numeric_limits<int32_t>::max();
            }
            if (yyjson_mut_is_uint(m_val)) {
                uint64_t v = yyjson_mut_get_uint(m_val);
                return v <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
            }
            if (yyjson_mut_is_real(m_val)) {
                double d = yyjson_mut_get_real(m_val);
                return d >= static_cast<double>(std::numeric_limits<int32_t>::min()) &&
                       d <= static_cast<double>(std::numeric_limits<int32_t>::max()) &&
                       d == static_cast<int32_t>(d);
            }
            return false;
        }
        if (std::holds_alternative<int64_t>(m_detached)) {
             int64_t v = std::get<int64_t>(m_detached);
             return v >= std::numeric_limits<int32_t>::min() && v <= std::numeric_limits<int32_t>::max();
        }
        if (std::holds_alternative<uint64_t>(m_detached)) {
             uint64_t v = std::get<uint64_t>(m_detached);
             return v <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
        }
        if (std::holds_alternative<double>(m_detached)) {
             double d = std::get<double>(m_detached);
             return d >= static_cast<double>(std::numeric_limits<int32_t>::min()) &&
                    d <= static_cast<double>(std::numeric_limits<int32_t>::max()) &&
                    d == static_cast<int32_t>(d);
        }
        return false;
    }

    bool IsUint() const {
        if (!IsNumber()) return false;
        if (m_val) {
             if (yyjson_mut_is_uint(m_val)) {
                 uint64_t v = yyjson_mut_get_uint(m_val);
                 return v <= std::numeric_limits<uint32_t>::max();
             }
             if (yyjson_mut_is_int(m_val)) {
                 int64_t v = yyjson_mut_get_sint(m_val);
                 return v >= 0 && v <= static_cast<int64_t>(std::numeric_limits<uint32_t>::max());
             }
             if (yyjson_mut_is_real(m_val)) {
                double d = yyjson_mut_get_real(m_val);
                return d >= 0 &&
                       d <= static_cast<double>(std::numeric_limits<uint32_t>::max()) &&
                       d == static_cast<uint32_t>(d);
             }
             return false;
        }
        if (std::holds_alternative<int64_t>(m_detached)) {
             int64_t v = std::get<int64_t>(m_detached);
             return v >= 0 && v <= static_cast<int64_t>(std::numeric_limits<uint32_t>::max());
        }
        if (std::holds_alternative<uint64_t>(m_detached)) {
             uint64_t v = std::get<uint64_t>(m_detached);
             return v <= static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
        }
        if (std::holds_alternative<double>(m_detached)) {
            double d = std::get<double>(m_detached);
            return d >= 0 &&
                   d <= static_cast<double>(std::numeric_limits<uint32_t>::max()) &&
                   d == static_cast<uint32_t>(d);
        }
        return false;
    }

    bool IsDouble() const { return IsNumber(); }

    bool IsInt64() const {
        if (!IsNumber()) return false;
        if (m_val) {
            if (yyjson_mut_is_int(m_val)) return true;
            if (yyjson_mut_is_uint(m_val)) {
                 uint64_t v = yyjson_mut_get_uint(m_val);
                 return v <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
            }
            if (yyjson_mut_is_real(m_val)) {
                double d = yyjson_mut_get_real(m_val);
                return d >= static_cast<double>(std::numeric_limits<int64_t>::min()) &&
                       d <= static_cast<double>(std::numeric_limits<int64_t>::max()) &&
                       d == static_cast<int64_t>(d);
            }
            return false;
        }
        if (std::holds_alternative<int64_t>(m_detached)) return true;
        if (std::holds_alternative<uint64_t>(m_detached)) {
            uint64_t v = std::get<uint64_t>(m_detached);
            return v <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
        }
        if (std::holds_alternative<double>(m_detached)) {
             double d = std::get<double>(m_detached);
             return d >= static_cast<double>(std::numeric_limits<int64_t>::min()) &&
                    d <= static_cast<double>(std::numeric_limits<int64_t>::max()) &&
                    d == static_cast<int64_t>(d);
        }
        return false;
    }

    bool IsUint64() const {
        if (!IsNumber()) return false;
        if (m_val) {
            if (yyjson_mut_is_uint(m_val)) return true;
            if (yyjson_mut_is_int(m_val)) {
                return yyjson_mut_get_sint(m_val) >= 0;
            }
            if (yyjson_mut_is_real(m_val)) {
                double d = yyjson_mut_get_real(m_val);
                return d >= 0 &&
                       d <= static_cast<double>(std::numeric_limits<uint64_t>::max()) &&
                       d == static_cast<uint64_t>(d);
            }
            return false;
        }
         if (std::holds_alternative<int64_t>(m_detached)) {
             return std::get<int64_t>(m_detached) >= 0;
         }
         if (std::holds_alternative<uint64_t>(m_detached)) return true;
        if (std::holds_alternative<double>(m_detached)) {
            double d = std::get<double>(m_detached);
            return d >= 0 &&
                   d <= static_cast<double>(std::numeric_limits<uint64_t>::max()) &&
                   d == static_cast<uint64_t>(d);
        }
         return false;
    }

    bool IsFloat() const { return IsDouble(); }

    bool GetBool() const {
        if (m_val) return yyjson_mut_get_bool(m_val);
        if (std::holds_alternative<bool>(m_detached)) return std::get<bool>(m_detached);
        return false;
    }
    int GetInt() const {
        if (m_val) {
            if (yyjson_mut_is_int(m_val)) return (int)yyjson_mut_get_sint(m_val);
            if (yyjson_mut_is_uint(m_val)) return (int)yyjson_mut_get_uint(m_val);
            if (yyjson_mut_is_real(m_val)) return (int)yyjson_mut_get_real(m_val);
            return 0;
        }
        if (std::holds_alternative<int64_t>(m_detached)) return (int)std::get<int64_t>(m_detached);
        if (std::holds_alternative<uint64_t>(m_detached)) return (int)std::get<uint64_t>(m_detached);
        if (std::holds_alternative<double>(m_detached)) return (int)std::get<double>(m_detached);
        return 0;
    }
    unsigned GetUint() const {
        if (m_val) {
            if (yyjson_mut_is_uint(m_val)) return (unsigned)yyjson_mut_get_uint(m_val);
            if (yyjson_mut_is_int(m_val)) return (unsigned)yyjson_mut_get_sint(m_val);
            if (yyjson_mut_is_real(m_val)) return (unsigned)yyjson_mut_get_real(m_val);
            return 0;
        }
        if (std::holds_alternative<int64_t>(m_detached)) return (unsigned)std::get<int64_t>(m_detached);
        if (std::holds_alternative<uint64_t>(m_detached)) return (unsigned)std::get<uint64_t>(m_detached);
        if (std::holds_alternative<double>(m_detached)) return (unsigned)std::get<double>(m_detached);
        return 0;
    }
    int64_t GetInt64() const {
       if (m_val) {
            if (yyjson_mut_is_int(m_val)) return yyjson_mut_get_sint(m_val);
            if (yyjson_mut_is_uint(m_val)) return (int64_t)yyjson_mut_get_uint(m_val);
            if (yyjson_mut_is_real(m_val)) return (int64_t)yyjson_mut_get_real(m_val);
            return 0;
       }
       if (std::holds_alternative<int64_t>(m_detached)) return std::get<int64_t>(m_detached);
       if (std::holds_alternative<uint64_t>(m_detached)) return (int64_t)std::get<uint64_t>(m_detached);
       if (std::holds_alternative<double>(m_detached)) return (int64_t)std::get<double>(m_detached);
       return 0;
    }
    uint64_t GetUint64() const {
        if (m_val) {
            if (yyjson_mut_is_uint(m_val)) return yyjson_mut_get_uint(m_val);
            if (yyjson_mut_is_int(m_val)) return (uint64_t)yyjson_mut_get_sint(m_val);
            if (yyjson_mut_is_real(m_val)) return (uint64_t)yyjson_mut_get_real(m_val);
            return 0;
        }
        if (std::holds_alternative<int64_t>(m_detached)) return (uint64_t)std::get<int64_t>(m_detached);
        if (std::holds_alternative<uint64_t>(m_detached)) return std::get<uint64_t>(m_detached);
        if (std::holds_alternative<double>(m_detached)) return (uint64_t)std::get<double>(m_detached);
        return 0;
    }
    double GetDouble() const {
        if (m_val) {
            if (yyjson_mut_is_real(m_val)) return yyjson_mut_get_real(m_val);
            if (yyjson_mut_is_int(m_val)) return (double)yyjson_mut_get_sint(m_val);
            if (yyjson_mut_is_uint(m_val)) return (double)yyjson_mut_get_uint(m_val);
            return 0.0;
        }
        if (std::holds_alternative<double>(m_detached)) return std::get<double>(m_detached);
        if (std::holds_alternative<int64_t>(m_detached)) return (double)std::get<int64_t>(m_detached);
        if (std::holds_alternative<uint64_t>(m_detached)) return (double)std::get<uint64_t>(m_detached);
        return 0.0;
    }
    float GetFloat() const { return (float)GetDouble(); }

    const char* GetString() const {
        if (m_val) return yyjson_mut_get_str(m_val);
        if (std::holds_alternative<std::string>(m_detached)) return std::get<std::string>(m_detached).c_str();
        return "";
    }
    std::size_t GetStringLength() const {
        if (m_val) return yyjson_mut_get_len(m_val);
        if (std::holds_alternative<std::string>(m_detached)) return std::get<std::string>(m_detached).size();
        return 0;
    }

    void SetObject() {
        m_type = kObjectType;
        if (!m_val) m_detached = ObjectData{};
    }

    void SetArray() {
        m_type = kArrayType;
        if (!m_val) m_detached = ArrayData{};
    }

    void SetString(const char* s, Allocator& allocator) {
        m_type = kStringType;
        if (m_val) yyjson_mut_set_str(m_val, s);
        else m_detached = std::string(s);
    }
    void SetString(const char* s, SizeType len, Allocator& allocator) {
        m_type = kStringType;
        if (m_val) yyjson_mut_set_strn(m_val, s, len);
        else m_detached = std::string(s, len);
    }

    ConstArray GetArray() const { return ConstArray(*this); }
    GenericArray<false, Value> GetArray() { return GenericArray<false, Value>(*this); }

    ConstObject GetObject() const { return ConstObject(*this); }
    GenericObject<false, Value> GetObject() { return GenericObject<false, Value>(*this); }

    bool HasMember(const char* name) const;
    void RemoveMember(const char* name);

    const Value& operator[](const char* name) const;
    Value& operator[](const char* name);

    const Value& operator[](int index) const { return (*this)[(SizeType)index]; }
    Value& operator[](int index) { return (*this)[(SizeType)index]; }

    const Value& operator[](SizeType index) const;
    Value& operator[](SizeType index);

    void AddMember(Value& name, Value& value, Allocator& allocator);
    void AddMember(Value& name, Value&& value, Allocator& allocator);
    void AddMember(const char* name, Value& value, Allocator& allocator);
    void AddMember(const char* name, Value&& value, Allocator& allocator);

    Value& CopyFrom(const Value& rhs, Allocator& allocator) {
        *this = Value(rhs, allocator);
        return *this;
    }

    SizeType Size() const;
    SizeType MemberCount() const;

    void PushBack(Value& value, Allocator& allocator);

    GenericMemberIterator MemberBegin();
    GenericMemberIterator MemberEnd();
    ConstMemberIterator MemberBegin() const;
    ConstMemberIterator MemberEnd() const;

    GenericArrayIterator Begin();
    GenericArrayIterator End();
    ConstValueIterator Begin() const;
    ConstValueIterator End() const;

    GenericArrayIterator begin();
    GenericArrayIterator end();
    ConstValueIterator begin() const;
    ConstValueIterator end() const;

    yyjson_mut_val* GetYYVal() const { return m_val; }
    yyjson_mut_val* ToYYVal(yyjson_mut_doc* doc);

    template <typename Handler>
    bool Accept(Handler& handler) const;

    using ObjectData = std::vector<std::pair<std::string, Value>>;
    using ArrayData = std::vector<Value>;
    using DetachedData = std::variant<std::monostate, bool, int64_t, uint64_t, double, std::string, ObjectData, ArrayData>;

protected:
    void UpdateTypeFromVal();
    Type m_type;
    yyjson_mut_val* m_val;
    DetachedData m_detached;
    mutable std::shared_ptr<ChildrenCache> m_children;
    friend struct ChildrenCache;
};

// Member struct
struct GenericMember {
    Value name;
    Value value;
};

// Iterators
class GenericMemberIterator {
    yyjson_mut_obj_iter m_iter; // For attached
    Value::ObjectData* m_detachedObj; // For detached
    size_t m_detachedIdx;

    GenericMember m_cache;
    bool m_has_next;
    bool m_is_valid;
public:
    GenericMemberIterator(yyjson_mut_val* obj) : m_detachedObj(nullptr), m_detachedIdx(0) {
        if (obj && yyjson_mut_is_obj(obj)) {
             yyjson_mut_obj_iter_init(obj, &m_iter);
             m_is_valid = true;
             Advance();
        } else {
            m_has_next = false;
            m_is_valid = false;
        }
    }
    GenericMemberIterator(Value::ObjectData* obj) : m_detachedObj(obj), m_detachedIdx(0) {
        if (obj) {
            m_is_valid = true;
            Advance();
        } else {
            m_has_next = false;
            m_is_valid = false;
        }
    }
    GenericMemberIterator() : m_detachedObj(nullptr), m_detachedIdx(0), m_has_next(false), m_is_valid(false) {}

    void Advance() {
        if (!m_is_valid) { m_has_next = false; return; }

        if (m_detachedObj) {
            if (m_detachedIdx < m_detachedObj->size()) {
                 auto& p = (*m_detachedObj)[m_detachedIdx];
                 // Construct m_cache from pair
                 // p.first is string. p.second is Value.
                 // We need Value for name. Value(const char*) creates String Value.
                 m_cache.name = Value(p.first.c_str());
                 m_cache.value = p.second;
                 // This effectively COPIES the value into cache.
                 // But iterating typically yields references?
                 // RapidJSON GenericMemberIterator derefs into GenericMember which has Values.

                 m_detachedIdx++;
                 m_has_next = true;
            } else {
                m_has_next = false;
            }
            return;
        }

        yyjson_mut_val* key = yyjson_mut_obj_iter_next(&m_iter);
        if (key) {
            m_cache.name = Value(key);
            m_cache.value = Value(yyjson_mut_obj_iter_get_val(key));
            m_has_next = true;
        } else {
            m_has_next = false;
        }
    }
    bool operator!=(const GenericMemberIterator& rhs) const { return m_has_next != rhs.m_has_next; }
    bool operator==(const GenericMemberIterator& rhs) const { return m_has_next == rhs.m_has_next; }
    GenericMemberIterator& operator++() { Advance(); return *this; }
    GenericMemberIterator operator++(int) { GenericMemberIterator tmp = *this; Advance(); return tmp; }
    GenericMember* operator->() { return &m_cache; }
    GenericMember& operator*() { return m_cache; }
};

class GenericArrayIterator {
    Value* m_parent;
    size_t m_idx;
public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef Value value_type;
    typedef std::ptrdiff_t difference_type;
    typedef Value* pointer;
    typedef Value& reference;

    GenericArrayIterator(Value* parent, size_t idx = 0) : m_parent(parent), m_idx(idx) {}
    GenericArrayIterator() : m_parent(nullptr), m_idx(0) {}

    Value& operator*();
    Value* operator->();
    Value operator[](difference_type n) const;

    GenericArrayIterator& operator++() { m_idx++; return *this; }
    GenericArrayIterator operator++(int) { GenericArrayIterator tmp = *this; m_idx++; return tmp; }
    GenericArrayIterator& operator--() { m_idx--; return *this; }
    GenericArrayIterator operator--(int) { GenericArrayIterator tmp = *this; m_idx--; return tmp; }
    GenericArrayIterator operator+(difference_type n) const { return GenericArrayIterator(m_parent, m_idx + n); }
    GenericArrayIterator operator-(difference_type n) const { return GenericArrayIterator(m_parent, m_idx - n); }
    difference_type operator-(const GenericArrayIterator& other) const { return (difference_type)m_idx - (difference_type)other.m_idx; }
    GenericArrayIterator& operator+=(difference_type n) { m_idx += n; return *this; }
    GenericArrayIterator& operator-=(difference_type n) { m_idx -= n; return *this; }

    bool operator==(const GenericArrayIterator& rhs) const { return m_parent == rhs.m_parent && m_idx == rhs.m_idx; }
    bool operator!=(const GenericArrayIterator& rhs) const { return !(*this == rhs); }
    bool operator<(const GenericArrayIterator& rhs) const { return m_idx < rhs.m_idx; }
    bool operator>(const GenericArrayIterator& rhs) const { return m_idx > rhs.m_idx; }
    bool operator<=(const GenericArrayIterator& rhs) const { return m_idx <= rhs.m_idx; }
    bool operator>=(const GenericArrayIterator& rhs) const { return m_idx >= rhs.m_idx; }
};
inline GenericArrayIterator operator+(std::ptrdiff_t n, const GenericArrayIterator& it) { return it + n; }

// Implementations

template <bool Const, typename ValueT>
inline typename GenericArray<Const, ValueT>::ValueIterator GenericArray<Const, ValueT>::Begin() const { return m_value.Begin(); }
template <bool Const, typename ValueT>
inline typename GenericArray<Const, ValueT>::ValueIterator GenericArray<Const, ValueT>::End() const { return m_value.End(); }
template <bool Const, typename ValueT>
inline typename GenericArray<Const, ValueT>::ValueIterator GenericArray<Const, ValueT>::begin() const { return m_value.Begin(); }
template <bool Const, typename ValueT>
inline typename GenericArray<Const, ValueT>::ValueIterator GenericArray<Const, ValueT>::end() const { return m_value.End(); }
template <bool Const, typename ValueT>
inline SizeType GenericArray<Const, ValueT>::Size() const { return m_value.Size(); }
template <bool Const, typename ValueT>
inline const ValueT& GenericArray<Const, ValueT>::operator[](SizeType index) const { return m_value[index]; }

template <bool Const, typename ValueT>
inline typename GenericObject<Const, ValueT>::MemberIterator GenericObject<Const, ValueT>::MemberBegin() const { return m_value.MemberBegin(); }
template <bool Const, typename ValueT>
inline typename GenericObject<Const, ValueT>::MemberIterator GenericObject<Const, ValueT>::MemberEnd() const { return m_value.MemberEnd(); }
template <bool Const, typename ValueT>
inline typename GenericObject<Const, ValueT>::MemberIterator GenericObject<Const, ValueT>::begin() const { return m_value.MemberBegin(); }
template <bool Const, typename ValueT>
inline typename GenericObject<Const, ValueT>::MemberIterator GenericObject<Const, ValueT>::end() const { return m_value.MemberEnd(); }
template <bool Const, typename ValueT>
inline SizeType GenericObject<Const, ValueT>::MemberCount() const { return m_value.MemberCount(); }
template <bool Const, typename ValueT>
inline bool GenericObject<Const, ValueT>::HasMember(const char* name) const { return m_value.HasMember(name); }
template <bool Const, typename ValueT>
inline const ValueT& GenericObject<Const, ValueT>::operator[](const char* name) const { return m_value[name]; }


inline Value::Value(const Value& rhs, Allocator& allocator) : m_type(rhs.m_type), m_val(nullptr) {
    if (rhs.m_val) {
       if (rhs.IsString()) m_detached = std::string(rhs.GetString());
       else if (rhs.IsInt()) m_detached = (int64_t)rhs.GetInt();
       else if (rhs.IsDouble()) m_detached = rhs.GetDouble();
       else if (rhs.IsBool()) m_detached = rhs.GetBool();
       else if (rhs.IsArray()) {
           m_detached = ArrayData{};
           auto& arr = std::get<ArrayData>(m_detached);
           for (auto it = rhs.Begin(); it != rhs.End(); ++it) {
               arr.emplace_back(*it, allocator);
           }
       }
       else if (rhs.IsObject()) {
           m_detached = ObjectData{};
           auto& obj = std::get<ObjectData>(m_detached);
           int count = 0;
           for (auto it = rhs.MemberBegin(); it != rhs.MemberEnd(); ++it) {
               Value k(it->name, allocator);
               Value v(it->value, allocator);
               obj.emplace_back(std::make_pair(k.GetString(), std::move(v)));
               count++;
           }
           if (count == 0) {
              std::cerr << "DEBUG: Copying Object, passed IsObject(), but Member loop found 0 items! rhs.m_val: " << (void*)rhs.m_val << std::endl;
           }
       }
    } else {
         m_detached = rhs.m_detached;
    }
    m_children.reset(); // Invalidate cache for the new copy
}

inline void Value::AddMember(Value& name, Value& value, Allocator& allocator) {
    if (!m_val) {
         if (std::holds_alternative<ObjectData>(m_detached)) {
             auto& obj = std::get<ObjectData>(m_detached);
             obj.push_back({name.GetString(), value});
             return;
         }
    }
    yyjson_mut_doc* doc = allocator.GetDoc();
    yyjson_mut_val* keyVal = name.ToYYVal(doc);
    yyjson_mut_val* valVal = value.ToYYVal(doc);
    yyjson_mut_obj_add(m_val, keyVal, valVal);
}

inline void Value::AddMember(const char* name, Value& value, Allocator& allocator) {
    Value n(name, allocator);
    AddMember(n, value, allocator);
}
inline void Value::AddMember(Value& name, Value&& value, Allocator& allocator) { AddMember(name, value, allocator); }
inline void Value::AddMember(const char* name, Value&& value, Allocator& allocator) { AddMember(name, value, allocator); }

inline GenericMemberIterator Value::MemberBegin() {
        if (m_val) return GenericMemberIterator(m_val);
        if (std::holds_alternative<ObjectData>(m_detached)) return GenericMemberIterator(&std::get<ObjectData>(m_detached));
        return GenericMemberIterator();
    }
inline GenericMemberIterator Value::MemberEnd() { return GenericMemberIterator(); }
inline GenericMemberIterator Value::MemberBegin() const { return const_cast<Value*>(this)->MemberBegin(); }
inline GenericMemberIterator Value::MemberEnd() const { return const_cast<Value*>(this)->MemberEnd(); }

inline GenericArrayIterator Value::Begin() { return GenericArrayIterator(this, 0); }
inline GenericArrayIterator Value::End() { return GenericArrayIterator(this, Size()); }
inline GenericArrayIterator Value::Begin() const { return const_cast<Value*>(this)->Begin(); }
inline GenericArrayIterator Value::End() const { return const_cast<Value*>(this)->End(); }

inline GenericArrayIterator Value::begin() { return Begin(); }
inline GenericArrayIterator Value::end() { return End(); }
inline GenericArrayIterator Value::begin() const { return Begin(); }
inline GenericArrayIterator Value::end() const { return End(); }

inline bool Value::HasMember(const char* name) const {
    bool found = false;
    if (m_val) found = (yyjson_mut_obj_get(m_val, name) != nullptr);
    else if (std::holds_alternative<ObjectData>(m_detached)) {
        const auto& obj = std::get<ObjectData>(m_detached);
        for (const auto& kv : obj) if (kv.first == name) { found = true; break; }
    }

    if (!found && strcmp(name, "name") == 0) {
        // Debugging handled elsewhere if needed
    }
    return found;
    return found;
}

inline void Value::RemoveMember(const char* name) {
    if (m_val) yyjson_mut_obj_remove_key(m_val, name);
    else if (std::holds_alternative<ObjectData>(m_detached)) {
        auto& obj = std::get<ObjectData>(m_detached);
        auto it = std::remove_if(obj.begin(), obj.end(), [&](const auto& pair){ return pair.first == name; });
        obj.erase(it, obj.end());
    }
}

inline const Value& Value::operator[](const char* name) const {
    if (!m_children) m_children = std::make_shared<ChildrenCache>();
    auto it = m_children->stringMap.find(name);
    if (it != m_children->stringMap.end()) {
        if (!it->second->m_val && m_val) {
             // Check if it appeared
             yyjson_mut_val* child = yyjson_mut_obj_get(m_val, name);
             if (child) {
                 // Update cache
                 *it->second = Value(child);
             }
        }
        return *it->second;
    }

    Value v;
    if (m_val) v = Value(yyjson_mut_obj_get(m_val, name));
    else if (std::holds_alternative<ObjectData>(m_detached)) {
         const auto& obj = std::get<ObjectData>(m_detached);
         for (const auto& kv : obj) if (kv.first == name) v = kv.second;
    }

    auto inserted = m_children->stringMap.emplace(name, std::make_unique<Value>(std::move(v)));
    return *inserted.first->second;
}
inline Value& Value::operator[](const char* name) {
     if (!m_children) m_children = std::make_shared<ChildrenCache>();
     auto it = m_children->stringMap.find(name);
     if (it != m_children->stringMap.end()) {
         if (!it->second->m_val && m_val) {
              // Check if it appeared
              yyjson_mut_val* child = yyjson_mut_obj_get(m_val, name);
              if (child) {
                  // Update cache
                  *it->second = Value(child);
              }
         }
         return *it->second;
     }

     Value v;
     if (m_val) {
         yyjson_mut_val* child = yyjson_mut_obj_get(m_val, name);
         v = Value(child);
     }
     else if (ObjectData* obj = std::get_if<ObjectData>(&m_detached)) {
         for (auto& kv : *obj) if (kv.first == name) v = kv.second;
     }

     auto inserted = m_children->stringMap.emplace(name, std::make_unique<Value>(std::move(v)));
     return *inserted.first->second;
}

inline const Value& Value::operator[](SizeType index) const {
    // Note: const access still needs caching if we want to return const reference to ephemeral value?
    // Actually, legacy code expects `const Value& v = arr[0]`.
    // If we return Value by value, it binds to const ref, but lifetime ends at statement.
    // If we return const Value&, we MUST cache.
     if (!m_children) m_children = std::make_shared<ChildrenCache>();
     auto it = m_children->intMap.find(index);
     if (it != m_children->intMap.end()) {
        return *it->second;
    }

    Value v;
    if (m_val) v = Value(yyjson_mut_arr_get(m_val, index));
    else if (std::holds_alternative<ArrayData>(m_detached)) {
         auto& src = std::get<ArrayData>(m_detached)[index];
         v = src;
    }

    auto inserted = m_children->intMap.emplace(index, std::make_unique<Value>(std::move(v)));
    return *inserted.first->second;
}

inline Value& Value::operator[](SizeType index) {
     if (!m_children) m_children = std::make_shared<ChildrenCache>();
     auto it = m_children->intMap.find(index);
     if (it != m_children->intMap.end()) {
        return *it->second;
    }

     Value v;
     if (m_val) {
        yyjson_mut_val* child = yyjson_mut_arr_get(m_val, index);
        v = Value(child);
     }
     else if (std::holds_alternative<ArrayData>(m_detached)) {
         v = std::get<ArrayData>(m_detached)[index];
     }

     auto inserted = m_children->intMap.emplace(index, std::make_unique<Value>(std::move(v)));
     return *inserted.first->second;
}

// ...


inline SizeType Value::Size() const {
    if (m_val) return (SizeType)yyjson_mut_get_len(m_val);
    if (std::holds_alternative<ArrayData>(m_detached)) return std::get<ArrayData>(m_detached).size();
    return 0;
}
inline SizeType Value::MemberCount() const {
    if (m_val && yyjson_mut_is_obj(m_val)) return (SizeType)yyjson_mut_get_len(m_val);
    if (std::holds_alternative<ObjectData>(m_detached)) return std::get<ObjectData>(m_detached).size();
    return 0;
}
inline void Value::PushBack(Value& value, Allocator& allocator) {
    if (m_val) {
        yyjson_mut_val* v = value.ToYYVal(allocator.GetDoc());
        yyjson_mut_arr_append(m_val, v);
    } else if (std::holds_alternative<ArrayData>(m_detached)) {
        std::get<ArrayData>(m_detached).push_back(value);
    }
}

inline yyjson_mut_val* Value::ToYYVal(yyjson_mut_doc* doc) {
    if (m_val) return m_val;
    if (std::holds_alternative<int64_t>(m_detached)) return yyjson_mut_sint(doc, std::get<int64_t>(m_detached));
    if (std::holds_alternative<uint64_t>(m_detached)) return yyjson_mut_uint(doc, std::get<uint64_t>(m_detached));
    if (std::holds_alternative<double>(m_detached)) return yyjson_mut_real(doc, std::get<double>(m_detached));
    if (std::holds_alternative<std::string>(m_detached)) return yyjson_mut_str(doc, std::get<std::string>(m_detached).c_str());
    if (std::holds_alternative<bool>(m_detached)) return yyjson_mut_bool(doc, std::get<bool>(m_detached));
    if (std::holds_alternative<ArrayData>(m_detached)) {
         yyjson_mut_val* arr = yyjson_mut_arr(doc);
         for(auto& v : std::get<ArrayData>(m_detached)) yyjson_mut_arr_append(arr, v.ToYYVal(doc));
         return arr;
    }
    if (std::holds_alternative<ObjectData>(m_detached)) {
         yyjson_mut_val* obj = yyjson_mut_obj(doc);
         for(auto& kv : std::get<ObjectData>(m_detached)) yyjson_mut_obj_add(obj, yyjson_mut_str(doc, kv.first.c_str()), kv.second.ToYYVal(doc));
         return obj;
    }
    return yyjson_mut_null(doc);
}
    inline void Value::UpdateTypeFromVal() {
    if (!m_val) { m_type = kNullType; return; }
    if (yyjson_mut_is_null(m_val)) m_type = kNullType;
    else if (yyjson_mut_is_bool(m_val)) m_type = yyjson_mut_get_bool(m_val) ? kTrueType : kFalseType;
    else if (yyjson_mut_is_int(m_val) || yyjson_mut_is_uint(m_val)) m_type = kNumberType;
    else if (yyjson_mut_is_real(m_val)) m_type = kNumberType;
    else if (yyjson_mut_is_str(m_val)) m_type = kStringType;
    else if (yyjson_mut_is_arr(m_val)) m_type = kArrayType;
    else if (yyjson_mut_is_obj(m_val)) m_type = kObjectType;
}

    template <typename Handler>
    bool Value::Accept(Handler& handler) const {
        // std::cout << "DEBUG: Accept called. m_val=" << (void*)m_val << std::endl;
        if (m_val) {
             if (yyjson_mut_is_null(m_val)) return handler.Null();
             if (yyjson_mut_is_bool(m_val)) return handler.Bool(yyjson_mut_get_bool(m_val));
             if (yyjson_mut_is_int(m_val)) {
                 int64_t v = yyjson_mut_get_sint(m_val);
                 if (v >= -2147483648LL && v <= 2147483647LL) return handler.Int((int)v);
                 return handler.Int64(v);
             }
             if (yyjson_mut_is_uint(m_val)) {
                 uint64_t v = yyjson_mut_get_uint(m_val);
                 if (v <= 4294967295ULL) return handler.Uint((unsigned)v);
                 return handler.Uint64(v);
             }
             if (yyjson_mut_is_real(m_val)) return handler.Double(yyjson_mut_get_real(m_val));
             if (yyjson_mut_is_str(m_val)) return handler.String(yyjson_mut_get_str(m_val), yyjson_mut_get_len(m_val), true);
             if (yyjson_mut_is_arr(m_val)) {
                 if (!handler.StartArray()) return false;
                 size_t len = yyjson_mut_get_len(m_val);
                 yyjson_mut_arr_iter iter;
                 yyjson_mut_arr_iter_init(m_val, &iter);
                 yyjson_mut_val *child;
                 size_t idx = 0;
                 while ((child = yyjson_mut_arr_iter_next(&iter))) {
                     // Check overlay
                     Value* valToAccept = nullptr;
                     if (m_children) {
                         auto it = m_children->intMap.find(idx);
                         if (it != m_children->intMap.end()) valToAccept = it->second.get();
                     }

                     if (valToAccept) {
                         if (!valToAccept->Accept(handler)) return false;
                     } else {
                         Value v(child);
                         if (!v.Accept(handler)) return false;
                     }
                     idx++;
                 }
                 return handler.EndArray(len);
             }
             if (yyjson_mut_is_obj(m_val)) {
                 if (!handler.StartObject()) return false;
                 size_t len = yyjson_mut_get_len(m_val);
                 yyjson_mut_obj_iter iter;
                 yyjson_mut_obj_iter_init(m_val, &iter);
                 yyjson_mut_val *key, *val;
                 while ((key = yyjson_mut_obj_iter_next(&iter))) {
                     val = yyjson_mut_obj_iter_get_val(key);
                     const char* k = yyjson_mut_get_str(key);
                     size_t kl = yyjson_mut_get_len(key);
                     if (!handler.Key(k, kl, true)) return false;

                     // Check overlay
                     Value* valToAccept = nullptr;
                     if (m_children) {
                         auto it = m_children->stringMap.find(k);
                         if (it != m_children->stringMap.end()) {
                             valToAccept = it->second.get();
                             std::cout << "DEBUG: Accept FOUND OVERLAY for key=" << k << " val=" << valToAccept << std::endl;
                         }
                     }
                     else std::cout << "DEBUG: Accept NO OVERLAY for key=" << k << std::endl;

                     if (valToAccept) {
                         if (!valToAccept->Accept(handler)) return false;
                     } else {
                         Value v(val);
                         if (!v.Accept(handler)) return false;
                     }
                 }
                 return handler.EndObject(len);
             }
             return true;
        }
        if (std::holds_alternative<int64_t>(m_detached)) return handler.Int64(std::get<int64_t>(m_detached));
        if (std::holds_alternative<double>(m_detached)) return handler.Double(std::get<double>(m_detached));
        if (std::holds_alternative<bool>(m_detached)) return handler.Bool(std::get<bool>(m_detached));
        if (std::holds_alternative<std::string>(m_detached)) {
             const std::string& s = std::get<std::string>(m_detached);
             return handler.String(s.c_str(), s.size(), true);
        }
        if (std::holds_alternative<ArrayData>(m_detached)) {
            if (!handler.StartArray()) return false;
            const auto& arr = std::get<ArrayData>(m_detached);
            size_t idx = 0;
            for (const auto& v : arr) {
                 // Check overlay
                 Value* valToAccept = nullptr;
                 if (m_children) {
                     auto it = m_children->intMap.find(idx);
                     if (it != m_children->intMap.end()) valToAccept = it->second.get();
                 }

                 if (valToAccept) {
                     if (!valToAccept->Accept(handler)) return false;
                 } else {
                     if (!v.Accept(handler)) return false;
                 }
                 idx++;
            }
            return handler.EndArray(arr.size());
        }
        if (std::holds_alternative<ObjectData>(m_detached)) {
            if (!handler.StartObject()) return false;
            const auto& obj = std::get<ObjectData>(m_detached);
            for (const auto& kv : obj) {
                if (!handler.Key(kv.first.c_str(), kv.first.size(), true)) return false;

                 // Check overlay
                 Value* valToAccept = nullptr;
                 if (m_children) {
                     auto it = m_children->stringMap.find(kv.first);
                     if (it != m_children->stringMap.end()) valToAccept = it->second.get();
                 }

                 if (valToAccept) {
                     if (!valToAccept->Accept(handler)) return false;
                 } else {
                     if (!kv.second.Accept(handler)) return false;
                 }
            }
            return handler.EndObject(obj.size());
        }
        if (m_type == kNullType) return handler.Null();
        return true;
    }


class Document : public Value {
public:
    Document() : Value(), m_doc(yyjson_mut_doc_new(nullptr)), m_allocator(m_doc) {
        Attach(yyjson_mut_doc_get_root(m_doc));
    }

    // Move Constructor
    Document(Document&& rhs) noexcept : Value(static_cast<Value&&>(rhs)), m_doc(rhs.m_doc), m_allocator(rhs.m_allocator) {
        rhs.m_doc = nullptr;
        rhs.m_allocator.SetDoc(nullptr);
        rhs.m_val = nullptr;
        rhs.m_detached = std::monostate{};
    }

    // Move Assignment
    Document& operator=(Document&& rhs) noexcept {
        if (this != &rhs) {
             if (m_doc) yyjson_mut_doc_free(m_doc);
             Value::operator=(static_cast<Value&&>(rhs));
             m_doc = rhs.m_doc;
             m_allocator = rhs.m_allocator;

             rhs.m_doc = nullptr;
             rhs.m_allocator.SetDoc(nullptr);
             rhs.m_val = nullptr;
             rhs.m_detached = std::monostate{};
        }
        return *this;
    }

    // Deep Copy Constructor
    Document(const Document& rhs) : Value(), m_doc(nullptr), m_allocator(nullptr) {
        if (rhs.m_doc) {
            m_doc = yyjson_mut_doc_new(nullptr);
            yyjson_mut_val* srcRoot = yyjson_mut_doc_get_root(rhs.m_doc);
            if (srcRoot) {
                yyjson_mut_val* dstRoot = yyjson_mut_val_mut_copy(m_doc, srcRoot);
                yyjson_mut_doc_set_root(m_doc, dstRoot);
            }
            m_allocator.SetDoc(m_doc);
            Attach(yyjson_mut_doc_get_root(m_doc));
        } else {
             // Should not happen for valid document, but handle it
             m_doc = yyjson_mut_doc_new(nullptr);
             m_allocator.SetDoc(m_doc);
             Attach(nullptr);
        }
        m_type = rhs.m_type; // Should be object/array/etc corresponding to root
    }

    // Deep Copy Assignment
    Document& operator=(const Document& rhs) {
        if (this != &rhs) {
            if (m_doc) yyjson_mut_doc_free(m_doc);

            if (rhs.m_doc) {
                m_doc = yyjson_mut_doc_new(nullptr);
                yyjson_mut_val* srcRoot = yyjson_mut_doc_get_root(rhs.m_doc);
                if (srcRoot) {
                    yyjson_mut_val* dstRoot = yyjson_mut_val_mut_copy(m_doc, srcRoot);
                    yyjson_mut_doc_set_root(m_doc, dstRoot);
                }
                m_allocator.SetDoc(m_doc);
                Attach(yyjson_mut_doc_get_root(m_doc));
            } else {
                m_doc = yyjson_mut_doc_new(nullptr);
                m_allocator.SetDoc(m_doc);
                Attach(nullptr);
            }
            m_type = rhs.m_type;
        }
        return *this;
    }

    ~Document() {
        if (m_doc) yyjson_mut_doc_free(m_doc);
    }
    Allocator& GetAllocator() { return m_allocator; }
    void Parse(const char* json) {
        if (m_doc) yyjson_mut_doc_free(m_doc);
        yyjson_doc* temp = yyjson_read(json, strlen(json), 0);
        if (temp) {
            m_doc = yyjson_doc_mut_copy(temp, nullptr);
            yyjson_doc_free(temp);
            Attach(yyjson_mut_doc_get_root(m_doc));
        } else {
             m_doc = yyjson_mut_doc_new(nullptr);
             Attach(nullptr);
             m_type = kNullType;
        }
        m_allocator.SetDoc(m_doc);
    }
    template <unsigned parseFlags, typename Stream>
    void ParseStream(Stream& is) {
        uint32_t flg = 0;
        if (parseFlags & kParseCommentsFlag) flg |= YYJSON_READ_ALLOW_COMMENTS;
        if (parseFlags & kParseTrailingCommasFlag) flg |= YYJSON_READ_ALLOW_TRAILING_COMMAS;
        ParseStream(is, flg);
    }

    template <typename Stream>
    void ParseStream(Stream& is, uint32_t flags = 0) {
        if (m_doc) yyjson_mut_doc_free(m_doc);
        FILE* fp = is.GetFP();
        if (fp) {
             yyjson_doc* temp = yyjson_read_fp(fp, flags, nullptr, nullptr);
                     if (temp) {
                         m_doc = yyjson_doc_mut_copy(temp, nullptr);
                         yyjson_doc_free(temp);
                         Attach(yyjson_mut_doc_get_root(m_doc));
                     } else {
                 m_doc = yyjson_mut_doc_new(nullptr);
                 Attach(nullptr);
                 m_type = kNullType;
             }
        } else {
             m_doc = yyjson_mut_doc_new(nullptr);
             Attach(nullptr);
             m_type = kNullType;
        }
        m_allocator.SetDoc(m_doc);
    }
    bool HasParseError() const { return m_type == kNullType && m_val == nullptr; }
    ParseErrorCode GetParseError() const { return HasParseError() ? kParseErrorDocumentEmpty : kParseErrorNone; }
private:
    yyjson_mut_doc* m_doc;
    Allocator m_allocator;
};

inline Value& GenericArrayIterator::operator*() {
    // std::cout << "DEBUG: Iterator deref m_parent=" << m_parent << " idx=" << m_idx << std::endl;
    return (*m_parent)[(SizeType)m_idx];
}
inline Value* GenericArrayIterator::operator->() { return &(*m_parent)[(SizeType)m_idx]; }
inline Value GenericArrayIterator::operator[](difference_type n) const { return (*m_parent)[(SizeType)(m_idx + n)]; }

} // namespace rapidjson

#endif // RAPIDJSON_DOCUMENT_H_
