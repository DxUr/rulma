#include "syntax_tree.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../extra/hash.h"

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
    uint32_t uid;
} Identifier;


typedef struct {
    Node base;
    LinkedList *last_child;
} Space;


typedef struct {
    Node base;
    LinkedList *last_child;
} Scope;


typedef struct {
    Node base;
    const Identifier *identifier;
    const Node *value;
} Let;


typedef struct {
    Node base;
    TypeType type;
    // FIXME: implement this
} Type;


typedef struct {
    Node base;
    const Node *param;
    const Node *ret_type;
    const Node *scope;
} Method;


NodeType nodeGetType(const Node *p_node) {
    return p_node->type;
}


Node *nodeIdentifierCreate(int p_uid) {
    Identifier *id = ALLOC(Identifier);
    *id = (Identifier){
        .base.type = NODE_IDENTIFIER,
        .uid = p_uid};
    return (Node *)id;
}


Node *nodeSpaceCreate() {
    Space *space = ALLOC(Space);
    *space = (Space){
        .base.type = NODE_SPACE,
        .last_child = NULL
    };
    return (Node*)space;
}


Node *nodeScopeCreate() {
    Scope *scope = ALLOC(Scope);
    *scope = (Scope){
        .base.type = NODE_SCOPE,
        .last_child = NULL
    };
    return (Node*)scope;
}


Node *nodeLetCreate(const Node *p_identifier) {
    assert(p_identifier && p_identifier->type == NODE_IDENTIFIER);
    Let *let = ALLOC(Let);
    *let = (Let){
        .base.type = NODE_LET,
        .identifier = (Identifier*)p_identifier
    };
    return (Node*)let;
}


Node *nodeMethodCreate() {
    Method *method = ALLOC(Method);
    *method = (Method){
        .base.type = NODE_METHOD,
        .param = NULL,
        .ret_type = NULL,
        .scope = NULL
    };
    return (Node*)method;
}


Node *nodeTypeCreate(TypeType p_type) {
    // FIXME: implement this
    Type *type = ALLOC(Type);
    *type = (Type){
        .base.type = NODE_TYPE,
        .type = p_type,
        // FIXME: implement this
    };
    return (Node*)type;
}


void nodeSpaceAddChild(Node *p_node, const Node *p_child) {
    assert(p_node && p_child && p_node->type == NODE_SPACE);
    Space *space = (Space*)p_node;
    space->last_child = linkedListCreate(space->last_child, (void*)p_child);
}


void nodeScopeAddChild(Node *p_node, const Node *p_child) {
    assert(p_node && p_child && p_node->type == NODE_SCOPE);
    Scope *scope = (Scope*)p_node;
    scope->last_child = linkedListCreate(scope->last_child, (void*)p_child);
}


void nodeLetSetValue(Node *p_node, const Node *p_value) {
    assert(p_node && p_value && p_node->type == NODE_LET);
    ((Let*)p_node)->value = p_value;
}


void nodeMethodSetParameters(Node *p_node, const Node *p_param) {
    assert(p_node && p_node->type == NODE_METHOD && p_param && p_param->type == NODE_PARAM);
    Method *method = (Method*)p_node;
    assert(!method->param);
    method->param = p_param;
}


void nodeMethodSetType(Node *p_node, const Node *p_type) {
    assert(p_node && p_node->type == NODE_METHOD && p_type && p_type->type == NODE_TYPE);
    Method *method = (Method*)p_node;
    assert(!method->ret_type);
    method->ret_type = p_type;
}


void nodeMethodSetScope(Node *p_node, const Node *p_scope) {
    assert(p_node && p_node->type == NODE_METHOD && p_scope && p_scope->type == NODE_SCOPE);
    Method *method = (Method*)p_node;
    assert(!method->scope);
    method->scope = p_scope;
}


Node *nodeTypeGetById(const Node *p_identifier) {
    // FIXME: implement this
    return nodeTypeCreate(TYPE_INTERFACE);
}




/* ************************************************
 * ************************************************
 * ************************************************
 * ************************************************
 */


static inline void _indent(int p_indent) { for (;p_indent; p_indent--) printf("  "); }

void nodeExpose(const Node *p_node) {
    if (!p_node){
        return;
    }
    static int indent = 0;
    switch (p_node->type) {
        case NODE_SPACE: {
            putchar('\n');
            Space *space = (Space*)p_node;
            _indent(indent);
            puts("space {");
            indent++;
            for (LinkedList *child = space->last_child; child; child = child->previous_sibling)
                nodeExpose((Node*)child->value);
            indent--;
            _indent(indent);
            puts("} #space");
            putchar('\n');
            return;
        }
        case NODE_SCOPE: {
            Scope *scope = (Scope*)p_node;
            putchar('\n');
            _indent(indent);
            puts("scope {");
            indent++;
            for (LinkedList *child = scope->last_child; child; child = child->previous_sibling)
                nodeExpose((Node*)child->value);
            indent--;
            _indent(indent);
            puts("} #scope");
            putchar('\n');
            return;
        }
        case NODE_LET: {
            Let *let = (Let*)p_node;
            if (!let->value)
                putchar('\n');
            _indent(indent);
            uint64_t id = hashBase53(let->identifier->uid);
            if (!let->value) {
                printf("let %.6s\n", (char*)&id);
                return;
            }

            printf("let %.6s {\n", (char*)&id);

            indent++;
            nodeExpose(let->value);
            indent--;
            _indent(indent);
            puts("} #let");
            putchar('\n');
            return;
        }
        case NODE_METHOD: {
            Method *method = (Method*)p_node;
            putchar('\n');
            _indent(indent);
            if (method->ret_type)
                printf("method %d {", ((Type*)method->ret_type)->type);
            else
                puts("method {");
            indent++;
            nodeExpose(method->scope);
            indent--;
            _indent(indent);
            puts("} #method");
            putchar('\n');
            return;
        }
        default:
            assert(0);
    }
}

