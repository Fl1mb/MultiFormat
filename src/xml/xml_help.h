#ifndef MULTIFORMAT_XML_HELP_H
#define MULTIFORMAT_XML_HELP_H

#include "../core/data_types.h"
#include <string.h>

static XMLAttribute* xml_attribute_create(const char* name, const char* value);
static void xml_attribute_free(XMLAttribute* attribute);
static XMLNode* xml_node_create(const char* name);
static void xml_node_add_attribute(XMLNode* node, const char* name, const char* value);
static void xml_node_add_child(XMLNode* node, XMLNode* child);
static void xml_node_free_recursive(XMLNode* node);

#endif // !MULTIFORMAT_XML_HELP_H
