#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

void program()
{
    int i = 0;

    while (!at_eof())
    {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

bool has_else()
{
    // printf("Entered has_else\n");
    Token *tmp = token;

    while (tmp)
    {
        // printf("Kind: %d\n", tmp->kind);

        if (tmp->kind == TK_ELSE)
        {
            // printf("Found ELSE\n");
            return true;
        }
        tmp = tmp->next;
    }

    return false;
}

Node *stmt()
{
    Node *node;

    if (consume_if())
    {
        if (has_else())
        {
            // printf("Found ELSE\n");
            node = calloc(1, sizeof(Node));
            node->kind = ND_IF_ELSE;
            expect("(");
            node->cond = expr();
            expect(")");
            node->stmt = stmt();
            skip_token();
            node->else_stmt = stmt();
            return node;
        }

        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->stmt = stmt();
        return node;
    }

    if (consume_while())
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->stmt = stmt();
        return node;
    }

    if (consume_return())
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    }
    else
    {
        node = expr();
    }

    if (!consume(";"))
        error_at(token->str, "expected ';'");

    return node;
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *node = equality();

    if (consume("="))
    {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_node(ND_NEQ, node, relational());
        }
        else
        {
            return node;
        }
    }
}

Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<"))
        {
            node = new_node(ND_LT, node, add());
        }
        else if (consume("<="))
        {
            node = new_node(ND_LEQ, node, add());
        }
        else if (consume(">"))
        {
            node = new_node(ND_LT, add(), node);
        }
        else if (consume(">="))
        {
            node = new_node(ND_LEQ, add(), node);
        }
        else
        {
            return node;
        }
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{

    if (consume("+"))
    {
        return primary();
    }
    else if (consume("-"))
    {
        // -x は 0-x に置き換える.
        return new_node(ND_SUB, new_node_num(0), primary());
    }

    return primary();
}

Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok)
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar)
        {
            node->offset = lvar->offset;
        }
        else
        {
            // printf("Locals offset: %d\n", locals->offset);
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;

            node->offset = lvar->offset;
            locals = lvar;
        }

        return node;
    }

    return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}