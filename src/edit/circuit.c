/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "circuit.h"

static int token;



/*
 * To read and write circuits.
 */
int
circuit_to_text(char *buf, struct circuit *c, uint16_t node, int is_right)
{
    int ret = 0;

    if (c->tree)
	switch (c->tree[node]) {
	case AND:
	    if (is_right) {
		ret += sprintf(buf, "(");
	    }
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 0);
	    ret += sprintf(buf + ret, " and ");
	    ret += circuit_to_text(buf + ret, c, RCHILD(node), 1);
	    if (is_right) {
		ret += sprintf(buf + ret, ")");
	    }
	    break;

	case OR:
	    if (is_right) {
		ret += sprintf(buf, "(");
	    }
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 0);
	    ret += sprintf(buf + ret, " or ");
	    ret += circuit_to_text(buf + ret, c, RCHILD(node), 1);
	    if (is_right) {
		ret += sprintf(buf + ret, ")");
	    }
	    break;

	case XOR:
	    if (is_right) {
		ret += sprintf(buf, "(");
	    }
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 0);
	    ret += sprintf(buf + ret, " xor ");
	    ret += circuit_to_text(buf + ret, c, RCHILD(node), 1);
	    if (is_right) {
		ret += sprintf(buf + ret, ")");
	    }
	    break;

	case NOT:
	    ret += sprintf(buf, "not ");
	    ret += circuit_to_text(buf + ret, c, LCHILD(node), 1);
	    break;

	case NONE:
	    break;

	default:
	    ret += sprintf(buf, "%d", c->tree[node]);
	    break;
	}
    return ret;
}


/*
 * To transform from text to circuit we parse the string, make a tree of the
 * expression and walk the tree to generate a circuit.
 */
int
text_to_circuit(char *buf, struct circuit *c, uint16_t node)
{
    c->size = TREE_SIZE;
    c->tree = realloc(c->tree, sizeof(uint16_t) * c->size);
    consume(&buf);
    tree_to_circuit(parse_expr_1(&buf), c, node);
    return 0;
}

void
tree_to_circuit(struct tree *t, struct circuit *c, uint16_t node)
{
    if (t) {
	if (node > c->size) {
	    c->size *= 2;
	    c->tree = realloc(c->tree, sizeof(uint16_t) * c->size);
	}
	c->tree[node] = t->token;
	tree_to_circuit(t->l, c, LCHILD(node));
	tree_to_circuit(t->r, c, RCHILD(node));
	free(t);
    }
}

#define TOKEN_AND	1000
#define TOKEN_OR	1001
#define TOKEN_XOR	1002
#define TOKEN_NOT	1003
#define TOKEN_LPAREN	1004
#define TOKEN_RPAREN	1005
#define TOKEN_EOF	1006

struct tree *
parse_expr_1(char **buf)
{
    struct tree *t = parse_expr_2(buf);
    if (token == TOKEN_OR) {
	struct tree *p = (struct tree *) malloc(sizeof(struct tree));
	p->token = OR;
	p->l = t;
	consume(buf);
	p->r = parse_expr_1(buf);
	return p;
    }
    return t;
}

struct tree *
parse_expr_2(char **buf)
{
    struct tree *t = parse_expr_3(buf);
    if (token == TOKEN_AND) {
	struct tree *p = (struct tree *) malloc(sizeof(struct tree));
	p->token = AND;
	p->l = t;
	consume(buf);
	p->r = parse_expr_2(buf);
	return p;
    }
    return t;
}

struct tree *
parse_expr_3(char **buf)
{
    struct tree *t = parse_expr_4(buf);
    if (token == TOKEN_XOR) {
	struct tree *p = (struct tree *) malloc(sizeof(struct tree));
	p->token = XOR;
	p->l = t;
	consume(buf);
	p->r = parse_expr_3(buf);
	return p;
    }
    return t;
}

struct tree *
parse_expr_4(char **buf)
{
    if (token == TOKEN_NOT) {
	struct tree *t = (struct tree *) malloc(sizeof(struct tree));
	t->token = NOT;
	consume(buf);
	t->l = parse_expr_4(buf);
	t->r = NULL;
	return t;
    }
    return parse_expr_5(buf);
}

struct tree *
parse_expr_5(char **buf)
{
    if (token == TOKEN_EOF) {
	return NULL;
    }
    if (token == TOKEN_LPAREN) {
	consume(buf);
	struct tree *t = parse_expr_1(buf);
	if (token == TOKEN_RPAREN) {
	    consume(buf);
	}
	return t;
    }
    struct tree *t = (struct tree *) malloc(sizeof(struct tree));
    t->token = token;
    t->l = NULL;
    t->r = NULL;
    consume(buf);
    return t;
}

void
consume(char **buf)
{
    int i = 0;
    char tok[128];

    for (;;)
	switch (**buf) {
	case '(':
	    ++*buf;
	    token = TOKEN_LPAREN;
	    return;
	case ')':
	    ++*buf;
	    token = TOKEN_RPAREN;
	    return;
	case '0' ... '9':
	    token = 0;
	    do {
		token *= 10;
		token += **buf - '0';
		++*buf;
	    }
	    while (**buf >= '0' && **buf <= '9');
	    return;

	case ' ':
	    ++*buf;
	    break;
	case 'a' ... 'z':
	    i = 0;
	    do {
		tok[i] = **buf;
		++*buf;
		++i;
	    }
	    while (**buf >= 'a' && **buf <= 'z');
	    tok[i] = 0;
	    token =
		strcmp(tok, "or") == 0 ? TOKEN_OR : strcmp(tok,
							   "and") ==
		0 ? TOKEN_AND : strcmp(tok,
				       "xor") ==
		0 ? TOKEN_XOR : strcmp(tok,
				       "not") == 0 ? TOKEN_NOT : TOKEN_EOF;
	    return;

	case 0:
	    token = TOKEN_EOF;
	    return;
	}
}
