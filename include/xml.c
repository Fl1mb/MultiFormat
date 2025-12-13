#include "xml.h"

XMLDocument* xml_parse(const char* xml_str){
    if(!xml_str){
        return NULL;
    }

    return xml_parse_str(xml_str);
}

XMLDocument* xml_parse_file(const char* filename){
    if(!filename){return NULL;}

    FILE* file = fopen(filename, "rb");
    if(!file)return NULL;

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(file_size <= 0){
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(file_size + 1);
    if(!buffer){
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if(bytes_read != (size_t)file_size){
        free(buffer);
        return NULL;
    }

    buffer[file_size] = '\0';

    XMLDocument* xml_doc = xml_parse(buffer);

    free(buffer);

    return xml_doc;
}

void xml_document_free(XMLDocument* doc){
    if(!doc)return;

    if(doc->root){
        xml_node_free_recursive(doc->root);
    }

    if(doc->error_message){
        free(doc->error_message);
    }

    free(doc);
}

int xml_has_error(const XMLDocument* doc){
    if(!doc)return -1;
    return doc->error ? 1 : 0;
}

const char* xml_get_error(const XMLDocument* doc){
    if(!doc || !doc->error){
        return NULL;
    }
    return doc->error_message;
}

int xml_get_error_line(const XMLDocument* doc){
    if (!doc || !doc->error) {
        return 0;
    }
    return doc->line;
}

int xml_get_error_column(const XMLDocument* doc) {
    if (!doc || !doc->error) {
        return 0;
    }
    return doc->column;
}

char* xml_serialize(const XMLDocument* doc){
    if(!doc || doc->error || !doc->root){
        return NULL;
    }
    return xml_serialize_document_compact(doc);
}

char* xml_serialize_pretty(const XMLDocument* doc, int indent_size){
    if (!doc || doc->error || !doc->root) {
        return NULL;
    }
    
    if (indent_size <= 0) {
        indent_size = 2;
    }
    
    if (indent_size > 8) {
        indent_size = 8;
    }
    
    return xml_serialize_document(doc, indent_size);
}

int xml_serialize_file(const XMLDocument* doc, const char* filename, int indent_size){
    if (!doc || !filename) {
        return -1;
    }
    
    return xml_serialize_to_file(doc, filename, indent_size);
}

static NodeContentInfo xml_get_content_info(const XMLNode* node) {
    NodeContentInfo info = {NULL, 0};
    if (!node || !node->content) {
        return info;
    }
    
    info.content = node->content;
    info.content_len = strlen(node->content);
    return info;
}

static int is_comment_content(const char* content, size_t len) {
    return (len >= 7 && 
            strncmp(content, "<!--", 4) == 0 && 
            strncmp(content + len - 3, "-->", 3) == 0);
}

static int is_cdata_content(const char* content, size_t len) {
    return (len >= 12 && 
            strncmp(content, "<![CDATA[", 9) == 0 && 
            strncmp(content + len - 3, "]]>", 3) == 0);
}

XMLDocument* xml_create_document(void) {
    XMLDocument* doc = (XMLDocument*)malloc(sizeof(XMLDocument));
    if (!doc) {
        return NULL;
    }
    
    memset(doc, 0, sizeof(XMLDocument));
    return doc;
}

XMLNode* xml_create_element(const char* name) {
    if (!name) {
        return NULL;
    }
    
    XMLNode* node = xml_node_create(name);
    return node;
}

XMLNode* xml_create_text(const char* text) {
    if (!text) {
        text = "";
    }
    
    XMLNode* node = (XMLNode*)malloc(sizeof(XMLNode));
    if (!node) {
        return NULL;
    }
    
    memset(node, 0, sizeof(XMLNode));
    node->content = strdup(text);
    
    return node;
}

XMLNode* xml_create_comment(const char* comment) {
    if (!comment) {
        comment = "";
    }
    
    size_t len = strlen(comment);
    char* full_comment = (char*)malloc(len + 7 + 1); // <!-- --> + null
    if (!full_comment) {
        return NULL;
    }
    
    sprintf(full_comment, "<!--%s-->", comment);
    
    XMLNode* node = xml_create_text(full_comment);
    free(full_comment);
    
    return node;
}

XMLNode* xml_create_cdata(const char* data) {
    if (!data) {
        data = "";
    }
    
    size_t len = strlen(data);
    char* full_cdata = (char*)malloc(len + 12 + 1); // <![CDATA[]]> + null
    if (!full_cdata) {
        return NULL;
    }
    
    sprintf(full_cdata, "<![CDATA[%s]]>", data);
    
    XMLNode* node = xml_create_text(full_cdata);
    free(full_cdata);
    
    return node;
}

XMLNode* xml_get_root(const XMLDocument* doc) {
    if (!doc) {
        return NULL;
    }
    return doc->root;
}

int xml_set_root(XMLDocument* doc, XMLNode* root) {
    if (!doc || !root) {
        return -1;
    }
    
    if (root->parent) {
        return -2;
    }
    
    if (doc->root) {
        xml_node_free_recursive(doc->root);
    }
    
    doc->root = root;
    return 0;
}

XMLNodeType xml_get_node_type(const XMLNode* node) {
    if (!node) {
        return XML_NODE_UNKNOWN;
    }
    
    if (node->name) {
        return XML_NODE_ELEMENT;
    }
    
    if (node->content) {
        NodeContentInfo info = xml_get_content_info(node);
        if (is_comment_content(info.content, info.content_len)) {
            return XML_NODE_COMMENT;
        }
        if (is_cdata_content(info.content, info.content_len)) {
            return XML_NODE_CDATA;
        }
        return XML_NODE_TEXT;
    }
    
    return XML_NODE_UNKNOWN;
}

const char* xml_node_type_to_string(XMLNodeType type) {
    switch (type) {
        case XML_NODE_ELEMENT: return "ELEMENT";
        case XML_NODE_TEXT: return "TEXT";
        case XML_NODE_COMMENT: return "COMMENT";
        case XML_NODE_CDATA: return "CDATA";
        case XML_NODE_UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

const char* xml_get_node_name(const XMLNode* node) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return NULL;
    }
    return node->name;
}

int xml_set_node_name(XMLNode* node, const char* name) {
    if (!node || !name || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return -1;
    }
    
    if (node->name) {
        free(node->name);
    }
    
    node->name = strdup(name);
    if (!node->name) {
        return -2;
    }
    
    return 0;
}

const char* xml_get_node_text(const XMLNode* node) {
    if (!node) {
        return NULL;
    }
    
    XMLNodeType type = xml_get_node_type(node);
    if (type == XML_NODE_TEXT) {
        return node->content;
    }
    else if (type == XML_NODE_ELEMENT) {
        return node->content;
    }
    else if (type == XML_NODE_COMMENT) {
        NodeContentInfo info = xml_get_content_info(node);
        if (info.content_len >= 7) {
            static char comment_buffer[1024];
            size_t text_len = info.content_len - 7;
            if (text_len >= sizeof(comment_buffer)) {
                text_len = sizeof(comment_buffer) - 1;
            }
            strncpy(comment_buffer, info.content + 4, text_len);
            comment_buffer[text_len] = '\0';
            return comment_buffer;
        }
    }
    else if (type == XML_NODE_CDATA) {

        NodeContentInfo info = xml_get_content_info(node);
        if (info.content_len >= 12) {
            static char cdata_buffer[1024];
            size_t content_len = info.content_len - 12;
            if (content_len >= sizeof(cdata_buffer)) {
                content_len = sizeof(cdata_buffer) - 1;
            }
            strncpy(cdata_buffer, info.content + 9, content_len);
            cdata_buffer[content_len] = '\0';
            return cdata_buffer;
        }
    }
    
    return NULL;
}

int xml_set_node_text(XMLNode* node, const char* text) {
    if (!node) {
        return -1;
    }
    
    XMLNodeType type = xml_get_node_type(node);
    
    if (type == XML_NODE_ELEMENT || type == XML_NODE_TEXT) {
        if (node->content) {
            free(node->content);
        }
        
        if (text) {
            node->content = strdup(text);
            if (!node->content) {
                return -2;
            }
        } else {
            node->content = NULL;
        }
        
        return 0;
    }
    
    return -3;
}



size_t xml_get_attribute_count(const XMLNode* node) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return 0;
    }
    
    size_t count = 0;
    XMLAttribute* attr = node->attributes;
    while (attr) {
        count++;
        attr = attr->next;
    }
    
    return count;
}

