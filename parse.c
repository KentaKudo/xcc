#include "xcc.h"

Vector *tokens;

void tokenise(char *p) {
  tokens = new_vector();
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *tok = malloc(sizeof(Token));

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      tok->ty = TK_RETURN;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p += 6;
      continue;
    }

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
      *p == '<' || *p == '>' || *p == '=' || *p == ';'
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

    if ('a' <= *p && *p <= 'z') {
      tok->ty = TK_IDENT;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p++;
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

Node *new_node_ident(char name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

Node *new_node_return(Node *lhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_RETURN;
  node->lhs = lhs;
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

Vector *code;
void program() {
  code = new_vector();
  while (((Token *)tokens->data[pos])->ty != TK_EOF) {
    vec_push(code, (void *) stmt());
  }
}

Node *stmt() {
  Node *node;

  if (consume(TK_RETURN)) {
    node = new_node_return(expr());
  } else {
    node = expr();
  }

  if (!consume(';'))
    error("expecting ';': %s", ((Token *)tokens->data[pos])->input);

  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if (consume('='))
    node = new_node('=', node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume(TK_EQ))
      node = new_node(ND_EQ, node, relational());
    if (consume(TK_NE))
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
    if (consume(TK_LE))
      node = new_node(ND_LE, node, add());
    if (consume('>'))
      node = new_node('>', node, add());
    if (consume(TK_GE))
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

  if (((Token *)tokens->data[pos])->ty == TK_IDENT) {
    char *input = ((Token *)tokens->data[pos++])->input;
    return new_node_ident(*input);
  }
  
  error("expecting a number or '(': %s", ((Token *)tokens->data[pos])->input);
}