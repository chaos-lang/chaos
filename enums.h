/*
 * Description: Enums module of the Chaos Programming Language's source
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

#ifndef KAOS_ENUMS_H
#define KAOS_ENUMS_H

enum Phase { INIT_PREPARSE, PREPARSE, INIT_PROGRAM, PROGRAM, INIT_JSON_PARSE, JSON_PARSE };

enum Type { K_VOID, K_BOOL, K_NUMBER, K_STRING, K_ANY, K_LIST, K_DICT };
enum ValueType { V_BOOL, V_INT, V_FLOAT, V_STRING, V_VOID, V_LIST, V_DICT };
enum Role { DEFAULT, PARAM, CALL_PARAM };

#endif
