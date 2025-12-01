cmake_minimum_required(VERSION 3.16)

include(CTest)

# Separate test executables
add_executable(json_tests 
    src/test_common.c
    src/json_test.c
)

add_executable(csv_tests 
    src/test_common.c
    src/csv_test.c
)

# Link libraries
target_link_libraries(json_tests multiformat)
target_link_libraries(csv_tests multiformat)

# Include directories for json_tests
target_include_directories(json_tests 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
)

# Include directories for csv_tests
target_include_directories(csv_tests 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
)

# Add tests to CTest
add_test(NAME json_tests COMMAND json_tests)
add_test(NAME csv_tests COMMAND csv_tests)

# Set output directories
set_target_properties(json_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/
)

set_target_properties(csv_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/
)

# Coverage settings for GCC/Clang
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(json_tests PRIVATE --coverage)
    target_link_options(json_tests PRIVATE --coverage)
    target_compile_options(csv_tests PRIVATE --coverage)
    target_link_options(csv_tests PRIVATE --coverage)
endif()