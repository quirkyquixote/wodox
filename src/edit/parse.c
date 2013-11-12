/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "edit_private.h"

/*----------------------------------------------------------------------------
 * A tree
 *----------------------------------------------------------------------------*/
enum {
    TOKEN_AND = 1000,
    TOKEN_OR = 1001,
    TOKEN_XOR = 1002,
    TOKEN_NOT = 1003,
    TOKEN_LPAREN = 1004,
    TOKEN_RPAREN = 1005,
    TOKEN_EOF = 1006,
};

struct tree {
    int token;
    struct tree *l;
    struct tree *r;
};

/* Create a valid circuit from a tree */
void tree_to_circuit(struct tree *t, struct circuit *c, uint16_t node);

/* Parse a string into a tree */
static struct tree *parse_or_expression(char **buf);
static struct tree *parse_and_expression(char **buf);
static struct tree *parse_xor_expression(char **buf);
static struct tree *parse_not_expression(char **buf);
static struct tree *parse_atomic_expression(char **buf);

/* Consume next valid token from string */
static void consume(char **buf);

/* Current token */
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
	    if (is_right)
		ret += sprintf(buf, "(");
	    ret += circuit_to_text(buf + ret, c, tree_lf(node), 0);
	    ret += sprintf(buf + ret, " and ");
	    ret += circuit_to_text(buf + ret, c, tree_rt(node), 1);
	    if (is_right)
		ret += sprintf(buf + ret, ")");
	    break;

	case OR:
	    if (is_right)
		ret += sprintf(buf, "(");
	    ret += circuit_to_text(buf + ret, c, tree_lf(node), 0);
	    ret += sprintf(buf + ret, " or ");
	    ret += circuit_to_text(buf + ret, c, tree_rt(node), 1);
	    if (is_right)
		ret += sprintf(buf + ret, ")");
	    break;

	case XOR:
	    if (is_right)
		ret += sprintf(buf, "(");
	    ret += circuit_to_text(buf + ret, c, tree_lf(node), 0);
	    ret += sprintf(buf + ret, " xor ");
	    ret += circuit_to_text(buf + ret, c, tree_rt(node), 1);
	    if (is_right)
		ret += sprintf(buf + ret, ")");
	    break;

	case NOT:
	    ret += sprintf(buf, "not ");
	    ret += circuit_to_text(buf + ret, c, tree_lf(node), 1);
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
    tree_to_circuit(parse_or_expression(&buf), c, node);
    return 0;
}

void
tree_to_circuit(struct tree *t, struct circuit *c, uint16_t node)
{
    if (t == NULL)
	return;

    if (node > c->size) {
	c->size *= 2;
	c->tree = realloc(c->tree, sizeof(uint16_t) * c->size);
    }

    c->tree[node] = t->token;
    tree_to_circuit(t->l, c, tree_lf(node));
    tree_to_circuit(t->r, c, tree_rt(node));
    free(t);
}

struct tree *
parse_or_expression(char **buf)
{
    struct tree *t;
    struct tree *p;

    t = parse_and_expression(buf);

    if (token != TOKEN_OR)
	return t;

    p = malloc(sizeof(*p));
    p->token = OR;
    p->l = t;
    consume(buf);
    p->r = parse_or_expression(buf);
    return p;
}

struct tree *
parse_and_expression(char **buf)
{
    struct tree *t;
    struct tree *p;

    t = parse_xor_expression(buf);

    if (token != TOKEN_AND)
	return t;

    p = malloc(sizeof(*p));
    p->token = AND;
    p->l = t;
    consume(buf);
    p->r = parse_and_expression(buf);
    return p;
}

struct tree *
parse_xor_expression(char **buf)
{
    struct tree *t;
    struct tree *p;

    t = parse_not_expression(buf);

    if (token != TOKEN_XOR)
	return t;

    p = malloc(sizeof(*p));
    p->token = XOR;
    p->l = t;
    consume(buf);
    p->r = parse_xor_expression(buf);
    return p;
}

struct tree *
parse_not_expression(char **buf)
{
    struct tree *t;

    if (token != TOKEN_NOT)
	return parse_atomic_expression(buf);

    t = malloc(sizeof(*t));
    t->token = NOT;
    consume(buf);
    t->l = parse_not_expression(buf);
    t->r = NULL;
    return t;
}

struct tree *
parse_atomic_expression(char **buf)
{
    struct tree *t;

    if (token == TOKEN_EOF)
	return NULL;

    if (token == TOKEN_LPAREN) {
	consume(buf);
	t = parse_or_expression(buf);

	if (token == TOKEN_RPAREN) 
	    consume(buf);
	
	return t;
    }

    t = malloc(sizeof(*t));
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

    for (;;) {
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
		strcmp(tok, "or") == 0 ? TOKEN_OR : 
		strcmp(tok, "and") == 0 ? TOKEN_AND : 
		strcmp(tok, "xor") == 0 ? TOKEN_XOR :
		strcmp(tok, "not") == 0 ? TOKEN_NOT :
		TOKEN_EOF;
	    return;

	case 0:
	    token = TOKEN_EOF;
	    return;
	}
    }
}
