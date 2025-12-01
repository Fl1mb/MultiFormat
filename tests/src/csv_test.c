#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/csv.h"

extern int count;

// Helper function to parse CSV from string
static CSVData* parse_csv_file_content(const char* content, const CSVParserConfig* config) {
    // Создаем временный файл с помощью стандартной функции
#ifdef _WIN32
    char temp_filename[L_tmpnam];
    tmpnam_s(temp_filename, L_tmpnam);
#else
    char temp_filename[] = "/tmp/csv_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    if (fd == -1) {
        return NULL;
    }
    close(fd);
#endif

    // Записываем содержимое во временный файл
    FILE* temp_file = fopen(temp_filename, "w");
    if (!temp_file) {
        return NULL;
    }

    fputs(content, temp_file);
    fclose(temp_file);

    // Парсим файл
    CSVData* data = parse_csv_file(temp_filename, config);

    // Удаляем временный файл
    remove(temp_filename);

    return data;
}

void test_csv_basic_parsing() {
    printf("=== CSV Basic Parsing Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Simple CSV with comma delimeter
    printf("Test 1: Simple comma-separated CSV\n");
    const char* simple_csv = "name,age,city\nJohn,30,New York\nAlice,25,Boston\nBob,35,Chicago";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(simple_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        passed &= (assertEquals(data->row_count, 3) == 0);
        passed &= (assertEquals(data->max_fields, 3) == 0);

        if (data->row_count > 0) {
            // Check first data row
            CSVRows* row = &data->rows[0];
            passed &= (assertStringsMatch(row->fields[0], "John") == 0);
            passed &= (assertStringsMatch(row->fields[1], "30") == 0);
            passed &= (assertStringsMatch(row->fields[2], "New York") == 0);
        }

        free_csv_data(data);
    }

    // Test 2: CSV with semicolon delimeter
    printf("Test 2: Semicolon-separated CSV\n");
    const char* semicolon_csv = "name;age;city\nJohn;30;New York\nAlice;25;Boston";

    config.delimeter = ';';
    CSVData* data2 = parse_csv_file_content(semicolon_csv, &config);
    passed &= (assertNotNull(data2) == 0);

    if (data2) {
        passed &= (assertEquals(data2->row_count, 2) == 0);
        passed &= (assertEquals(data2->max_fields, 3) == 0);

        if (data2->row_count > 0) {
            CSVRows* row = &data2->rows[0];
            passed &= (assertStringsMatch(row->fields[0], "John") == 0);
            passed &= (assertStringsMatch(row->fields[1], "30") == 0);
            passed &= (assertStringsMatch(row->fields[2], "New York") == 0);
        }

        free_csv_data(data2);
    }

    // Test 3: CSV with tab delimeter
    printf("Test 3: Tab-separated CSV (TSV)\n");
    const char* tsv = "name\tage\tcity\nJohn\t30\tNew York\nAlice\t25\tBoston";

    config.delimeter = '\t';
    CSVData* data3 = parse_csv_file_content(tsv, &config);
    passed &= (assertNotNull(data3) == 0);

    if (data3) {
        passed &= (assertEquals(data3->row_count, 2) == 0);
        passed &= (assertEquals(data3->max_fields, 3) == 0);

        if (data3->row_count > 0) {
            CSVRows* row = &data3->rows[0];
            passed &= (assertStringsMatch(row->fields[0], "John") == 0);
            passed &= (assertStringsMatch(row->fields[1], "30") == 0);
            passed &= (assertStringsMatch(row->fields[2], "New York") == 0);
        }

        free_csv_data(data3);
    }

    printf("✓ CSV Basic Parsing Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_with_quotes() {
    printf("=== CSV With Quotes Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: CSV with quoted fields containing commas
    printf("Test 1: Quoted fields with commas inside\n");
    const char* quoted_csv = "name,address,salary\n"
        "John Doe,\"123 Main St, Apt 4B, Boston, MA\",55000\n"
        "Jane Smith,\"456 Oak Ave, New York, NY\",62000";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(quoted_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        passed &= (assertEquals(data->row_count, 2) == 0);
        passed &= (assertEquals(data->max_fields, 3) == 0);

        if (data->row_count > 0) {
            // Check first row
            CSVRows* row1 = &data->rows[0];
            passed &= (assertStringsMatch(row1->fields[0], "John Doe") == 0);
            passed &= (assertStringsMatch(row1->fields[1], "123 Main St, Apt 4B, Boston, MA") == 0);
            passed &= (assertStringsMatch(row1->fields[2], "55000") == 0);

            // Check second row
            CSVRows* row2 = &data->rows[1];
            passed &= (assertStringsMatch(row2->fields[0], "Jane Smith") == 0);
            passed &= (assertStringsMatch(row2->fields[1], "456 Oak Ave, New York, NY") == 0);
            passed &= (assertStringsMatch(row2->fields[2], "62000") == 0);
        }

        free_csv_data(data);
    }

    // Test 2: CSV with escaped quotes
    printf("Test 2: Escaped quotes in fields\n");
    const char* escaped_csv = "id,comment\n"
        "1,\"He said \"\"Hello\"\" to me\"\n"
        "2,\"Normal comment\"";

    CSVData* data2 = parse_csv_file_content(escaped_csv, &config);
    passed &= (assertNotNull(data2) == 0);

    if (data2) {
        passed &= (assertEquals(data2->row_count, 2) == 0);
        passed &= (assertEquals(data2->max_fields, 2) == 0);

        if (data2->row_count > 0) {
            CSVRows* row = &data2->rows[0];
            passed &= (assertStringsMatch(row->fields[0], "1") == 0);
            // Note: Our parser should handle escaped quotes correctly
        }

        free_csv_data(data2);
    }

    // Test 3: Mixed quoted and unquoted fields
    printf("Test 3: Mixed quoted and unquoted fields\n");
    const char* mixed_csv = "field1,field2,field3\n"
        "simple,\"quoted,field\",simple2\n"
        "\"quoted\",simple,\"quoted,with,commas\"";

    CSVData* data3 = parse_csv_file_content(mixed_csv, &config);
    passed &= (assertNotNull(data3) == 0);

    if (data3) {
        passed &= (assertEquals(data3->row_count, 2) == 0);
        passed &= (assertEquals(data3->max_fields, 3) == 0);

        if (data3->row_count > 0) {
            CSVRows* row1 = &data3->rows[0];
            passed &= (assertStringsMatch(row1->fields[0], "simple") == 0);
            passed &= (assertStringsMatch(row1->fields[1], "quoted,field") == 0);
            passed &= (assertStringsMatch(row1->fields[2], "simple2") == 0);

            CSVRows* row2 = &data3->rows[1];
            passed &= (assertStringsMatch(row2->fields[0], "quoted") == 0);
            passed &= (assertStringsMatch(row2->fields[1], "simple") == 0);
            passed &= (assertStringsMatch(row2->fields[2], "quoted,with,commas") == 0);
        }

        free_csv_data(data3);
    }

    printf("✓ CSV With Quotes Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_empty_and_missing_fields() {
    printf("=== CSV Empty and Missing Fields Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: CSV with empty fields
    printf("Test 1: CSV with empty fields\n");
    const char* empty_fields_csv = "name,age,email,phone\n"
        "John,30,john@example.com,\n"
        "Alice,,alice@example.com,123-456-7890\n"
        ",25,,555-1234";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(empty_fields_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        passed &= (assertEquals(data->row_count, 3) == 0);
        passed &= (assertEquals(data->max_fields, 4) == 0);

        if (data->row_count > 0) {
            // Check first row
            CSVRows* row1 = &data->rows[0];
            passed &= (assertStringsMatch(row1->fields[0], "John") == 0);
            passed &= (assertStringsMatch(row1->fields[1], "30") == 0);
            passed &= (assertStringsMatch(row1->fields[2], "john@example.com") == 0);
            passed &= (assertStringsMatch(row1->fields[3], "") == 0);  // Empty field

            // Check second row
            CSVRows* row2 = &data->rows[1];
            passed &= (assertStringsMatch(row2->fields[0], "Alice") == 0);
            passed &= (assertStringsMatch(row2->fields[1], "") == 0);  // Empty field
            passed &= (assertStringsMatch(row2->fields[2], "alice@example.com") == 0);
            passed &= (assertStringsMatch(row2->fields[3], "123-456-7890") == 0);

            // Check third row
            CSVRows* row3 = &data->rows[2];
            passed &= (assertStringsMatch(row3->fields[0], "") == 0);  // Empty field
            passed &= (assertStringsMatch(row3->fields[1], "25") == 0);
            passed &= (assertStringsMatch(row3->fields[2], "") == 0);  // Empty field
            passed &= (assertStringsMatch(row3->fields[3], "555-1234") == 0);
        }

        free_csv_data(data);
    }

    // Test 2: CSV with trailing commas (missing fields at end)
    printf("Test 2: CSV with trailing commas\n");
    const char* trailing_csv = "a,b,c,d\n"
        "1,2,3,\n"
        "4,5,,\n"
        "6,,,";

    CSVData* data2 = parse_csv_file_content(trailing_csv, &config);
    passed &= (assertNotNull(data2) == 0);

    if (data2) {
        passed &= (assertEquals(data2->row_count, 3) == 0);
        passed &= (assertEquals(data2->max_fields, 4) == 0);

        if (data2->row_count > 0) {
            CSVRows* row1 = &data2->rows[0];
            passed &= (assertEquals(row1->count, 4) == 0);
            passed &= (assertStringsMatch(row1->fields[3], "") == 0);

            CSVRows* row2 = &data2->rows[1];
            passed &= (assertEquals(row2->count, 4) == 0);
            passed &= (assertStringsMatch(row2->fields[2], "") == 0);
            passed &= (assertStringsMatch(row2->fields[3], "") == 0);
        }

        free_csv_data(data2);
    }

    // Test 3: Completely empty CSV
    printf("Test 3: Empty CSV content\n");
    const char* empty_csv = "";

    CSVData* data3 = parse_csv_file_content(empty_csv, &config);
    // Should return NULL or empty data structure
    if (data3) {
        passed &= (assertEquals(data3->row_count, 0) == 0);
        free_csv_data(data3);
    }

    printf("✓ CSV Empty and Missing Fields Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_search_functionality() {
    printf("=== CSV Search Functionality Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Create test CSV data
    const char* test_csv = "id,name,age,department,salary\n"
        "101,John Smith,30,Engineering,75000\n"
        "102,Jane Doe,28,Marketing,65000\n"
        "103,Bob Johnson,35,Engineering,80000\n"
        "104,Alice Brown,42,HR,60000\n"
        "105,Charlie Wilson,30,Sales,70000\n"
        "106,David Lee,28,Engineering,72000";

    CSVParserConfig config = { ',', '"', true, true, false };
    CSVData* data = parse_csv_file_content(test_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        // Test 1: Search for age 30
        printf("Test 1: Search for age = 30\n");
        int count1 = search_in_csv(data, 2, "30");  // age is at index 2 (0-based)
        passed &= (assertEquals(count1, 2) == 0);

        // Test 2: Search for department "Engineering"
        printf("Test 2: Search for department = Engineering\n");
        int count2 = search_in_csv(data, 3, "Engineering");  // department at index 3
        passed &= (assertEquals(count2, 3) == 0);

        // Test 3: Search for name "Jane Doe"
        printf("Test 3: Search for name = Jane Doe\n");
        int count3 = search_in_csv(data, 1, "Jane Doe");  // name at index 1
        passed &= (assertEquals(count3, 1) == 0);

        // Test 4: Search for non-existent value
        printf("Test 4: Search for non-existent value\n");
        int count4 = search_in_csv(data, 1, "Non Existent");
        passed &= (assertEquals(count4, 0) == 0);

        // Test 5: Search with invalid field index
        printf("Test 5: Search with invalid field index\n");
        int count5 = search_in_csv(data, 10, "test");
        passed &= (assertEquals(count5, 0) == 0);  // Should return 0, not -1

        free_csv_data(data);
    }

    printf("✓ CSV Search Functionality Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_large_dataset() {
    printf("=== CSV Large Dataset Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Generate large CSV data (1000 rows)
    printf("Test 1: Parse 1000-row CSV\n");
    char* large_csv = (char*)malloc(1000000);  // 1MB buffer
    if (!large_csv) {
        printf("Memory allocation failed\n");
        return;
    }

    // Create header
    strcpy(large_csv, "id,name,age,score,department\n");

    // Generate 1000 rows
    for (int i = 1; i <= 1000; i++) {
        char row[100];
        sprintf(row, "%d,Person%d,%d,%.2f,Dept%d\n",
            i, i, 20 + (i % 40), 50.0 + (i % 50), i % 10);
        strcat(large_csv, row);
    }

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(large_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        passed &= (assertEquals(data->row_count, 1000) == 0);
        passed &= (assertEquals(data->max_fields, 5) == 0);

        // Verify first row
        if (data->row_count > 0) {
            CSVRows* row1 = &data->rows[0];
            passed &= (assertStringsMatch(row1->fields[0], "1") == 0);
            passed &= (assertStringsMatch(row1->fields[1], "Person1") == 0);
        }

        // Verify last row
        if (data->row_count >= 1000) {
            CSVRows* last_row = &data->rows[999];
            passed &= (assertStringsMatch(last_row->fields[0], "1000") == 0);
            passed &= (assertStringsMatch(last_row->fields[1], "Person1000") == 0);
        }

        // Test search on large dataset
        int engineering_count = 0;
        for (int i = 0; i < data->row_count; i++) {
            CSVRows* row = &data->rows[i];
            if (row->count > 4 && strstr(row->fields[4], "Dept") != NULL) {
                engineering_count++;
            }
        }
        passed &= (assertEquals(engineering_count, 1000) == 0);

        free_csv_data(data);
    }

    free(large_csv);

    printf("✓ CSV Large Dataset Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_different_delimeters_comprehensive() {
    printf("=== CSV Different delimeters Comprehensive Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test various delimeters
    struct {
        char delimeter;
        const char* name;
        const char* test_data;
    } delimeter_tests[] = {
        {',', "Comma", "a,b,c\n1,2,3\n4,5,6"},
        {';', "Semicolon", "a;b;c\n1;2;3\n4;5;6"},
        {'\t', "Tab", "a\tb\tc\n1\t2\t3\n4\t5\t6"},
        {'|', "Pipe", "a|b|c\n1|2|3\n4|5|6"},
        {':', "Colon", "a:b:c\n1:2:3\n4:5:6"},
        {'#', "Hash", "a#b#c\n1#2#3\n4#5#6"}
    };

    for (size_t i = 0; i < sizeof(delimeter_tests) / sizeof(delimeter_tests[0]); i++) {
        printf("Test %zu: %s delimeter\n", i + 1, delimeter_tests[i].name);

        CSVParserConfig config = { delimeter_tests[i].delimeter, '"', true, true, true };
        CSVData* data = parse_csv_file_content(delimeter_tests[i].test_data, &config);
        passed &= (assertNotNull(data) == 0);

        if (data) {
            passed &= (assertEquals(data->row_count, 2) == 0);
            passed &= (assertEquals(data->max_fields, 3) == 0);

            if (data->row_count > 0) {
                CSVRows* row = &data->rows[0];
                passed &= (assertStringsMatch(row->fields[0], "1") == 0);
                passed &= (assertStringsMatch(row->fields[1], "2") == 0);
                passed &= (assertStringsMatch(row->fields[2], "3") == 0);
            }

            free_csv_data(data);
        }
    }

    printf("✓ CSV Different delimeters Comprehensive Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_parser_debug() {
    printf("\n=== PARSER DEBUG TEST ===\n");

    // Создаем простой CSV файл для тестирования
    const char* test_csv =
        "name,age,city\n"
        "John,30,New York\n"
        "Alice,25,Boston\n"
        "Bob,35,Chicago";

    // Сохраняем во временный файл
    FILE* tmp = fopen("test_parser.csv", "w");
    if (tmp) {
        fputs(test_csv, tmp);
        fclose(tmp);
    }

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file("test_parser.csv", &config);

    if (data) {
        printf("\nPARSER RESULTS:\n");
        printf("Expected: 3 rows\n");
        printf("Actual: %d rows\n", data->row_count);

        if (data->row_count != 3) {
            printf("ERROR: Missing rows!\n");

            // Проверяем, какие строки были пропущены
            for (int i = 0; i < data->row_count; i++) {
                printf("Row %d: ", i);
                if (data->rows[i].count >= 1) {
                    printf("%s\n", data->rows[i].fields[0]);
                }
                else {
                    printf("(no fields)\n");
                }
            }
        }

        free_csv_data(data);
    }
    printf("=== END PARSER DEBUG TEST ===\n\n");
}

void test_csv_header_functions() {
    printf("=== CSV Header Functions Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: CSV with header
    printf("Test 1: CSV with header\n");
    const char* csv_with_header = "name,age,city,department\n"
        "John,30,New York,Engineering\n"
        "Alice,25,Boston,Marketing\n"
        "Bob,35,Chicago,Sales";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(csv_with_header, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        // Test csv_get_header
        CSVRows* header = csv_get_header(data);
        passed &= (assertNotNull(header) == 0);

        if (header) {
            passed &= (assertEquals(header->count, 4) == 0);
            passed &= (assertStringsMatch(header->fields[0], "name") == 0);
            passed &= (assertStringsMatch(header->fields[1], "age") == 0);
            passed &= (assertStringsMatch(header->fields[2], "city") == 0);
            passed &= (assertStringsMatch(header->fields[3], "department") == 0);
        }

        // Test csv_get_field_name
        const char* field_name = csv_get_field_name(data, 1);
        passed &= (assertNotNull(field_name) == 0);
        if (field_name) {
            passed &= (assertStringsMatch(field_name, "age") == 0);
        }

        // Test with invalid index
        const char* invalid_field = csv_get_field_name(data, 10);
        passed &= (assertNull(invalid_field) == 0);

        // Test negative index
        const char* negative_field = csv_get_field_name(data, -1);
        passed &= (assertNull(negative_field) == 0);

        free_csv_data(data);
    }

    // Test 2: CSV without header
    printf("Test 2: CSV without header\n");
    const char* csv_without_header = "John,30,New York\n"
        "Alice,25,Boston";

    config.has_header = false;
    CSVData* data2 = parse_csv_file_content(csv_without_header, &config);
    passed &= (assertNotNull(data2) == 0);

    if (data2) {
        // Should return NULL for header functions
        CSVRows* header = csv_get_header(data2);
        passed &= (assertNull(header) == 0);

        const char* field_name = csv_get_field_name(data2, 0);
        passed &= (assertNull(field_name) == 0);

        free_csv_data(data2);
    }

    printf("✓ CSV Header Functions Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_find_field_index() {
    printf("=== CSV Find Field Index Test ===\n");
    reset_test_counter();

    int passed = 1;

    const char* test_csv = "id,name,age,department,salary\n"
        "101,John,30,Engineering,75000\n"
        "102,Alice,25,Marketing,65000";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(test_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        // Test valid field names
        int name_index = can_find_field_index(data, "name");
        passed &= (assertEquals(name_index, 1) == 0);

        int age_index = can_find_field_index(data, "age");
        passed &= (assertEquals(age_index, 2) == 0);

        int dept_index = can_find_field_index(data, "department");
        passed &= (assertEquals(dept_index, 3) == 0);

        // Test case sensitivity
        int case_sensitive = can_find_field_index(data, "NAME");
        passed &= (assertEquals(case_sensitive, -1) == 0);

        // Test non-existent field
        int not_found = can_find_field_index(data, "nonexistent");
        passed &= (assertEquals(not_found, -1) == 0);

        // Test NULL parameters
        int null_test1 = can_find_field_index(NULL, "name");
        passed &= (assertEquals(null_test1, -1) == 0);

        int null_test2 = can_find_field_index(data, NULL);
        passed &= (assertEquals(null_test2, -1) == 0);

        free_csv_data(data);
    }

    printf("✓ CSV Find Field Index Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_search_by_name() {
    printf("=== CSV Search By Name Test ===\n");
    reset_test_counter();

    int passed = 1;

    const char* test_csv = "id,name,age,department,active\n"
        "1,John Doe,30,Engineering,true\n"
        "2,Jane Smith,28,Marketing,true\n"
        "3,Bob Johnson,35,Engineering,false\n"
        "4,Alice Brown,42,HR,true\n"
        "5,Charlie Wilson,30,Engineering,true\n"
        "6,David Lee,28,Engineering,true";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(test_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        // Test search by existing field name
        int engineering_count = search_in_csv_by_name(data, "department", "Engineering");
        passed &= (assertEquals(engineering_count, 4) == 0);

        int age_30_count = search_in_csv_by_name(data, "age", "30");
        passed &= (assertEquals(age_30_count, 2) == 0);

        int active_true = search_in_csv_by_name(data, "active", "true");
        passed &= (assertEquals(active_true, 5) == 0);

        // Test search by non-existent field name
        int not_found = search_in_csv_by_name(data, "nonexistent", "value");
        passed &= (assertEquals(not_found, 0) == 0);

        // Test search with non-existent value
        int no_matches = search_in_csv_by_name(data, "department", "Accounting");
        passed &= (assertEquals(no_matches, 0) == 0);

        // Test NULL parameters
        int null_test1 = search_in_csv_by_name(NULL, "department", "Engineering");
        passed &= (assertEquals(null_test1, 0) == 0);

        int null_test2 = search_in_csv_by_name(data, NULL, "Engineering");
        passed &= (assertEquals(null_test2, 0) == 0);

        int null_test3 = search_in_csv_by_name(data, "department", NULL);
        passed &= (assertEquals(null_test3, 0) == 0);

        free_csv_data(data);
    }

    printf("✓ CSV Search By Name Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_search_by_index() {
    printf("=== CSV Search By Index Test ===\n");
    reset_test_counter();

    int passed = 1;

    const char* test_csv = "Apple,Banana,Cherry\n"
        "Red,Yellow,Red\n"
        "Green,Yellow,Red\n"
        "Red,Green,Red\n"
        "Yellow,Yellow,Yellow";

    CSVParserConfig config = { ',', '"', true, true, false }; // no header
    CSVData* data = parse_csv_file_content(test_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        // Test search by valid indices
        int red_in_col0 = search_in_csv_by_index(data, 0, "Red");
        passed &= (assertEquals(red_in_col0, 2) == 0);

        int yellow_in_col1 = search_in_csv_by_index(data, 1, "Yellow");
        passed &= (assertEquals(yellow_in_col1, 3) == 0);

        int red_in_col2 = search_in_csv_by_index(data, 2, "Red");
        passed &= (assertEquals(red_in_col2, 3) == 0);

        // Test search with out-of-bounds index
        int out_of_bounds = search_in_csv_by_index(data, 10, "Red");
        passed &= (assertEquals(out_of_bounds, 0) == 0);

        // Test search with negative index
        int negative_index = search_in_csv_by_index(data, -1, "Red");
        passed &= (assertEquals(negative_index, 0) == 0);

        // Test empty string search
        int empty_search = search_in_csv_by_index(data, 0, "");
        passed &= (assertEquals(empty_search, 0) == 0);

        free_csv_data(data);
    }

    printf("✓ CSV Search By Index Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_export_with_header() {
    printf("=== CSV Export With Header Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test data with header
    const char* original_csv = "name,age,city\n"
        "John,30,\"New York, NY\"\n"
        "Alice,25,Boston\n"
        "Bob,35,\"Chicago, IL\"";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(original_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        // Test 1: Export with comma delimiter including header
        printf("Test 1: Export with header\n");
        export_csv(data, "test_export_with_header.csv", ',');

        // Read back and verify
        CSVParserConfig read_config = { ',', '"', true, true, true };
        CSVData* exported_data = parse_csv_file("test_export_with_header.csv", &read_config);
        passed &= (assertNotNull(exported_data) == 0);

        if (exported_data) {
            passed &= (assertEquals(exported_data->row_count, 3) == 0);

            // Check that header was exported
            CSVRows* header = csv_get_header(exported_data);
            passed &= (assertNotNull(header) == 0);
            if (header) {
                passed &= (assertEquals(header->count, 3) == 0);
                passed &= (assertStringsMatch(header->fields[0], "name") == 0);
                passed &= (assertStringsMatch(header->fields[1], "age") == 0);
                passed &= (assertStringsMatch(header->fields[2], "city") == 0);
            }

            free_csv_data(exported_data);
        }

        // Test 2: Export with semicolon delimiter
        printf("Test 2: Export with semicolon delimiter\n");
        export_csv(data, "test_export_semicolon_header.csv", ';');

        read_config.delimeter = ';';
        CSVData* semicolon_data = parse_csv_file("test_export_semicolon_header.csv", &read_config);
        passed &= (assertNotNull(semicolon_data) == 0);

        if (semicolon_data) {
            passed &= (assertEquals(semicolon_data->row_count, 3) == 0);
            free_csv_data(semicolon_data);
        }

        // Test 3: Export data without header (should still export header!)
        printf("Test 3: Verify header is always exported\n");
        config.has_header = false;
        CSVData* data_no_header = parse_csv_file_content(original_csv, &config);
        if (data_no_header) {
            export_csv(data_no_header, "test_export_no_header_config.csv", ',');

            // Even though parsed without header, export should still work
            CSVData* exported_no_header = parse_csv_file("test_export_no_header_config.csv", &read_config);
            if (exported_no_header) {
                // File should still be readable
                passed &= (assertGreaterThan(exported_no_header->row_count, 0) == 0);
                free_csv_data(exported_no_header);
            }
            free_csv_data(data_no_header);
        }

        free_csv_data(data);

        // Clean up test files
        remove("test_export_with_header.csv");
        remove("test_export_semicolon_header.csv");
        remove("test_export_no_header_config.csv");
    }

    printf("✓ CSV Export With Header Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_edge_cases_comprehensive() {
    printf("=== CSV Edge Cases Comprehensive Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Test 1: Empty CSV file
    printf("Test 1: Empty CSV file\n");
    const char* empty_csv = "";
    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* empty_data = parse_csv_file_content(empty_csv, &config);
    // Can be NULL or valid with 0 rows
    if (empty_data) {
        passed &= (assertEquals(empty_data->row_count, 0) == 0);
        free_csv_data(empty_data);
    }

    // Test 2: CSV with only header
    printf("Test 2: CSV with only header\n");
    const char* only_header = "field1,field2,field3\n";
    CSVData* header_only_data = parse_csv_file_content(only_header, &config);
    if (header_only_data) {
        passed &= (assertEquals(header_only_data->row_count, 0) == 0);
        CSVRows* header = csv_get_header(header_only_data);
        passed &= (assertNotNull(header) == 0);
        if (header) {
            passed &= (assertEquals(header->count, 3) == 0);
        }
        free_csv_data(header_only_data);
    }

    // Test 3: CSV with special characters in field names
    printf("Test 3: Special characters in field names\n");
    const char* special_fields = "\"First,Name\",\"Last.Name\",\"Age (years)\"\n"
        "John,Doe,30\n"
        "Jane,Smith,25";
    CSVData* special_data = parse_csv_file_content(special_fields, &config);
    if (special_data) {
        CSVRows* header = csv_get_header(special_data);
        passed &= (assertNotNull(header) == 0);
        if (header && header->count >= 3) {
            // Fields with commas should be parsed correctly
            passed &= (assertStringsMatch(header->fields[0], "First,Name") == 0);
            passed &= (assertStringsMatch(header->fields[1], "Last.Name") == 0);
            passed &= (assertStringsMatch(header->fields[2], "Age (years)") == 0);
        }

        // Test search by name with special characters
        int john_count = search_in_csv_by_name(special_data, "First,Name", "John");
        passed &= (assertEquals(john_count, 1) == 0);

        free_csv_data(special_data);
    }

    // Test 4: Mixed field counts with search
    printf("Test 4: Mixed field counts with search\n");
    const char* mixed_fields = "id,name,tags\n"
        "1,John,\"tag1,tag2\"\n"
        "2,Jane,tag3\n"
        "3,Bob,";  // Empty last field
    CSVData* mixed_data = parse_csv_file_content(mixed_fields, &config);
    if (mixed_data) {
        // Search should handle rows with different field counts
        int john_count = search_in_csv_by_name(mixed_data, "name", "John");
        passed &= (assertEquals(john_count, 1) == 0);

        // Search for empty field
        int empty_tags = search_in_csv_by_name(mixed_data, "tags", "");
        passed &= (assertEquals(empty_tags, 1) == 0);

        free_csv_data(mixed_data);
    }

    printf("✓ CSV Edge Cases Comprehensive Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

void test_csv_integration() {
    printf("=== CSV Integration Test ===\n");
    reset_test_counter();

    int passed = 1;

    // Comprehensive test using all functions together
    const char* integration_csv =
        "employee_id,full_name,department,start_date,salary,active\n"
        "101,\"Doe, John\",Engineering,2020-01-15,75000.50,true\n"
        "102,\"Smith, Jane\",Marketing,2019-03-22,65000.00,true\n"
        "103,\"Johnson, Bob\",Engineering,2018-06-10,82000.75,true\n"
        "104,\"Brown, Alice\",HR,2021-11-30,58000.25,false\n"
        "105,\"Wilson, Charlie\",Engineering,2020-08-05,71000.00,true\n"
        "106,\"Lee, David\",Sales,2022-02-14,68000.50,true";

    CSVParserConfig config = { ',', '"', true, true, true };
    CSVData* data = parse_csv_file_content(integration_csv, &config);
    passed &= (assertNotNull(data) == 0);

    if (data) {
        printf("Test 1: Verify header functions\n");
        // Get header
        CSVRows* header = csv_get_header(data);
        passed &= (assertNotNull(header) == 0);
        passed &= (assertEquals(header->count, 6) == 0);

        // Get field names
        const char* dept_field = csv_get_field_name(data, 2);
        passed &= (assertStringsMatch(dept_field, "department") == 0);

        // Find field indices
        int salary_index = can_find_field_index(data, "salary");
        passed &= (assertEquals(salary_index, 4) == 0);

        printf("Test 2: Search by name\n");
        int engineering_count = search_in_csv_by_name(data, "department", "Engineering");
        passed &= (assertEquals(engineering_count, 3) == 0);

        int active_true = search_in_csv_by_name(data, "active", "true");
        passed &= (assertEquals(active_true, 5) == 0);

        printf("Test 3: Search by index\n");
        int active_false = search_in_csv_by_index(data, 5, "false");
        passed &= (assertEquals(active_false, 1) == 0);

        printf("Test 4: Export and re-import\n");
        export_csv(data, "integration_test.csv", ',');

        CSVData* exported_data = parse_csv_file("integration_test.csv", &config);
        passed &= (assertNotNull(exported_data) == 0);

        if (exported_data) {
            // Verify data integrity after export/import
            passed &= (assertEquals(exported_data->row_count, data->row_count) == 0);

            // Verify search still works
            int exported_engineering = search_in_csv_by_name(exported_data, "department", "Engineering");
            passed &= (assertEquals(exported_engineering, 3) == 0);

            free_csv_data(exported_data);
        }

        remove("integration_test.csv");
        free_csv_data(data);
    }

    printf("✓ CSV Integration Test: %s\n\n", passed ? "PASSED" : "FAILED");
}

int main() {
    printf("Starting Comprehensive CSV Tests\n\n");

    test_csv_basic_parsing();
    test_csv_with_quotes();
    test_csv_empty_and_missing_fields();
    test_csv_search_functionality();
    test_csv_large_dataset();
    test_csv_different_delimeters_comprehensive();

    test_csv_header_functions();
    test_csv_find_field_index();
    test_csv_search_by_name();
    test_csv_search_by_index();
    test_csv_export_with_header();
    test_csv_edge_cases_comprehensive();
    test_csv_integration();

    test_parser_debug();

    printf("=== All CSV Tests Completed ===\n");
    return 0;
}