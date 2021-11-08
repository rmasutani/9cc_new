// Tokenizer

typedef enum
{
    TK_RESERVED,
    TK_IDENT, // alphabetの小文字なら変数
    TK_NUM,
    TK_EOF,
    TK_RETURN,
    TK_FOR,
    TK_WHILE,
    TK_IF,
    TK_ELSE,
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

Token *tokenize(char *p);

// global variables
Token *token;
char *user_input; // プログラムの文字列全体を保存

// errors
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

bool consume(char *op);
Token *consume_ident();
bool consume_return();
bool consume_if();
bool consume_while();
void expect(char *op);
int expect_number();
bool at_eof();
void skip_token();

// Parser

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,  // ==
    ND_NEQ, // "!="
    ND_LT,  // <
    ND_LEQ, // <=
    ND_NUM,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF,
    ND_IF_ELSE,
    ND_WHILE,
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val; // kindがNUMのときだけ使う
    int offset;

    Node *cond; // kind が ND_IF または ND_IF_ELSE の時だけ使う.
    Node *stmt;
    Node *else_stmt;
};

Node *code[100];

void program();
Node *stmt();
Node *assign();
Node *ident();
Node *equality();
Node *relational();
Node *add();
Node *primary();
Node *unary();
Node *expr();
Node *mul();

// Generator
void gen(Node *node);

typedef struct LVar LVar;

// ローカル変数型
struct LVar
{
    LVar *next;
    char *name;
    int len; // 名前の長さ
    int offset;
};

// ローカル変数を持っておく連結リスト. localsは先頭を指す.
LVar *locals;

LVar *find_lvar(Token *tok);