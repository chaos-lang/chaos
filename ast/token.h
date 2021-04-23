/*
 * Description: Token module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#ifndef KAOS_TOKEN_H
#define KAOS_TOKEN_H

#define TT_ILLEGAL ""
#define TT_NEWLINE "\n"
#define TT_COMMENT1 "//"
#define TT_COMMENT2 "#"
#define TT_ASSIGN "="
#define TT_ADD "+"
#define TT_SUB "-"
#define TT_MUL "*"
#define TT_QUO "/"
#define TT_REM "%"
#define TT_BACKSLASH "\\"
#define TT_LPAREN "("
#define TT_RPAREN ")"
#define TT_LBRACK "["
#define TT_RBRACK "]"
#define TT_LBRACE "{"
#define TT_RBRACE "}"
#define TT_COMMA ","
#define TT_PERIOD "."
#define TT_EQL "=="
#define TT_NEQ "!="
#define TT_GTR ">"
#define TT_LSS "<"
#define TT_GEQ ">="
#define TT_LEQ "<="
#define TT_LAND "&&"
#define TT_LOR "||"
#define TT_NOT "!"
#define TT_AND "&"
#define TT_OR "|"
#define TT_XOR "^"
#define TT_TILDE "~"
#define TT_SHL "<<"
#define TT_SHR ">>"
#define TT_INC "++"
#define TT_DEC "--"
#define TT_COLON ":"
#define TT_EXIT "exit"
#define TT_PRINT "print"
#define TT_ECHO "echo"
#define TT_PRETTY "pretty"
#define TT_TRUE "true"
#define TT_FALSE "false"
#define TT_FUNCTION_TABLE "function_table"
#define TT_DEL "del"
#define TT_RETURN "return"
#define TT_DEFAULT "default"
#define TT_TIMES_DO "times do"
#define TT_END "end"
#define TT_FOREACH "foreach"
#define TT_AS "as"
#define TT_FROM "from"
#define TT_INFINITE "INFINITE"

enum Token {
    ILLEGAL_tok,
    NEWLINE_tok,
    COMMENT1_tok,
    COMMENT2_tok,
    ASSIGN_tok,
    ADD_tok,
    SUB_tok,
    MUL_tok,
    QUO_tok,
    REM_tok,
    BACKSLASH_tok,
    LPAREN_tok,
    RPAREN_tok,
    LBRACK_tok,
    RBRACK_tok,
    LBRACE_tok,
    RBRACE_tok,
    COMMA_tok,
    PERIOD_tok,
    EQL_tok,
    NEQ_tok,
    GTR_tok,
    LSS_tok,
    GEQ_tok,
    LEQ_tok,
    LAND_tok,
    LOR_tok,
    NOT_tok,
    AND_tok,
    OR_tok,
    XOR_tok,
    TILDE_tok,
    SHL_tok,
    SHR_tok,
    INC_tok,
    DEC_tok,
    COLON_tok,
    EXIT_tok,
    PRINT_tok,
    ECHO_tok,
    PRETTY_tok,
    TRUE_tok,
    FALSE_tok,
    SYMBOL_TABLE_tok,
    FUNCTION_TABLE_tok,
    DEL_tok,
    RETURN_tok,
    DEFAULT_tok,
    TIMES_DO_tok,
    END_tok,
    FOREACH_tok,
    AS_tok,
    FROM_tok,
    INFINITE_tok,
};

#endif
