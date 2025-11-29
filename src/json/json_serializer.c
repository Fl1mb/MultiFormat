#include "json_serializer.h"


void serializer_init(json_serializer_t* serializer, int pretty)
{
	serializer->buffer = malloc(JSON_SERIALIZER_INIT_SIZE);
	serializer->capacity = JSON_SERIALIZER_INIT_SIZE;
	serializer->length = 0;
	serializer->pretty = pretty;
	serializer->indent_level = 0;

	if (serializer->buffer) {
		serializer->buffer[0] = '\0';
	}
}

void serializer_free(json_serializer_t* serializer)
{
	if (!serializer)return;
	if (serializer->buffer) {
		free(serializer->buffer);
	}
}

int serializer_ensure_capacity(json_serializer_t* serializer, size_t needed)
{
	if (serializer->length + needed + 1 >= serializer->capacity) {
		size_t new_capacity = serializer->capacity * 2;
		while (serializer->length + needed + 1 >= new_capacity) {
			new_capacity *= 2;
		}

		char* new_buffer = realloc(serializer->buffer, new_capacity);
		if (!new_buffer) {
			return 0;
		}

		serializer->buffer = new_buffer;
		serializer->capacity = new_capacity;
	}
	return 1;
}

int serializer_append(json_serializer_t* serializer, const char* str)
{
	size_t len = strlen(str);
	if (!serializer_ensure_capacity(serializer, len)) {
		return 0;
	}

	memcpy(serializer->buffer + serializer->length, str, len);
	serializer->length += len;
	serializer->buffer[serializer->length] = '\0';
	
	return 1;
}

int serializer_append_indent(json_serializer_t* serializer)
{
	if (!serializer->pretty)return 1;

	int indent_spaces = serializer->indent_level * JSON_INDENT_SIZE;
	if (!serializer_ensure_capacity(serializer, indent_spaces + 1)) {
		return 0;
	}

	for (int i = 0; i < indent_spaces; i++) {
		serializer->buffer[serializer->length++] = ' ';
	}
	serializer->buffer[serializer->length] = '\0';
	return 1;
}

int serializer_append_char(json_serializer_t* serializer, char c)
{
	if (!serializer_ensure_capacity(serializer, 1)) {
		return 0;
	}

	serializer->buffer[serializer->length++] = c;
	serializer->buffer[serializer->length] = '\0';
	return 1;
}

char* escape_string(const char* str)
{
	if (!str) return NULL;

	size_t len = strlen(str);
	size_t escaped_len = 0;

	for (size_t i = 0; i < len; i++) {
		switch (str[i]) {
		case '"':  escaped_len += 2; break;  // \"
		case '\\': escaped_len += 2; break;  // 
		case '\b': escaped_len += 2; break;  // \b
		case '\f': escaped_len += 2; break;  // \f
		case '\n': escaped_len += 2; break;  // \n
		case '\r': escaped_len += 2; break;  // \r
		case '\t': escaped_len += 2; break;  // \t
		default:
			if ((unsigned char)str[i] < 0x20) {
				escaped_len += 6; // \uXXXX
			}
			else {
				escaped_len += 1;
			}
			break;
		}
	}

	char* escaped = malloc(escaped_len + 1);
	if (!escaped) return NULL;

	size_t pos = 0;
	for (size_t i = 0; i < len; i++) {
		switch (str[i]) {
		case '"':
			escaped[pos++] = '\\'; escaped[pos++] = '"'; break;
		case '\\':
			escaped[pos++] = '\\'; escaped[pos++] = '\\'; break;
		case '\b':
			escaped[pos++] = '\\'; escaped[pos++] = 'b'; break;
		case '\f':
			escaped[pos++] = '\\'; escaped[pos++] = 'f'; break;
		case '\n':
			escaped[pos++] = '\\'; escaped[pos++] = 'n'; break;
		case '\r':
			escaped[pos++] = '\\'; escaped[pos++] = 'r'; break;
		case '\t':
			escaped[pos++] = '\\'; escaped[pos++] = 't'; break;
		default:
			if ((unsigned char)str[i] < 0x20) {
				// Управляющие символы - экранируем как \u00XX
				snprintf(escaped + pos, 7, "\\u%04x", (unsigned char)str[i]);
				pos += 6;
			}
			else {
				escaped[pos++] = str[i];
			}
			break;
		}
	}
	escaped[pos] = '\0';
	return escaped;
}

