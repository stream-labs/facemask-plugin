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

#pragma once
#include <inttypes.h>
#include <string>
#include <vector>

std::vector<uint8_t> base64_decode(const char* buf, size_t bufsz, const char* alphabet);

// RFC 2045 - MIME
static const char* base64_mime_alphabet =
/* A-Z */ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
/* a-z */ "abcdefghijklmnopqrstuvwxyz"
/* 0-9 */ "0123456789"
/* RFC */ "+/="; // = is optional padding.

// RFC 4648 - URL and Filename safe base64
static const char* base64_rfc4648_alphabet =
/* A-Z */ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
/* a-z */ "abcdefghijklmnopqrstuvwxyz"
/* 0-9 */ "0123456789"
/* RFC */ "-_="; // = is optional padding.


// Shamelessly taken from: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
std::string base64_encode(uint8_t const* buf, size_t bufLen);
std::vector<uint8_t> base64_decode(std::string const&);

// Added zlib compression
// Note: base64_decodeZ checks if data is zlib encoded, and returns 
//       the data correctly if it is not.
std::string base64_encodeZ(uint8_t const* buf, size_t bufLen);
std::vector<uint8_t> base64_decodeZ(std::string const&);

