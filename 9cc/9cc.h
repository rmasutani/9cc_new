// Tokenizer

typedef enum
{
    TK_RESERVED,
    TK_IDENT, // alphabetの小文字なら変数
    TK_NUM,
    TK_EOF,
    TK_RETURN,
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
void expect(char *op);
int expect_number();
bool at_eof();

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
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val; // kindがNUMのときだけ使う
    int offset;
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