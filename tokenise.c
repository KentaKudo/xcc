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
      int len = 0;
      while (is_alnum(p[len])) {
        len++;
      }
      char *name = malloc(sizeof(char) * len);
      strncpy(name, p, len);
      tok->name = name;
      vec_push(tokens, (void *)tok);
      p += len;
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
