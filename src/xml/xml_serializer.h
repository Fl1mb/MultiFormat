#ifndef MULTIFORMAT_XML_SERIALIZER_H
#define MULTIFORMAT_XML_SERIALIZER_H

#include "../core/data_types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t position;
    int indent_size;
    int depth;
    int error;
    char* error_msg;
} XMLSerializerContext;

// Serializer Help funcs
static void xml_serializer_init(XMLSerializerContext* ctx, int indent_size);
static void xml_serializer_free(XMLSerializerContext* ctx);
static int xml_serializer_ensure_space(XMLSerializerContext* ctx, size_t needed);
static int xml_serializer_write(XMLSerializerContext* ctx, const char* str);
static int xml_serializer_write_char(XMLSerializerContext* ctx, char ch);
static int xml_serializer_write_indent(XMLSerializerContext* ctx);
static int xml_serializer_write_newline(XMLSerializerContext* ctx);
static char* xml_escape_string(const char* str);
static int xml_serialize_declaration(XMLSerializerContext* ctx);
static int xml_serialize_attributes(XMLSerializerContext* ctx, const XMLNode* node);
static int xml_serialize_element(XMLSerializerContext* ctx, const XMLNode* node);
static int xml_serialize_text(XMLSerializerContext* ctx, const char* text);
static int xml_serialize_comment(XMLSerializerContext* ctx, const char* comment);
static int xml_serialize_cdata(XMLSerializerContext* ctx, const char* data);
static int xml_serialize_node_internal(XMLSerializerContext* ctx, const XMLNode* node);
static void xml_serializer_set_error(XMLSerializerContext* ctx, const char* message);
static XMLNodeType xml_determine_node_type(const XMLNode* node);
static char* xml_extract_comment_text(const char* comment_content);
static char* xml_extract_cdata_content(const char* cdata_content);
static int xml_serialize_plain_text(XMLSerializerContext* ctx, const XMLNode* node);
static int xml_serialize_comment_node(XMLSerializerContext* ctx, const XMLNode* node);
static int xml_serialize_cdata_node(XMLSerializerContext* ctx, const XMLNode* node);
static int xml_serialize_node_internal(XMLSerializerContext* ctx, const XMLNode* node);

// Main XML Serializer API
char* xml_serialize_document(const XMLDocument* doc, int indent_size);
char* xml_serialize_node_str(const XMLNode* node, int indent_size);
int xml_serialize_to_file(const XMLDocument* doc, const char* filename, int indent_size);
int xml_serialize_node_to_file(const XMLNode* node, const char* filename, int indent_size);
char* xml_serialize_document_compact(const XMLDocument* doc);
char* xml_serialize_document_pretty(const XMLDocument* doc);
const char* xml_serializer_get_error(const XMLSerializerContext* ctx);

#endif // MULTIFORMAT_XML_SERIALIZER_H