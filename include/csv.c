#include "csv.h"

CSVData* parse_csv_file(const char* filename, const CSVParserConfig* config)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Ошибка: Не удалось открыть файл '%s'\n", filename);
        return NULL;
    }

    CSVData* data = malloc(sizeof(CSVData));
    if (!data) {
        fclose(file);
        return NULL;
    }

    data->rows = NULL;
    data->row_count = 0;
    data->max_fields = 0;
    data->header.fields = NULL;
    data->header.count = 0;
    data->have_header = false;

    CSVRows* rows = NULL;
    int capacity = 0;
    int row_count = 0;
    int line_number = 0;


    if (config->has_header) {
        char* header_line = read_file(file);
        if (header_line) {
            printf("Header: %s\n", header_line);

            CSVRows header_row = parse_csv_line(header_line, config);
            data->header = header_row;
            data->have_header = true;

            free(header_line);
        }
        else {
            fclose(file);
            free(data);
            return NULL;
        }
        line_number++;
    }

    while (1) {
        char* line = read_file(file);
        if (!line) {
            break;
        }

        line_number++;

        if (config->skip_empty) {
            char* trimmed = trim_string(line);
            if (strlen(trimmed) == 0) {
                free(line);
                continue;
            }
        }

        CSVRows row = parse_csv_line(line, config);
        free(line);

        if (row.fields == NULL) {
            continue;
        }
        if (row_count >= capacity) {
            capacity = capacity == 0 ? 16 : capacity * 2;
            CSVRows* new_rows = realloc(rows, capacity * sizeof(CSVRows));
            if (!new_rows) {
                fprintf(stderr, "Error: Memory allocation at line %d\n", line_number);
                free_csv(&row);
                break;
            }
            rows = new_rows;
        }
        rows[row_count] = row;
        row_count++;

        if (row.count > data->max_fields) {
            data->max_fields = row.count;
        }
    }

    fclose(file);

    data->rows = rows;
    data->row_count = row_count;

    return data;
}

void free_csv_data(CSVData* data)
{
    if (!data) return;

    free_csv(&data->header);

    for (int i = 0; i < data->row_count; i++) {
        free_csv(&data->rows[i]);
    }

    free(data->rows);
    free(data);
}

int search_in_csv(const CSVData* data, int field_index, const char* value)
{
    return search_in_csv_by_index(data, field_index, value);
}

void export_csv(const CSVData* data, const char* filename, char delimiter)
{
    if (!data || !filename) {
        fprintf(stderr, "Error: NULL parameters\n");
        return;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", filename);
        return;
    }
    if (data->have_header && data->header.fields) {
        for (int j = 0; j < data->header.count; j++) {
            const char* field = data->header.fields[j];
            if (!field) field = "";

            int needs_quotes = strchr(field, delimiter) || strchr(field, '"');

            if (needs_quotes) {
                fprintf(file, "\"");
                for (const char* p = field; *p; p++) {
                    if (*p == '"') fprintf(file, "\"\"");
                    else fputc(*p, file);
                }
                fprintf(file, "\"");
            }
            else {
                fprintf(file, "%s", field);
            }

            if (j < data->header.count - 1) {
                fputc(delimiter, file);
            }
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < data->row_count; i++) {
        const CSVRows* row = &data->rows[i];

        if (!row || !row->fields) {
            fprintf(file, "\n");
            continue;
        }

        for (int j = 0; j < row->count; j++) {
            const char* field = row->fields[j];
            if (!field) field = "";

            int needs_quotes = strchr(field, delimiter) || strchr(field, '"') ||
                strchr(field, '\n') || strchr(field, '\r');

            if (needs_quotes) {
                fprintf(file, "\"");
                for (const char* p = field; *p; p++) {
                    if (*p == '"') fprintf(file, "\"\"");
                    else fputc(*p, file);
                }
                fprintf(file, "\"");
            }
            else {
                fprintf(file, "%s", field);
            }

            if (j < row->count - 1) {
                fputc(delimiter, file);
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

CSVRows* csv_get_header(CSVData* data)
{
    if (!data || !data->have_header)return NULL;
    return &data->header;
}

const char* csv_get_field_name(const CSVData* data, int field_index)
{
    if (!data || !data->have_header || field_index < 0) return NULL;
    if (field_index >= data->header.count) return NULL;
    return data->header.fields[field_index];
}

int can_find_field_index(const CSVData* data, const char* field_name)
{
    if (!data || !data->have_header || !field_name) return -1;

    for (int i = 0; i < data->header.count; i++) {
        if (data->header.fields[i] &&
            strcmp(data->header.fields[i], field_name) == 0) {
            return i;
        }
    }
    return -1;
}

int search_in_csv_by_index(const CSVData* data, int field_index, const char* value)
{
    if (!data || !value || field_index < 0) return 0;

    int found = 0;

    for (int i = 0; i < data->row_count; ++i) {
        const CSVRows* row = &data->rows[i];

        if (!row || !row->fields) continue;
        if (field_index >= row->count ) continue;
        if (!row->fields[field_index]) continue;

        if (strcmp(row->fields[field_index], value) == 0) {
            found++;
        }
    }
    return found;
}

int search_in_csv_by_name(const CSVData* data, const char* field_name, const char* value)
{
    if (!data || !field_name || !value) return 0;

    int field_index = can_find_field_index(data, field_name);
    if (field_index == -1) return 0;

    return search_in_csv_by_index(data, field_index, value);
}



