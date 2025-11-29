#include "json_parser.h"

void skip_whitespace(json_parser_t* parser) {
	while (parser->pos < parser->len && isspace(parser->json[parser->pos])) {
		parser->pos++;
	}
}

int is_eof(json_parser_t* parser) {
	return parser->pos >= parser->len;
}

char current_char(json_parser_t* parser) {
	return parser->json[parser->pos];
}

char next_char(json_parser_t* parser) {
	if (parser->pos < parser->len) {
		return parser->json[parser->pos++];
	}
	return '\0';
}

void set_error(json_parser_t* parser, const char* message) {
	if (parser->error == NULL) {
		parser->error = malloc(256);
		snprintf(parser->error, 256, "%s at position %zu", message, parser->pos);
	}
}

json_value_t* create_value(json_type_t type) {
	json_value_t* value = malloc(sizeof(json_value_t));
	if (value) {
		value->type = type;
		memset(&value->data, 0, sizeof(value->data));
	}
	return value;
}

json_value_t* parse_null(json_parser_t* parser) {
	if (parser->pos + 3 < parser->len &&
		parser->json[parser->pos] == 'n' &&
		parser->json[parser->pos + 1] == 'u' &&
		parser->json[parser->pos + 2] == 'l' &&
		parser->json[parser->pos + 3] == 'l') {
		parser->pos += 4;
		return create_value(JSON_NULL);
	}
	set_error(parser, "Expected 'null'");
	return NULL;
}

json_value_t* parse_boolean(json_parser_t* parser) {
	if (parser->pos + 3 < parser->len &&
		parser->json[parser->pos] == 't' &&
		parser->json[parser->pos + 1] == 'r' &&
		parser->json[parser->pos + 2] == 'u' &&
		parser->json[parser->pos + 3] == 'e') {
		parser->pos += 4;
		json_value_t* value = create_value(JSON_BOOL);
		if (value) value->data.boolean = 1;
		return value;
	}

	if (parser->pos + 4 < parser->len &&
		parser->json[parser->pos] == 'f' &&
		parser->json[parser->pos + 1] == 'a' &&
		parser->json[parser->pos + 2] == 'l' &&
		parser->json[parser->pos + 3] == 's' &&
		parser->json[parser->pos + 4] == 'e') {
		parser->pos += 5;
		json_value_t* value = create_value(JSON_BOOL);
		if (value) value->data.boolean = 0;
		return value;
	}

	set_error(parser, "Expected 'true' or 'false'");
	return NULL;
}

json_value_t* parse_number(json_parser_t* parser) {
	char* endptr;
	const char* start = parser->json + parser->pos;
	double number = strtod(start, &endptr);

	if (endptr == start) {
		set_error(parser, "Expected number");
		return NULL;
	}

	parser->pos += (endptr - start);
	json_value_t* value = create_value(JSON_NUMBER);
	if (value)value->data.number = number;

	return value;
}

json_value_t* parse_string(json_parser_t* parser) {
	if (current_char(parser) != '"') {
		set_error(parser, "Expected string");
		return NULL;
	}

	parser->pos++;

	size_t start = parser->pos;

	while (parser->pos < parser->len && current_char(parser) != '"') {
		if (current_char(parser) == '\\')parser->pos++; 
		parser->pos++;
	}

	if (parser->pos >= parser->len) {
		set_error(parser, "Untermitated string");
		return NULL;
	}

	size_t length = parser->pos - start;
	parser->pos++;

	json_value_t* value = create_value(JSON_STRING);

	if (!value)return NULL;

	value->data.string = malloc(length + 1);
	if (!value->data.string) {
		free(value);
		return NULL;
	}

	strncpy(value->data.string, parser->json + start, length);
	value->data.string[length] = '\0';
	
	return value;
}

