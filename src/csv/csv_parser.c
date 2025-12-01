#include "csv_parser.h"

char* read_file(FILE* file)
{
	size_t capacity = 128;
	size_t length = 0;

	char* buffer = malloc(capacity);

	if (!buffer)return NULL;

	int ch;
	while ((ch = fgetc(file)) != EOF && ch != '\n') {
		if ((length + 1) >= capacity) {
			capacity *= 2;
			char* new_buffer = realloc(buffer, capacity);
			if (!new_buffer) {
				free(buffer);
				return NULL;
			}
			buffer = new_buffer;
		}
		buffer[length++] = (char)ch;
	}
	if (length == 0 && ch == EOF) {
		free(buffer);
		return NULL;
	}
	buffer[length] = '\0';

	if (length > 0 && buffer[length - 1] == '\r') {
		buffer[length - 1] = '\0';
		length--;
	}

	return buffer;
}

char* trim_string(char* str)
{
	if (!str)return NULL;

	while (isspace((unsigned char)*str)) {
		str++;
	}
	if (*str == '\0')return str;

	char* end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) {
		end--;
	}
	*(end + 1) = '\0';
	return str;
}

CSVRows parse_csv_line(const char* line, const CSVParserConfig* config)
{
	CSVRows row = { NULL, 0 };

	if (!line || strlen(line) == 0) {
		return row;
	}

	char** fields = malloc(sizeof(char*));
	if (!fields)return row;

	int capacity = 1;
	int field_count = 0;

	char* current = line;
	bool in_quotes = false;
	char* field_start = current;

	while (*current) {
		if (*current == config->quote_char) {
			in_quotes = !in_quotes;
		}
		else if (*current == config->delimeter && !in_quotes) {
			*current = '\0';

			char* field_value = field_start;

			if (field_value[0] == config->quote_char &&
				field_value[strlen(field_value) - 1] == config->quote_char) {
				field_value[strlen(field_value) - 1] = '\0';
				field_value++;
			}

			if (config->trim_spaces) {
				field_value = trim_string(field_value);
			}

			if (field_count >= capacity) {
				capacity *= 2;
				char** new_fields = realloc(fields, capacity * sizeof(char*));
				if (!new_fields) {
					for (int i = 0; i < field_count; i++) {
						free(fields[i]);
					}
					free(fields);
					return row;
				}
				fields = new_fields;
			}

			fields[field_count] = malloc(strlen(field_value) + 1);
			strcpy(fields[field_count], field_value);
			field_count++;

			field_start = current + 1;
		}
		current++;
	}

	if (field_start <= current) {
		char* field_value = field_start;

		if (field_value[0] == config->quote_char &&
			field_value[strlen(field_value) - 1] == config->quote_char) {
			field_value[strlen(field_value) - 1] = '\0';
			field_value++;
		}

		if (config->trim_spaces) {
			field_value = trim_string(field_value);
		}

		if (field_count >= capacity) {
			capacity++;
			char** new_fields = realloc(fields, capacity * sizeof(char*));
			if (!new_fields) {
				for (int i = 0; i < field_count; i++) {
					free(fields[i]);
				}
				free(fields);
				return row;
			}
			fields = new_fields;
		}

		fields[field_count] = malloc(strlen(field_value) + 1);
		strcpy(fields[field_count], field_value);
		field_count++;
	}

	if (field_count > 0) {
		char** resized = realloc(fields, field_count * sizeof(char*));
		if (resized) {
			fields = resized;
		}
	}
	else {
		free(fields);
		fields = NULL;
	}

	row.fields = fields;
	row.count = field_count;
	return row;
}

void free_csv(CSVRows* row)
{
	if (!row) return;

	for (int i = 0; i < row->count; i++) {
		free(row->fields[i]);
	}
	free(row->fields);

	row->fields = NULL;
	row->count = 0;
}




