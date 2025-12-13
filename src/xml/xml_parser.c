#include "xml_parser.h"

static void xml_skip_whitespaces(XMLParserContext* ctx)
{
	while (ctx->index < ctx->buffer_len
		&& isspace(ctx->buffer[ctx->index])) 
	{
		if (ctx->buffer[ctx->index] == '\n') {
			ctx->line++;
			ctx->column = 1;
		}
		else {
			ctx->column++;
		}
		ctx->index++;
	}
}

static void xml_set_error(XMLParserContext* ctx, const char* message){
	if(!ctx) return;

	ctx->error = 1;
	if(ctx->error_msg){
		free(ctx->error_msg);
	}
	ctx->error_msg = message;
}

static void xml_skip_spaces(XMLParserContext* ctx){
	while (ctx->index < ctx->buffer_len && 
           (ctx->buffer[ctx->index] == ' ' || 
            ctx->buffer[ctx->index] == '\t')) {
        ctx->column++;
        ctx->index++;
    }
}

static char* xml_parse_name(XMLParserContext* ctx){
	if(ctx->index >= ctx->buffer_len ||
		!(isalpha(ctx->buffer[ctx->index])) ||
		ctx->buffer[ctx->index] == '_' ||
		ctx->buffer[ctx->index] == ':'
	){
		xml_set_error(ctx, "Expected name");
		return NULL;
	}

	const char* start = ctx->buffer + ctx->index;
	size_t length = 0;

	while (ctx->index < ctx->buffer_len &&
           (isalnum(ctx->buffer[ctx->index]) ||
            ctx->buffer[ctx->index] == '_' ||
            ctx->buffer[ctx->index] == ':' ||
            ctx->buffer[ctx->index] == '-' ||
            ctx->buffer[ctx->index] == '.')) {
        ctx->index++;
        ctx->column++;
        length++;
    }

	char* name = (char*)malloc(length + 1);
	if(!name){
		xml_set_error(ctx, "Memory allocation failed");
		return NULL;
	}

	strncpy(name, start, length);
	name[length] = '\0';
	return name;
}

static char* xml_parse_string(XMLParserContext* ctx, char delimiter){
	if (ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != delimiter) {
        xml_set_error(ctx, "Expected string delimiter");
        return NULL;
    }

	ctx->index++;
	ctx->column++;

	const char* start = ctx->buffer + ctx->index;
	size_t length = 0;

	while(ctx->index < ctx->buffer_len &&
		ctx->buffer[ctx->index] != delimiter
	){
		if(ctx->buffer[ctx->index] == '&'){
			
			while (ctx->index < ctx->buffer_len && 
                   ctx->buffer[ctx->index] != ';') {
                ctx->index++;
                ctx->column++;
                length++;
			}

			if (ctx->index < ctx->buffer_len) {
                ctx->index++;
                ctx->column++;
                length++;
            }
		}
		else {
            if (ctx->buffer[ctx->index] == '\n') {
                ctx->line++;
                ctx->column = 1;
            } else {
                ctx->column++;
            }
            ctx->index++;
            length++;
        }
	}

	if (ctx->index >= ctx->buffer_len) {
        xml_set_error(ctx, "Unterminated string");
        free((void*)start);
        return NULL;
    }

	char* str = (char*)malloc(length + 1);
    if (!str) {
        xml_set_error(ctx, "Memory allocation failed");
        return NULL;
    }

    strncpy(str, start, length);
    str[length] = '\0';

	ctx->index++;
	ctx->column++;
	return str;
}

static void xml_parse_attributes(XMLParserContext* ctx, XMLNode* node){
	xml_skip_spaces(ctx);

	while(
		ctx->index < ctx->buffer_len &&
		ctx->buffer[ctx->index] != '>' &&
		ctx->buffer[ctx->index] != '/' &&
		ctx->buffer[ctx->index] != '?'
	){
		char* attr_name = xml_parse_name(ctx);
		if(!attr_name){
			return;
		}

		xml_skip_spaces(ctx);

		if(ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != '='){
			xml_set_error(ctx, "Expected '=' after attribute name");
			free(attr_name);
			return;
		}

		ctx->index++;
		ctx->column++;

		xml_skip_spaces(ctx);

		char* attr_value = NULL;

		if
		(
			ctx->index < ctx->buffer_len &&
			(ctx->buffer[ctx->index] == '\'' ||
			ctx->buffer[ctx->index] == '"'
			)
		){
			attr_value = xml_parse_string(ctx, ctx->buffer[ctx->index]);
		}

		if(!attr_value){
			free(attr_name);
			return;
		}

		xml_node_add_attribute(node, attr_name, attr_value);

		free(attr_name);
		free(attr_value);
		xml_skip_spaces(ctx);
	}
}

