#include "syntax_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <extra/hash.h>

#define ALLOC(T) (T*)malloc(sizeof(T))


typedef struct LinkedList LinkedList;
struct LinkedList {
    LinkedList *previous_sibling;
    const void *value;
};


LinkedList *linkedListCreate(LinkedList *p_previous, void *p_value) {
    LinkedList *ll = ALLOC(LinkedList);
    ll->value = p_value;
    ll->previous_sibling = p_previous;
    return ll;
}


struct Node {
    NodeType type;
};


typedef struct {
    Node base;
    int value;
} NodeIdentifier;


typedef struct {
    Node base;
    LinkedList *last_child;
} NodeSpace;


typedef struct {
    Node base;
    LinkedList *last_child;
} NodeScope;


typedef struct {
    Node base;
    const NodeIdentifier *identifier;
    const Node *value;
} NodeLet;


typedef struct {
    Node base;
    const Node *param;
    const Node *ret_type;
    const Node *scope;
} NodeMethod;


NodeType nodeGetType(const Node *p_node) {
    return p_node->type;
}


Node *nodeIdentifierCreate(int p_value) {
    NodeIdentifier *id = ALLOC(NodeIdentifier);
    *id = (NodeIdentifier){
        .base.type = NODE_IDENTIFIER,
        .value = p_value};
    return (Node *)id;
}


Node *nodeSpaceCreate() {
    NodeSpace *space = ALLOC(NodeSpace);
    *space = (NodeSpace){
        .base.type = NODE_SPACE,
        .last_child = NULL
    };
    return (Node*)space;
}


Node *nodeScopeCreate() {
    NodeScope *scope = ALLOC(NodeScope);
    *scope = (NodeScope){
        .base.type = NODE_SCOPE,
        .last_child = NULL
    };
    return (Node*)scope;
}


Node *nodeLetCreate(const Node *p_identifier) {
    assert(p_identifier && p_identifier->type == NODE_IDENTIFIER);
    NodeLet *let = ALLOC(NodeLet);
    *let = (NodeLet){
        .base.type = NODE_LET,
        .identifier = (NodeIdentifier*)p_identifier
    };
    return (Node*)let;
}


Node *nodeMethodCreate() {
    // FIXME: impl
    NodeMethod *method = ALLOC(NodeMethod);
    *method = (NodeMethod){
        .base.type = NODE_METHOD,
        .param = NULL,
        .ret_type = NULL,
        .scope = NULL
    };
    return (Node*)method;
}


void nodeSpaceAddChild(Node *p_node, const Node *p_child) {
    assert(p_node && p_child && p_node->type == NODE_SPACE);
    NodeSpace *space = (NodeSpace*)p_node;
    space->last_child = linkedListCreate(space->last_child, (void*)p_child);
}


void nodeScopeAddChild(Node *p_node, const Node *p_child) {
    assert(p_node && p_child && p_node->type == NODE_SCOPE);
    NodeScope *scope = (NodeScope*)p_node;
    scope->last_child = linkedListCreate(scope->last_child, (void*)p_child);
}


void nodeLetSetValue(Node *p_node, const Node *p_value) {
    assert(p_node && p_value && p_node->type == NODE_LET);
    ((NodeLet*)p_node)->value = p_value;
}



void nodeMethodSetParameters(Node *p_node, const Node *p_param) {
    assert(p_node && p_node->type == NODE_METHOD && p_param && p_param->type == NODE_PARAM);
    NodeMethod *method = (NodeMethod*)p_node;
    assert(!method->param);
    method->param = p_param;
}


void nodeMethodSetType(Node *p_node, const Node *p_type) {
    assert(p_node && p_node->type == NODE_METHOD && p_type && p_type->type == NODE_TYPE);
    NodeMethod *method = (NodeMethod*)p_node;
    assert(!method->ret_type);
    method->ret_type = p_type;
}


void nodeMethodSetScope(Node *p_node, const Node *p_scope) {
    assert(p_node && p_node->type == NODE_METHOD && p_scope && p_scope->type == NODE_SCOPE);
    NodeMethod *method = (NodeMethod*)p_node;
    assert(!method->scope);
    method->scope = p_scope;
}




static inline void _indent(int p_indent) { for (;p_indent; p_indent--) printf("  "); }

void nodeExpose(const Node *p_node) {
    if (!p_node){
        return;
    }
    static int indent = 0;
    switch (p_node->type) {
        case NODE_SPACE:
            NodeSpace *space = (NodeSpace*)p_node;
            _indent(indent);
            puts("space");
            indent++;
            for (LinkedList *child = space->last_child; child; child = child->previous_sibling)
                nodeExpose((Node*)child->value);
            indent--;
            return;
        case NODE_SCOPE:
            NodeScope *scope = (NodeScope*)p_node;
            _indent(indent);
            puts("scope");
            indent++;
            for (LinkedList *child = scope->last_child; child; child = child->previous_sibling)
                nodeExpose((Node*)child->value);
            indent--;
            return;
        case NODE_LET:
            NodeLet *let = (NodeLet*)p_node;
            _indent(indent);
            printf("let %u\n", let->identifier->value);
            indent++;
            nodeExpose(let->value);
            indent--;
            return;
        default:
            assert(0);
    }
}

