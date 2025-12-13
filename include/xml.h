#ifndef MULTIFORMAT_XML_H
#define MULTIFORMAT_XML_H

#include <stddef.h>
#include "../src/xml/xml_parser.h"
#include "../src/xml/xml_serializer.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // ============================
    // XML PARSING FUNCTIONS
    // ============================

    /**
     * @brief Parse an XML string into a document structure
     *
     * @param xml_str XML string to parse (null-terminated)
     * @return XMLDocument* Pointer to the parsed XML document, NULL on error
     *
     * @details This function parses an XML string and creates a DOM tree
     *          structure in memory. Supports elements, attributes, text nodes,
     *          comments, and CDATA sections.
     *
     * @note Memory must be freed using xml_document_free()
     * @warning The input string must be well-formed XML
     *
     * @example
     * @code
     * const char* xml = "<book><title>Example</title></book>";
     * XMLDocument* doc = xml_parse(xml);
     * if (doc) {
     *     // Work with document
     *     xml_document_free(doc);
     * }
     * @endcode
     */
    XMLDocument* xml_parse(const char* xml_str);

    /**
     * @brief Read and parse an XML file
     *
     * @param filename Path to the XML file
     * @return XMLDocument* Pointer to the parsed XML document, NULL on error
     *
     * @details This function reads the entire file content into memory and
     *          parses it as XML. Supports UTF-8 encoding.
     *
     * @note Maximum file size is limited by available memory
     * @warning Returns NULL on file read error or parse error
     *
     * @example
     * @code
     * XMLDocument* config = xml_parse_file("config.xml");
     * if (config) {
     *     // Read configuration
     *     xml_document_free(config);
     * }
     * @endcode
     */
    XMLDocument* xml_parse_file(const char* filename);

     // ============================
    // XML SERIALIZATION FUNCTIONS
    // ============================

    /**
     * @brief Serialize XML document into a compact string
     *
     * @param doc Pointer to the XML document
     * @return char* XML string (null-terminated), NULL on error
     *
     * @details Converts internal XML DOM structure to a string in compact format
     *          (without unnecessary spaces and line breaks).
     *
     * @note Caller is responsible for freeing memory with free()
     * @warning For empty or invalid documents may return NULL
     *
     * @example
     * @code
     * char* xml_str = xml_serialize(doc);
     * if (xml_str) {
     *     printf("XML: %s\n", xml_str);
     *     free(xml_str);
     * }
     * @endcode
     */
    char* xml_serialize(const XMLDocument* doc);

    /**
     * @brief Serialize XML document into a pretty-printed string
     *
     * @param doc Pointer to the XML document
     * @param indent_size Number of spaces per indentation level (0-8)
     * @return char* Formatted XML string (null-terminated), NULL on error
     *
     * @details Converts internal XML structure to a human-readable string
     *          with proper indentation and line breaks. Useful for debugging
     *          and configuration files.
     *
     * @note Caller is responsible for freeing memory with free()
     * @param indent_size Default is 2 if 0 is specified
     *
     * @example
     * @code
     * char* pretty_xml = xml_serialize_pretty(doc, 4);
     * if (pretty_xml) {
     *     printf("%s\n", pretty_xml);
     *     free(pretty_xml);
     * }
     * @endcode
     */
    char* xml_serialize_pretty(const XMLDocument* doc, int indent_size);
    
    /**
     * @brief Serialize XML document and save to a file
     *
     * @param doc Pointer to the XML document
     * @param filename Path to the output file
     * @param indent_size Number of spaces per indentation level (0 for compact)
     * @return int 0 on success, non-zero error code on failure
     *
     * @details Serializes XML data and writes it to a file. Can output
     *          in compact or pretty-printed format. Overwrites existing file.
     *
     * @note Creates file if it doesn't exist
     * @warning Returns error if file cannot be written
     *
     * @example
     * @code
     * if (xml_serialize_file(doc, "output.xml", 2) == 0) {
     *     printf("File saved successfully\n");
     * }
     * @endcode
     */
    int xml_serialize_file(const XMLDocument* doc, const char* filename, int indent_size);
    
    // ============================
    // DOCUMENT MANAGEMENT FUNCTIONS
    // ============================

    /**
     * @brief Free memory allocated for XML document
     *
     * @param doc Pointer to the XML document
     *
     * @details Recursively frees all memory associated with the XML document,
     *          including all nodes, attributes, and text content.
     *
     * @note Safe to call with NULL
     * @warning Pointer becomes invalid after this call
     *
     * @example
     * @code
     * XMLDocument* doc = xml_parse(xml_str);
     * // ... use document ...
     * xml_document_free(doc);
     * // doc is now invalid
     * @endcode
     */
    void xml_document_free(XMLDocument* doc);

    /**
     * @brief Check if document parsing resulted in an error
     *
     * @param doc Pointer to the XML document
     * @return int 1 if document has error, 0 if valid, -1 for NULL input
     *
     * @details Returns error status of the document. Use xml_get_error()
     *          to get error details.
     *
     * @example
     * @code
     * if (xml_has_error(doc)) {
     *     printf("Error: %s\n", xml_get_error(doc));
     * }
     * @endcode
     */
    int xml_has_error(const XMLDocument* doc);

    /**
     * @brief Get error message from document
     *
     * @param doc Pointer to the XML document
     * @return const char* Error message string, NULL if no error
     *
     * @details Returns the error message from the last parsing operation.
     *          The string is owned by the document and should not be freed.
     *
     * @note Returns NULL for documents without errors
     * @warning String becomes invalid after xml_document_free()
     */
    const char* xml_get_error(const XMLDocument* doc);

    /**
     * @brief Get line number where error occurred
     *
     * @param doc Pointer to the XML document
     * @return int Line number (1-based), 0 if no error or invalid document
     *
     * @details Returns the line number where parsing error occurred.
     *          Useful for debugging malformed XML files.
     *
     * @example
     * @code
     * if (xml_has_error(doc)) {
     *     printf("Error at line %d: %s\n", 
     *            xml_get_error_line(doc), 
     *            xml_get_error(doc));
     * }
     * @endcode
     */
    int xml_get_error_line(const XMLDocument* doc);

     /**
     * @brief Get column number where error occurred
     *
     * @param doc Pointer to the XML document
     * @return int Column number (1-based), 0 if no error or invalid document
     *
     * @details Returns the column number where parsing error occurred.
     *          Combined with line number for precise error location.
     */
    int xml_get_error_column(const XMLDocument* doc);


    // ============================
    // DOCUMENT CREATION FUNCTIONS
    // ============================

    /**
     * @brief Create a new empty XML document
     *
     * @return XMLDocument* New document, NULL on memory error
     *
     * @example
     * @code
     * XMLDocument* doc = xml_create_document();
     * if (doc) {
     *     // Add content
     *     xml_document_free(doc);
     * }
     * @endcode
     */
    XMLDocument* xml_create_document(void);

    /**
     * @brief Create a new XML element
     *
     * @param name Element name
     * @return XMLNode* New element node, NULL on error
     *
     * @example
     * @code
     * XMLNode* book = xml_create_element("book");
     * @endcode
     */
    XMLNode* xml_create_element(const char* name);

     /**
     * @brief Create a new XML text node
     *
     * @param text Text content
     * @return XMLNode* New text node, NULL on error
     *
     * @example
     * @code
     * XMLNode* text = xml_create_text("Hello World");
     * @endcode
     */
    XMLNode* xml_create_text(const char* text);

    /**
     * @brief Create a new XML comment node
     *
     * @param comment Comment text
     * @return XMLNode* New comment node, NULL on error
     *
     * @example
     * @code
     * XMLNode* comment = xml_create_comment("Generated automatically");
     * @endcode
     */
    XMLNode* xml_create_comment(const char* comment);

    /**
     * @brief Create a new XML CDATA node
     *
     * @param data CDATA content
     * @return XMLNode* New CDATA node, NULL on error
     *
     * @example
     * @code
     * XMLNode* cdata = xml_create_cdata("<html>content</html>");
     * @endcode
     */
    XMLNode* xml_create_cdata(const char* data);

    // ============================
    // DOCUMENT STRUCTURE FUNCTIONS
    // ============================

    /**
     * @brief Get the root element of a document
     *
     * @param doc XML document
     * @return XMLNode* Root element, NULL if empty
     *
     * @example
     * @code
     * XMLNode* root = xml_get_root(doc);
     * @endcode
     */
    XMLNode* xml_get_root(const XMLDocument* doc);

    /**
     * @brief Set the root element of a document
     *
     * @param doc XML document
     * @param root Root element to set
     * @return int 0 on success, non-zero on error
     *
     * @example
     * @code
     * XMLNode* root = xml_create_element("catalog");
     * xml_set_root(doc, root);
     * @endcode
     */
    int xml_set_root(XMLDocument* doc, XMLNode* root);

    // ============================
    // NODE PROPERTY FUNCTIONS
    // ============================

    /**
     * @brief Get node type
     *
     * @param node XML node
     * @return XMLNodeTypeEnum Node type
     *
     * @example
     * @code
     * XMLNodeTypeEnum type = xml_get_node_type(node);
     * if (type == XML_NODE_ELEMENT) {
     *     printf("This is an element\n");
     * }
     * @endcode
     */
    XMLNodeType xml_get_node_type(const XMLNode* node);

     /**
     * @brief Get node type as string
     *
     * @param type Node type enumeration
     * @return const char* Type name string
     *
     * @example
     * @code
     * const char* type_name = xml_node_type_to_string(xml_get_node_type(node));
     * @endcode
     */
    const char* xml_node_type_to_string(XMLNodeType type);

    /**
     * @brief Get element name
     *
     * @param node XML element node
     * @return const char* Element name, NULL for non-element nodes
     *
     * @example
     * @code
     * const char* name = xml_get_node_name(node);
     * if (name) {
     *     printf("Element name: %s\n", name);
     * }
     * @endcode
     */
    const char* xml_get_node_name(const XMLNode* node);

    /**
     * @brief Set element name
     *
     * @param node XML element node
     * @param name New element name
     * @return int 0 on success, non-zero on error
     *
     * @example
     * @code
     * xml_set_node_name(element, "newName");
     * @endcode
     */
    int xml_set_node_name(XMLNode* node, const char* name);

    /**
     * @brief Get node text content
     *
     * @param node XML node
     * @return const char* Text content, NULL if no content
     *
     * @note For element nodes, returns direct text content (not child text)
     *
     * @example
     * @code
     * const char* text = xml_get_node_text(node);
     * if (text) {
     *     printf("Text: %s\n", text);
     * }
     * @endcode
     */
    const char* xml_get_node_text(const XMLNode* node);

    /**
     * @brief Set node text content
     *
     * @param node XML node
     * @param text Text content to set
     * @return int 0 on success, non-zero on error
     *
     * @note For element nodes, sets direct text content
     *
     * @example
     * @code
     * xml_set_node_text(node, "New text content");
     * @endcode
     */
    int xml_set_node_text(XMLNode* node, const char* text);

    /**
     * @brief Get all text content from element and its descendants
     *
     * @param node XML element node
     * @return char* Concatenated text (must be freed), NULL on error
     *
     * @example
     * @code
     * char* full_text = xml_get_full_text(node);
     * if (full_text) {
     *     printf("All text: %s\n", full_text);
     *     free(full_text);
     * }
     * @endcode
     */
    char* xml_get_full_text(const XMLNode* node);

    // ============================
    // ATTRIBUTE FUNCTIONS
    // ============================

    /**
     * @brief Get number of attributes
     *
     * @param node XML element node
     * @return size_t Attribute count, 0 for non-elements or no attributes
     *
     * @example
     * @code
     * size_t attr_count = xml_get_attribute_count(node);
     * @endcode
     */
    size_t xml_get_attribute_count(const XMLNode* node);

    /**
     * @brief Get attribute name by index
     *
     * @param node XML element node
     * @param index Attribute index (0-based)
     * @return const char* Attribute name, NULL on error
     *
     * @example
     * @code
     * const char* attr_name = xml_get_attribute_name(node, 0);
     * @endcode
     */
    const char* xml_get_attribute_name(const XMLNode* node, size_t index);

    /**
     * @brief Get attribute value by index
     *
     * @param node XML element node
     * @param index Attribute index (0-based)
     * @return const char* Attribute value, NULL on error
     *
     * @example
     * @code
     * const char* attr_value = xml_get_attribute_value(node, 0);
     * @endcode
     */
    const char* xml_get_attribute_value(const XMLNode* node, size_t index);

    /**
     * @brief Get attribute value by name
     *
     * @param node XML element node
     * @param name Attribute name
     * @return const char* Attribute value, NULL if attribute not found
     *
     * @example
     * @code
     * const char* id = xml_get_attribute(node, "id");
     * @endcode
     */
    const char* xml_get_attribute(const XMLNode* node, const char* name);

    /**
     * @brief Set or update an attribute
     *
     * @param node XML element node
     * @param name Attribute name
     * @param value Attribute value
     * @return int 0 on success, non-zero on error
     *
     * @example
     * @code
     * xml_set_attribute(node, "id", "123");
     * @endcode
     */
    int xml_set_attribute(XMLNode* node, const char* name, const char* value);

    /**
     * @brief Remove an attribute
     *
     * @param node XML element node
     * @param name Attribute name to remove
     * @return int 0 on success, non-zero on error
     *
     * @example
     * @code
     * xml_remove_attribute(node, "id");
     * @endcode
     */
    int xml_remove_attribute(XMLNode* node, const char* name);

     /**
     * @brief Check if attribute exists
     *
     * @param node XML element node
     * @param name Attribute name
     * @return int 1 if exists, 0 if not, -1 on error
     *
     * @example
     * @code
     * if (xml_has_attribute(node, "required") == 1) {
     *     // Attribute exists
     * }
     * @endcode
     */
    int xml_has_attribute(const XMLNode* node, const char* name);

    // ============================
    // TREE NAVIGATION FUNCTIONS
    // ============================

    /**
     * @brief Get parent node
     *
     * @param node XML node
     * @return XMLNode* Parent node, NULL if root
     *
     * @example
     * @code
     * XMLNode* parent = xml_get_parent(node);
     * @endcode
     */
    XMLNode* xml_get_parent(const XMLNode* node);

    /**
     * @brief Get first child node
     *
     * @param node XML element node
     * @return XMLNode* First child, NULL if no children
     *
     * @example
     * @code
     * XMLNode* first_child = xml_get_first_child(node);
     * @endcode
     */
    XMLNode* xml_get_first_child(const XMLNode* node);

    /**
     * @brief Get next sibling node
     *
     * @param node XML node
     * @return XMLNode* Next sibling, NULL if last sibling
     *
     * @example
     * @code
     * XMLNode* sibling = xml_get_next_sibling(node);
     * @endcode
     */
    XMLNode* xml_get_next_sibling(const XMLNode* node);

    /**
     * @brief Get previous sibling node
     *
     * @param node XML node
     * @return XMLNode* Previous sibling, NULL if first sibling
     *
     * @example
     * @code
     * XMLNode* prev = xml_get_previous_sibling(node);
     * @endcode
     */
    XMLNode* xml_get_previous_sibling(const XMLNode* node);

    /**
     * @brief Get number of child nodes
     *
     * @param node XML element node
     * @return size_t Child count
     *
     * @example
     * @code
     * size_t child_count = xml_get_child_count(node);
     * @endcode
     */
    size_t xml_get_child_count(const XMLNode* node);

    /**
     * @brief Get child node by index
     *
     * @param node XML element node
     * @param index Child index (0-based)
     * @return XMLNode* Child node, NULL on error
     *
     * @example
     * @code
     * XMLNode* child = xml_get_child(node, 0);
     * @endcode
     */
    XMLNode* xml_get_child(const XMLNode* node, size_t index);

    // ============================
    // TREE MODIFICATION FUNCTIONS
    // ============================

    /**
     * @brief Add a child node
     *
     * @param parent Parent element node
     * @param child Child node to add
     * @return int 0 on success, non-zero on error
     *
     * @note Child is added at the end
     *
     * @example
     * @code
     * xml_add_child(parent, child);
     * @endcode
     */
    int xml_add_child(XMLNode* parent, XMLNode* child);

    /**
     * @brief Insert a child node at position
     *
     * @param parent Parent element node
     * @param child Child node to insert
     * @param index Insert position (0 = first child)
     * @return int 0 on success, non-zero on error
     *
     * @example
     * @code
     * xml_insert_child(parent, child, 0); // Insert as first child
     * @endcode
     */
    int xml_insert_child(XMLNode* parent, XMLNode* child, size_t index);

    /**
     * @brief Remove a child node
     *
     * @param parent Parent element node
     * @param child Child node to remove
     * @return int 0 on success, non-zero on error
     *
     * @note Child is detached but not freed
     *
     * @example
     * @code
     * xml_remove_child(parent, child);
     * @endcode
     */
    int xml_remove_child(XMLNode* parent, XMLNode* child);

    /**
     * @brief Remove and free a child node
     *
     * @param parent Parent element node
     * @param child Child node to remove and free
     * @return int 0 on success, non-zero on error
     *
     * @example
     * @code
     * xml_remove_and_free_child(parent, child);
     * @endcode
     */
    int xml_remove_and_free_child(XMLNode* parent, XMLNode* child);

    /**
     * @brief Remove all children
     *
     * @param node Parent element node
     * @param free_children If non-zero, free children; if zero, just detach
     *
     * @example
     * @code
     * xml_remove_all_children(node, 1); // Remove and free all children
     * @endcode
     */
    void xml_remove_all_children(XMLNode* node, int free_children);

    /**
     * @brief Clone a node and its descendants
     *
     * @param node Node to clone
     * @return XMLNode* Cloned node, NULL on error
     *
     * @example
     * @code
     * XMLNode* clone = xml_clone_node(node);
     * @endcode
     */
    XMLNode* xml_clone_node(const XMLNode* node);

     // ============================
    // QUERY FUNCTIONS
    // ============================

    /**
     * @brief Find first child element by name
     *
     * @param parent Parent element node
     * @param name Element name to find
     * @return XMLNode* Found element, NULL if not found
     *
     * @example
     * @code
     * XMLNode* title = xml_find_child(parent, "title");
     * @endcode
     */
    XMLNode* xml_find_child(const XMLNode* parent, const char* name);


    /**
     * @brief Find all child elements by name
     *
     * @param parent Parent element node
     * @param name Element name to find
     * @param count Output: number of elements found
     * @return XMLNode** Array of found elements (must be freed), NULL if none
     *
     * @example
     * @code
     * size_t count;
     * XMLNode** books = xml_find_children(catalog, "book", &count);
     * for (size_t i = 0; i < count; i++) {
     *     // Process book
     * }
     * free(books);
     * @endcode
     */
    XMLNode** xml_find_children(const XMLNode* parent, const char* name, size_t* count);

    /**
     * @brief Find element by path
     *
     * @param root Root element for search
     * @param path Path expression (e.g., "catalog/book/title")
     * @return XMLNode* Found element, NULL if not found
     *
     * @example
     * @code
     * XMLNode* title = xml_find_by_path(root, "catalog/book/title");
     * @endcode
     */
    XMLNode* xml_find_by_path(const XMLNode* root, const char* path);

    // ============================
    // VALIDATION FUNCTIONS
    // ============================

    /**
     * @brief Validate XML document
     *
     * @param doc XML document
     * @return int 1 if valid, 0 if invalid, -1 on error
     *
     * @example
     * @code
     * if (xml_validate(doc) == 1) {
     *     printf("Document is valid\n");
     * }
     * @endcode
     */
    int xml_validate(const XMLDocument* doc);

     /**
     * @brief Check if XML string is well-formed
     *
     * @param xml_str XML string to check
     * @return int 1 if well-formed, 0 if not, -1 on error
     *
     * @example
     * @code
     * if (xml_is_well_formed(xml_string) == 1) {
     *     printf("XML is well-formed\n");
     * }
     * @endcode
     */
    int xml_is_well_formed(const char* xml_str);

    // ============================
    // UTILITY FUNCTIONS
    // ============================

    /**
     * @brief Escape XML special characters
     *
     * @param str String to escape
     * @return char* Escaped string (must be freed), NULL on error
     *
     * @example
     * @code
     * char* escaped = xml_escape_string("AT&T <Cable>");
     * free(escaped);
     * @endcode
     */
    char* xml_escape_string(const char* str);

    /**
     * @brief Unescape XML entities
     *
     * @param str String with XML entities
     * @return char* Unescaped string (must be freed), NULL on error
     *
     * @example
     * @code
     * char* unescaped = xml_unescape_string("AT&amp;T &lt;Cable&gt;");
     * free(unescaped);
     * @endcode
     */
    char* xml_unescape_string(const char* str);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // !MULTIFORMAT_XML_H
