/*
 * Face Masks for SlOBS
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
#include <Windows.h>

#include "base64.h"
#include <iostream>

namespace zlib {
#include <zlib.h>
}

// first 2 bytes of zlib compressed data
static const unsigned char ZLIB_BYTE1 = 120;
static const unsigned char ZLIB_BYTE2 = 156;


static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool is_base64(uint8_t c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

void base64_decode(std::string const& encoded_string, std::vector<uint8_t>& ret) {
	size_t in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	uint8_t char_array_4[4], char_array_3[3];

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = (uint8_t)base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = (uint8_t)base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret.push_back(char_array_3[j]);
	}
	// yield
	::Sleep(0);
}


void base64_decodeZ(std::string const& encoded, std::vector<uint8_t>& decompressed) {
	// base64 decode first
	std::vector<uint8_t> decoded;
	base64_decode(encoded, decoded);

	// only decompress if this is zlib data
	if (((unsigned char*)decoded.data())[0] == ZLIB_BYTE1 &&
		((unsigned char*)decoded.data())[1] == ZLIB_BYTE2) {

		// get original size from end of data
		size_t destLenT;
		memcpy(&destLenT,
			decoded.data() + decoded.size() - sizeof(size_t), sizeof(size_t));

		zlib::uLongf destLen = (zlib::uLongf)destLenT;
		zlib::uLongf srcLen = (zlib::uLongf)(decoded.size() - sizeof(size_t));

		// now decompress with zlib
		decompressed.resize(destLen);
		zlib::uncompress((zlib::Bytef*)decompressed.data(), &destLen,
			(zlib::Bytef*)decoded.data(), srcLen);
	}
	// yield
	::Sleep(0);
}

size_t zlib_size(const std::vector<uint8_t>& decoded) {
	// only get size if this is zlib data
	if (((unsigned char*)decoded.data())[0] == ZLIB_BYTE1 &&
		((unsigned char*)decoded.data())[1] == ZLIB_BYTE2) {

		// get original size from end of data
		size_t destLenT;
		memcpy(&destLenT,
			decoded.data() + decoded.size() - sizeof(size_t), sizeof(size_t));

		return destLenT;
	}
	// not zlib data
	return 0;
}

void zlib_decode(const std::vector<uint8_t>& decoded, uint8_t* outbuf) {
	// only decompress if this is zlib data
	if (((unsigned char*)decoded.data())[0] == ZLIB_BYTE1 &&
		((unsigned char*)decoded.data())[1] == ZLIB_BYTE2) {

		// get original size from end of data
		size_t destLenT;
		memcpy(&destLenT,
			decoded.data() + decoded.size() - sizeof(size_t), sizeof(size_t));

		zlib::uLongf destLen = (zlib::uLongf)destLenT;
		zlib::uLongf srcLen = (zlib::uLongf)(decoded.size() - sizeof(size_t));

		// now decompress with zlib
		zlib::uncompress((zlib::Bytef*)outbuf, &destLen,
			(zlib::Bytef*)decoded.data(), srcLen);
	}
	// yield
	::Sleep(0);
}

