cmake_minimum_required(VERSION 3.16)
project(multiformat VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" ON)

set(MULTIFORMAT_SOURCES
    include/json.c    

    src/json/json_parser.c
    src/json/json_serializer.c

)

add_library(multiformat ${MULTIFORMAT_SOURCES}  "src/json/json_parser.h" "src/json/json_serializer.h")

target_include_directories(multiformat 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(multiformat PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS multiformat
    EXPORT multiformat-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT multiformat-targets
    FILE multiformat-config.cmake
    NAMESPACE multiformat::
    DESTINATION lib/cmake/multiformat
)