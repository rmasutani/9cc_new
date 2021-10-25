#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

// 入力全体を表す文字列の途中を指すポインタを受け取る
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号の時にはトークンを1つ読み進める、それ以外の場合はエラー
bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return NULL;
    }
    Token *tmp_token = token;
    token = token->next;
    return tmp_token;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected \"%s\"", op);
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "Not a number.");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok; // ここで操作するのはトークンであって文字列ではないことに注意
    return tok;
}

Token *tokenize(char *p)
{
    // printf("tokenization start");
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // 2文字のトークンのマッチを先にやる
        if (!memcmp(p, "<=", 2) || !memcmp(p, "==", 2) || !memcmp(p, ">=", 2) || !memcmp(p, "!=", 2))
        {
            // printf("Checking 2-letter token.");
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>;=", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1); // ポインタを一つ読み進めてから渡す
            cur->len = 1;
            continue;
        }

        if ('a' <= *p && *p <= 'z')
        {
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error("Unable to tokenize.");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

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

// Node *new_node_ident()

// void program() {
//     // stmtがある限りリターンし続ける
//     for (;;) {
//         if (consume(''))
//     }
// }

Node *stmt()
{
    Node *node = expr();
    expect(";");
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
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }

    return new_node_num(expect_number());
}
