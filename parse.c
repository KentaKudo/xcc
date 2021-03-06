#include "xcc.h"

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

Node *new_node_ident(char *name) {
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

Node *new_node_if(Node *condition, Node *ifbody, Node *alt) {
  Node *body = malloc(sizeof(Node));
  body->ty = ND_IFBODY;
  body->lhs = ifbody;
  body->rhs = alt;
  Node *block = malloc(sizeof(Node));
  block->ty = ND_IFBLOCK;
  block->lhs = condition;
  block->rhs = body;
  return block;
}

Node *new_node_while(Node *condition, Node *body) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_WHILE;
  node->lhs = condition;
  node->rhs = body;
  return node;
}

Node *new_node_for(Node *init, Node *cond, Node *loop, Node *body) {
  Node *initcond = malloc(sizeof(Node));
  initcond->ty = ND_FORINITCOND;
  initcond->lhs = init;
  initcond->rhs = cond;
  Node *loopbody = malloc(sizeof(Node));
  loopbody->ty = ND_FORLOOPBODY;
  loopbody->lhs = loop;
  loopbody->rhs = body;
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FOR;
  node->lhs = initcond;
  node->rhs = loopbody;
  return node;
}

Node *new_node_block(Vector *block) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_BLOCK;
  node->block = block;
  return node;
}

Node *new_node_fcall(char *name, Vector *args) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FCALL;
  node->name = name;
  node->args = args;
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
Map *offsets;
int varCnt = 0;

void program() {
  code = new_vector();
  offsets = new_map();
  while (((Token *)tokens->data[pos])->ty != TK_EOF) {
    vec_push(code, (void *) stmt());
  }
}

Node *stmt() {
  Node *node;

  if (consume('{')) {
    Vector *block = new_vector();
    while (!consume('}'))
      vec_push(block, stmt());
    return new_node_block(block);
  }

  if (consume(TK_IF)) {
    if (!consume('('))
      error("expecting '(': %s", ((Token *)tokens->data[pos])->input);
    Node *condition = expr();
    if (!consume(')'))
      error("expecting ')': %s", ((Token *)tokens->data[pos])->input);
    Node *body = stmt();
    if (consume(TK_ELSE))
      return new_node_if(condition, body, stmt());

    return new_node_if(condition, body, NULL);
  }

  if (consume(TK_WHILE)) {
    if (!consume('('))
      error("expecting '(': %s", ((Token *)tokens->data[pos])->input);
    Node *condition = expr();
    if (!consume(')'))
      error("expecting ')': %s", ((Token *)tokens->data[pos])->input);
    return new_node_while(condition, stmt());
  }

  if (consume(TK_FOR)) {
    if (!consume('('))
      error("expecting '(': %s", ((Token *)tokens->data[pos])->input);
    Node *init = NULL, *cond = NULL, *loop = NULL;
    if (!consume(';')) {
      init = expr();
      if (!consume(';'))
        error("expecting ';': %s", ((Token *)tokens->data[pos])->input);
    }
    if (!consume(';')) {
      cond = expr();
      if (!consume(';'))
        error("expecting ';': %s", ((Token *)tokens->data[pos])->input);
    }
    if (!consume(')')) {
      loop = expr();
      if (!consume(')'))
        error("expecting ')': %s", ((Token *)tokens->data[pos])->input);
    }
    return new_node_for(init, cond, loop, stmt());
  }

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
  if (((Token *)tokens->data[pos])->ty == TK_NUM)
    return new_node_num(((Token *)tokens->data[pos++])->val);

  if (((Token *)tokens->data[pos])->ty == TK_IDENT) {
    char *name = ((Token *)tokens->data[pos++])->name;
    
    if (consume('(')) { // function call
      Vector *args = new_vector();
      if (!consume(')')) {
        vec_push(args, expr());
        while (consume(','))
          vec_push(args, expr());
        if (!consume(')'))
          error("expecting ')': %s", ((Token *)tokens->data[pos])->input);
      }
      return new_node_fcall(name, args);
    }

    void *val = map_get(offsets, name);
    if (val == NULL) {
      map_put(offsets, name, varCnt * 8);
      varCnt++;
    }

    return new_node_ident(name);
  }
  
  if (consume('(')) {
    Node *node = expr();
    if (!consume(')'))
      error("expecting ')': %s", ((Token *)tokens->data[pos])->input);
    return node;
  }
  
  error("expecting a number or '(': %s", ((Token *)tokens->data[pos])->input);
}
