#include "xml_help.h"

static XMLAttribute* xml_attribute_create(const char* name, const char* value)
{
	XMLAttribute* attr = (XMLAttribute*)malloc(sizeof(XMLAttribute));

	if (!attr)return NULL;

	attr->name = strdup(name);
	attr->value = strdup(value);
	attr->next = NULL;

	return attr;
}

static void xml_attribute_free(XMLAttribute* attribute)
{
	if (!attribute)return;

	free(attribute->name);
	free(attribute->value);
	free(attribute);
}

static XMLNode* xml_node_create(const char* name)
{
	XMLNode* node = (XMLNode*)malloc(sizeof(XMLNode));
	if (!node)return NULL;

	node->attributes = NULL;
	node->child = NULL;
	node->content = NULL;
	node->parent = NULL;
	node->next = NULL;
	node->name = strdup(name);

	return node;
}

static void xml_node_add_attribute(XMLNode* node, const char* name, const char* value)
{
	if (!name || !node || !value)return;

	XMLAttribute* attr = xml_attribute_create(name, value);

	if (!attr)return;

	attr->next = node->attributes;
	node->attributes = attr;
}

static void xml_node_add_child(XMLNode* node, XMLNode* child)
{
	if (!node || !child)return;

	child->parent = node;
	child->next = node->child;
	node->child = child;
}

static void xml_node_free_recursive(XMLNode* node)
{
	if (!node)return;

	XMLAttribute* attr = node->attributes;
	while (attr) {
		XMLAttribute* next = attr->next;
		xml_attribute_free(attr);
		attr = next;
	}

	XMLNode* child = node->child;
	while (child) {
		XMLNode* next = child->next;
		xml_node_free_recursive(child);
		child = next;
	}

	free(node->name);
	free(node->content);
	free(node);
}

