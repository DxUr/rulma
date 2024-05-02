#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

typedef enum {
    NODE_IDENTIFIER,
    NODE_TYPE,
    NODE_SPACE,
    NODE_SCOPE,
    NODE_LET,
    NODE_METHOD,
    NODE_PARAM,
} NodeType;

typedef enum {
    TYPE_STRUCTURE,
    TYPE_ENUM,
    TYPE_INTERFACE,
} TypeType;

typedef struct Node Node;


void nodeExpose(const Node *p_node);


Node *nodeIdentifierCreate(int p_value);
Node *nodeSpaceCreate();
Node *nodeScopeCreate();
Node *nodeLetCreate(const Node *p_identifier);
Node *nodeMethodCreate();
Node *nodeTypeCreate(TypeType p_type);


void nodeSpaceAddChild(Node *p_node, const Node *p_child);
void nodeScopeAddChild(Node *p_node, const Node *p_child);
void nodeLetSetValue(Node *p_node, const Node *p_value);
void nodeMethodSetParameters(Node *p_node, const Node *p_param);
void nodeMethodSetType(Node *p_node, const Node *p_type);
void nodeMethodSetScope(Node *p_node, const Node *p_scope);
Node *nodeTypeGetById(const Node *p_identifier);

NodeType nodeGetType(const Node *p_node);

#endif // SYNTAX_TREE_H