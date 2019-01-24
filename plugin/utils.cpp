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
#include "p.h"
#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include "base64.h"

#include <immintrin.h>
#include <cstdint>

#include <assert.h>

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
		std::wstring stbuff(tbuff);
		// This is to keep the changes minimal
		// TODO change return type to std::string
		static char buff[256];
		std::string str = ConvertWstringToString(stbuff);
		strcpy(buff, str.c_str());
#else
		char* buff = tbuff;
#endif
		return buff;
	}

	const char* Base64ToTempFile(std::string base64String) {
		std::vector<uint8_t> decoded;
		base64_decodeZ(base64String, decoded);
		const char* fn = GetTempFileName();
		std::fstream f(fn, std::ios::out | std::ios::binary);
		f.write((char*)decoded.data(), decoded.size());
		f.close();
		return fn;
	}

	/*
	* Map functions with conversion string to wstring
	*/

	void DeleteTempFile(std::string filename) {
		DeleteTempFile(Utils::ConvertStringToWstring(filename));
	}


	std::vector<std::string> ListFolderRecursive(std::string path, std::string glob = "*") {
		std::vector<std::string> res;
		std::vector<std::wstring> wres = ListFolderRecursive(ConvertStringToWstring(path), ConvertStringToWstring(glob));
		for (int i = 0; i < wres.size(); i++)
		{
			res.push_back(ConvertWstringToString(wres[i]));
		}
		return res;
	}
	
	/*
	 * Function implementations with wstring
	 */

	void DeleteTempFile(std::wstring filename) {
		::DeleteFile(filename.c_str());
	}

	std::vector<std::wstring> ListFolder(std::wstring path, std::wstring glob) {
		std::vector<std::wstring> r;

		WIN32_FIND_DATA search_data;
		memset(&search_data, 0, sizeof(WIN32_FIND_DATA));

		std::wstring ss = path + L"\\" + glob;
		HANDLE handle = FindFirstFile(ss.c_str(), &search_data);
		while (handle != INVALID_HANDLE_VALUE)
		{
			std::wstring fn = search_data.cFileName;
			if (search_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				continue;
			}
			r.push_back(fn);

			if (FindNextFile(handle, &search_data) == FALSE)
				break;
		}

		FindClose(handle);
		return r;
	}

	std::vector<std::wstring> ListFolderRecursive(std::wstring path, std::wstring glob) {
		std::vector<std::wstring> r = ListFolder(path, glob);

		WIN32_FIND_DATA search_data;
		memset(&search_data, 0, sizeof(WIN32_FIND_DATA));

		std::wstring ss = path + L"\\*";
		HANDLE handle = FindFirstFile(ss.c_str(), &search_data);
		while (handle != INVALID_HANDLE_VALUE)
		{
			std::wstring fn = search_data.cFileName;
			if (search_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (fn != L"." && fn != L"..") {
					std::vector<std::wstring> rr = ListFolderRecursive(path + L"\\" + fn, glob);
					for (int i = 0; i < rr.size(); i++) {
						rr[i] = fn + L"\\" + rr[i];
					}
					r.insert(r.end(), rr.begin(), rr.end());
				}
			}
			if (FindNextFile(handle, &search_data) == FALSE)
				break;
		}

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


	std::string dirname(const std::string &p) {
		int e = p.length() - 1;
		const char* pch = p.c_str();
		while (e >= 0) {
			if (pch[e] == '/' || pch[e] == '\\')
				break;
			e--;
		}
		if (e < 0) e = 0;
		return p.substr(0, e);
	}

	//
	// http://bits.mdminhazulhaque.io/cpp/find-and-replace-all-occurrences-in-cpp-string.html
	//
	void find_and_replace(std::string& source, std::string const& find, std::string const& replace) {
		for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length();
		}
	}

	float hermite(float t, float p1, float p2, float t1, float t2) {
		return  (2.0f * p1 - 2.0f * p2 + t1 + t2) * t*t*t +
				(-3.0f * p1 + 3.0f * p2 - 2.0f * t1 - t2) * t*t +
				t1 * t +
				p1;
	}

	// AVX2 optimized memcpy
	//
	// https://stackoverflow.com/questions/2963898/faster-alternative-to-memcpy
	//
	void fastMemcpy(void *pvDest, void *pvSrc, size_t nBytes) {
		// must be 32-byte aligned, and multiple of 32 bytes in size
		assert(nBytes % 32 == 0);
		assert((intptr_t(pvDest) & 31) == 0);
		assert((intptr_t(pvSrc) & 31) == 0);
		const __m256i *pSrc = reinterpret_cast<const __m256i*>(pvSrc);
		__m256i *pDest = reinterpret_cast<__m256i*>(pvDest);
		int64_t nVects = nBytes / sizeof(*pSrc);
		for (; nVects > 0; nVects--, pSrc++, pDest++) {
			const __m256i loaded = _mm256_stream_load_si256(pSrc);
			_mm256_stream_si256(pDest, loaded);
		}
		_mm_sfence();
	}

	int count_spaces(const std::string& s) {
		const char* p = s.data();
		if (!p) return 0;
		int count = 0;
		while (*p) {
			if (*p++ == ' ')
				count++;
		}
		return count;
	}

	std::wstring ConvertStringToWstring(const std::string &str)
	{
		if (str.empty())
		{
			return std::wstring();
		}
		int num_chars = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
		std::wstring wstrTo;
		if (num_chars)
		{
			wstrTo.resize(num_chars);
			if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstrTo[0], num_chars))
			{
				return wstrTo;
			}
		}
		return std::wstring();
	}

	std::string ConvertWstringToString(const std::wstring& s)
	{
		int len;
		int slength = (int)s.length();
		len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, 0, 0, 0, 0);
		std::string r(len, '\0');
		WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, &r[0], len, 0, 0);
		return r;
	}

}

int test() {
	return 5;
}