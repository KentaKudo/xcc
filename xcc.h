#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256, // integer
  TK_IDENT,     // identifier
  TK_EOF,       // the end of input
  TK_EQ,        // ==
  TK_NE,        // !=
  TK_LE,        // <=
  TK_GE,        // >=
  TK_RETURN,    // return
};

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

extern Vector *tokens;
extern Vector *code;
extern int pos;
extern int varCnt;
extern Map *offsets;

typedef struct {
  int ty;      // type
  int val;     // value if ty == TK_NUM
  char *name;  // name if ty == TK_IDENT
  char *input; // for error message
} Token;

enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_EQ,
  ND_NE,
  ND_LE,
  ND_GE,
  ND_RETURN,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char *name;
} Node;

// parse.c
void tokenise(char *p);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);
int consume(int ty);
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

// codegen.c
void gen(Node *node);

// container.c
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
int is_alnum(char c);
void error(char *fmt, ...);
void expect(int line, int expected, int actual);
void test_vector();
void test_map();
void runtest();
