/* asm8085 (C) 2019 Marinus Oosters */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "parser_types.h"
#include "expression.h"

/* Get the opcode number for s. Returns -1 if not a valid operator. */
enum opcode op_from_str(const char *s);

/* Get the directive number for s. Returns -1 if not a valid operator. */
enum directive dir_from_str(const char *s);

/* Read a file, parsing the lines as it goes. 
 *
 * Returns NULL if the file cannot be opened or if there was a parse error. 
 */
struct line *read_file(const char *filename);

/* Free a line, recursively if needed (i.e. free all the following lines too). 
 * When freeing only one line, the next line is returned; otherwise NULL is returned.
 */
struct line *free_line(struct line *line, char recursive);

/* Parse a line. */
struct line *parse_line(const char *text, struct line *prev, const char *filename, char *error);

/* Parse a register */
enum reg_e parse_reg(const char *text);

/* Parse a register pair */
enum reg_pair parse_reg_pair(const char *text);

/* Parse a string */
char *parse_str(const char *text);

/* Parse an argument */
char parse_argmt(enum argmt_type types, struct argmt *argmt, const struct lineinfo *info);


#endif