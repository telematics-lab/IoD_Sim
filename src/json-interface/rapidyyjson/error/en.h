/*
 * RapidJSON wrapper using yyjson
 */

#ifndef RAPIDYYJSON_ERROR_EN_H_
#define RAPIDYYJSON_ERROR_EN_H_

#include "../rapidyyjson.h"

namespace rapidyyjson {

inline const char* GetParseError_En(int) {
    return "Parse error";
}

} // namespace rapidyyjson

#endif // RAPIDYYJSON_ERROR_EN_H_
