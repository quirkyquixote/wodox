/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009 
 */

#pragma once

#include "types.h"


/*
 * To parse a circuit from text and format text from a circuit.
 */
int circuit_to_text(char *buf, struct circuit *c, uint16_t node,
			   int is_right);
int text_to_circuit(char *buf, struct circuit *c, uint16_t node);

void tree_to_circuit(struct tree *t, struct circuit *c,
			    uint16_t node);

struct tree *parse_expr_1(char **buf);
struct tree *parse_expr_2(char **buf);
struct tree *parse_expr_3(char **buf);
struct tree *parse_expr_4(char **buf);
struct tree *parse_expr_5(char **buf);

void consume(char **buf);

