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

std::vector<uint8_t> base64_decode(const char* buf, size_t bufsz, const char* alphabet) {
	std::vector<uint8_t> out(0);

	// Calculate output size.
	size_t outSize = 1;
	const char* ptr = strchr(buf, alphabet[64]);
	if (ptr != nullptr) {
		outSize = buf - ptr;
	} else {
		outSize = strlen(buf);
	}
	out.resize(outSize * 6 / 8 + 1); // Base64 is 6-bits per character, char is 8.

	// 2-bit Aligned Writing
	size_t op = 0, opb = 0;

	for (size_t p = 0; p < bufsz; p++) {
		const char inChar = buf[p];

		// Stop decoding on padding.
		if (inChar == alphabet[64])
			break;

		const char* value = strrchr(alphabet, inChar);
		if (value == nullptr) {
			throw std::runtime_error("malformed Base64 string");
		}
		char trueChar = *value & 0x3F;

		// Decode bit-aligned.
		if (opb == 0) {
			out[op] = trueChar;
		} else if (opb == 2) {
			out[op] = out[op] & 0x03 | (trueChar << 2);
		} else if (opb == 4) {
			out[op] = out[op] & 0x0F | ((trueChar & 0x0F) << 4);
			out[op + 1] = (trueChar & 0x30) >> 4;
		} else if (opb == 6) {
			out[op] = out[op] & 0x3F | ((trueChar & 0x03) << 6);
			out[op + 1] = (trueChar & 0x3C) >> 2;
		}
		opb += 6;
		if (opb >= 8) {
			opb -= 8;
			op++;
		}
	}

	return out;
}






std::string base64_encode(uint8_t const* buf, size_t bufLen) {
	std::string ret;
	int i = 0;
	int j = 0;
	uint8_t char_array_3[3];
	uint8_t char_array_4[4];

	while (bufLen--) {
		char_array_3[i++] = *(buf++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}

std::vector<uint8_t> base64_decode(std::string const& encoded_string) {
	size_t in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	uint8_t char_array_4[4], char_array_3[3];
	std::vector<uint8_t> ret;

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

		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}

	return ret;
}

std::string base64_encodeZ(uint8_t const* buf, size_t bufLen) {
	// compress with zlib first
	zlib::uLongf destLen = zlib::compressBound((zlib::uLongf)bufLen);
	std::vector<uint8_t> dest(destLen + sizeof(size_t));
	zlib::compress((zlib::Bytef*)dest.data(), &destLen,
		(zlib::Bytef*)buf, (zlib::uLongf)bufLen);
	
	// add decompressed size to end of data
	memcpy(dest.data() + destLen, &bufLen, sizeof(size_t));
	destLen += sizeof(size_t);
	dest.resize(destLen);

	// now base64 it
	return base64_encode(dest.data(), dest.size());
}

std::vector<uint8_t> base64_decodeZ(std::string const& encoded) {
	// base64 decode first
	std::vector<uint8_t> decoded = base64_decode(encoded);

	// make sure this is zlib data
	if (((unsigned char*)decoded.data())[0] == ZLIB_BYTE1 &&
		((unsigned char*)decoded.data())[1] == ZLIB_BYTE2) {

		// get original size from end of data
		size_t destLenT;
		memcpy(&destLenT,
			decoded.data() + decoded.size() - sizeof(size_t), sizeof(size_t));

		zlib::uLongf destLen = (zlib::uLongf)destLenT;
		zlib::uLongf srcLen = (zlib::uLongf)(decoded.size() - sizeof(size_t));

		// now decompress with zlib
		std::vector<uint8_t> decompressed(destLen);
		zlib::uncompress((zlib::Bytef*)decompressed.data(), &destLen,
			(zlib::Bytef*)decoded.data(), srcLen);

		return decompressed;
	}
	else {

		// not zlib data. just return what we have.
		return decoded;
	}
}

