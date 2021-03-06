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

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      tok->ty = TK_IF;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      tok->ty = TK_ELSE;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      tok->ty = TK_WHILE;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      tok->ty = TK_FOR;
      tok->input = p;
      vec_push(tokens, (void *)tok);
      p += 3;
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
      *p == '<' || *p == '>' || *p == '=' || *p == ';' || *p == '{' || *p == '}' ||
      *p == ','
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
