/*
 * Description: Intermediate Representation module of the Chaos Programming Language's source
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

#ifndef IR_H
#define IR_H

#include "types.h"

typedef struct AST AST;

enum IROpCode {
    // >>> Function Declaration <<<
    DECLARE_LABEL,
    PROLOG, MAIN_PROLOG,
    DECLARE_ARG,
    GETARG,
    RETR, RETI,
    // >>> Function Calls <<<
    PREPARE, CALLR, CALL,
    PUTARGR, PUTARGI,
    RETVAL,
    // >>> Transfer Operations <<<
    MOVR, MOVI, FMOV, FMOVR,
    ALLOCAI, REF_ALLOCAI,
    // >>> Load Operations <<<
    LDR, LDXR, FLDR, FLDXR,
    // >>> Store Operations <<<
    STR, STXR, FSTR, FSTXR,
    // >>> Binary Arithmetic Operations <<<
    ADDR, ADDI,
    SUBR, SUBI,
    MULR, MULI,
    DIVR, DIVI,
    MODR, MODI,
    ANDR, ANDI,
    ORR, ORI,
    XORR, XORI,
    LSHR, LSHI,
    RSHR, RSHI,
    // >>> Unary Arithmetic Operations <<<
    NEGR, FNEGR,
    NOTR,
    //  >>> Compare Instructions <<<
    EQR, NER, GTR, LTR, GER, LER,
    // >>> Conversions <<<
    EXTR, TRUNCR,
    // >>> Branch Operations & Jumps <<<
    BEQR, BEQI,
    JMPI,
    PATCH,
    // >>> Non-Atomic Instructions <<<
    // Dynamic Arithmetic
    DYN_ADD, DYN_SUB, DYN_MUL, DYN_DIV, DYN_NEG,
    // Dynamic Comparison
    DYN_EQR, DYN_NER, DYN_GTR, DYN_LTR, DYN_GER, DYN_LER,
    // Dynamic Logic
    DYN_LAND, DYN_LOR, DYN_LNOT,
    // Dynamic Printing
    DYN_PRNT, DYN_ECHO, DYN_PRETTY_PRNT, DYN_PRETTY_ECHO,
    // Dynamic Exit
    DYN_EXIT,
    // Dynamic Index Delete
    DYN_STR_INDEX_DELETE, DYN_LIST_INDEX_DELETE, DYN_DICT_KEY_DELETE,
    // Dynamic Index Access
    DYN_STR_INDEX_ACCESS, DYN_COMP_ACCESS,
    // Dynamic Index Update
    DYN_LIST_INDEX_UPDATE, DYN_DICT_KEY_UPDATE,
    // Dynamic Type Conversion
    DYN_BOOL_TO_STR,
    DYN_STR_TO_BOOL,
    // Dynamic Create New List
    DYN_NEW_LIST, DYN_NEW_DICT,
    // Dynamic Composite Helpers
    DYN_GET_COMP_SIZE,
    // Dynamic Loop Break
    DYN_BREAK, DYN_BREAK_HANDLE,
    // Debug
    DEBUG,
    HLT,
    NUM_INSTRUCTIONS
};

typedef struct KaosIR KaosIR;
typedef struct KaosInst KaosInst;
typedef struct KaosOp KaosOp;

typedef struct KaosIR {
    KaosInst** arr;
    i64 capacity;
    i64 size;
    i64 hlt_count;
} KaosIR;

typedef struct KaosInst {
    i64 op_code;
    KaosOp* op1;
    KaosOp* op2;
    KaosOp* op3;
    KaosOp* op4;
    AST* ast;
} KaosInst;

enum IRType { IR_REG, IR_VAL };
enum IRValueType { IR_INT, IR_FLOAT, IR_STRING };
enum IRRegister {
    R0,  R1,  R2,  R3,  R4,  R5,  R6,  R7,
    R8,  R9,  R10, R11, R12, R13, R14, R15,
    IR_NUM_REGISTERS
};

typedef struct KaosOp {
    enum IRType type;
    enum IRRegister reg;
    enum IRValueType value_type;
    union IRValue {
        i64 i;
        f64 f;
        byte *s;
    } value;
} KaosOp;

#endif
