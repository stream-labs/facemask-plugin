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
#include "ImageWrapper.hpp"
#include "utils.h"

#define ICV_BASE  (1)

// Intel IPP
extern "C" {
#include <iw/iw_image.h>
#include <iw/iw_image_transform.h>
#include <iw/iw_image_color.h>
}

#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/obs-module.h>
#pragma warning( pop )


namespace smll {


	ImageWrapper::ImageWrapper() 
		: w(0), h(0), stride(0), type(IMAGETYPE_INVALID),
		data(nullptr), unalignedData(nullptr) {
	}

	ImageWrapper::ImageWrapper(int _w, int _h, int _s, ImageType _t, char* d)
		: w(_w), h(_h), stride(_s), type(_t), data(d), unalignedData(nullptr) {
	}

	ImageWrapper::~ImageWrapper() {
		if (unalignedData) {
			blog(LOG_DEBUG, "deleting %x", intptr_t(unalignedData));
			delete[] unalignedData;
		}
		unalignedData = nullptr;
	}

	int	ImageWrapper::getStride() const {
		return (stride == 0) ? (w * getNumElems()) : stride;
	}

	int ImageWrapper::getSize() const {
		return h * getStride();
	}

	int	ImageWrapper::getNumElems() const {
		return std::vector<int>({ 1, 3, 3, 4, 4, 3, 0, 0 })[(int)type];
	}

	IwiColorFmt smllToIwi(ImageType t) {
		switch (t) {
		case IMAGETYPE_LUMA:
			return iwiColorGray;
		case IMAGETYPE_RGB:
			return iwiColorRGB;
		case IMAGETYPE_BGR:
			return iwiColorBGR;
		case IMAGETYPE_RGBA:
			return iwiColorRGBA;
		case IMAGETYPE_BGRA:
			return iwiColorBGRA;
		default:
			break;
		};
		return iwiColorUndefined;
	}

	void ImageWrapper::ResizeTo(ImageWrapper& other) const {

		IppStatus result;

		IwiImage dest;
		IwiSize dest_size;
		dest_size.width = other.w;
		dest_size.height = other.h;
		result = iwiImage_InitExternal(&dest, dest_size, ipp8u, other.getNumElems(), NULL, other.data, other.getStride());

		IwiImage src;
		IwiSize src_size;
		src_size.width = other.w;
		src_size.height = other.h;
		result = iwiImage_InitExternalConst(&src, src_size, ipp8u, getNumElems(), NULL, data, getStride());

		iwiResize(&src, &dest, ippLinear, NULL, ippBorderInMem, NULL, NULL);
	}

	void ImageWrapper::ColorConvertTo(ImageWrapper& other) const {

		IppStatus result;

		IwiImage dest;
		IwiColorFmt dest_colorfmt = smllToIwi(other.type);
		IwiSize dest_size;
		dest_size.width = other.w;
		dest_size.height = other.h;
		result = iwiImage_InitExternal(&dest, dest_size, ipp8u, other.getNumElems(), NULL, other.data, other.getStride());

		IwiImage src;
		IwiColorFmt src_colorfmt = smllToIwi(type);
		IwiSize src_size;
		src_size.width = other.w;
		src_size.height = other.h;
		result = iwiImage_InitExternalConst(&src, src_size, ipp8u, getNumElems(), NULL, data, getStride());

		const IwiImage* const srcList[1] = { &src };
		IwiImage* const destList[1] = { &dest };
		iwiColorConvert(srcList, src_colorfmt, destList, dest_colorfmt, 1.0f, NULL, NULL);
	}

	void    ImageWrapper::AlignedAlloc() {
		if (unalignedData)
			delete[] unalignedData;
		size_t sz = getSize();
		unalignedData = new char[sz + 32];
		blog(LOG_DEBUG, "allocated %x", intptr_t(unalignedData));
		data = (char*)ALIGN_32(unalignedData);
	}
}