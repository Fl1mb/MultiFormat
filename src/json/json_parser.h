#ifndef MULTIFORMAT_JSON_PARSER_H
#define MULTIFORMAT_JSON_PARSER_H

#include "../core/data_types.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define JSON_PARSE_STACK_INIT_SIZE 256

typedef struct {
	const char* json;
	size_t pos;
	size_t len;
	char* error;
}json_parser_t;

void skip_whitespace(json_parser_t* parser);
int is_eof(json_parser_t* parser);
char current_char(json_parser_t* parser);
char next_char(json_parser_t* parser);
void set_error(json_parser_t* parser, const char* message);
json_value_t* create_value(json_type_t type);
json_value_t* parse_null(json_parser_t* parser);
json_value_t* parse_boolean(json_parser_t* parser);
json_value_t* parse_number(json_parser_t* parser);
json_value_t* parse_string(json_parser_t* parser);
json_value_t* parse_array(json_parser_t* parser);
json_value_t* parse_value(json_parser_t* parser);
json_value_t* parse_object(json_parser_t* parser);



#endif // MULTIFORMAT_JSON_PARSER_H