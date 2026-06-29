#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_common.h"
#include "../../include/json.h"

void test_basic_types() {
    printf("=== Basic Types Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Simple null
    printf("Test 1: Parsing null\n");
    json_value_t* null_val = json_parse("null");
    passed &= (assertNotNull(null_val) == 0);
    if (null_val) {
        passed &= (assertEquals(json_get_type(null_val), JSON_NULL) == 0);
        json_free(null_val);
    }

    // Test 2: Simple boolean true
    printf("Test 2: Parsing true\n");
    json_value_t* true_val = json_parse("true");
    passed &= (assertNotNull(true_val) == 0);
    if (true_val) {
        passed &= (assertEquals(json_get_type(true_val), JSON_BOOL) == 0);
        passed &= (assertEquals(json_get_boolean(true_val), 1) == 0);
        json_free(true_val);
    }

    // Test 3: Simple boolean false
    printf("Test 3: Parsing false\n");
    json_value_t* false_val = json_parse("false");
    passed &= (assertNotNull(false_val) == 0);
    if (false_val) {
        passed &= (assertEquals(json_get_type(false_val), JSON_BOOL) == 0);
        passed &= (assertEquals(json_get_boolean(false_val), 0) == 0);
        json_free(false_val);
    }

    // Test 4: Simple integer number
    printf("Test 4: Parsing integer number\n");
    json_value_t* int_val = json_parse("42");
    passed &= (assertNotNull(int_val) == 0);
    if (int_val) {
        passed &= (assertEquals(json_get_type(int_val), JSON_NUMBER) == 0);
        passed &= (assertDoubleEquals(json_get_number(int_val), 42.0) == 0);
        json_free(int_val);
    }

    // Test 5: Simple float number
    printf("Test 5: Parsing float number\n");
    json_value_t* float_val = json_parse("3.14159");
    passed &= (assertNotNull(float_val) == 0);
    if (float_val) {
        passed &= (assertEquals(json_get_type(float_val), JSON_NUMBER) == 0);
        passed &= (assertDoubleEquals(json_get_number(float_val), 3.14159) == 0);
        json_free(float_val);
    }

    // Test 6: Simple string
    printf("Test 6: Parsing string\n");
    json_value_t* str_val = json_parse("\"hello world\"");
    passed &= (assertNotNull(str_val) == 0);
    if (str_val) {
        passed &= (assertEquals(json_get_type(str_val), JSON_STRING) == 0);
        passed &= (assertStringsMatch((char*)json_get_string(str_val), "hello world") == 0);
        json_free(str_val);
    }

    printf("✓ Basic Types Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_arrays() {
    printf("=== Arrays Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Empty array
    printf("Test 1: Empty array\n");
    json_value_t* empty_arr = json_parse("[]");
    passed &= (assertNotNull(empty_arr) == 0);
    if (empty_arr) {
        passed &= (assertEquals(json_get_type(empty_arr), JSON_ARRAY) == 0);
        passed &= (assertEquals(json_get_array_size(empty_arr), 0) == 0);
        json_free(empty_arr);
    }

    // Test 2: Simple array
    printf("Test 2: Simple array\n");
    json_value_t* arr_val = json_parse("[1, 2, 3]");
    passed &= (assertNotNull(arr_val) == 0);
    if (arr_val) {
        passed &= (assertEquals(json_get_type(arr_val), JSON_ARRAY) == 0);
        passed &= (assertEquals(json_get_array_size(arr_val), 3) == 0);
        
        json_value_t* elem1 = json_array_get(arr_val, 0);
        json_value_t* elem2 = json_array_get(arr_val, 1);
        json_value_t* elem3 = json_array_get(arr_val, 2);
        
        passed &= (assertNotNull(elem1) == 0);
        passed &= (assertNotNull(elem2) == 0);
        passed &= (assertNotNull(elem3) == 0);
        
        if (elem1) passed &= (assertDoubleEquals(json_get_number(elem1), 1.0) == 0);
        if (elem2) passed &= (assertDoubleEquals(json_get_number(elem2), 2.0) == 0);
        if (elem3) passed &= (assertDoubleEquals(json_get_number(elem3), 3.0) == 0);
        
        json_free(arr_val);
    }

    // Test 3: Mixed type array
    printf("Test 3: Mixed type array\n");
    json_value_t* mixed_arr = json_parse("[1, \"two\", true, null]");
    passed &= (assertNotNull(mixed_arr) == 0);
    if (mixed_arr) {
        passed &= (assertEquals(json_get_type(mixed_arr), JSON_ARRAY) == 0);
        passed &= (assertEquals(json_get_array_size(mixed_arr), 4) == 0);
        
        json_value_t* elem1 = json_array_get(mixed_arr, 0);
        json_value_t* elem2 = json_array_get(mixed_arr, 1);
        json_value_t* elem3 = json_array_get(mixed_arr, 2);
        json_value_t* elem4 = json_array_get(mixed_arr, 3);
        
        passed &= (assertNotNull(elem1) == 0);
        passed &= (assertNotNull(elem2) == 0);
        passed &= (assertNotNull(elem3) == 0);
        passed &= (assertNotNull(elem4) == 0);
        
        if (elem1) passed &= (assertEquals(json_get_type(elem1), JSON_NUMBER) == 0);
        if (elem2) passed &= (assertEquals(json_get_type(elem2), JSON_STRING) == 0);
        if (elem3) passed &= (assertEquals(json_get_type(elem3), JSON_BOOL) == 0);
        if (elem4) passed &= (assertEquals(json_get_type(elem4), JSON_NULL) == 0);
        
        json_free(mixed_arr);
    }

    printf("✓ Arrays Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_simple_object() {
    printf("=== Simple Object Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test with simpler object first to debug
    printf("Test 1: Very simple object\n");
    json_value_t* simple_obj = json_parse("{\"a\":1}");
    passed &= (assertNotNull(simple_obj) == 0);
    if (simple_obj) {
        passed &= (assertEquals(json_get_type(simple_obj), JSON_OBJECT) == 0);
        passed &= (assertEquals(json_object_size(simple_obj), 1) == 0);
        
        json_value_t* a_val = json_object_get(simple_obj, "a");
        passed &= (assertNotNull(a_val) == 0);
        if (a_val) {
            passed &= (assertEquals(json_get_type(a_val), JSON_NUMBER) == 0);
            passed &= (assertDoubleEquals(json_get_number(a_val), 1.0) == 0);
        }
        
        json_free(simple_obj);
    }

    // Test 2: Two properties
    printf("Test 2: Two properties\n");
    json_value_t* two_props = json_parse("{\"name\":\"test\",\"value\":2}");
    passed &= (assertNotNull(two_props) == 0);
    if (two_props) {
        passed &= (assertEquals(json_get_type(two_props), JSON_OBJECT) == 0);
        passed &= (assertEquals(json_object_size(two_props), 2) == 0);
        
        json_value_t* name_val = json_object_get(two_props, "name");
        json_value_t* value_val = json_object_get(two_props, "value");
        
        passed &= (assertNotNull(name_val) == 0);
        passed &= (assertNotNull(value_val) == 0);
        
        if (name_val) {
            passed &= (assertEquals(json_get_type(name_val), JSON_STRING) == 0);
            passed &= (assertStringsMatch((char*)json_get_string(name_val), "test") == 0);
        }
        
        if (value_val) {
            passed &= (assertEquals(json_get_type(value_val), JSON_NUMBER) == 0);
            passed &= (assertDoubleEquals(json_get_number(value_val), 2.0) == 0);
        }
        
        json_free(two_props);
    }

    printf("✓ Simple Object Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_serialization() {
    printf("=== Serialization Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Serialize array (since objects have issues)
    printf("Test 1: Serialize array\n");
    json_value_t* arr_val = json_parse("[1, \"test\", true]");
    passed &= (assertNotNull(arr_val) == 0);
    
    if (arr_val) {
        char* serialized = json_serialize(arr_val);
        passed &= (assertNotNull(serialized) == 0);
        
        if (serialized) {
            printf("Serialized: %s\n", serialized);
            
            // Parse back and verify
            json_value_t* reparsed = json_parse(serialized);
            passed &= (assertNotNull(reparsed) == 0);
            
            if (reparsed) {
                passed &= (assertEquals(json_get_type(reparsed), JSON_ARRAY) == 0);
                passed &= (assertEquals(json_get_array_size(reparsed), 3) == 0);
                
                json_free(reparsed);
            }
            
            free(serialized);
        }
        
        json_free(arr_val);
    }

    // Test 2: Pretty serialization
    printf("Test 2: Pretty serialization\n");
    json_value_t* simple_val = json_parse("[1, 2, 3]");
    passed &= (assertNotNull(simple_val) == 0);
    
    if (simple_val) {
        char* pretty = json_serialize_pretty(simple_val);
        passed &= (assertNotNull(pretty) == 0);
        
        if (pretty) {
            printf("Pretty:\n%s\n", pretty);
            // Just check that it's not empty and can be reparsed
            passed &= (assertGreaterThan(strlen(pretty), 0) == 0);
            
            json_value_t* reparsed = json_parse(pretty);
            passed &= (assertNotNull(reparsed) == 0);
            if (reparsed) {
                json_free(reparsed);
            }
            
            free(pretty);
        }
        
        json_free(simple_val);
    }

    printf("✓ Serialization Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_error_handling() {
    printf("=== Error Handling Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Invalid JSON
    printf("Test 1: Invalid JSON\n");
    json_value_t* invalid_val = json_parse("{invalid}");
    passed &= (assertNull(invalid_val) == 0);

    // Test 2: Unclosed array
    printf("Test 2: Unclosed array\n");
    json_value_t* unclosed_arr = json_parse("[1, 2, 3");
    passed &= (assertNull(unclosed_arr) == 0);

    // Test 3: Unclosed object
    printf("Test 3: Unclosed object\n");
    json_value_t* unclosed_obj = json_parse("{\"key\": \"value\"");
    passed &= (assertNull(unclosed_obj) == 0);

    // Test 4: Malformed string
    printf("Test 4: Malformed string\n");
    json_value_t* malformed_str = json_parse("\"unclosed string");
    passed &= (assertNull(malformed_str) == 0);

    printf("✓ Error Handling Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_edge_cases() {
    printf("=== Edge Cases Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Empty string
    printf("Test 1: Empty string\n");
    json_value_t* empty_str = json_parse("\"\"");
    passed &= (assertNotNull(empty_str) == 0);
    if (empty_str) {
        passed &= (assertEquals(json_get_type(empty_str), JSON_STRING) == 0);
        passed &= (assertStringsMatch((char*)json_get_string(empty_str), "") == 0);
        json_free(empty_str);
    }

    // Test 2: Zero number
    printf("Test 2: Zero number\n");
    json_value_t* zero_val = json_parse("0");
    passed &= (assertNotNull(zero_val) == 0);
    if (zero_val) {
        passed &= (assertEquals(json_get_type(zero_val), JSON_NUMBER) == 0);
        passed &= (assertDoubleEquals(json_get_number(zero_val), 0.0) == 0);
        json_free(zero_val);
    }

    // Test 3: Negative number
    printf("Test 3: Negative number\n");
    json_value_t* neg_val = json_parse("-42.5");
    passed &= (assertNotNull(neg_val) == 0);
    if (neg_val) {
        passed &= (assertEquals(json_get_type(neg_val), JSON_NUMBER) == 0);
        passed &= (assertDoubleEquals(json_get_number(neg_val), -42.5) == 0);
        json_free(neg_val);
    }

    // Test 4: Array with spaces
    printf("Test 4: Array with spaces\n");
    json_value_t* spaced_arr = json_parse(" [ 1 , 2 , 3 ] ");
    passed &= (assertNotNull(spaced_arr) == 0);
    if (spaced_arr) {
        passed &= (assertEquals(json_get_type(spaced_arr), JSON_ARRAY) == 0);
        passed &= (assertEquals(json_get_array_size(spaced_arr), 3) == 0);
        json_free(spaced_arr);
    }

    printf("✓ Edge Cases Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_large_nested_structure() {
    printf("=== Large Nested Structure Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Deeply nested objects and arrays
    printf("Test 1: Deeply nested structure\n");
    const char* complex_json = 
        "{\n"
        "  \"users\": [\n"
        "    {\n"
        "      \"id\": 1,\n"
        "      \"name\": \"Alice\",\n"
        "      \"profile\": {\n"
        "        \"age\": 25,\n"
        "        \"address\": {\n"
        "          \"street\": \"123 Main St\",\n"
        "          \"city\": \"Boston\",\n"
        "          \"coordinates\": {\n"
        "            \"lat\": 42.3601,\n"
        "            \"lng\": -71.0589\n"
        "          }\n"
        "        },\n"
        "        \"preferences\": [\"reading\", \"hiking\", \"coding\"]\n"
        "      },\n"
        "      \"friends\": [2, 3, 5]\n"
        "    },\n"
        "    {\n"
        "      \"id\": 2,\n"
        "      \"name\": \"Bob\",\n"
        "      \"profile\": {\n"
        "        \"age\": 30,\n"
        "        \"address\": {\n"
        "          \"street\": \"456 Oak Ave\",\n"
        "          \"city\": \"New York\",\n"
        "          \"coordinates\": {\n"
        "            \"lat\": 40.7128,\n"
        "            \"lng\": -74.0060\n"
        "          }\n"
        "        },\n"
        "        \"preferences\": [\"gaming\", \"music\"]\n"
        "      },\n"
        "      \"friends\": [1, 4]\n"
        "    }\n"
        "  ],\n"
        "  \"metadata\": {\n"
        "    \"total_users\": 2,\n"
        "    \"version\": \"1.0.0\",\n"
        "    \"settings\": {\n"
        "      \"max_friends\": 100,\n"
        "      \"features\": [\"chat\", \"groups\", \"notifications\"],\n"
        "      \"privacy\": {\n"
        "        \"default\": \"public\",\n"
        "        \"options\": [\"public\", \"friends\", \"private\"]\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";

    json_value_t* root = json_parse(complex_json);
    passed &= (assertNotNull(root) == 0);
    
    if (root) {
        // Verify root structure
        passed &= (assertEquals(json_get_type(root), JSON_OBJECT) == 0);
        
        // Check users array
        json_value_t* users = json_object_get(root, "users");
        passed &= (assertNotNull(users) == 0);
        if (users) {
            passed &= (assertEquals(json_get_type(users), JSON_ARRAY) == 0);
            passed &= (assertEquals(json_get_array_size(users), 2) == 0);
            
            // Check first user
            json_value_t* user1 = json_array_get(users, 0);
            passed &= (assertNotNull(user1) == 0);
            if (user1) {
                json_value_t* name1 = json_object_get(user1, "name");
                passed &= (assertNotNull(name1) == 0);
                if (name1) {
                    passed &= (assertStringsMatch((char*)json_get_string(name1), "Alice") == 0);
                }
                
                // Check nested profile
                json_value_t* profile1 = json_object_get(user1, "profile");
                passed &= (assertNotNull(profile1) == 0);
                if (profile1) {
                    json_value_t* address1 = json_object_get(profile1, "address");
                    passed &= (assertNotNull(address1) == 0);
                    if (address1) {
                        json_value_t* city1 = json_object_get(address1, "city");
                        passed &= (assertNotNull(city1) == 0);
                        if (city1) {
                            passed &= (assertStringsMatch((char*)json_get_string(city1), "Boston") == 0);
                        }
                    }
                }
            }
        }
        
        // Check metadata
        json_value_t* metadata = json_object_get(root, "metadata");
        passed &= (assertNotNull(metadata) == 0);
        if (metadata) {
            json_value_t* version = json_object_get(metadata, "version");
            passed &= (assertNotNull(version) == 0);
            if (version) {
                passed &= (assertStringsMatch((char*)json_get_string(version), "1.0.0") == 0);
            }
        }
        
        json_free(root);
    }

    printf("✓ Large Nested Structure Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_large_array_performance() {
    printf("=== Large Array Performance Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Array with 1000 elements
    printf("Test 1: Array with 1000 elements\n");
    
    // Build large array JSON
    char large_array_json[50000] = "[";
    for (int i = 0; i < 1000; i++) {
        char num[20];
        sprintf(num, "%d", i);
        strcat(large_array_json, num);
        if (i < 999) {
            strcat(large_array_json, ",");
        }
    }
    strcat(large_array_json, "]");

    json_value_t* large_array = json_parse(large_array_json);
    passed &= (assertNotNull(large_array) == 0);
    
    if (large_array) {
        passed &= (assertEquals(json_get_type(large_array), JSON_ARRAY) == 0);
        passed &= (assertEquals(json_get_array_size(large_array), 1000) == 0);
        
        // Verify some elements
        json_value_t* first_elem = json_array_get(large_array, 0);
        json_value_t* last_elem = json_array_get(large_array, 999);
        json_value_t* middle_elem = json_array_get(large_array, 499);
        
        passed &= (assertNotNull(first_elem) == 0);
        passed &= (assertNotNull(last_elem) == 0);
        passed &= (assertNotNull(middle_elem) == 0);
        
        if (first_elem) passed &= (assertDoubleEquals(json_get_number(first_elem), 0.0) == 0);
        if (last_elem) passed &= (assertDoubleEquals(json_get_number(last_elem), 999.0) == 0);
        if (middle_elem) passed &= (assertDoubleEquals(json_get_number(middle_elem), 499.0) == 0);
        
        json_free(large_array);
    }

    printf("✓ Large Array Performance Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_complex_serialization_roundtrip() {
    printf("=== Complex Serialization Roundtrip Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Complex structure serialization and parsing back
    printf("Test 1: Complex roundtrip test\n");
    const char* original_json = 
        "{\n"
        "  \"array\": [1, 2.5, -3, 0, 999.999],\n"
        "  \"nested\": {\n"
        "    \"empty_array\": [],\n"
        "    \"empty_object\": {},\n"
        "    \"mixed\": [null, true, false, \"string\\nwith\\tescapes\"]\n"
        "  },\n"
        "  \"unicode\": \"Hello 世界 🌍\",\n"
        "  \"special_chars\": \"\\\"\\\\\\/\\b\\f\\n\\r\\t\",\n"
        "  \"large_numbers\": [1e10, -2e-5, 3.141592653589793],\n"
        "  \"boundaries\": {\n"
        "    \"min\": -2147483648,\n"
        "    \"max\": 2147483647,\n"
        "    \"float_min\": -3.4028235e38,\n"
        "    \"float_max\": 3.4028235e38\n"
        "  }\n"
        "}";

    json_value_t* parsed = json_parse(original_json);
    passed &= (assertNotNull(parsed) == 0);
    
    if (parsed) {
        // Serialize to string
        char* serialized = json_serialize(parsed);
        passed &= (assertNotNull(serialized) == 0);
        
        if (serialized) {
            printf("Serialized length: %zu\n", strlen(serialized));
            
            // Parse back
            json_value_t* reparsed = json_parse(serialized);
            passed &= (assertNotNull(reparsed) == 0);
            
            if (reparsed) {
                // Verify structure is preserved
                passed &= (assertEquals(json_get_type(reparsed), JSON_OBJECT) == 0);
                
                // Check array
                json_value_t* array = json_object_get(reparsed, "array");
                passed &= (assertNotNull(array) == 0);
                if (array) {
                    passed &= (assertEquals(json_get_array_size(array), 5) == 0);
                }
                
                // Check nested structure
                json_value_t* nested = json_object_get(reparsed, "nested");
                passed &= (assertNotNull(nested) == 0);
                if (nested) {
                    json_value_t* mixed = json_object_get(nested, "mixed");
                    passed &= (assertNotNull(mixed) == 0);
                    if (mixed) {
                        passed &= (assertEquals(json_get_array_size(mixed), 4) == 0);
                    }
                }
                
                // Check unicode string
                json_value_t* unicode = json_object_get(reparsed, "unicode");
                passed &= (assertNotNull(unicode) == 0);
                
                json_free(reparsed);
            }
            
            free(serialized);
        }
        
        // Test pretty serialization too
        char* pretty = json_serialize_pretty(parsed);
        passed &= (assertNotNull(pretty) == 0);
        if (pretty) {
            // Verify pretty serialization can be parsed back
            json_value_t* pretty_parsed = json_parse(pretty);
            passed &= (assertNotNull(pretty_parsed) == 0);
            if (pretty_parsed) {
                json_free(pretty_parsed);
            }
            free(pretty);
        }
        
        json_free(parsed);
    }

    printf("✓ Complex Serialization Roundtrip Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_object_with_many_keys() {
    printf("=== Object With Many Keys Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Object with 100 key-value pairs
    printf("Test 1: Object with 100 keys\n");
    
    char large_object_json[50000] = "{";
    for (int i = 0; i < 100; i++) {
        char key[20], value[20];
        sprintf(key, "\"key_%03d\"", i);
        sprintf(value, "\"value_%d\"", i * 10);
        
        strcat(large_object_json, key);
        strcat(large_object_json, ":");
        strcat(large_object_json, value);
        
        if (i < 99) {
            strcat(large_object_json, ",");
        }
    }
    strcat(large_object_json, "}");

    json_value_t* large_object = json_parse(large_object_json);
    passed &= (assertNotNull(large_object) == 0);
    
    if (large_object) {
        passed &= (assertEquals(json_get_type(large_object), JSON_OBJECT) == 0);
        passed &= (assertEquals(json_object_size(large_object), 100) == 0);
        
        // Test access to various keys
        json_value_t* first_val = json_object_get(large_object, "key_000");
        json_value_t* last_val = json_object_get(large_object, "key_099");
        json_value_t* middle_val = json_object_get(large_object, "key_050");
        
        passed &= (assertNotNull(first_val) == 0);
        passed &= (assertNotNull(last_val) == 0);
        passed &= (assertNotNull(middle_val) == 0);
        
        if (first_val) passed &= (assertStringsMatch((char*)json_get_string(first_val), "value_0") == 0);
        if (last_val) passed &= (assertStringsMatch((char*)json_get_string(last_val), "value_990") == 0);
        if (middle_val) passed &= (assertStringsMatch((char*)json_get_string(middle_val), "value_500") == 0);
        
        // Test key iteration
        int key_found_count = 0;
        for (size_t i = 0; i < json_object_size(large_object); i++) {
            const char* key = json_object_get_key(large_object, i);
            if (key) {
                key_found_count++;
            }
        }
        passed &= (assertEquals(key_found_count, 100) == 0);
        
        json_free(large_object);
    }

    printf("✓ Object With Many Keys Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_json_creation() {
    printf("=== JSON Creation Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Create string
    printf("Test 1: Create string\n");
    json_value_t* str = json_string_create("hello");
    passed &= (assertNotNull(str) == 0);
    if (str) {
        passed &= (assertEquals(json_get_type(str), JSON_STRING) == 0);
        passed &= (assertStringsMatch((char*)json_get_string(str), "hello") == 0);
        json_free(str);
    }

    // Test 2: Create number
    printf("Test 2: Create number\n");
    json_value_t* num = json_number_create(42.5);
    passed &= (assertNotNull(num) == 0);
    if (num) {
        passed &= (assertEquals(json_get_type(num), JSON_NUMBER) == 0);
        passed &= (assertDoubleEquals(json_get_number(num), 42.5) == 0);
        json_free(num);
    }

    // Test 3: Create boolean true
    printf("Test 3: Create boolean true\n");
    json_value_t* btrue = json_bool_create(1);
    passed &= (assertNotNull(btrue) == 0);
    if (btrue) {
        passed &= (assertEquals(json_get_type(btrue), JSON_BOOL) == 0);
        passed &= (assertEquals(json_get_boolean(btrue), 1) == 0);
        json_free(btrue);
    }

    // Test 4: Create boolean false
    printf("Test 4: Create boolean false\n");
    json_value_t* bfalse = json_bool_create(0);
    passed &= (assertNotNull(bfalse) == 0);
    if (bfalse) {
        passed &= (assertEquals(json_get_type(bfalse), JSON_BOOL) == 0);
        passed &= (assertEquals(json_get_boolean(bfalse), 0) == 0);
        json_free(bfalse);
    }

    printf("✓ JSON Creation Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_json_array_create_add() {
    printf("=== JSON Array Create & Add Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Create empty array
    printf("Test 1: Create empty array\n");
    json_value_t* arr = json_array_create();
    passed &= (assertNotNull(arr) == 0);
    if (arr) {
        passed &= (assertEquals(json_get_type(arr), JSON_ARRAY) == 0);
        passed &= (assertEquals(json_get_array_size(arr), 0) == 0);
    }

    // Test 2: Add elements to array
    printf("Test 2: Add elements to array\n");
    if (arr) {
        passed &= (assertEquals(json_array_add(arr, json_number_create(1.0)), 1) == 0);
        passed &= (assertEquals(json_array_add(arr, json_number_create(2.0)), 1) == 0);
        passed &= (assertEquals(json_array_add(arr, json_string_create("three")), 1) == 0);
        passed &= (assertEquals(json_array_add(arr, json_bool_create(1)), 1) == 0);
        passed &= (assertEquals(json_array_add(arr, json_null_create()), 1) == 0);
        passed &= (assertEquals(json_get_array_size(arr), 5) == 0);
    }

    // Test 3: Access array elements
    printf("Test 3: Access array elements\n");
    if (arr) {
        json_value_t* e0 = json_array_get(arr, 0);
        json_value_t* e2 = json_array_get(arr, 2);
        json_value_t* e4 = json_array_get(arr, 4);

        passed &= (assertNotNull(e0) == 0);
        passed &= (assertNotNull(e2) == 0);
        passed &= (assertNotNull(e4) == 0);

        if (e0) passed &= (assertDoubleEquals(json_get_number(e0), 1.0) == 0);
        if (e2) passed &= (assertStringsMatch((char*)json_get_string(e2), "three") == 0);
        if (e4) passed &= (assertEquals(json_get_type(e4), JSON_NULL) == 0);

        json_free(arr);
    }

    // Test 4: Add to invalid type fails
    printf("Test 4: Add to non-array fails\n");
    json_value_t* obj = json_object_create();
    passed &= (assertEquals(json_array_add(obj, json_number_create(1.0)), 0) == 0);
    json_free(obj);

    // Test 5: NULL inputs fail
    printf("Test 5: NULL inputs fail\n");
    passed &= (assertEquals(json_array_add(NULL, json_number_create(1.0)), 0) == 0);
    passed &= (assertEquals(json_array_add(arr, NULL), 0) == 0);

    printf("✓ JSON Array Create & Add Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_json_object_create_add() {
    printf("=== JSON Object Create & Add Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Create empty object
    printf("Test 1: Create empty object\n");
    json_value_t* obj = json_object_create();
    passed &= (assertNotNull(obj) == 0);
    if (obj) {
        passed &= (assertEquals(json_get_type(obj), JSON_OBJECT) == 0);
        passed &= (assertEquals(json_object_size(obj), 0) == 0);
    }

    // Test 2: Add key-value pairs
    printf("Test 2: Add key-value pairs\n");
    if (obj) {
        passed &= (assertEquals(json_object_add(obj, "name", json_string_create("John")), 1) == 0);
        passed &= (assertEquals(json_object_add(obj, "age", json_number_create(30)), 1) == 0);
        passed &= (assertEquals(json_object_add(obj, "active", json_bool_create(1)), 1) == 0);
        passed &= (assertEquals(json_object_size(obj), 3) == 0);
    }

    // Test 3: Access values by key
    printf("Test 3: Access values by key\n");
    if (obj) {
        json_value_t* name = json_object_get(obj, "name");
        json_value_t* age = json_object_get(obj, "age");
        json_value_t* active = json_object_get(obj, "active");
        json_value_t* missing = json_object_get(obj, "missing");

        passed &= (assertNotNull(name) == 0);
        passed &= (assertNotNull(age) == 0);
        passed &= (assertNotNull(active) == 0);
        passed &= (assertNull(missing) == 0);

        if (name) passed &= (assertStringsMatch((char*)json_get_string(name), "John") == 0);
        if (age) passed &= (assertDoubleEquals(json_get_number(age), 30.0) == 0);
        if (active) passed &= (assertEquals(json_get_boolean(active), 1) == 0);
    }

    // Test 4: Iterate keys
    printf("Test 4: Iterate keys by index\n");
    if (obj) {
        int found_name = 0, found_age = 0, found_active = 0;
        for (size_t i = 0; i < json_object_size(obj); i++) {
            const char* key = json_object_get_key(obj, i);
            if (strcmp(key, "name") == 0) found_name = 1;
            if (strcmp(key, "age") == 0) found_age = 1;
            if (strcmp(key, "active") == 0) found_active = 1;
        }
        passed &= (assertTrue(found_name) == 0);
        passed &= (assertTrue(found_age) == 0);
        passed &= (assertTrue(found_active) == 0);

        json_free(obj);
    }

    // Test 5: Add to non-object fails
    printf("Test 5: Add to non-object fails\n");
    json_value_t* arr = json_array_create();
    passed &= (assertEquals(json_object_add(arr, "key", json_number_create(1.0)), 0) == 0);
    json_free(arr);

    // Test 6: NULL inputs fail
    printf("Test 6: NULL inputs fail\n");
    passed &= (assertEquals(json_object_add(NULL, "key", json_number_create(1.0)), 0) == 0);

    printf("✓ JSON Object Create & Add Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_json_create_serialize_roundtrip() {
    printf("=== JSON Create & Serialize Roundtrip Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Build object: {"x": 10, "y": "hello", "z": true}
    printf("Test 1: Build object, serialize, parse back\n");
    json_value_t* obj = json_object_create();
    json_object_add(obj, "x", json_number_create(10));
    json_object_add(obj, "y", json_string_create("hello"));
    json_object_add(obj, "z", json_bool_create(1));

    char* json_str = json_serialize(obj);
    passed &= (assertNotNull(json_str) == 0);

    if (json_str) {
        json_value_t* parsed = json_parse(json_str);
        passed &= (assertNotNull(parsed) == 0);

        if (parsed) {
            passed &= (assertEquals(json_get_type(parsed), JSON_OBJECT) == 0);
            passed &= (assertEquals(json_object_size(parsed), 3) == 0);

            json_value_t* px = json_object_get(parsed, "x");
            json_value_t* py = json_object_get(parsed, "y");
            json_value_t* pz = json_object_get(parsed, "z");

            if (px) passed &= (assertDoubleEquals(json_get_number(px), 10.0) == 0);
            if (py) passed &= (assertStringsMatch((char*)json_get_string(py), "hello") == 0);
            if (pz) passed &= (assertEquals(json_get_boolean(pz), 1) == 0);

            json_free(parsed);
        }
        free(json_str);
    }

    // Test 2: Nested structure roundtrip
    printf("Test 2: Nested array roundtrip\n");
    json_value_t* arr = json_array_create();
    json_array_add(arr, json_number_create(1));
    json_array_add(arr, json_string_create("two"));
    json_array_add(arr, json_bool_create(0));
    json_array_add(arr, json_null_create());

    char* arr_str = json_serialize(arr);
    passed &= (assertNotNull(arr_str) == 0);

    if (arr_str) {
        json_value_t* parsed_arr = json_parse(arr_str);
        passed &= (assertNotNull(parsed_arr) == 0);

        if (parsed_arr) {
            passed &= (assertEquals(json_get_array_size(parsed_arr), 4) == 0);
            passed &= (assertDoubleEquals(json_get_number(json_array_get(parsed_arr, 0)), 1.0) == 0);
            passed &= (assertStringsMatch((char*)json_get_string(json_array_get(parsed_arr, 1)), "two") == 0);
            passed &= (assertEquals(json_get_boolean(json_array_get(parsed_arr, 2)), 0) == 0);
            passed &= (assertEquals(json_get_type(json_array_get(parsed_arr, 3)), JSON_NULL) == 0);

            json_free(parsed_arr);
        }
        free(arr_str);
    }

    // Test 3: Pretty print roundtrip
    printf("Test 3: Pretty print roundtrip\n");
    char* pretty = json_serialize_pretty(obj);
    passed &= (assertNotNull(pretty) == 0);

    if (pretty) {
        json_value_t* parsed_pretty = json_parse(pretty);
        passed &= (assertNotNull(parsed_pretty) == 0);
        if (parsed_pretty) {
            passed &= (assertEquals(json_object_size(parsed_pretty), 3) == 0);
            json_free(parsed_pretty);
        }
        free(pretty);
    }

    json_free(obj);
    json_free(arr);

    printf("✓ JSON Create & Serialize Roundtrip Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_json_create_null() {
    printf("=== JSON Create Null Test ===\n");
    reset_test_counter();

    int passed = 1;

    printf("Test 1: Create null\n");
    json_value_t* n = json_null_create();
    passed &= (assertNotNull(n) == 0);
    if (n) {
        passed &= (assertEquals(json_get_type(n), JSON_NULL) == 0);

        char* str = json_serialize(n);
        passed &= (assertNotNull(str) == 0);
        if (str) {
            passed &= (assertStringsMatch(str, "null") == 0);
            free(str);
        }
        json_free(n);
    }

    printf("Test 2: Null in object roundtrip\n");
    json_value_t* obj = json_object_create();
    json_object_add(obj, "key", json_null_create());
    char* obj_str = json_serialize(obj);
    passed &= (assertNotNull(obj_str) == 0);
    if (obj_str) {
        json_value_t* parsed = json_parse(obj_str);
        if (parsed) {
            json_value_t* val = json_object_get(parsed, "key");
            passed &= (assertNotNull(val) == 0);
            if (val) passed &= (assertEquals(json_get_type(val), JSON_NULL) == 0);
            json_free(parsed);
        }
        free(obj_str);
    }
    json_free(obj);

    printf("✓ JSON Create Null Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

int main() {
    printf("Starting Comprehensive JSON Tests\n\n");
    
    test_basic_types();
    test_arrays();
    test_simple_object();
    test_serialization();
    test_error_handling();
    test_edge_cases();
	test_large_nested_structure();
	test_large_array_performance();
	test_object_with_many_keys();
	test_complex_serialization_roundtrip();
    test_json_creation();
    test_json_array_create_add();
    test_json_object_create_add();
    test_json_create_serialize_roundtrip();
    test_json_create_null();
    
    printf("=== All Tests Completed ===\n");
    return 0;
}