static void xml_parse_comment(XMLParserContext* ctx){
	if
	(
		ctx->index + 3 >= ctx->buffer_len ||
		strncmp(ctx->buffer + ctx->index, "!--", 3) != 0
	)
	{
		xml_set_error(ctx, "Invalid comment start");
		return;
	}

	ctx->index += 3;
	ctx->column += 3;

	while(ctx->index + 2 < ctx->buffer_len){
		if
		(
			ctx->buffer[ctx->index] == '-' &&
            ctx->buffer[ctx->index + 1] == '-' &&
            ctx->buffer[ctx->index + 2] == '>'
		)
		{
			ctx->index += 3;
            ctx->column += 3;
            return;
		}

		if (ctx->buffer[ctx->index] == '\n') {
            ctx->line++;
            ctx->column = 1;
        } else {
            ctx->column++;
        }
		
		ctx->index++;
	}

	xml_set_error(ctx, "Unterminated comment");
}

static void xml_parse_cdata(XMLParserContext* ctx){
	if (ctx->index + 8 >= ctx->buffer_len ||
        strncmp(ctx->buffer + ctx->index, "![CDATA[", 8) != 0) {
        xml_set_error(ctx, "Invalid CDATA start");
        return;
    }

	ctx->index += 8;
    ctx->column += 8;
    
    const char* start = ctx->buffer + ctx->index;
    size_t length = 0;

	 while (ctx->index + 2 < ctx->buffer_len) {
        if (ctx->buffer[ctx->index] == ']' &&
            ctx->buffer[ctx->index + 1] == ']' &&
            ctx->buffer[ctx->index + 2] == '>') {
            
            if (ctx->current_node) {
                char* cdata = (char*)malloc(length + 1);
                if (cdata) {
                    strncpy(cdata, start, length);
                    cdata[length] = '\0';
                    
                    if (ctx->current_node->content) {
                        size_t old_len = strlen(ctx->current_node->content);
                        size_t new_len = old_len + length;
                        char* new_content = (char*)realloc(ctx->current_node->content, new_len + 1);
                        if (new_content) {
                            strcpy(new_content + old_len, cdata);
                            ctx->current_node->content = new_content;
                        }
                        free(cdata);
                    } else {
                        ctx->current_node->content = cdata;
                    }
                }
            }
            
            ctx->index += 3;
            ctx->column += 3;
            return;
        }
        
        if (ctx->buffer[ctx->index] == '\n') {
            ctx->line++;
            ctx->column = 1;
        } else {
            ctx->column++;
        }
        
        ctx->index++;
        length++;
    }
	
	xml_set_error(ctx, "Unterminated CDATA");
}

static void xml_parse_declaration(XMLParserContext* ctx){
	
	ctx->index += 2;
	ctx->column += 2;

	char* decl_name = xml_parse_name(ctx);
    if (!decl_name) {
        return;
    }

	if (strcmp(decl_name, "xml") != 0) {
        free(decl_name);
        while (ctx->index < ctx->buffer_len) {
            if (ctx->buffer[ctx->index] == '?' &&
                ctx->index + 1 < ctx->buffer_len &&
                ctx->buffer[ctx->index + 1] == '>') {
                ctx->index += 2;
                ctx->column += 2;
                return;
            }
            ctx->index++;
            ctx->column++;
        }
        xml_set_error(ctx, "Unterminated processing instruction");
        return;
    }

	free(decl_name);
    
    xml_skip_spaces(ctx);

	while (ctx->index < ctx->buffer_len && 
           ctx->buffer[ctx->index] != '?') {
        
        char* attr_name = xml_parse_name(ctx);
        if (!attr_name) {
            return;
        }
        
        xml_skip_spaces(ctx);
        
        if (ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != '=') {
            free(attr_name);
            xml_set_error(ctx, "Expected '=' in XML declaration");
            return;
        }
        
        ctx->index++;
        ctx->column++;
        
        xml_skip_spaces(ctx);
        
        char* attr_value = NULL;
        if (ctx->index < ctx->buffer_len && 
            (ctx->buffer[ctx->index] == '\'' || 
             ctx->buffer[ctx->index] == '"')) {
            attr_value = xml_parse_string(ctx, ctx->buffer[ctx->index]);
        }
        
        if (!attr_value) {
            free(attr_name);
            return;
        }

        free(attr_name);
        free(attr_value);
        
        xml_skip_spaces(ctx);
    }

	if (ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != '?') {
        xml_set_error(ctx, "Expected '?' in XML declaration");
        return;
    }

	ctx->index++;
    ctx->column++;

	if (ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != '>') {
        xml_set_error(ctx, "Expected '>' after XML declaration");
        return;
    }
    
    ctx->index++;
    ctx->column++;
}

