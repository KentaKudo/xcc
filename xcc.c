#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the type of tokens
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

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

typedef struct {
  int ty;      // type
  int val;     // value
  char *input; // for error message
} Token;

Vector *tokens;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void tokenise(char *p) {
  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *tok = malloc(sizeof(Token));

    if (strncmp(p, "==", 2) == 0) {
      tok->ty = TK_EQ;
      tok->input = "==";
      vec_push(tokens, (void *)tok);
      p++; p++;
      continue;
    }

    if (strncmp(p, "!=", 2) == 0) {
      tok->ty = TK_NE;
      tok->input = "!=";
      vec_push(tokens, (void *)tok);
      p++; p++;
      continue;
    }

    if (strncmp(p, "<=", 2) == 0) {
      tok->ty = TK_LE;
      tok->input = "<=";
      vec_push(tokens, (void *)tok);
      p++; p++;
      continue;
    }

    if (strncmp(p, ">=", 2) == 0) {
      tok->ty = TK_GE;
      tok->input = ">=";
      vec_push(tokens, (void *)tok);
      p++; p++;
      continue;
    }

    if (
      *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' ||
      *p == '<' || *p == '>'
    ) {
      tok->ty = *p;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tok->ty = TK_NUM;
      tok->input = p;
      tok->val = strtol(p, &p, 10);
      vec_push(tokens, (void *)tok);
      continue;
    }

    error("untokenisable: %s", p);
    exit(1);
  }

  Token *tok = malloc(sizeof(Token));
  tok->ty = TK_EOF;
  tok->input = p;
  vec_push(tokens, (void *)tok);
}

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

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int pos = 0;
int consume(int ty) {
  Token *tok = tokens->data[pos];
  if (tok->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

Node *expr() {
  Node *node = equality();
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume(ND_EQ))
      node = new_node(ND_EQ, node, relational());
    if (consume(ND_NE))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume('<'))
      node = new_node('<', node, add());
    if (consume(ND_LE))
      node = new_node(ND_LE, node, add());
    if (consume('>'))
      node = new_node('>', node, add());
    if (consume(ND_GE))
      node = new_node(ND_GE, node, add());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  
  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, unary());
    else if (consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume('+'))
    return term();
  if (consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
}

Node *term() {
  if (consume('(')) {
    Node *node = expr();
    if (!consume(')'))
      error("expecting ')': %s", ((Token *)tokens->data[pos])->input);
    return node;
  }

  if (((Token *)tokens->data[pos])->ty == TK_NUM)
    return new_node_num(((Token *)tokens->data[pos++])->val);
  
  error("expecting a number or '(': %s", ((Token *)tokens->data[pos])->input);
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  imul rdi\n");
    break;
  case '/':
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case '<':
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case '>':
    printf("  cmp rdi, rax\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GE:
    printf("  cmp rdi, rax\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

void expect(int line, int expected, int actual);
void runtest();

void expect(int line, int expected, int actual) {
  if (expected == actual)
    return;
  fprintf(stderr, "%d: %d expected, got %d\n", line, expected, actual);
  exit(1);
}

void runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
    vec_push(vec, (void *)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (long)vec->data[0]);
  expect(__LINE__, 50, (long)vec->data[50]);
  expect(__LINE__, 99, (long)vec->data[99]);

  printf("OK\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  if (strncmp(argv[1], "-test", 5) == 0) {
    runtest();
    return 0;
  }

  tokens = new_vector();
  tokenise(argv[1]);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
