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
#include "SingleValueKalman.hpp"
#include <CppUTest/TestHarness.h>

TEST_GROUP(kalmanTest) {};

TEST(kalmanTest, singleValueKalmanTest) {
	const int testNum = 5;
	const double error = 0.5;
	double testSamples[testNum][2] = {
		//update Value, expected result
		{ 2.2, 2.19 },
		{ 3.3, 3.295 },
		{ 4.4, 4.397 },
		{ -5.5, -0.8 },
		{ 2.2, 0.36 }, //same update
	};
	smll::SingleValueKalman kalman;
	kalman.Init(1.1);

	// veirfy update results
	for (size_t i = 0; i < testNum; i++) {
		double actualValue = kalman.Update(testSamples[i][0]);
		CHECK(std::abs(testSamples[i][1] - actualValue) < error);
	}
	
}
