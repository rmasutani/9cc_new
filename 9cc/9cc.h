// Tokenizer

typedef enum
{
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
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
    // ND_GT,  // > -> 必要ないかも
    ND_LEQ, // <=
    // ND_GEQ, // >= ->　上と同様. 必要ないかも
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val; // kindがNUMのときだけ使う
};

Node *equality();
Node *relational();
Node *add();
Node *primary();
Node *unary();
Node *expr();
Node *mul();

// Generator
void gen(Node *node);