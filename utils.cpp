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
#include "utils.h"
#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include "base64.h"


namespace Utils {

	const char* GetTempPath() {
		static TCHAR tbuff[256];
		::GetTempPath(256, tbuff);
#ifdef UNICODE
		static char buff[256];
		wcstombs(buff, (wchar_t*)tbuff, wcslen((wchar_t*)tbuff) + 1);
#else
		char* buff = tbuff;
#endif
		return buff;
	}

	const char* GetTempFileName() {
		TCHAR dbuff[256];
		static TCHAR tbuff[256];
		::GetTempPath(256, dbuff);
		::GetTempFileName(dbuff, nullptr, 0, tbuff);
#ifdef UNICODE
		static char buff[256];
		wcstombs(buff, (wchar_t*)tbuff, wcslen((wchar_t*)tbuff) + 1);
#else
		char* buff = tbuff;
#endif
		return buff;
	}

	const char* Base64ToTempFile(std::string base64String) {
		std::vector<uint8_t> decoded = base64_decodeZ(base64String);
		const char* fn = GetTempFileName();
		std::fstream f(fn, std::ios::out | std::ios::binary);
		f.write((char*)decoded.data(), decoded.size());
		f.close();
		return fn;
	}

	void DeleteTempFile(std::string filename) {
		::DeleteFile(filename.c_str());
	}

	std::vector<std::string> ListFolder(std::string path, std::string glob) {
		std::vector<std::string> r;

		WIN32_FIND_DATA search_data;
		memset(&search_data, 0, sizeof(WIN32_FIND_DATA));

		std::string ss = path + "\\" + glob;
		HANDLE handle = FindFirstFile(ss.c_str(), &search_data);
		while (handle != INVALID_HANDLE_VALUE)
		{
			r.push_back(search_data.cFileName);

			if (FindNextFile(handle, &search_data) == FALSE)
				break;
		}

		//Close the handle after use or memory/resource leak
		FindClose(handle);
		return r;
	}


	// ------------------------------------------------------------------------------
	// https://stackoverflow.com/questions/236129/most-elegant-way-to-split-a-string
	//
	template<typename Out>
	void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}
	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}


	extern std::string dirname(const std::string &p) {
		size_t e = p.length() - 1;
		const char* pch = p.c_str();
		while (e >= 0) {
			if (pch[e] == '/' || pch[e] == '\\')
				break;
			e--;
		}
		if (e < 0) e = 0;
		return p.substr(0, e);
	}

}
