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

TEST_GROUP(base64Test) {};

TEST(base64Test, base64DecodeZTest) {
	std::string const encodedText = "eJxzS0xOVfBNLM4uVkjLL1IIzvF3CgYAR0YGuQ==";
	std::vector<uint8_t> actaulResult;
	std::string const expectedResult = "Face Masks for SlOBS";
	base64_decodeZ(encodedText, actaulResult);

	//check length
	CHECK_EQUAL(actaulResult.size(), expectedResult.size());

	//verify content
	for (size_t i = 0; i < actaulResult.size(); i++){
		CHECK_EQUAL(actaulResult[i], expectedResult[i]);
	}
}

TEST(base64Test, zlibSizeTest) {
	//inflated: Face Masks for SlOBS 
	std::vector<uint8_t> testData = { 120, 156, 115, 75, 76, 78, 85, 240, 77, 44,
		206, 46, 86, 72, 203, 47, 82, 8, 206, 241, 119, 10, 6, 0, 71, 70, 6, 185 };
	const int expectedSize = 20;
	
	int actualSize = zlib_size(testData);

	//check sizes
	CHECK_EQUAL(actualSize, expectedSize);

}

TEST(base64Test, zlibDecodeTest) {
	//inflated: Face Masks for SlOBS 
	std::vector<uint8_t> testData = { 120, 156, 115, 75, 76, 78, 85, 240, 77, 44,
		206, 46, 86, 72, 203, 47, 82, 8, 206, 241, 119, 10, 6, 0, 71, 70, 6, 185 };
	std::vector<uint8_t> actaulResult;
	std::string const expectedResult = "Face Masks for SlOBS";
	actaulResult.resize(expectedResult.length());
	zlib_decode(testData, actaulResult.data());

	//check length
	CHECK_EQUAL(actaulResult.size(), expectedResult.size());

	//verify content
	for (size_t i = 0; i < actaulResult.size(); i++) {
		CHECK_EQUAL(actaulResult[i], expectedResult[i]);
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