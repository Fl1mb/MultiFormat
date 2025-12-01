#ifndef MULTIFORMAT_CSV_PARSER_H
#define MULTIFORMAT_CSV_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "../core/data_types.h"


typedef struct {
	char delimeter;
	char quote_char;
	bool trim_spaces;
	bool skip_empty;
	bool has_header;
}CSVParserConfig;

char* read_file(FILE* file);
char* trim_string(char* str);
CSVRows parse_csv_line(const char* line, const CSVParserConfig* config);
void free_csv(CSVRows* row);


#endif // MULTIFORMAT_CSV_PARSER_H
