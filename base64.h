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

// Shamelessly taken from: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
void base64_decode(std::string const& inbuf, std::vector<uint8_t>& outbuf);

// Added zlib compression
// Note: base64_decodeZ checks if data is zlib encoded, and returns 
//       the data correctly if it is not.
void base64_decodeZ(std::string const& inbuf, std::vector<uint8_t>& outbuf);

// If you need to alloc your buffer yourself (for alignment, say)
// use base64_decode then use these methods
size_t zlib_size(const std::vector<uint8_t>& buf);
void zlib_decode(const std::vector<uint8_t>& inbuf, uint8_t* outbuf);
