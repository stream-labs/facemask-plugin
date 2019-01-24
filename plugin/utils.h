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

// ALIGN_xx : macros to align a memory address
#define ALIGN_2(XXX) ((intptr_t(XXX) & 0x1)  ? ((intptr_t(XXX) + 0x2) & ~0x1)  : intptr_t(XXX))
#define ALIGN_4(XXX) ((intptr_t(XXX) & 0x3)  ? ((intptr_t(XXX) + 0x4) & ~0x3)  : intptr_t(XXX))
#define ALIGN_8(XXX) ((intptr_t(XXX) & 0x7)  ? ((intptr_t(XXX) + 0x8) & ~0x7)  : intptr_t(XXX))
#define ALIGN_16(XXX) ((intptr_t(XXX) & 0xF)  ? ((intptr_t(XXX) + 0x10) & ~0xF)  : intptr_t(XXX))
#define ALIGN_32(XXX) ((intptr_t(XXX) & 0x1F) ? ((intptr_t(XXX) + 0x20) & ~0x1F) : intptr_t(XXX))


#define MAKE32COLOR(r,g,b,a) \
((((uint32_t)(a) << 24) & 0xFF000000) |\
 (((uint32_t)(b) << 16) & 0x00FF0000) |\
 (((uint32_t)(g) << 8)  & 0x0000FF00) |\
 (((uint32_t)(r) << 0)  & 0x000000FF))


namespace Utils {

	extern const char* GetTempPath();
	extern const char* GetTempFileName();
	extern const char* Base64ToTempFile(std::string base64String);
	extern std::vector<std::string> split(const std::string &s, char delim);
	extern std::string dirname(const std::string &p);
	extern int count_spaces(const std::string& s);
	extern void find_and_replace(std::string& source, std::string const& find, std::string const& replace);

	extern void DeleteTempFile(std::string filename);
	extern std::vector<std::string> ListFolderRecursive(std::string path, std::string glob);

	//wstring functions
	extern void DeleteTempFile(std::wstring filename);
	extern std::vector<std::wstring> ListFolder(std::wstring path, std::wstring glob = L"*");
	extern std::vector<std::wstring> ListFolderRecursive(std::wstring path, std::wstring glob = L"*");

	extern float hermite(float t, float p1, float p2, float t1 = 0.0f, float t2 = 0.0f);
	extern void fastMemcpy(void *pvDest, void *pvSrc, size_t nBytes);

	extern std::wstring ConvertStringToWstring(const std::string &str);
	extern std::string ConvertWstringToString(const std::wstring& s);
}

