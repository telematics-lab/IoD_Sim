/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDJSON_RAPIDJSON_H_
#define RAPIDJSON_RAPIDJSON_H_

#include <cstddef>
#include <string>
#include <cassert>
#include <cstdint>

// Stubs for RapidJSON system header compatibility
#define RAPIDJSON_DIAG_PUSH
#define RAPIDJSON_DIAG_POP
#define RAPIDJSON_DIAG_OFF(x)
#define RAPIDJSON_STATIC_ASSERT(x) static_assert(x, #x)
#define RAPIDJSON_ASSERT(x) assert(x)

#define RAPIDJSON_NAMESPACE_BEGIN namespace rapidjson {
#define RAPIDJSON_NAMESPACE_END }
#define RAPIDJSON_ERROR_CHARTYPE char
#define RAPIDJSON_NOEXCEPT noexcept
#define RAPIDJSON_FORCEINLINE inline
#define RAPIDJSON_ALIGN(x) (((x) + ((sizeof(void*)) - 1)) & ~((sizeof(void*)) - 1))
#define RAPIDJSON_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))

#define RAPIDJSON_LITTLEENDIAN 0
#define RAPIDJSON_BIGENDIAN 1
#ifndef RAPIDJSON_ENDIAN
#define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#endif

#define RAPIDJSON_HAS_STDSTRING 1

#ifndef RAPIDJSON_UNLIKELY
#define RAPIDJSON_UNLIKELY(x) (x)
#endif
#ifndef RAPIDJSON_LIKELY
#define RAPIDJSON_LIKELY(x) (x)
#endif

#ifndef RAPIDJSON_NEW
#define RAPIDJSON_NEW(x) new x
#endif
#ifndef RAPIDJSON_DELETE
#define RAPIDJSON_DELETE(x) delete x
#endif

namespace rapidjson {

enum Type {
    kNullType = 0,
    kFalseType = 1,
    kTrueType = 2,
    kObjectType = 3,
    kArrayType = 4,
    kStringType = 5,
    kNumberType = 6
};

// SizeType definition
typedef unsigned SizeType;

enum ParseFlag {
    kParseNoFlags = 0,
    kParseInsituFlag = 1,
    kParseValidateEncodingFlag = 2,
    kParseIterativeFlag = 4,
    kParseStopWhenDoneFlag = 8,
    kParseFullPrecisionFlag = 16,
    kParseCommentsFlag = 32,
    kParseNumbersAsStringsFlag = 64,
    kParseTrailingCommasFlag = 128,
    kParseNanAndInfFlag = 256,
    kParseEscapedApostropheFlag = 512
};

// Error codes
enum ParseErrorCode {
    kParseErrorNone = 0,
    kParseErrorDocumentEmpty,
    kParseErrorValueInvalid
};

inline const char* GetParseError_En(ParseErrorCode) { return "Parse Error"; }

} // namespace rapidjson

#endif // RAPIDJSON_RAPIDJSON_H_
