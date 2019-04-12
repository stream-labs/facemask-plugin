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
#ifndef __SMLL_OBS_TEXTURE_HPP__
#define __SMLL_OBS_TEXTURE_HPP__


#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#include <libobs/graphics/graphics.h>
#pragma warning( pop )


namespace smll {

	// Wrapper for gs_texture_t
	// - I'd like to just use gs_texture_get_width/height but they 
	// seem to return 0 alot
	class OBSTexture
	{
	public:
		int				width;
		int				height;
		gs_texture_t*	texture;

		OBSTexture() : width(0), height(0), texture(nullptr) {}
		OBSTexture(const OBSTexture& t) { *this = t; }
		OBSTexture& operator=(const OBSTexture& t) {
			width = t.width;
			height = t.height;
			texture = t.texture;
			return *this;
		}
		bool operator==(const OBSTexture& t) {
			return (width == t.width &&
				height == t.height &&
				texture == t.texture);
		}
	};

} // smll namespace

#endif // __SMLL_OBS_TEXTURE_HPP__