static void xml_parse_content(XMLParserContext* ctx){
	const char* start = ctx->buffer + ctx->index;
    size_t length = 0;
    
    while (ctx->index < ctx->buffer_len && 
           ctx->buffer[ctx->index] != '<') {

        if (ctx->buffer[ctx->index] == '&') {
            if (ctx->index + 3 < ctx->buffer_len &&
                strncmp(ctx->buffer + ctx->index, "&lt;", 4) == 0) {
                if (ctx->current_node && !ctx->current_node->content) {
                    ctx->current_node->content = strdup("<");
                } else if (ctx->current_node && ctx->current_node->content) {
                    size_t old_len = strlen(ctx->current_node->content);
                    char* new_content = (char*)realloc(ctx->current_node->content, old_len + 2);
                    if (new_content) {
                        new_content[old_len] = '<';
                        new_content[old_len + 1] = '\0';
                        ctx->current_node->content = new_content;
                    }
                }
                ctx->index += 4;
                ctx->column += 4;
                length += 1;
                continue;
            }
            else if (ctx->index + 3 < ctx->buffer_len &&
                     strncmp(ctx->buffer + ctx->index, "&gt;", 4) == 0) {
                if (ctx->current_node && !ctx->current_node->content) {
                    ctx->current_node->content = strdup(">");
                } else if (ctx->current_node && ctx->current_node->content) {
                    size_t old_len = strlen(ctx->current_node->content);
                    char* new_content = (char*)realloc(ctx->current_node->content, old_len + 2);
                    if (new_content) {
                        new_content[old_len] = '>';
                        new_content[old_len + 1] = '\0';
                        ctx->current_node->content = new_content;
                    }
                }
                ctx->index += 4;
                ctx->column += 4;
                length += 1;
                continue;
            }
            else if (ctx->index + 4 < ctx->buffer_len &&
                     strncmp(ctx->buffer + ctx->index, "&amp;", 5) == 0) {
                if (ctx->current_node && !ctx->current_node->content) {
                    ctx->current_node->content = strdup("&");
                } else if (ctx->current_node && ctx->current_node->content) {
                    size_t old_len = strlen(ctx->current_node->content);
                    char* new_content = (char*)realloc(ctx->current_node->content, old_len + 2);
                    if (new_content) {
                        new_content[old_len] = '&';
                        new_content[old_len + 1] = '\0';
                        ctx->current_node->content = new_content;
                    }
                }
                ctx->index += 5;
                ctx->column += 5;
                length += 1;
                continue;
            }
        }
        
        if (ctx->buffer[ctx->index] == '\n') {
            ctx->line++;
            ctx->column = 1;
        } else {
            ctx->column++;
        }
        
        ctx->index++;
        length++;
    }
    
    if (length > 0 && ctx->current_node) {
        char* content = (char*)malloc(length + 1);
        if (content) {
            strncpy(content, start, length);
            content[length] = '\0';

            if (ctx->current_node->content) {
                size_t old_len = strlen(ctx->current_node->content);
                size_t new_len = old_len + length;
                char* new_content = (char*)realloc(ctx->current_node->content, new_len + 1);
                if (new_content) {
                    strcpy(new_content + old_len, content);
                    ctx->current_node->content = new_content;
                }
                free(content);
            } else {
                ctx->current_node->content = content;
            }
        }
    }
}

