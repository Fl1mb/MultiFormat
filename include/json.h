#ifndef MULTIFORMAT_JSON_H
#define MULTIFORMAT_JSON_H

#include <stddef.h>
#include "../src/core/data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

    // parse funcs
    json_value_t* json_parse(const char* json_str);
    json_value_t* json_parse_file(const char* filename);

    // serialize funcs
    char* json_serialize(const json_value_t* value);
    char* json_serialize_pretty(const json_value_t* value);
    int json_serialize_file(const json_value_t* value, const char* filename);

    // utils
    void json_free(json_value_t* value);
    json_type_t json_get_type(const json_value_t* value);
    const char* json_type_to_string(json_type_t type);

    // access to data
    int json_get_boolean(const json_value_t* value);
    double json_get_number(const json_value_t* value);
    const char* json_get_string(const json_value_t* value);
    size_t json_get_array_size(const json_value_t* value);
    json_value_t* json_array_get(const json_value_t* value, size_t index);
    size_t json_object_size(const json_value_t* value);
    const char* json_object_get_key(const json_value_t* value, size_t index);
    json_value_t* json_get_value(const json_value_t* value, size_t index);
    json_value_t* json_object_get(const json_value_t* value, const char* key);

#ifdef __cplusplus
}
#endif

#endif // MULTIFORMAT_JSON_H