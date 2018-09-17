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
#include "utils.h"
#include <cmath>

TEST_GROUP(UtilsTest) {};

TEST(UtilsTest, splitTest) {
	const std::string s = ".For norland produce age wishing.To figure on it spring season up."
		"Her provision acuteness had excellent two why intention..As called"
		" mr needed praise at.Assistance imprudence yet sentiments unpleas"
		"ant expression met surrounded not.Be at talked ye though secure.";
	const int expectedResultLength = 8;
	const std::string expectedResult[expectedResultLength] = {
		"",
		"For norland produce age wishing",
		"To figure on it spring season up",
		"Her provision acuteness had excellent two why intention",
		"",
		"As called mr needed praise at",
		"Assistance imprudence yet sentiments unpleasant expression met surrounded not",
		"Be at talked ye though secure"
	};
	std::vector<std::string> actualResult = Utils::split(s, '.');

	//verify length
	CHECK_EQUAL(expectedResultLength, actualResult.size());

	//verify results
	for (size_t i = 0; i < expectedResultLength; i++) {
		STRCMP_EQUAL(expectedResult[i].c_str(), actualResult[i].c_str());
	}
}

TEST(UtilsTest, dirnameTest) {
	const int testLength = 3;
	const std::string testDirs[testLength] = {
		"C:\\Users\\streamlabs\\Documents\\file.txt",
		"C:/Users/streamlabs/Documents/file.txt",
		"file.txt" //edge case
	};
	const std::string expectedResult[testLength] = {
		"C:\\Users\\streamlabs\\Documents",
		"C:/Users/streamlabs/Documents",
		"" //edge case
	}; 

	//verify results
	for (size_t i = 0; i < testLength; i++) {
		const std::string actual = Utils::dirname(testDirs[i]);
		STRCMP_EQUAL(expectedResult[i].c_str(), actual.c_str());
	}
}

TEST(UtilsTest, findAndReplaceTest) {
	const int testLength = 3;
	std::string testDirs[testLength] = {
		"C:\\Users\\streamlabs\\Documents\\file.txt",
		"C:/Users/streamlabs\\Documents/file.txt",
		"C:/Users/streamlabs/Documents/file.txt" //not found
	}; 
	const std::string expected = "C:/Users/streamlabs/Documents/file.txt";
	const std::string findTxt = "\\";
	const std::string replaceTxt = "/";
	//verify results
	for (size_t i = 0; i < testLength; i++) {
		Utils::find_and_replace(testDirs[i], findTxt, replaceTxt);
		STRCMP_EQUAL(expected.c_str(), testDirs[i].c_str());
	}

	//longer find replace example
	std::string testString = "test cpputest testing overtested";
	std::string expectedString = "run cppurun runing overruned";
	Utils::find_and_replace(testString, "test", "run");
	STRCMP_EQUAL(expectedString.c_str(), testString.c_str());

	//edge case emoty string
	std::string emptyString = "";
	Utils::find_and_replace(emptyString, findTxt, replaceTxt);
	STRCMP_EQUAL("", emptyString.c_str());
}

TEST(UtilsTest, hermiteTest) {
	const int testLength = 4;
	const float error = 0.5; // error which happens becaouse of floats
	float tests[testLength][6] = {
		// t,p1,p2, t1,t2, result
		{ 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 3.9787f },
		{ 8.2f, -1.54f, 0, 1.1f, 4, 1015.004028f },
		{ 5, 7.2f, -5.3f, -2.4f, 4, 2402.699951f },
		{ 0, 0, 0, 0, 0, 0 },//edge case
	};

	for (size_t i = 0; i < testLength; i++) {
		float result = Utils::hermite(tests[i][0], tests[i][1], tests[i][2], tests[i][3], tests[i][4]);
		CHECK(std::abs(tests[i][5] - result) < error);
	}
}

TEST(UtilsTest, countSpacesTest) {
	const int testLength = 3;
	const std::string testTexts[testLength] = {
		"For norland produce age wishing.To figure on it spring season up.",
		"     test  test     ",
		"" //edge case
	};
	const int expectedResult[testLength] = {
		10,
		12,
		0
	};

	//verify results
	for (size_t i = 0; i < testLength; i++) {
		int actual = Utils::count_spaces(testTexts[i]);
		CHECK_EQUAL(expectedResult[i], actual);
	}
}

TEST(UtilsTest, convertStringToWstringTest) {
	const int testLength = 3;
	const std::string testTexts[testLength] = {
		"\xE1\x83\xA2\xE1\x83\x94\xE1\x83\xA1\xE1\x83\xA2\xE1\x83\x98",
		"\xE1\x83\xA2\xE1\x83\x94\xE1\x83\xA1 test",
		""
	};
	const std::wstring expectedResult[testLength] = {
		L"ტესტი",
		L"ტეს test", //mixed
		L"" //edge case
	};

	//verify results
	for (size_t i = 0; i < testLength; i++) {
		const std::wstring actual = Utils::ConvertStringToWstring(testTexts[i]);
		CHECK(expectedResult[i].compare(actual) == 0);
	}
}

TEST(UtilsTest, convertWstringToStringTest) {
	const int testLength = 3;
	const std::wstring testTexts[testLength] = {
		L"ტესტი",
		L"ტეს test", //mixed
		L"" //edge case
	};
	const std::string expectedResult[testLength] = {
		"\xE1\x83\xA2\xE1\x83\x94\xE1\x83\xA1\xE1\x83\xA2\xE1\x83\x98",
		"\xE1\x83\xA2\xE1\x83\x94\xE1\x83\xA1 test",
		""
	};

	//verify results
	for (size_t i = 0; i < testLength; i++) {
		const std::string actual = Utils::ConvertWstringToString(testTexts[i]);
		STRCMP_EQUAL(expectedResult[i].c_str(), actual.c_str());
	}
}