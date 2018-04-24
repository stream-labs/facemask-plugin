/*
* Face Masks for SlOBS
*
* Copyright (C) 2017 General Workings Inc
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#pragma once
#include <Windows.h>
#include <string>
#include <fstream>
#include <vector>
#include "base64.h"

// ALIGNED : macro to align a memory address
#define ALIGN_16(XXX) (((size_t)(XXX) & 0xF) ? (((size_t)(XXX) + 0x10) & 0xFFFFFFFFFFFFFFF0ULL) : (size_t)(XXX))
#define ALIGN_32(XXX) (((size_t)(XXX) & 0x1F) ? (((size_t)(XXX) + 0x20) & 0xFFFFFFFFFFFFFFE0ULL) : (size_t)(XXX))


#define MAKE32COLOR(r,g,b,a) \
((((uint32_t)(a) << 24) & 0xFF000000) |\
 (((uint32_t)(b) << 16) & 0x00FF0000) |\
 (((uint32_t)(g) << 8)  & 0x0000FF00) |\
 (((uint32_t)(r) << 0)  & 0x000000FF))


namespace Utils {

	extern const char* GetTempPath();
	extern const char* GetTempFileName();
	extern const char* Base64ToTempFile(std::string base64String);
	extern void DeleteTempFile(std::string filename);
	extern std::vector<std::string> ListFolder(std::string path, std::string glob = "*");
	extern std::vector<std::string> ListFolderRecursive(std::string path, std::string glob = "*");
	extern std::vector<std::string> split(const std::string &s, char delim);
	extern std::string dirname(const std::string &p);
	extern void find_and_replace(std::string& source, std::string const& find, std::string const& replace);
	extern float hermite(float t, float p1, float p2, float t1 = 0.0f, float t2 = 0.0f);
	extern void fastMemcpy(void *pvDest, void *pvSrc, size_t nBytes);
}
