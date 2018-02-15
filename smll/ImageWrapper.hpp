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

class ImageWrapper
{
public:
	int				w;
	int				h;
	int				stride;
	ImageType		type;
	char*			data;

	ImageWrapper();
	ImageWrapper(int _w, int _h, int _s, ImageType _t, char* d);

	int		getStride() const;
	int		getSize() const;
	int		getNumElems() const;

	void	ResizeTo(ImageWrapper& other) const;
};


} // smll namespace
#endif // __SMLL_IMAGE_PROCESSOR_HPP__

