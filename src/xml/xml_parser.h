#ifndef MULTIFORMAT_XML_PARSER_H
#define MULTIFORMAT_XML_PARSER_H

#include "xml_help.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	XMLNode* current_node;
	XMLNode* root;
	int line;
	int column;
	const char* position;
	const char* buffer;
	size_t buffer_len;
	size_t index;
	int error;
	char* error_msg;
}XMLParserContext;

// Help funcs
static void xml_skip_whitespaces(XMLParserContext* ctx);
static void xml_skip_spaces(XMLParserContext* ctx);
static void xml_parse_content(XMLParserContext* ctx);
static void xml_parse_comment(XMLParserContext* ctx);
static void xml_parse_cdata(XMLParserContext* ctx);
static void xml_parse_declaration(XMLParserContext* ctx);
static void xml_parse_element(XMLParserContext* ctx);
static void xml_parse_attribute(XMLParserContext* ctx, XMLNode* node);
static char* xml_parse_string(XMLParserContext* ctx, char delimiter);
static char* xml_parse_name(XMLParserContext* ctx);
static void xml_set_error(XMLParserContext* ctx, const char* message);

// Main API funcs
XMLDocument* xml_parse_str(const char* xml_string);
void xml_document_free(XMLDocument* doc);
void xml_print_node(XMLNode* node, int indent);
void xml_print_document(XMLDocument* doc);

#endif // !MULTIFORMAT_XML_PARSER_H
