#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

#include <stddef.h>

typedef enum{
    DT_JSON,
    DT_CSV,
    DT_XML
}data_format_t;

typedef struct 
{
    data_format_t format;
    void* internal_data;
} data_object_t;

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type_t;

typedef struct json_value json_value_t;

struct json_object_entry {
    char* key;
    json_value_t* value;
};

struct json_value {
    json_type_t type;
    union {
        int boolean;
        double number;
        char* string;
        struct {
            json_value_t** values;
            size_t count;
            size_t capacity;
        } array;
        struct {
            struct json_object_entry* entries;
            size_t count;
            size_t capacity;
        } object;
    } data;
};

typedef struct free_stack_node {
    json_value_t* value;
    struct free_stack_node* next;
}free_stack_node_t;


typedef struct {
    char** fields;
    int count;
}CSVRows;

typedef struct {
    CSVRows* rows;
    int row_count;
    int max_fields;
    CSVRows header;
    int have_header;
}CSVData;

typedef struct XMLAttribute {
    char* name;
    char* value;
    struct XMLAttribute* next;
}XMLAttribute;

typedef struct XMLNode {
    char* name;
    char* content;
    XMLAttribute* attributes;
    struct XMLNode* parent;
    struct XMLNode* child;
    struct XMLNode* next;
}XMLNode;

typedef struct {
    XMLNode* root;
    int error;
    char* error_message;
    int line;
    int column;
}XMLDocument;

typedef enum{
    XML_NODE_ELEMENT,
    XML_NODE_TEXT,
    XML_NODE_COMMENT,
    XML_NODE_CDATA,
    XML_NODE_UNKNOWN
} XMLNodeType;

typedef struct {
    const char* content;
    size_t content_len;
} NodeContentInfo;

#endif // DATA_TYPES_H_