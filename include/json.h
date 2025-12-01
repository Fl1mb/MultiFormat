#ifndef MULTIFORMAT_JSON_H
#define MULTIFORMAT_JSON_H

#include <stddef.h>
#include "../src/core/data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

    // ============================
    // JSON PARSING FUNCTIONS
    // ============================

    /**
     * @brief Parse a JSON string into a data structure
     *
     * @param json_str JSON string to parse (null-terminated)
     * @return json_value_t* Pointer to the root JSON element, NULL on error
     *
     * @details This function parses a JSON string and creates a tree-like
     *          structure in memory. Supports all standard JSON types: objects,
     *          arrays, strings, numbers, booleans, and null.
     *
     * @note Memory must be freed using json_free()
     * @warning The input string must be valid JSON
     *
     * @example
     * @code
     * const char* json = "{\"name\": \"John\", \"age\": 30}";
     * json_value_t* root = json_parse(json);
     * if (root) {
     *     // Work with data
     *     json_free(root);
     * }
     * @endcode
     */
    json_value_t* json_parse(const char* json_str);

    /**
     * @brief Read and parse a JSON file
     *
     * @param filename Path to the JSON file
     * @return json_value_t* Pointer to the root JSON element, NULL on error
     *
     * @details This function reads the entire file content into memory and
     *          parses it as JSON. Automatically detects file encoding (UTF-8).
     *
     * @note Maximum file size is limited by available memory
     * @warning Returns NULL on file read error
     *
     * @example
     * @code
     * json_value_t* config = json_parse_file("config.json");
     * if (config) {
     *     // Read configuration
     *     json_free(config);
     * }
     * @endcode
     */
    json_value_t* json_parse_file(const char* filename);

    // ============================
    // JSON SERIALIZATION FUNCTIONS
    // ============================

    /**
     * @brief Serialize JSON structure into a compact string
     *
     * @param value Pointer to the root JSON element
     * @return char* JSON string (null-terminated), NULL on error
     *
     * @details Converts internal JSON structure to a string in compact format
     *          (without unnecessary spaces and line breaks).
     *
     * @note Caller is responsible for freeing memory with free()
     * @warning For empty or invalid values may return NULL
     *
     * @example
     * @code
     * char* json_str = json_serialize(root);
     * if (json_str) {
     *     printf("JSON: %s\n", json_str);
     *     free(json_str);
     * }
     * @endcode
     */
    char* json_serialize(const json_value_t* value);

    /**
     * @brief Serialize JSON structure into a pretty-printed string
     *
     * @param value Pointer to the root JSON element
     * @return char* Formatted JSON string (null-terminated), NULL on error
     *
     * @details Converts internal JSON structure to a human-readable string
     *          with proper indentation and line breaks. Useful for debugging
     *          and configuration files.
     *
     * @note Caller is responsible for freeing memory with free()
     * @example See json_serialize()
     */
    char* json_serialize_pretty(const json_value_t* value);

    /**
     * @brief Serialize JSON structure and save to a file
     *
     * @param value Pointer to the root JSON element
     * @param filename Path to the output file
     * @return int 0 on success, non-zero error code on failure
     *
     * @details Serializes JSON data and writes it to a file. Uses pretty
     *          printing for better readability. Overwrites existing file.
     *
     * @note Creates file if it doesn't exist
     * @warning Returns error if file cannot be written
     *
     * @example
     * @code
     * if (json_serialize_file(config, "output.json") == 0) {
     *     printf("File saved successfully\n");
     * }
     * @endcode
     */
    int json_serialize_file(const json_value_t* value, const char* filename);

    // ============================
    // UTILITY FUNCTIONS
    // ============================

    /**
     * @brief Free memory allocated for JSON structure
     *
     * @param value Pointer to the root JSON element
     *
     * @details Recursively frees all memory associated with the JSON structure,
     *          including nested objects and arrays.
     *
     * @note Safe to call with NULL
     * @warning Pointer becomes invalid after this call
     *
     * @example
     * @code
     * json_value_t* data = json_parse(json_str);
     * // ... use data ...
     * json_free(data);
     * // data is now invalid
     * @endcode
     */
    void json_free(json_value_t* value);

    /**
     * @brief Get the type of a JSON value
     *
     * @param value Pointer to JSON element
     * @return json_type_t Type enumeration value
     *
     * @details Returns the data type of the JSON value. Use with
     *          json_type_to_string() for human-readable type names.
     *
     * @warning Returns JSON_TYPE_NULL for invalid or NULL input
     *
     * @example
     * @code
     * json_type_t type = json_get_type(value);
     * printf("Type: %s\n", json_type_to_string(type));
     * @endcode
     */
    json_type_t json_get_type(const json_value_t* value);

    /**
     * @brief Convert JSON type enumeration to string
     *
     * @param type JSON type enumeration value
     * @return const char* Human-readable type name
     *
     * @details Returns constant string with type name. Useful for logging
     *          and debugging. Returns "UNKNOWN" for invalid type values.
     *
     * @note The returned string should not be freed
     *
     * @example See json_get_type()
     */
    const char* json_type_to_string(json_type_t type);

    // ============================
    // DATA ACCESS FUNCTIONS
    // ============================

    /**
     * @brief Get boolean value from JSON element
     *
     * @param value Pointer to JSON boolean element
     * @return int 1 for true, 0 for false, -1 for type mismatch or NULL
     *
     * @details Extracts boolean value from JSON element. Returns error
     *          if the element is not of boolean type.
     *
     * @warning Always check return value for errors
     *
     * @example
     * @code
     * int enabled = json_get_boolean(value);
     * if (enabled >= 0) {
     *     printf("Enabled: %s\n", enabled ? "true" : "false");
     * }
     * @endcode
     */
    int json_get_boolean(const json_value_t* value);

    /**
     * @brief Get numeric value from JSON element
     *
     * @param value Pointer to JSON number element
     * @return double Numeric value, 0.0 for type mismatch or NULL
     *
     * @details Extracts double-precision floating point value from JSON
     *          number element. JSON numbers are always parsed as doubles.
     *
     * @warning Check element type before calling
     *
     * @example
     * @code
     * if (json_get_type(value) == JSON_TYPE_NUMBER) {
     *     double price = json_get_number(value);
     *     printf("Price: %.2f\n", price);
     * }
     * @endcode
     */
    double json_get_number(const json_value_t* value);

    /**
     * @brief Get string value from JSON element
     *
     * @param value Pointer to JSON string element
     * @return const char* String value, NULL for type mismatch or NULL input
     *
     * @details Returns pointer to internal string buffer. The string is
     *          null-terminated and should not be modified or freed.
     *
     * @note The pointer is valid until json_free() is called
     * @example See json_parse() example
     */
    const char* json_get_string(const json_value_t* value);

    /**
     * @brief Get size of JSON array
     *
     * @param value Pointer to JSON array element
     * @return size_t Number of elements in array, 0 for type mismatch or NULL
     *
     * @details Returns the number of elements in a JSON array. Use with
     *          json_array_get() to access individual elements.
     *
     * @warning Always verify element type before calling
     *
     * @example
     * @code
     * size_t count = json_get_array_size(array);
     * for (size_t i = 0; i < count; i++) {
     *     json_value_t* element = json_array_get(array, i);
     *     // Process element
     * }
     * @endcode
     */
    size_t json_get_array_size(const json_value_t* value);

    /**
     * @brief Get element from JSON array by index
     *
     * @param value Pointer to JSON array element
     * @param index Zero-based index of element to retrieve
     * @return json_value_t* Pointer to array element, NULL on error
     *
     * @details Returns pointer to JSON element at specified index in array.
     *          The returned pointer is valid until the parent array is freed.
     *
     * @note Does not perform bounds checking - use json_get_array_size() first
     * @warning Returns NULL for invalid index or non-array types
     *
     * @example See json_get_array_size()
     */
    json_value_t* json_array_get(const json_value_t* value, size_t index);

    /**
     * @brief Get size of JSON object (number of key-value pairs)
     *
     * @param value Pointer to JSON object element
     * @return size_t Number of key-value pairs, 0 for type mismatch or NULL
     *
     * @details Returns the number of properties in a JSON object.
     *          Use with json_object_get_key() and json_get_value() to
     *          iterate over object properties.
     *
     * @example
     * @code
     * size_t prop_count = json_object_size(obj);
     * for (size_t i = 0; i < prop_count; i++) {
     *     const char* key = json_object_get_key(obj, i);
     *     json_value_t* val = json_get_value(obj, i);
     *     printf("%s: ...\n", key);
     * }
     * @endcode
     */
    size_t json_object_size(const json_value_t* value);

    /**
     * @brief Get key from JSON object by index
     *
     * @param value Pointer to JSON object element
     * @param index Zero-based index of key to retrieve
     * @return const char* Key string, NULL on error
     *
     * @details Returns the key (property name) at specified index in object.
     *          Keys are returned in the order they were parsed from JSON.
     *
     * @note The string should not be modified or freed
     * @warning Index must be less than json_object_size()
     *
     * @example See json_object_size()
     */
    const char* json_object_get_key(const json_value_t* value, size_t index);

    /**
     * @brief Get value from JSON object by index
     *
     * @param value Pointer to JSON object element
     * @param index Zero-based index of value to retrieve
     * @return json_value_t* Pointer to value, NULL on error
     *
     * @details Returns the value associated with the key at specified index.
     *          Equivalent to json_object_get(obj, json_object_get_key(obj, index)).
     *
     * @note More efficient than json_object_get() when iterating
     * @warning Index must be less than json_object_size()
     *
     * @example See json_object_size()
     */
    json_value_t* json_get_value(const json_value_t* value, size_t index);

    /**
     * @brief Get value from JSON object by key name
     *
     * @param value Pointer to JSON object element
     * @param key Property name to look up
     * @return json_value_t* Pointer to value, NULL if key not found
     *
     * @details Searches for a property with the specified key name in the
     *          JSON object. Key comparison is case-sensitive.
     *
     * @note Returns NULL if key doesn't exist
     * @example
     * @code
     * json_value_t* age = json_object_get(person, "age");
     * if (age && json_get_type(age) == JSON_TYPE_NUMBER) {
     *     printf("Age: %.0f\n", json_get_number(age));
     * }
     * @endcode
     */
    json_value_t* json_object_get(const json_value_t* value, const char* key);

#ifdef __cplusplus
}
#endif

#endif // MULTIFORMAT_JSON_H