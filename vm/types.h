/*
 * Description: CPU types module of the Chaos Programming Language's source
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

#ifndef TYPES_H
#define TYPES_H

#define byte unsigned char
#define u64 unsigned long long
#define i64 long long
#define f64 double

typedef struct KaosIR KaosIR;
typedef struct KaosInst KaosInst;


typedef struct {
    KaosIR* program;

    // instruction counter
    i64 ic;

    // stack
    int* stack;

    // current instruction
    KaosInst* inst;

    unsigned short debug_level;
} cpu;

#endif