int serialize_value(json_serializer_t* serializer, const json_value_t* value)
{
	if (!value) return serialize_null(serializer);

	switch (value->type) {
	case JSON_NULL:
		return serialize_null(serializer);
	case JSON_BOOL:
		return serialize_boolean(serializer, value);
	case JSON_NUMBER:
		return serialize_number(serializer, value);
	case JSON_STRING:
		return serialize_string(serializer, value);
	case JSON_ARRAY:
		return serialize_array(serializer, value);
	case JSON_OBJECT:
		return serialize_object(serializer, value);
	default:
		return 0;
	}
}

int serialize_null(json_serializer_t* serializer)
{
	return serializer_append(serializer, "null");
}

int serialize_boolean(json_serializer_t* serializer, const json_value_t* value)
{
	return serializer_append(serializer, value->data.boolean ? "true" : "false");
}

int serialize_number(json_serializer_t* serializer, const json_value_t* value)
{
	char* buffer[64];

	double num = value->data.number;
	if (num == (long long)num) {
		snprintf(buffer, sizeof(buffer), "%lld", (long long)num);
	}
	else {
		snprintf(buffer, sizeof(buffer), "%.15g", num);
	}
	return serializer_append(serializer, buffer);
}

int serialize_string(json_serializer_t* serializer, const json_value_t* value)
{
	char* escaped = escape_string(value->data.string);
	if (!escaped) return 0;

	int result = serializer_append_char(serializer, '"') &&
		serializer_append(serializer, escaped) &&
		serializer_append_char(serializer, '"');

	free(escaped);
	return result;
}

int serialize_array(json_serializer_t* serializer, const json_value_t* value)
{
	if (!serializer_append_char(serializer, '[')) return 0;

	if (serializer->pretty && value->data.array.count > 0) {
		serializer->indent_level++;
		if (!serializer_append_char(serializer, '\n')) return 0;
	}

	for (size_t i = 0; i < value->data.array.count; i++) {
		if (i > 0) {
			if (!serializer_append_char(serializer, ',')) return 0;
			if (serializer->pretty) {
				if (!serializer_append_char(serializer, '\n')) return 0;
			}
		}

		if (serializer->pretty) {
			if (!serializer_append_indent(serializer)) return 0;
		}

		if (!serialize_value(serializer, value->data.array.values[i])) {
			return 0;
		}
	}

	if (serializer->pretty && value->data.array.count > 0) {
		serializer->indent_level--;
		if (!serializer_append_char(serializer, '\n')) return 0;
		if (!serializer_append_indent(serializer)) return 0;
	}

	return serializer_append_char(serializer, ']');
}

int serialize_object(json_serializer_t* serializer, const json_value_t* value)
{
	if (!serializer_append_char(serializer, '{')) return 0;

	if (serializer->pretty && value->data.object.count > 0) {
		serializer->indent_level++;
		if (!serializer_append_char(serializer, '\n')) return 0;
	}

	for (size_t i = 0; i < value->data.object.count; i++) {
		if (i > 0) {
			if (!serializer_append_char(serializer, ',')) return 0;
			if (serializer->pretty) {
				if (!serializer_append_char(serializer, '\n')) return 0;
			}
		}

		if (serializer->pretty) {
			if (!serializer_append_indent(serializer)) return 0;
		}

		// Ключ
		char* escaped_key = escape_string(value->data.object.entries[i].key);
		if (!escaped_key) return 0;

		if (!serializer_append_char(serializer, '"') ||
			!serializer_append(serializer, escaped_key) ||
			!serializer_append_char(serializer, '"')) {
			free(escaped_key);
			return 0;
		}
		free(escaped_key);

		if (serializer->pretty) {
			if (!serializer_append(serializer, ": ")) return 0;
		}
		else {
			if (!serializer_append_char(serializer, ':')) return 0;
		}

		// Значение
		if (!serialize_value(serializer, value->data.object.entries[i].value)) {
			return 0;
		}
	}

	if (serializer->pretty && value->data.object.count > 0) {
		serializer->indent_level--;
		if (!serializer_append_char(serializer, '\n')) return 0;
		if (!serializer_append_indent(serializer)) return 0;
	}

	return serializer_append_char(serializer, '}');
}
















