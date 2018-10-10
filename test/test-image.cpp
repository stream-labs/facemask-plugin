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
#include "ImageWrapper.hpp"

TEST_GROUP(imageTest) {};

// lenna image rgb compressed
static const uint8_t _pixel_map[] = {
	0x00, 0x00, 0x07, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x07, 0x00,
	0x00, 0x00, 0x07, 0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0x52, 0x57, 0xd3, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4d, 0x41, 0x00,
	0x00, 0xb1, 0x8f, 0x0b, 0xfc, 0x61, 0x05, 0x00, 0x00, 0x00, 0x20, 0x63, 0x48, 0x52, 0x4d, 0x00, 0x00, 0x7a, 0x26, 0x00, 0x00,
	0x80, 0x84, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x80, 0xe8, 0x00, 0x00, 0x75, 0x30, 0x00, 0x00, 0xea, 0x60, 0x00, 0x00, 0x3a,
	0x98, 0x00, 0x00, 0x17, 0x70, 0x9c, 0xba, 0x51, 0x3c, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x12, 0x74,
	0x00, 0x00, 0x12, 0x74, 0x01, 0xde, 0x66, 0x1f, 0x78, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4b, 0x47, 0x44, 0x00, 0xff, 0x00, 0xff,
	0x00, 0xff, 0xa0, 0xbd, 0xa7, 0x93, 0x00, 0x00, 0x00, 0x07, 0x74, 0x49, 0x4d, 0x45, 0x07, 0xe2, 0x05, 0x0a, 0x0f, 0x19, 0x33
};
// width and heigth
static const int width = 7;
static const int height = 7;

TEST(imageTest, propertiestTest) {

	smll::ImageWrapper image(width, height, 0, smll::IMAGETYPE_RGB, (char*)_pixel_map);
	
	//get actual data of properties
	int actualSize = image.getSize();
	int actualStride = image.getStride();
	int actualNums = image.getNumElems();

	CHECK_EQUAL(147, actualSize);
	CHECK_EQUAL(21, actualStride);
	CHECK_EQUAL(3, actualNums);

}


TEST(imageTest, resizeTest) {

	smll::ImageWrapper image(width, height, 0, smll::IMAGETYPE_RGB, (char*)_pixel_map);
	uint8_t * resizedImageData= new uint8_t[(width * height * 3) / 4];
	smll::ImageWrapper resizedImage(width / 2, height/2, 0, smll::IMAGETYPE_RGB, (char*)resizedImageData);
	
	image.ResizeTo(resizedImage);

	for (size_t i = 0; i < height/2; i++) {
		for (size_t j = 0; j < (width/2)*3; j++) {
			char expected = *(image.data + ((width*3*i) + j));
			char actual = *(resizedImage.data + (((width / 2) * 3)*i + j));
			CHECK_EQUAL(expected, actual);
		}
	}
	
	//release memory for test
	delete[] resizedImageData;

}