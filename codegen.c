#include "xcc.h"

int labelNr = 0;

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT)
    error("expecting identifier");

  void *val = map_get(offsets, node->name);
  if (val == NULL) {
    error("undefined vairable: %s\n", node->name);
  }
  int offset = (long)val;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  if (node->ty == ND_IFBLOCK) {
    gen(node->lhs);

    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    gen(node->rhs);
    return;
  }

  if (node->ty == ND_IFBODY) {
    if (node->rhs == NULL) { // if
      printf("  je  .Lend%d\n", labelNr);
      gen(node->lhs);
      printf(".Lend%d:\n", labelNr);
      labelNr++;
    } else { // if-else
      int elseLabel = labelNr++;
      int endLabel = labelNr++;
      printf("  je  .Lelse%d\n", elseLabel);
      gen(node->lhs);
      printf("  jmp .Lend%d\n", endLabel);
      printf(".Lelse%d:\n", elseLabel);
      gen(node->rhs);
      printf(".Lend%d:\n", endLabel);
    }
    return;
  }

  if (node->ty == ND_WHILE) {
    printf(".Lbegin%d:\n", labelNr);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", labelNr);
    gen(node->rhs);
    printf("  jmp .Lbegin%d\n", labelNr);
    printf(".Lend%d:\n", labelNr);
    labelNr++;
    return;
  }

  if (node->ty == ND_FOR) {
    if (node->lhs->lhs) // init
      gen(node->lhs->lhs);
    printf(".Lbegin%d:\n", labelNr);
    if (node->lhs->rhs) // cond
      gen(node->lhs->rhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", labelNr);
    if (node->rhs->lhs) // loop
      gen(node->rhs->lhs);
    gen(node->rhs->rhs); // body
    printf("  jmp .Lbegin%d\n", labelNr);
    printf(".Lend%d:\n", labelNr);
    labelNr++;
    return;
  }

  if (node->ty == ND_BLOCK) {
    for (int i = 0; i < node->block->len; i++) {
      gen((Node *)node->block->data[i]);
      printf("  pop rax\n");
    }
    printf("  push rax\n");
    return;
  }
  
  // binary operators
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
