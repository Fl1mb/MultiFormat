#include "xml_serializer.h"

static void xml_serializer_init(XMLSerializerContext* ctx, int indent_size) {
    if (!ctx) return;
    
    memset(ctx, 0, sizeof(XMLSerializerContext));
    ctx->indent_size = indent_size > 0 ? indent_size : 0;
    ctx->buffer_size = 4096;
    ctx->buffer = (char*)malloc(ctx->buffer_size);
    if (ctx->buffer) {
        ctx->buffer[0] = '\0';
    } else {
        ctx->error = 1;
        ctx->error_msg = strdup("Memory allocation failed");
    }
}

static void xml_serializer_free(XMLSerializerContext* ctx) {
    if (!ctx) return;
    
    if (ctx->buffer) {
        free(ctx->buffer);
    }
    if (ctx->error_msg) {
        free(ctx->error_msg);
    }
}

static int xml_serializer_ensure_space(XMLSerializerContext* ctx, size_t needed) {
    if (!ctx || !ctx->buffer) return -1;
    
    if (ctx->position + needed + 1 >= ctx->buffer_size) {
        size_t new_size = ctx->buffer_size * 2;
        while (new_size < ctx->position + needed + 1) {
            new_size *= 2;
        }
        
        char* new_buffer = (char*)realloc(ctx->buffer, new_size);
        if (!new_buffer) {
            xml_serializer_set_error(ctx, "Memory allocation failed");
            return -1;
        }
        
        ctx->buffer = new_buffer;
        ctx->buffer_size = new_size;
    }
    
    return 0;
}

static int xml_serializer_write(XMLSerializerContext* ctx, const char* str) {
    if (!ctx || !str || ctx->error) return -1;
    
    size_t len = strlen(str);
    if (xml_serializer_ensure_space(ctx, len) != 0) {
        return -1;
    }
    
    memcpy(ctx->buffer + ctx->position, str, len);
    ctx->position += len;
    ctx->buffer[ctx->position] = '\0';
    
    return 0;
}

static int xml_serializer_write_char(XMLSerializerContext* ctx, char ch) {
    if (!ctx || ctx->error) return -1;
    
    if (xml_serializer_ensure_space(ctx, 1) != 0) {
        return -1;
    }
    
    ctx->buffer[ctx->position++] = ch;
    ctx->buffer[ctx->position] = '\0';
    
    return 0;
}

static int xml_serializer_write_indent(XMLSerializerContext* ctx) {
    if (!ctx || ctx->error || ctx->indent_size <= 0) return -1;
    
    int spaces = ctx->depth * ctx->indent_size;
    for (int i = 0; i < spaces; i++) {
        if (xml_serializer_write_char(ctx, ' ') != 0) {
            return -1;
        }
    }
    
    return 0;
}

static int xml_serializer_write_newline(XMLSerializerContext* ctx) {
    if (!ctx || ctx->error || ctx->indent_size <= 0) return -1;
    
    return xml_serializer_write_char(ctx, '\n');
}

static char* xml_escape_string(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    size_t escaped_len = 0;
    
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '&': escaped_len += 5; break;  // &amp;
            case '<': escaped_len += 4; break;  // &lt;
            case '>': escaped_len += 4; break;  // &gt;
            case '"': escaped_len += 6; break;  // &quot;
            case '\'': escaped_len += 6; break; // &apos;
            default: escaped_len += 1; break;
        }
    }

    char* escaped = (char*)malloc(escaped_len + 1);
    if (!escaped) return NULL;
    
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '&':
                memcpy(escaped + pos, "&amp;", 5);
                pos += 5;
                break;
            case '<':
                memcpy(escaped + pos, "&lt;", 4);
                pos += 4;
                break;
            case '>':
                memcpy(escaped + pos, "&gt;", 4);
                pos += 4;
                break;
            case '"':
                memcpy(escaped + pos, "&quot;", 6);
                pos += 6;
                break;
            case '\'':
                memcpy(escaped + pos, "&apos;", 6);
                pos += 6;
                break;
            default:
                escaped[pos++] = str[i];
                break;
        }
    }
    
    escaped[pos] = '\0';
    return escaped;
}