json_value_t* parse_array(json_parser_t* parser) {
	if (current_char(parser) != '[') {
		set_error(parser, "Expected array");
		return NULL;
	}

	parser->pos++;
	skip_whitespace(parser);

	json_value_t* array = create_value(JSON_ARRAY);
	if (!array)return NULL;

	array->data.array.capacity = 8;
	array->data.array.values = malloc(sizeof(json_value_t*) * array->data.array.capacity);
	array->data.array.count = 0;

	if (!array->data.array.values) {
		free(array);
		return NULL;
	}

	if (current_char(parser) == ']') {
		parser->pos++;
		return array;
	}

	while (!is_eof(parser)) {
		skip_whitespace(parser);

		json_value_t* element = parse_value(parser);

		if (!element) {
			json_free(array);
			return NULL;
		}

		if (array->data.array.count >= array->data.array.capacity) {
			size_t new_capacity = array->data.array.capacity * 2;
			json_value_t** new_values = realloc(array->data.array.values,
				sizeof(json_value_t*) * new_capacity);
			if (!new_values) {
				json_free(element);
				json_free(array);
				return NULL;
			}
			array->data.array.values = new_values;
			array->data.array.capacity = new_capacity;
		}

		array->data.array.values[array->data.array.count++] = element;

		skip_whitespace(parser);

		if (current_char(parser) == ']') {
			parser->pos++;
			break;
		}

		if (current_char(parser) != ',') {
			set_error(parser, "Expected ',' or ']'");
			json_free(array);
			return NULL;
		}

		parser->pos++;		
	}
	return array;
}

json_value_t* parse_value(json_parser_t* parser) {
	skip_whitespace(parser);

	if (is_eof(parser)) {
		set_error(parser, "Unexpected end of input");
		return NULL;
	}

	char c = current_char(parser);

	switch (c)
	{
	case 'n':return parse_null(parser);
	case 't':
	case 'f':return parse_boolean(parser);
	case '"':return parse_string(parser);
	case '[':return parse_array(parser);
	case '{':return parse_object(parser);
	default:
		if (isdigit(c) || c == '-') {
			return parse_number(parser);
		}
		set_error(parser, "Unexpected character");
		return NULL;
	}
}

json_value_t* parse_object(json_parser_t* parser) {
	if(current_char(parser) != '{'){
		set_error(parser, "Expected Object");
		return NULL;
	}

	parser->pos++;
	skip_whitespace(parser);

	json_value_t* object = create_value(JSON_OBJECT);
	if (!object) return NULL;

	object->data.object.capacity = 8;
	object->data.object.entries = malloc(sizeof(struct json_object_entry) * object->data.object.capacity);
	object->data.object.count = 0;

	if (!object->data.object.entries) {
		free(object);
		return NULL;
	}

	if (current_char(parser) == '}') {
		parser->pos++;
		return object;
	}

	while (!is_eof(parser)) {
		skip_whitespace(parser);

		if (current_char(parser) != '"') {
			set_error(parser, "Expected string key");
			json_free(object);
			return NULL;
		}

		json_value_t* key_value = parse_string(parser);
		if (!key_value) {
			json_free(object);
			return NULL;
		}

		skip_whitespace(parser);

		if (current_char(parser) != ':') {
			set_error(parser, "Expected ':' after key");
			json_free(key_value);
			json_free(object);
			return NULL;
		}

		parser->pos++;
		skip_whitespace(parser);

		json_value_t* value = parse_value(parser);
		if (!value) {
			json_free(key_value);
			json_free(object);
			return NULL;
		}

		if (object->data.object.count >= object->data.object.capacity) {
			size_t new_capacity = object->data.object.capacity * 2;
			struct json_object_entry* new_entries = realloc(object->data.object.entries,
				sizeof(struct json_object_entry) * new_capacity);
			if (!new_entries) {
				json_free(key_value);
				json_free(value);
				json_free(object);
				return NULL;
			}
			object->data.object.entries = new_entries;
			object->data.object.capacity = new_capacity;
		}

		object->data.object.entries[object->data.object.count].key = key_value->data.string;
		object->data.object.entries[object->data.object.count].value = value;
		object->data.object.count++;

		free(key_value);

		skip_whitespace(parser);

		if (current_char(parser) == '}') {
			parser->pos++;
			break;
		}

		if (current_char(parser) != ',') {
			set_error(parser, "Expected ',' or '}'");
			json_free(object);
			return NULL;
		}

		parser->pos++;
	}
	return object;
}


