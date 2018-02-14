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
#ifndef __SMLL_IMAGE_PROCESSOR_HPP__
#define __SMLL_IMAGE_PROCESSOR_HPP__

#include <vector>

namespace smll {

typedef enum ImageType
{
	IMAGETYPE_LUMA,
	IMAGETYPE_RGB,
	IMAGETYPE_BGR,
	IMAGETYPE_RGBA,
	IMAGETYPE_BGRA,
	IMAGETYPE_YUV,

	IMAGETYPE_NUMTYPES,
	IMAGETYPE_INVALID,

} ImageType;

struct ImageWrapper
{
	union {
		int				w;
		int				width;
	};
	union {
		int				h;
		int				height;
	};
	int				stride;
	ImageType		type;
	char*			data;

	ImageWrapper() : w(0), h(0), stride(0), type(IMAGETYPE_INVALID), 
		data(nullptr) {}
	ImageWrapper(int _w, int _h, int _s, ImageType _t, char* d)
		: w(_w), h(_h), stride(_s), type(_t), data(d) {}
	int				getStride() const { 
		return (stride == 0) ? (w * getNumElems()) : stride; }
	int				getNumElems() const {
		return std::vector<int>({ 1, 3, 3, 4, 4, 3, 0, 0 })[(int)type];
	}
};


} // smll namespace
#endif // __SMLL_IMAGE_PROCESSOR_HPP__

