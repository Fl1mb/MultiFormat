cmake_minimum_required(VERSION 3.16)

include(CTest)

# Простая настройка тестов
set(TEST_SOURCES
    src/json_test.c
)

add_executable(tests_multiformat ${TEST_SOURCES})
target_link_libraries(tests_multiformat multiformat)

# Включаем все необходимые директории
target_include_directories(tests_multiformat 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/json
)

# Добавляем тест в CTest
add_test(NAME json_tests COMMAND tests_multiformat)

# Настройка выходной директории
set_target_properties(tests_multiformat PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/
)

# Настройки для coverage (GCC/Clang)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(tests_multiformat PRIVATE --coverage)
    target_link_options(tests_multiformat PRIVATE --coverage)
endif()