const char* xml_get_attribute_name(const XMLNode* node, size_t index) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return NULL;
    }
    
    XMLAttribute* attr = node->attributes;
    size_t i = 0;
    
    while (attr && i < index) {
        attr = attr->next;
        i++;
    }
    
    return (attr && i == index) ? attr->name : NULL;
}

const char* xml_get_attribute_value(const XMLNode* node, size_t index) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return NULL;
    }
    
    XMLAttribute* attr = node->attributes;
    size_t i = 0;
    
    while (attr && i < index) {
        attr = attr->next;
        i++;
    }
    
    return (attr && i == index) ? attr->value : NULL;
}

const char* xml_get_attribute(const XMLNode* node, const char* name) {
    if (!node || !name || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return NULL;
    }
    
    XMLAttribute* attr = node->attributes;
    while (attr) {
        if (attr->name && strcmp(attr->name, name) == 0) {
            return attr->value;
        }
        attr = attr->next;
    }
    
    return NULL;
}

int xml_set_attribute(XMLNode* node, const char* name, const char* value) {
    if (!node || !name || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return -1;
    }
    
    XMLAttribute* attr = node->attributes;
    XMLAttribute* prev = NULL;
    
    while (attr) {
        if (attr->name && strcmp(attr->name, name) == 0) {
            if (attr->value) {
                free(attr->value);
            }
            attr->value = value ? strdup(value) : NULL;
            return 0;
        }
        prev = attr;
        attr = attr->next;
    }
    
    XMLAttribute* new_attr = xml_attribute_create(name, value ? value : "");
    if (!new_attr) {
        return -2;
    }
    
    new_attr->next = node->attributes;
    node->attributes = new_attr;
    
    return 0;
}