static void xml_parse_element(XMLParserContext* ctx){
	ctx->index++;
	ctx->column++;

	if (ctx->index < ctx->buffer_len && ctx->buffer[ctx->index] == '!') {
        ctx->index++;
        ctx->column++;
        if (ctx->index < ctx->buffer_len && ctx->buffer[ctx->index] == '-') {
            xml_parse_comment(ctx);
        } else if (ctx->index < ctx->buffer_len && ctx->buffer[ctx->index] == '[') {
            xml_parse_cdata(ctx);
        }
        return;
    }

	 if (ctx->index < ctx->buffer_len && ctx->buffer[ctx->index] == '?') {
        xml_parse_declaration(ctx);
        return;
    }

	int is_closing = 0;
    if (ctx->index < ctx->buffer_len && ctx->buffer[ctx->index] == '/') {
        is_closing = 1;
        ctx->index++;
        ctx->column++;
    }

	char* tag_name = xml_parse_name(ctx);
    if (!tag_name) {
        return;
    }

	if(is_closing){
		xml_skip_whitespaces(ctx);
        
        if (ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != '>') {
            free(tag_name);
            xml_set_error(ctx, "Expected '>' in closing tag");
            return;
        }
        
        ctx->index++;
        ctx->column++;
        
        if (!ctx->current_node || strcmp(ctx->current_node->name, tag_name) != 0) {
            free(tag_name);
            xml_set_error(ctx, "Mismatched closing tag");
            return;
        }
        
        ctx->current_node = ctx->current_node->parent;
        
        free(tag_name);
        return;
	}

	XMLNode* new_node = xml_node_create(tag_name);
    free(tag_name);
    
    if (!new_node) {
        xml_set_error(ctx, "Failed to create XML node");
        return;
    }

	new_node->parent = ctx->current_node;

	if (ctx->current_node) {
        xml_node_add_child(ctx->current_node, new_node);
    } else if (!ctx->root) {
        ctx->root = new_node;
    } else {
        xml_node_free_recursive(new_node);
        xml_set_error(ctx, "Multiple root elements");
        return;
    }

	ctx->current_node = new_node;

	xml_skip_spaces(ctx);
    xml_parse_attribute(ctx, new_node);

	int self_closing = 0;
    if (ctx->index < ctx->buffer_len && ctx->buffer[ctx->index] == '/') {
        self_closing = 1;
        ctx->index++;
        ctx->column++;
    }
    
    xml_skip_spaces(ctx);

	if (ctx->index >= ctx->buffer_len || ctx->buffer[ctx->index] != '>') {
        xml_set_error(ctx, "Expected '>' after element");
        return;
    }
    
    ctx->index++;
    ctx->column++;
    
    if (self_closing) {
        ctx->current_node = ctx->current_node->parent;
    }
}

XMLDocument* xml_parse_str(const char* xml_string){
	if(!xml_string){
		return NULL;
	}

	XMLParserContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.buffer = xml_string;
    ctx.buffer_len = strlen(xml_string);
    ctx.line = 1;
    ctx.column = 1;

	while (ctx.index < ctx.buffer_len && !ctx.error) {
        xml_skip_whitespaces(&ctx);
        
        if (ctx.index >= ctx.buffer_len) {
            break;
        }
        
        if (ctx.buffer[ctx.index] == '<') {
            xml_parse_element(&ctx);
        } else {
            xml_parse_content(&ctx);
        }
    }

	if (!ctx.error && ctx.current_node != NULL) {
        xml_set_error(&ctx, "Unclosed elements");
    }

	XMLDocument* doc = (XMLDocument*)malloc(sizeof(XMLDocument));
    if (!doc) {
        if (ctx.root) xml_node_free_recursive(ctx.root);
        if (ctx.error_msg) free(ctx.error_msg);
        return NULL;
    }

	doc->root = ctx.root;
    doc->error = ctx.error;
    doc->error_message = ctx.error_msg;
    doc->line = ctx.line;
    doc->column = ctx.column;
    
    return doc;
}

void xml_document_free(XMLDocument* doc){
	if (!doc) return;
    
    if (doc->root) {
        xml_node_free_recursive(doc->root);
    }
    
    if (doc->error_message) {
        free(doc->error_message);
    }
    
    free(doc);
}

void xml_print_node(XMLNode* node, int indent){
	if (!node) return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    printf("<%s", node->name);
    
    XMLAttribute* attr = node->attributes;
    while (attr) {
        printf(" %s=\"%s\"", attr->name, attr->value);
        attr = attr->next;
    }
    
    if ((node->child || (node->content && strlen(node->content) > 0)) && 
        !(node->content && strchr(node->content, '\n'))) {
        printf(">");
        
        if (node->content) {
            printf("%s", node->content);
        }
        
        if (node->child) {
            printf("\n");
            XMLNode* child = node->child;
            while (child) {
                xml_print_node(child, indent + 1);
                child = child->next;
            }
            
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
        }
        
        printf("</%s>\n", node->name);
    } 
    else if (node->content && strchr(node->content, '\n')) {
        printf(">\n");
        char* content_copy = strdup(node->content);
        char* line = strtok(content_copy, "\n");
        while (line) {
            for (int i = 0; i < indent + 1; i++) {
                printf("  ");
            }
            printf("%s\n", line);
            line = strtok(NULL, "\n");
        }
        free(content_copy);
        
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("</%s>\n", node->name);
    }
    else {
        printf("/>\n");
    }
}

void xml_print_document(XMLDocument* doc){
	if (!doc) {
        printf("NULL document\n");
        return;
    }
    
    if (doc->error) {
        printf("Error: %s at line %d, column %d\n", 
               doc->error_message, doc->line, doc->column);
        return;
    }
    
    if (!doc->root) {
        printf("Empty document\n");
        return;
    }
    
    xml_print_node(doc->root, 0);
}