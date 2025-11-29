#include "json.h"
#include "../src/json/json_parser.h"
#include "../src/json/json_serializer.h"

json_value_t* json_parse(const char* json_str)
{
	if (!json_str) {
		return NULL;
	}

	json_parser_t parser = {
		.json = json_str,
		.pos = 0,
		.len = strlen(json_str),
		.error = NULL
	};

	json_value_t* result = parse_value(&parser);

	if (parser.error) {
		fprintf(stderr, "JSON parse error: %s\n", parser.error);
		free(parser.error);
		if (result)json_free(result);
		return NULL;
	}

	skip_whitespace(&parser);
	if (!is_eof(&parser)) {
		fprintf(stderr, "JSON parse error: Extra data after JSON\n");
		if (result)json_free(result);
		return NULL;
	}

	return result;
}

json_value_t* json_parse_file(const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (file_size <= 0) {
		fclose(file);
		return NULL;
	}

	char* buffer = malloc(file_size + 1);
	if (!buffer) {
		fclose(file);
		return NULL;
	}

	size_t read_size = fread(buffer, 1, file_size, file);
	buffer[read_size] = '\0';

	fclose(file);

	json_value_t* result = json_parse(buffer);

	free(buffer);
	return result;
}

char* json_serialize(const json_value_t* value)
{
	if (!value)return NULL;

	json_serializer_t serializer;

	serializer_init(&serializer, 0);

	if (!serialize_value(&serializer, value)) {
		serializer_free(&serializer);
		return NULL;
	}
	return serializer.buffer;
}

char* json_serialize_pretty(const json_value_t* value)
{
	if (!value)return NULL;

	json_serializer_t serializer;

	serializer_init(&serializer, 1);

	if (!serialize_value(&serializer, value)) {
		serializer_free(&serializer);
		return NULL;
	}
	return serializer.buffer;
}

int json_serialize_file(const json_value_t* value, const char* filename)
{
	char* json_str = json_serialize(value);
	if (!json_str) return 0;

	FILE* file = fopen(filename, "w");
	if (!file) {
		free(json_str);
		return 0;
	}

	fputs(json_str, file);
	fclose(file);
	free(json_str);
	return 1;
}

void json_free(json_value_t* value)
{
	if (!value) return;

	free_stack_node_t* stack = NULL;

	free_stack_node_t* node = malloc(sizeof(free_stack_node_t));
	node->value = value;
	node->next = stack;
	stack = node;

	while (stack != NULL) {
		free_stack_node_t* current = stack;
		stack = stack->next;

		json_value_t* value = current->value;
		free(current);

		if (!value)continue;

		switch (value->type)
		{
		case JSON_STRING:
			free(value->data.string);
			break;
		case JSON_ARRAY:
			for (size_t i = 0; i < value->data.array.count; ++i) {
				if (value->data.array.values[i]) {
					free_stack_node_t* new_node = malloc(sizeof(free_stack_node_t));
					new_node->value = value->data.array.values[i];
					new_node->next = stack;
					stack = new_node;
				}
			}
			free(value->data.array.values);
			break;
		case JSON_OBJECT:
			for (size_t i = 0; i < value->data.object.count; i++) {
				free(value->data.object.entries[i].key);

				if (value->data.object.entries[i].value) {
					free_stack_node_t* new_node = malloc(sizeof(free_stack_node_t));
					new_node->value = value->data.object.entries[i].value;
					new_node->next = stack;
					stack = new_node;
				}
			}
			free(value->data.object.entries);
			break;
		default:
			break;
		}

		free(value);
	}
}

json_type_t json_get_type(const json_value_t* value)
{
	if (!value)return JSON_NULL;
	return value->type;
}

const char* json_type_to_string(json_type_t type)
{
	switch (type)
	{
	case JSON_NULL:
		return "null";
	case JSON_BOOL:
		return "boolean";
	case JSON_STRING:
		return "string";
	case JSON_NUMBER:
		return "number";
	case JSON_ARRAY:
		return "array";
	case JSON_OBJECT:
		return "object";
	default:
		return "unknown";
	}
}

int json_get_boolean(const json_value_t* value)
{
	if (!value || value->type != JSON_BOOL) {
		return 0;
	}
	return value->data.boolean;
}

double json_get_number(const json_value_t* value)
{
	if (!value || value->type != JSON_NUMBER) {
		return 0.0; 
	}
	return value->data.number;
}

const char* json_get_string(const json_value_t* value)
{
	if (!value || value->type != JSON_STRING) {
		return NULL; 
	}
	return value->data.string;
}

size_t json_get_array_size(const json_value_t* value)
{
	if (!value || value->type != JSON_ARRAY) {
		return 0; 
	}
	return value->data.array.count;
}

json_value_t* json_array_get(const json_value_t* value, size_t index)
{
	if (!value || value->type != JSON_ARRAY) {
		return NULL;
	}
	if (index >= value->data.array.count) {
		return NULL;
	}
	return value->data.array.values[index];
}

size_t json_object_size(const json_value_t* value)
{
	if (!value || value->type != JSON_OBJECT) {
		return 0;
	}
	return value->data.object.count;
}

const char* json_object_get_key(const json_value_t* value, size_t index)
{
	if (!value || value->type != JSON_OBJECT) {
		return NULL;
	}
	if (index >= value->data.object.count) {
		return NULL;
	}
	return value->data.object.entries[index].key;
}

json_value_t* json_object_get_value(const json_value_t* value, size_t index)
{
	if (!value || value->type != JSON_OBJECT) {
		return NULL;
	}
	if (index >= value->data.object.count) {
		return NULL;
	}
	return value->data.object.entries[index].value;
}

json_value_t* json_object_get(const json_value_t* value, const char* key)
{
	if (!value || value->type != JSON_OBJECT || !key) {
		return NULL;
	}

	for (size_t i = 0; i < value->data.object.count; i++) {
		if (strcmp(value->data.object.entries[i].key, key) == 0) {
			return value->data.object.entries[i].value;
		}
	}
	return NULL;
}


json_value_t* json_get_value(const json_value_t* value, size_t index){
	if(!value || value->type != JSON_OBJECT || index < 0){
		return NULL;
	}
	if(index >= value->data.object.count) return NULL;

	return value->data.object.entries[index].value;
}