int xml_remove_attribute(XMLNode* node, const char* name) {
    if (!node || !name || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return -1;
    }
    
    XMLAttribute* attr = node->attributes;
    XMLAttribute* prev = NULL;
    
    while (attr) {
        if (attr->name && strcmp(attr->name, name) == 0) {
            if (prev) {
                prev->next = attr->next;
            } else {
                node->attributes = attr->next;
            }
            
            xml_attribute_free(attr);
            return 0;
        }
        prev = attr;
        attr = attr->next;
    }
    
    return -2;
}

int xml_has_attribute(const XMLNode* node, const char* name) {
    if (!node || !name || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return -1;
    }
    
    return xml_get_attribute(node, name) != NULL ? 1 : 0;
}

XMLNode* xml_get_parent(const XMLNode* node) {
    if (!node) {
        return NULL;
    }
    return node->parent;
}

XMLNode* xml_get_first_child(const XMLNode* node) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return NULL;
    }
    return node->child;
}

XMLNode* xml_get_next_sibling(const XMLNode* node) {
    if (!node) {
        return NULL;
    }
    return node->next;
}

XMLNode* xml_get_previous_sibling(const XMLNode* node) {
    if (!node || !node->parent) {
        return NULL;
    }
    
    XMLNode* child = node->parent->child;
    XMLNode* prev = NULL;
    
    while (child && child != node) {
        prev = child;
        child = child->next;
    }
    
    return prev;
}

size_t xml_get_child_count(const XMLNode* node) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return 0;
    }
    
    size_t count = 0;
    XMLNode* child = node->child;
    while (child) {
        count++;
        child = child->next;
    }
    
    return count;
}

