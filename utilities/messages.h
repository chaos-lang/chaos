/*
 * Description: Messages module of the Chaos Programming Language's source
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

#ifndef KAOS_MESSAGES_H
#define KAOS_MESSAGES_H

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   if !defined(__clang__)
#      undef NTDDI_VERSION
#      define NTDDI_VERSION 0x06000000
#   endif
#   include <windows.h>
#   include <initguid.h>
#   include <KnownFolders.h>
#   include <Shlobj.h>
#endif

#if defined(__clang__) && (defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
#   include "windows/getopt.h"
#else
#   include <getopt.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#   include <sys/syslimits.h>
#elif !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#   include <linux/limits.h>
#endif

void greet();
void print_bye_bye();
void print_help();
void print_license();

#endif
