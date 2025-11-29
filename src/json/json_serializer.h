#ifndef MULTIFORMAT_JSON_SERIALIZER_H
#define MULTIFORMAT_JSON_SERIALIZER_H

#include "../core/data_types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define JSON_SERIALIZER_INIT_SIZE 256
#define JSON_INDENT_SIZE 2

typedef struct {
	char* buffer;
	size_t length;
	size_t capacity;
	int pretty;
	int indent_level;
}json_serializer_t;


void serializer_init(json_serializer_t* serializer, int pretty);
void serializer_free(json_serializer_t* serializer);
int serializer_ensure_capacity(json_serializer_t* serializer, size_t needed);
int serializer_append(json_serializer_t* serializer, const char* str);
int serializer_append_indent(json_serializer_t* serializer);
int serializer_append_char(json_serializer_t* serializer, char c);
char* escape_string(const char* str);
int serialize_value(json_serializer_t* serializer, const json_value_t* value);
int serialize_null(json_serializer_t* serializer);
int serialize_boolean(json_serializer_t* serializer, const json_value_t* value);
int serialize_number(json_serializer_t* serializer, const json_value_t* value);
int serialize_string(json_serializer_t* serializer, const json_value_t* value);
int serialize_array(json_serializer_t* serializer, const json_value_t* value);
int serialize_object(json_serializer_t* serializer, const json_value_t* value);



#endif // MILTIFORMAT_JSON_SERIALIZER_H