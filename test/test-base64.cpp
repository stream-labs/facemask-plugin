/*
* Face Masks for SlOBS
* smll - streamlabs machine learning library
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
#include <CppUTest/TestHarness.h>
#include "Plugin/base64.h"
#include <cmath>
#include <iostream>
using namespace std;
TEST_GROUP(base64Test) {};

TEST(base64Test, base64EncodeDecodeZTest) {
	std::vector<uint8_t> actaulResult;
	std::string const input = "Face Masks for SlOBS";
	string encoded = base64_encodeZ((uint8_t *const)input.c_str(), input.size());
	base64_decodeZ(encoded, actaulResult);

	//check length
	CHECK_EQUAL(input.size(), actaulResult.size());

	//verify content
	for (size_t i = 0; i < actaulResult.size(); i++){
		
		CHECK_EQUAL(input[i], actaulResult[i], );
	}
}

TEST(base64Test, base64DecodeTest) {
	std::string const encodedText = "RmFjZSBNYXNrcyBmb3IgU2xPQlM=";
	std::vector<uint8_t> actaulResult;
	std::string const expectedResult = "Face Masks for SlOBS";
	base64_decode(encodedText, actaulResult);

	//check length
	CHECK_EQUAL(actaulResult.size(), expectedResult.size());

	//verify content
	for (size_t i = 0; i < actaulResult.size(); i++) {
		CHECK_EQUAL(actaulResult[i], expectedResult[i]);
	}
}