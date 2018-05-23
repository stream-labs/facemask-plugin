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
#pragma once

#include "landmarks.hpp"

extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/graphics/graphics.h>
#pragma warning( pop )
}

#include <array>

namespace smll {

	class TriangulationResult
	{
	public:
		enum {
			IDXBUFF_INVALID = -1,

			IDXBUFF_BACKGROUND = 0,
			IDXBUFF_FACE,
			IDXBUFF_HULL,
			IDXBUFF_LINES,

			NUM_INDEX_BUFFERS,
		};

		typedef std::array<LandmarkBitmask, NUM_INDEX_BUFFERS> BitmaskTable;

		gs_vertbuffer_t*		vertexBuffer;
		gs_indexbuffer_t*		indexBuffers[NUM_INDEX_BUFFERS];
		bool					buildLines;

		// flags for triangulation/rendering
		bool					autoBGRemoval;
		bool					cartoonMode;

		TriangulationResult();
		~TriangulationResult();

		void DestroyBuffers();
		void DestroyLineBuffer();
		void TakeBuffersFrom(TriangulationResult& other);

		static const BitmaskTable& GetBitmasks();

	private:
		static BitmaskTable bitmasks;
	};

}

