#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_EQ  = 100, // ==
  TK_NE  = 101, // !=
  TK_LE  = 102, // <=
  TK_GE  = 103, // >=
  TK_NUM = 256, // integer
  TK_EOF,       // the end of input
};

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

extern Vector *tokens;
extern int pos;

typedef struct {
  int ty;      // type
  int val;     // value
  char *input; // for error message
} Token;

enum {
  ND_EQ  = 100,
  ND_NE  = 101,
  ND_LE  = 102,
  ND_GE  = 103,
  ND_NUM = 256,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

// parse.c
void tokenise(char *p);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
int consume(int ty);
Node *expr();
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
void error(char *fmt, ...);
void expect(int line, int expected, int actual);
void runtest();
