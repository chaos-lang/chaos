/*
 * Description: CPU flags module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
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

#include "flags.h"

void clear_flags(cpu *c)
{
	c->zero = 0;
	c->ltz = 0;
	c->gtz = 0;
}

void set_flags(cpu *c, i64 a, i64 b)
{
	i64 res = a - b;
	c->zero = (res == 0);
	c->ltz = (res < 0);
	c->gtz = (res > 0);
}

void fset_flags(cpu *c, f64 a, f64 b)
{
	f64 res = a - b;
	c->zero = (res == 0);
	c->ltz = (res < 0);
	c->gtz = (res > 0);
}
