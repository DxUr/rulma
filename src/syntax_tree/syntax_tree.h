#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

typedef enum {
    NODE_IDENTIFIER,
    NODE_SPACE,
    NODE_LET,
} NodeType;

typedef struct Node Node;

void nodeExpose(const Node *p_node);

Node *nodeIdentifierCreate(int p_value);
Node *nodeSpaceCreate();
Node *nodeLetCreate(const Node *p_identifier);

void nodeSpaceAddChild(Node *p_node, const Node *p_child);
void nodeLetSetValue(Node *p_node, const Node *p_value);

NodeType nodeGetType(const Node *p_node);

#endif // SYNTAX_TREE_H