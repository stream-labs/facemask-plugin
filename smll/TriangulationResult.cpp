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
#include "TriangulationResult.hpp"

extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/obs.h>
#pragma warning( pop )
}

namespace smll {

	TriangulationResult::TriangulationResult() : vertexBuffer(nullptr),
		buildLines(false) {
		for (int i = 0; i < NUM_INDEX_BUFFERS; i++) {
			indexBuffers[i] = nullptr;
		}
	}

	TriangulationResult::~TriangulationResult() {
		DestroyBuffers();
	}

	void TriangulationResult::DestroyBuffers() {
		obs_enter_graphics();
		if (vertexBuffer)
			gs_vertexbuffer_destroy(vertexBuffer);
		vertexBuffer = nullptr;
		for (int i = 0; i < NUM_INDEX_BUFFERS; i++) {
			if (indexBuffers[i])
				gs_indexbuffer_destroy(indexBuffers[i]);
			indexBuffers[i] = nullptr;
		}
		obs_leave_graphics();
	}

	void TriangulationResult::DestroyLineBuffer() {
		obs_enter_graphics();
		if (indexBuffers[IDXBUFF_LINES])
			gs_indexbuffer_destroy(indexBuffers[IDXBUFF_LINES]);
		obs_leave_graphics();
		indexBuffers[IDXBUFF_LINES] = nullptr;
	}

	void TriangulationResult::TakeBuffersFrom(TriangulationResult& other) {

		obs_enter_graphics();
		if (other.vertexBuffer) {
			if (vertexBuffer) {
				gs_vertexbuffer_destroy(vertexBuffer);
			}
			vertexBuffer = other.vertexBuffer;
			other.vertexBuffer = nullptr;
		}
		for (int i = 0; i < NUM_INDEX_BUFFERS; i++) {
			if (other.indexBuffers[i]) {
				if (indexBuffers[i]) {
					gs_indexbuffer_destroy(indexBuffers[i]);
				}
				indexBuffers[i] = other.indexBuffers[i];
				other.indexBuffers[i] = nullptr;
			}
		}
		obs_leave_graphics();
	}

}
