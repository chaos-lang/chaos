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

enum registers {
	R0, R1, R2, R3, R4, R5, R6, R7,
	NUM_REGISTERS
};

typedef struct {
	i64 *mem;
	i64 max_mem;

	// registers
	i64 pc;
	i64 sp;
	i64 r[8];

	// instruction parts
	i64 inst;
	i64 dest;
	i64 src;

	// flags
	i64 zero;
	i64 ltz;
	i64 gtz;
} cpu;

#endif
