/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDYYJSON_RAPIDYYJSON_H_
#define RAPIDYYJSON_RAPIDYYJSON_H_

#include <cstddef>
#include <string>
#include <cassert>
#include <cstdint>

// Stubs for RapidJSON system header compatibility
#define RAPIDYYJSON_DIAG_PUSH
#define RAPIDYYJSON_DIAG_POP
#define RAPIDYYJSON_DIAG_OFF(x)
#define RAPIDYYJSON_STATIC_ASSERT(x) static_assert(x, #x)
#define RAPIDYYJSON_ASSERT(x) assert(x)

#define RAPIDYYJSON_NAMESPACE_BEGIN namespace rapidyyjson {
#define RAPIDYYJSON_NAMESPACE_END }
#define RAPIDYYJSON_ERROR_CHARTYPE char
#define RAPIDYYJSON_NOEXCEPT noexcept
#define RAPIDYYJSON_FORCEINLINE inline
#define RAPIDYYJSON_ALIGN(x) (((x) + ((sizeof(void*)) - 1)) & ~((sizeof(void*)) - 1))
#define RAPIDYYJSON_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))

#define RAPIDYYJSON_LITTLEENDIAN 0
#define RAPIDYYJSON_BIGENDIAN 1
#ifndef RAPIDYYJSON_ENDIAN
#define RAPIDYYJSON_ENDIAN RAPIDYYJSON_LITTLEENDIAN
#endif

#define RAPIDYYJSON_HAS_STDSTRING 1

#ifndef RAPIDYYJSON_UNLIKELY
#define RAPIDYYJSON_UNLIKELY(x) (x)
#endif
#ifndef RAPIDYYJSON_LIKELY
#define RAPIDYYJSON_LIKELY(x) (x)
#endif

#ifndef RAPIDYYJSON_NEW
#define RAPIDYYJSON_NEW(x) new x
#endif
#ifndef RAPIDYYJSON_DELETE
#define RAPIDYYJSON_DELETE(x) delete x
#endif

namespace rapidyyjson {

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

} // namespace rapidyyjson

#endif // RAPIDYYJSON_RAPIDYYJSON_H_