static int xml_serialize_declaration(XMLSerializerContext* ctx) {
    return xml_serializer_write(ctx, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
}

static int xml_serialize_attributes(XMLSerializerContext* ctx, const XMLNode* node) {
    if (!ctx || !node) return -1;
    
    XMLAttribute* attr = node->attributes;
    while (attr && !ctx->error) {
        if (attr->name && attr->value) {
            if (xml_serializer_write_char(ctx, ' ') != 0) {
                return -1;
            }

            if (xml_serializer_write(ctx, attr->name) != 0) {
                return -1;
            }
            
            if (xml_serializer_write(ctx, "=\"") != 0) {
                return -1;
            }
            
            char* escaped_value = xml_escape_string(attr->value);
            if (!escaped_value) {
                xml_serializer_set_error(ctx, "Failed to escape attribute value");
                return -1;
            }
            
            if (xml_serializer_write(ctx, escaped_value) != 0) {
                free(escaped_value);
                return -1;
            }
            
            free(escaped_value);
            
            if (xml_serializer_write_char(ctx, '"') != 0) {
                return -1;
            }
        }
        attr = attr->next;
    }
    
    return 0;
}

static int xml_serialize_element(XMLSerializerContext* ctx, const XMLNode* node) {
    if (!ctx || !node || !node->name) return -1;
    
    if (ctx->indent_size > 0 && ctx->depth > 0) {
        if (xml_serializer_write_indent(ctx) != 0) {
            return -1;
        }
    }
    
    if (xml_serializer_write_char(ctx, '<') != 0) {
        return -1;
    }
    
    if (xml_serializer_write(ctx, node->name) != 0) {
        return -1;
    }

    if (xml_serialize_attributes(ctx, node) != 0) {
        return -1;
    }
    
    int has_children = node->child != NULL;
    int has_content = node->content != NULL && strlen(node->content) > 0;

    int has_only_text_children = 0;
    if (has_children && !has_content) {
        XMLNode* child = node->child;
        has_only_text_children = 1;
        
        while (child) {
            XMLNodeType child_type = xml_determine_node_type(child);
            if (child_type != XML_NODE_TEXT && child_type != XML_NODE_CDATA) {
                has_only_text_children = 0;
                break;
            }
            child = child->next;
        }
    }
    
    if (!has_children && !has_content) {
        if (xml_serializer_write(ctx, "/>") != 0) {
            return -1;
        }
        
        if (ctx->indent_size > 0) {
            if (xml_serializer_write_newline(ctx) != 0) {
                return -1;
            }
        }
        
        return 0;
    }
    
    if (xml_serializer_write_char(ctx, '>') != 0) {
        return -1;
    }

    if (has_content) {
        char* escaped_content = xml_escape_string(node->content);
        if (!escaped_content) {
            xml_serializer_set_error(ctx, "Failed to escape content");
            return -1;
        }
        
        if (xml_serializer_write(ctx, escaped_content) != 0) {
            free(escaped_content);
            return -1;
        }
        
        free(escaped_content);
        
        if (has_children && ctx->indent_size > 0) {
            if (xml_serializer_write_newline(ctx) != 0) {
                return -1;
            }
        }
    }

    if (has_children) {
        if (ctx->indent_size > 0 && !has_only_text_children) {
            if (xml_serializer_write_newline(ctx) != 0) {
                return -1;
            }
        }
        
        ctx->depth++;

        XMLNode* child = node->child;
        while (child && !ctx->error) {
            XMLNodeType child_type = xml_determine_node_type(child);

            if (has_only_text_children && (child_type == XML_NODE_TEXT || child_type == XML_NODE_CDATA)) {
                int result = xml_serialize_node_internal(ctx, child);
                if (result != 0) {
                    ctx->depth--;
                    return -1;
                }
            } else {
                if (xml_serialize_node_internal(ctx, child) != 0) {
                    ctx->depth--;
                    return -1;
                }
            }
            child = child->next;
        }

        ctx->depth--;

        if (ctx->indent_size > 0 && !has_only_text_children) {
            if (xml_serializer_write_indent(ctx) != 0) {
                return -1;
            }
        }
    }
    
    if (xml_serializer_write(ctx, "</") != 0) {
        return -1;
    }
    
    if (xml_serializer_write(ctx, node->name) != 0) {
        return -1;
    }
    
    if (xml_serializer_write_char(ctx, '>') != 0) {
        return -1;
    }
    
    if (ctx->indent_size > 0 && (has_children || ctx->depth == 0)) {
        if (xml_serializer_write_newline(ctx) != 0) {
            return -1;
        }
    }
    
    return 0;
}

static int xml_serialize_text(XMLSerializerContext* ctx, const char* text) {
     if (!ctx || !text) return -1;

    int only_whitespace = 1;
    for (const char* p = text; *p; p++) {
        if (!isspace((unsigned char)*p)) {
            only_whitespace = 0;
            break;
        }
    }
    
    if (only_whitespace && ctx->indent_size > 0) {
        return xml_serializer_write(ctx, text);
    }
    
    char* escaped_text = xml_escape_string(text);
    if (!escaped_text) {
        xml_serializer_set_error(ctx, "Failed to escape text");
        return -1;
    }
    
    int result = xml_serializer_write(ctx, escaped_text);
    free(escaped_text);
    
    return result;
}

static int xml_serialize_comment(XMLSerializerContext* ctx, const char* comment) {
    if (!ctx || !comment) return -1;
    
    if (ctx->indent_size > 0) {
        if (xml_serializer_write_indent(ctx) != 0) {
            return -1;
        }
    }

    if (xml_serializer_write(ctx, "<!--") != 0) {
        return -1;
    }
    
    if (xml_serializer_write(ctx, comment) != 0) {
        return -1;
    }
    
    if (xml_serializer_write(ctx, "-->") != 0) {
        return -1;
    }
    
    if (ctx->indent_size > 0) {
        if (xml_serializer_write_newline(ctx) != 0) {
            return -1;
        }
    }
    
    return 0;
}

static int xml_serialize_cdata(XMLSerializerContext* ctx, const char* data) {
    if (!ctx || !data) return -1;
    
    if (ctx->indent_size > 0) {
        if (xml_serializer_write_indent(ctx) != 0) {
            return -1;
        }
    }
    
    if (xml_serializer_write(ctx, "<![CDATA[") != 0) {
        return -1;
    }
    
    if (xml_serializer_write(ctx, data) != 0) {
        return -1;
    }
    
    if (xml_serializer_write(ctx, "]]>") != 0) {
        return -1;
    }
    
    if (ctx->indent_size > 0) {
        if (xml_serializer_write_newline(ctx) != 0) {
            return -1;
        }
    }
    
    return 0;
}

static void xml_serializer_set_error(XMLSerializerContext* ctx, const char* message) {
    if (!ctx || !message) return;
    
    ctx->error = 1;
    if (ctx->error_msg) {
        free(ctx->error_msg);
    }
    ctx->error_msg = strdup(message);
}

static XMLNodeType xml_determine_node_type(const XMLNode* node){
    if (!node) {
        return XML_NODE_UNKNOWN;
    }

    if (!node->name && node->content) {
        const char* content = node->content;
        size_t len = strlen(content);
        if (len >= 7 && strncmp(content, "<!--", 4) == 0 && 
            strncmp(content + len - 3, "-->", 3) == 0) {
            return XML_NODE_COMMENT;
        }
        else if (len >= 12 && strncmp(content, "<![CDATA[", 9) == 0 && 
                 strncmp(content + len - 3, "]]>", 3) == 0) {
            return XML_NODE_CDATA;
        }
        else {
            return XML_NODE_TEXT;
        }
    }else if(node->name){
        return XML_NODE_ELEMENT;
    }
    return XML_NODE_UNKNOWN;
}

static char* xml_extract_comment_text(const char* comment_content){
    if (!comment_content) return NULL;
    
    size_t len = strlen(comment_content);
    if (len < 7) return NULL;

    size_t text_len = len - 7;
    char* text = (char*)malloc(text_len + 1);
    if (!text) return NULL;
    
    strncpy(text, comment_content + 4, text_len);
    text[text_len] = '\0';
    
    return text;
}

static char* xml_extract_cdata_content(const char* cdata_content){
    if (!cdata_content) return NULL;
    
    size_t len = strlen(cdata_content);
    if (len < 12) return NULL; 

    size_t content_len = len - 12;
    char* content = (char*)malloc(content_len + 1);
    if (!content) return NULL;
    
    strncpy(content, cdata_content + 9, content_len);
    content[content_len] = '\0';
    
    return content;
}

static int xml_serialize_plain_text(XMLSerializerContext* ctx, const XMLNode* node){
    if (!ctx || !node || !node->content) return -1;
    
    if (ctx->indent_size > 0 && ctx->depth > 0) {
        XMLNode* parent = node->parent;
        if (parent && parent->child != node) {
            if (xml_serializer_write_indent(ctx) != 0) {
                return -1;
            }
        }
    }
    
    return xml_serialize_text(ctx, node->content);
}

static int xml_serialize_comment_node(XMLSerializerContext* ctx, const XMLNode* node){
     if (!ctx || !node || !node->content) return -1;
    
    char* comment_text = xml_extract_comment_text(node->content);
    if (!comment_text) {
        comment_text = strdup(node->content);
        if (!comment_text) {
            xml_serializer_set_error(ctx, "Memory allocation failed");
            return -1;
        }
    }
    
    int result = xml_serialize_comment(ctx, comment_text);
    free(comment_text);
    
    return result;
}

static int xml_serialize_cdata_node(XMLSerializerContext* ctx, const XMLNode* node){
    if (!ctx || !node || !node->content) return -1;

    char* cdata_content = xml_extract_cdata_content(node->content);
    if (!cdata_content) {
        // If extraction fails, use raw content
        cdata_content = strdup(node->content);
        if (!cdata_content) {
            xml_serializer_set_error(ctx, "Memory allocation failed");
            return -1;
        }
    }

    int result = xml_serialize_cdata(ctx, cdata_content);
    free(cdata_content);
    
    return result;
}

static int xml_serialize_node_internal(XMLSerializerContext* ctx, const XMLNode* node){
    if (!ctx || !node) return -1;
    
    XMLNodeType node_type = xml_determine_node_type(node);
    
    switch (node_type) {
        case XML_NODE_ELEMENT:
            return xml_serialize_element(ctx, node);
            
        case XML_NODE_TEXT:
            return xml_serialize_plain_text(ctx, node);
            
        case XML_NODE_COMMENT:
            return xml_serialize_comment_node(ctx, node);
            
        case XML_NODE_CDATA:
            return xml_serialize_cdata_node(ctx, node);
            
        case XML_NODE_UNKNOWN:
        default:
            xml_serializer_set_error(ctx, "Unknown node type");
            return -1;
    }
}



char* xml_serialize_document(const XMLDocument* doc, int indent_size) {
    if (!doc || doc->error || !doc->root) {
        return NULL;
    }
    
    XMLSerializerContext ctx;
    xml_serializer_init(&ctx, indent_size);
    
    if (ctx.error) {
        xml_serializer_free(&ctx);
        return NULL;
    }
    
    if (xml_serialize_declaration(&ctx) != 0) {
        xml_serializer_free(&ctx);
        return NULL;
    }
    
    if (ctx.indent_size > 0) {
        if (xml_serializer_write_newline(&ctx) != 0) {
            xml_serializer_free(&ctx);
            return NULL;
        }
    }
    
    if (xml_serialize_node_internal(&ctx, doc->root) != 0) {
        xml_serializer_free(&ctx);
        return NULL;
    }
    
    char* result = strdup(ctx.buffer);
    xml_serializer_free(&ctx);
    
    return result;
}

char* xml_serialize_node_str(const XMLNode* node, int indent_size) {
    if (!node) {
        return NULL;
    }
    
    XMLSerializerContext ctx;
    xml_serializer_init(&ctx, indent_size);
    
    if (ctx.error) {
        xml_serializer_free(&ctx);
        return NULL;
    }
    
    if (xml_serialize_node_internal(&ctx, node) != 0) {
        xml_serializer_free(&ctx);
        return NULL;
    }
    
    char* result = strdup(ctx.buffer);
    xml_serializer_free(&ctx);
    
    return result;
}

int xml_serialize_to_file(const XMLDocument* doc, const char* filename, int indent_size) {
    if (!doc || !filename) {
        return -1;
    }
    
    char* xml_str = xml_serialize_document(doc, indent_size);
    if (!xml_str) {
        return -1;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        free(xml_str);
        return -1;
    }
    
    size_t len = strlen(xml_str);
    size_t written = fwrite(xml_str, 1, len, file);
    fclose(file);
    free(xml_str);
    
    return (written == len) ? 0 : -1;
}

int xml_serialize_node_to_file(const XMLNode* node, const char* filename, int indent_size) {
    if (!node || !filename) {
        return -1;
    }
    
    char* xml_str = xml_serialize_node_str(node, indent_size);
    if (!xml_str) {
        return -1;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        free(xml_str);
        return -1;
    }
    
    size_t len = strlen(xml_str);
    size_t written = fwrite(xml_str, 1, len, file);
    fclose(file);
    free(xml_str);
    
    return (written == len) ? 0 : -1;
}

char* xml_serialize_document_compact(const XMLDocument* doc) {
    return xml_serialize_document(doc, 0);
}

char* xml_serialize_document_pretty(const XMLDocument* doc) {
    return xml_serialize_document(doc, 2);
}

const char* xml_serializer_get_error(const XMLSerializerContext* ctx) {
    if (!ctx || !ctx->error) {
        return NULL;
    }
    return ctx->error_msg;
}