XMLNode* xml_get_child(const XMLNode* node, size_t index) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return NULL;
    }
    
    XMLNode* child = node->child;
    size_t i = 0;
    
    while (child && i < index) {
        child = child->next;
        i++;
    }
    
    return (child && i == index) ? child : NULL;
}

int xml_add_child(XMLNode* parent, XMLNode* child) {
    if (!parent || !child || xml_get_node_type(parent) != XML_NODE_ELEMENT) {
        return -1;
    }
    
    if (child->parent) {
        return -2;
    }

    child->parent = parent;
    
    if (!parent->child) {
        parent->child = child;
    } else {
        XMLNode* last = parent->child;
        while (last->next) {
            last = last->next;
        }
        last->next = child;
    }
    
    return 0;
}

int xml_insert_child(XMLNode* parent, XMLNode* child, size_t index) {
    if (!parent || !child || xml_get_node_type(parent) != XML_NODE_ELEMENT) {
        return -1;
    }
    if (child->parent) {
        return -2;
    }
    child->parent = parent;
    
    if (index == 0 || !parent->child) {
        child->next = parent->child;
        parent->child = child;
        return 0;
    }
    
    XMLNode* prev = parent->child;
    size_t i = 0;
    
    while (prev->next && i < index - 1) {
        prev = prev->next;
        i++;
    }
    
    child->next = prev->next;
    prev->next = child;
    
    return 0;
}


int xml_remove_child(XMLNode* parent, XMLNode* child) {
    if (!parent || !child || child->parent != parent) {
        return -1;
    }
    
    if (parent->child == child) {
        parent->child = child->next;
    } else {
        XMLNode* prev = parent->child;
        while (prev && prev->next != child) {
            prev = prev->next;
        }
        
        if (prev) {
            prev->next = child->next;
        } else {
            return -2; 
        }
    }
    
    child->parent = NULL;
    child->next = NULL;
    
    return 0;
}

int xml_remove_and_free_child(XMLNode* parent, XMLNode* child) {
    int result = xml_remove_child(parent, child);
    if (result == 0) {
        xml_node_free_recursive(child);
    }
    return result;
}

void xml_remove_all_children(XMLNode* node, int free_children) {
    if (!node || xml_get_node_type(node) != XML_NODE_ELEMENT) {
        return;
    }
    
    XMLNode* child = node->child;
    while (child) {
        XMLNode* next = child->next;
        
        child->parent = NULL;
        
        if (free_children) {
            xml_node_free_recursive(child);
        }
        
        child = next;
    }
    
    node->child = NULL;
}

XMLNode* xml_clone_node(const XMLNode* node) {
    if (!node) {
        return NULL;
    }
    
    XMLNode* clone = (XMLNode*)malloc(sizeof(XMLNode));
    if (!clone) {
        return NULL;
    }
    
    memset(clone, 0, sizeof(XMLNode));
    
    if (node->name) {
        clone->name = strdup(node->name);
        if (!clone->name) {
            free(clone);
            return NULL;
        }
    }
    
    if (node->content) {
        clone->content = strdup(node->content);
        if (!clone->content) {
            free(clone->name);
            free(clone);
            return NULL;
        }
    }
    
    XMLAttribute* attr = node->attributes;
    XMLAttribute* last_attr = NULL;
    
    while (attr) {
        XMLAttribute* new_attr = xml_attribute_create(attr->name, attr->value);
        if (!new_attr) {
            xml_node_free_recursive(clone);
            return NULL;
        }
        
        if (last_attr) {
            last_attr->next = new_attr;
        } else {
            clone->attributes = new_attr;
        }
        
        last_attr = new_attr;
        attr = attr->next;
    }
    
    XMLNode* child = node->child;
    XMLNode* last_child = NULL;
    
    while (child) {
        XMLNode* child_clone = xml_clone_node(child);
        if (!child_clone) {
            xml_node_free_recursive(clone);
            return NULL;
        }
        
        child_clone->parent = clone;
        
        if (last_child) {
            last_child->next = child_clone;
        } else {
            clone->child = child_clone;
        }
        
        last_child = child_clone;
        child = child->next;
    }
    
    return clone;
}

