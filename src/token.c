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

bool consume_return()
{
    if (token->kind != TK_RETURN)
        return false;

    token = token->next;
    return true;
}

bool consume_if()
{
    if (token->kind != TK_IF)
        return false;

    token = token->next;
    return true;
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

char *strndup(const char *s, size_t n)
{
    char *p;
    size_t n1;

    for (n1 = 0; n1 < n && s[n1] != '\0'; n1++)
        continue;
    p = malloc(n + 1);
    if (p != NULL)
    {
        memcpy(p, s, n1);
        p[n1] = '\0';
    }
    return p;
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

        if (strncmp(p, "return", 6) == 0 && !isalnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !isalnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !isalnum(p[4]))
        {
            cur = new_token(TK_IF, cur, p, 4);
            p += 4;
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

        // ローカル変数の読み込み
        // 1文字目は alphabet もしくは underscore
        if (isalpha(*p) || *p == '_')
        {
            char *q = p;
            while (isalpha(*q) || isdigit(*q) || *q == '_')
                q++;

            int len = q - p;
            char *name = strndup(p, len);
            cur = new_token(TK_IDENT, cur, name, len);
            p += len;
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