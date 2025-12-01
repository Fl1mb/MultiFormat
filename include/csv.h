#ifndef MULTIFORMAT_CSV_H
#define MULTIFORMAT_CSV_H

#include "../src/csv/csv_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Read and parse a CSV file with custom delimiter
     *
     * @param filename Path to the CSV file to read
     * @param config Parser configuration (delimiter, quotes, options)
     * @return CSVData* Pointer to structure with parsed data, NULL on error
     *
     * @details This function reads the entire CSV file into memory and parses it
     *          according to the specified configuration. Supports various delimiters
     *          (comma, semicolon, tab, etc.), quotes, and multiline fields.
     *          Memory must be freed using free_csv_data().
     *
     * @note Example usage:
     * @code
     * CSVParserConfig config = {',', '"', true, true, true};
     * CSVData* data = parse_csv_file("data.csv", &config);
     * if (data) {
     *     // Process data
     *     free_csv_data(data);
     * }
     * @endcode
     */
    CSVData* parse_csv_file(const char* filename, const CSVParserConfig* config);

    /**
     * @brief Free memory allocated for CSV data
     *
     * @param data Pointer to CSVData structure to free
     *
     * @details This function recursively frees all memory allocated during CSV
     *          file parsing. Safe to pass NULL.
     *
     * @warning The pointer becomes invalid after calling this function
     */
    void free_csv_data(CSVData* data);

    /**
     * @brief Search for rows by value in a specified field
     *
     * @param data CSV data to search in
     * @param field_index Index of the field to search (0-based)
     * @param value String value to search for
     * @return int Number of matches found, -1 on error
     *
     * @details This function performs exact string matching of the value in the
     *          specified field across all CSV rows. Search is case-sensitive.
     *          If a row has fewer fields than field_index, it is ignored.
     *
     * @note Example:
     * @code
     * // Search for all records with age 30
     * int count = search_in_csv(data, 1, "30");
     * printf("Found %d people aged 30\n", count);
     * @endcode
     */
    int search_in_csv(const CSVData* data, int field_index, const char* value);

    /**
     * @brief Export CSV data to a file with specified delimiter
     *
     * @param data CSV data to export
     * @param filename Output file name
     * @param delimiter Delimiter to use in the output file
     *
     * @details This function saves CSV data to a new file with the given delimiter.
     *          Automatically escapes quotes and delimiters in fields when necessary.
     *          If the file exists, it will be overwritten.
     *
     * @note Examples:
     * @code
     * // Export to standard CSV
     * export_csv(data, "output.csv", ',');
     * // Export to TSV (tab-separated)
     * export_csv(data, "data.tsv", '\t');
     * // Export to semicolon-separated format
     * export_csv(data, "european.csv", ';');
     * @endcode
     */
    void export_csv(const CSVData* data, const char* filename, char delimiter);

    /**
    * @brief Get the header row of CSV data
    *
    * @param data Pointer to CSVData structure containing parsed CSV data
    * @return CSVRows* Pointer to the header row, NULL if no header exists or data is NULL
    *
    * @details This function returns a pointer to the CSV header row if it exists.
    *          The header is stored separately from data rows when has_header is true
    *          in the parser configuration.
    *
    * @note The returned pointer should not be freed separately.
    *       It will be freed when free_csv_data() is called.
    *
    * @warning Modifying the returned header structure may corrupt the CSV data.
    *
    * @example
    * @code
    * CSVParserConfig config = {',', '"', true, true, true};
    * CSVData* data = parse_csv_file("data.csv", &config);
    * if (data) {
    *     CSVRows* header = csv_get_header(data);
    *     if (header) {
    *         printf("Header has %d fields\n", header->count);
    *     }
    *     free_csv_data(data);
    * }
    * @endcode
    */
    CSVRows* csv_get_header(CSVData* data);

    /**
     * @brief Get the name of a field by its index
     *
     * @param data Pointer to CSVData structure containing parsed CSV data
     * @param field_index Zero-based index of the field in the header
     * @return const char* Field name string, NULL if no header, invalid index, or data is NULL
     *
     * @details This function retrieves the name of a CSV field from the header
     *          by its index. Useful for displaying column names or mapping
     *          field indices to human-readable names.
     *
     * @note Returns NULL if the CSV has no header or the index is out of bounds.
     *       The returned string should not be modified or freed.
     *
     * @example
     * @code
     * // Get the name of the third field (index 2)
     * const char* field_name = csv_get_field_name(data, 2);
     * if (field_name) {
     *     printf("Third field is called: %s\n", field_name);
     * }
     * @endcode
     */
    const char* csv_get_field_name(const CSVData* data, int field_index);

    /**
     * @brief Find the index of a field by its name
     *
     * @param data Pointer to CSVData structure containing parsed CSV data
     * @param field_name Name of the field to find (case-sensitive)
     * @return int Zero-based index of the field, -1 if not found or no header
     *
     * @details This function searches for a field name in the CSV header and
     *          returns its index. Useful for converting field names to indices
     *          for use with other CSV functions.
     *
     * @note Search is case-sensitive. Returns -1 if the field is not found
     *       or if the CSV has no header.
     *
     * @example
     * @code
     * int age_index = can_find_field_index(data, "age");
     * if (age_index != -1) {
     *     printf("Age field is at index %d\n", age_index);
     *     // Now you can use age_index with other functions
     *     int count = search_in_csv_by_index(data, age_index, "30");
     * }
     * @endcode
     */
    int can_find_field_index(const CSVData* data, const char* field_name);

    /**
     * @brief Search for rows by value in a specified field index
     *
     * @param data Pointer to CSVData structure containing parsed CSV data
     * @param field_index Zero-based index of the field to search
     * @param value String value to search for (case-sensitive)
     * @return int Number of matching rows found, 0 if none found or invalid parameters
     *
     * @details This function performs exact string matching in the specified field
     *          across all data rows. Returns the count of rows where the field value
     *          matches the search value exactly.
     *
     * @note Search is case-sensitive. Rows with fewer fields than field_index
     *       are ignored. Returns 0 for invalid parameters or no matches.
     *
     * @example
     * @code
     * // Search for rows where the second field (index 1) equals "New York"
     * int matches = search_in_csv_by_index(data, 1, "New York");
     * printf("Found %d people in New York\n", matches);
     * @endcode
     */
    int search_in_csv_by_index(const CSVData* data, int field_index, const char* value);

    /**
     * @brief Search for rows by value in a specified field name
     *
     * @param data Pointer to CSVData structure containing parsed CSV data
     * @param field_name Name of the field to search (case-sensitive)
     * @param value String value to search for (case-sensitive)
     * @return int Number of matching rows found, 0 if none found or invalid parameters
     *
     * @details This function first finds the index of the specified field name
     *          in the header, then searches for the value in that field across
     *          all data rows. Combines can_find_field_index() and search_in_csv_by_index().
     *
     * @note Both field name and value comparisons are case-sensitive.
     *       Returns 0 if the field name is not found in the header.
     *
     * @example
     * @code
     * // Search for rows where the "age" field equals "30"
     * int matches = search_in_csv_by_name(data, "age", "30");
     * printf("Found %d people aged 30\n", matches);
     * @endcode
     */
    int search_in_csv_by_name(const CSVData* data, const char* field_name, const char* value);
#ifdef __cplusplus
}
#endif // cplusplus

#endif // MULTIFORMAT_CSV_H