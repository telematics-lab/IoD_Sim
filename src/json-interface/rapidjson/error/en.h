/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDJSON_ERROR_EN_H_
#define RAPIDJSON_ERROR_EN_H_

#include "../rapidjson.h"

namespace rapidjson {

inline const char* GetParseError_En(int) {
    return "Parse error";
}

} // namespace rapidjson

#endif // RAPIDJSON_ERROR_EN_H_