XMLNode* xml_find_child(const XMLNode* parent, const char* name) {
    if (!parent || !name || xml_get_node_type(parent) != XML_NODE_ELEMENT) {
        return NULL;
    }
    
    XMLNode* child = parent->child;
    while (child) {
        if (xml_get_node_type(child) == XML_NODE_ELEMENT) {
            const char* child_name = xml_get_node_name(child);
            if (child_name && strcmp(child_name, name) == 0) {
                return child;
            }
        }
        child = child->next;
    }
    
    return NULL;
}

XMLNode** xml_find_children(const XMLNode* parent, const char* name, size_t* count) {
    if (!parent || !name || !count || xml_get_node_type(parent) != XML_NODE_ELEMENT) {
        *count = 0;
        return NULL;
    }
    
    size_t found_count = 0;
    XMLNode* child = parent->child;
    
    while (child) {
        if (xml_get_node_type(child) == XML_NODE_ELEMENT) {
            const char* child_name = xml_get_node_name(child);
            if (child_name && strcmp(child_name, name) == 0) {
                found_count++;
            }
        }
        child = child->next;
    }
    
    if (found_count == 0) {
        *count = 0;
        return NULL;
    }
    
    XMLNode** result = (XMLNode**)malloc(found_count * sizeof(XMLNode*));
    if (!result) {
        *count = 0;
        return NULL;
    }
    
    size_t index = 0;
    child = parent->child;
    
    while (child) {
        if (xml_get_node_type(child) == XML_NODE_ELEMENT) {
            const char* child_name = xml_get_node_name(child);
            if (child_name && strcmp(child_name, name) == 0) {
                result[index++] = (XMLNode*)child; // const cast
            }
        }
        child = child->next;
    }
    
    *count = found_count;
    return result;
}

XMLNode* xml_find_by_path(const XMLNode* root, const char* path) {
    if (!root || !path || xml_get_node_type(root) != XML_NODE_ELEMENT) {
        return NULL;
    }
    
    char* path_copy = strdup(path);
    if (!path_copy) {
        return NULL;
    }
    
    XMLNode* current = (XMLNode*)root; 
    char* token = strtok(path_copy, "/");
    
    while (token && current) {
        current = xml_find_child(current, token);
        token = strtok(NULL, "/");
    }
    
    free(path_copy);
    return current;
}

int xml_is_well_formed(const char* xml_str) {
    if (!xml_str) {
        return -1;
    }
    
    XMLDocument* doc = xml_parse(xml_str);
    if (!doc) {
        return 0;
    }
    
    int well_formed = (doc->error == 0);
    xml_document_free(doc);
    
    return well_formed ? 1 : 0;
}

char* xml_escape_string(const char* str) {
    return xml_escape_string(str);
}

char* xml_unescape_string(const char* str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (!result) {
        return NULL;
    }
    
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '&') {
            if (i + 4 < len && strncmp(&str[i], "&amp;", 5) == 0) {
                result[pos++] = '&';
                i += 4;
            }
            else if (i + 3 < len && strncmp(&str[i], "&lt;", 4) == 0) {
                result[pos++] = '<';
                i += 3;
            }
            else if (i + 3 < len && strncmp(&str[i], "&gt;", 4) == 0) {
                result[pos++] = '>';
                i += 3;
            }
            else if (i + 5 < len && strncmp(&str[i], "&quot;", 6) == 0) {
                result[pos++] = '"';
                i += 5;
            }
            else if (i + 5 < len && strncmp(&str[i], "&apos;", 6) == 0) {
                result[pos++] = '\'';
                i += 5;
            }
            else {
                result[pos++] = str[i];
            }
        } else {
            result[pos++] = str[i];
        }
    }
    
    result[pos] = '\0';
    
    result = (char*)realloc(result, pos + 1);
    return result;
}