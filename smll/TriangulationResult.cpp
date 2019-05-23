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

	const TriangulationResult::BitmaskTable&
		TriangulationResult::GetBitmasks() {
		if (bitmasks[IDXBUFF_BACKGROUND].none()) {

			bitmasks[IDXBUFF_BACKGROUND].set(BORDER_POINT);

			bitmasks[IDXBUFF_FACE].set(JAW_1);
			bitmasks[IDXBUFF_FACE].set(JAW_2);
			bitmasks[IDXBUFF_FACE].set(JAW_3);
			bitmasks[IDXBUFF_FACE].set(JAW_4);
			bitmasks[IDXBUFF_FACE].set(JAW_5);
			bitmasks[IDXBUFF_FACE].set(JAW_6);
			bitmasks[IDXBUFF_FACE].set(JAW_7);
			bitmasks[IDXBUFF_FACE].set(JAW_8);
			bitmasks[IDXBUFF_FACE].set(JAW_9);
			bitmasks[IDXBUFF_FACE].set(JAW_10);
			bitmasks[IDXBUFF_FACE].set(JAW_11);
			bitmasks[IDXBUFF_FACE].set(JAW_12);
			bitmasks[IDXBUFF_FACE].set(JAW_13);
			bitmasks[IDXBUFF_FACE].set(JAW_14);
			bitmasks[IDXBUFF_FACE].set(JAW_15);
			bitmasks[IDXBUFF_FACE].set(JAW_16);
			bitmasks[IDXBUFF_FACE].set(JAW_17);
			bitmasks[IDXBUFF_FACE].set(HEAD_1);
			bitmasks[IDXBUFF_FACE].set(HEAD_2);
			bitmasks[IDXBUFF_FACE].set(HEAD_3);
			bitmasks[IDXBUFF_FACE].set(HEAD_4);
			bitmasks[IDXBUFF_FACE].set(HEAD_5);
			bitmasks[IDXBUFF_FACE].set(HEAD_6);
			bitmasks[IDXBUFF_FACE].set(HEAD_7);
			bitmasks[IDXBUFF_FACE].set(HEAD_8);
			bitmasks[IDXBUFF_FACE].set(HEAD_9);
			bitmasks[IDXBUFF_FACE].set(HEAD_10);
			bitmasks[IDXBUFF_FACE].set(HEAD_11);
			bitmasks[IDXBUFF_FACE].set(MOUTH_OUTER_4);

			// these are extra, put it in the lines slot
			// considered part of the face for some operations,
			// but not all.
			bitmasks[IDXBUFF_LINES].set(EYE_LEFT_1);
			bitmasks[IDXBUFF_LINES].set(EYE_LEFT_2);
			bitmasks[IDXBUFF_LINES].set(EYE_LEFT_3);
			bitmasks[IDXBUFF_LINES].set(EYE_LEFT_4);
			bitmasks[IDXBUFF_LINES].set(EYE_LEFT_5);
			bitmasks[IDXBUFF_LINES].set(EYE_LEFT_6);
			bitmasks[IDXBUFF_LINES].set(EYE_RIGHT_1);
			bitmasks[IDXBUFF_LINES].set(EYE_RIGHT_2);
			bitmasks[IDXBUFF_LINES].set(EYE_RIGHT_3);
			bitmasks[IDXBUFF_LINES].set(EYE_RIGHT_4);
			bitmasks[IDXBUFF_LINES].set(EYE_RIGHT_5);
			bitmasks[IDXBUFF_LINES].set(EYE_RIGHT_6);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_1);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_2);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_3);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_5);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_6);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_7);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_8);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_9);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_10);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_11);
			bitmasks[IDXBUFF_LINES].set(MOUTH_OUTER_12);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_LEFT_1);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_LEFT_2);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_LEFT_3);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_LEFT_4);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_LEFT_5);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_RIGHT_1);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_RIGHT_2);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_RIGHT_3);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_RIGHT_4);
			bitmasks[IDXBUFF_LINES].set(EYEBROW_RIGHT_5);
			bitmasks[IDXBUFF_LINES].set(NOSE_1);
			bitmasks[IDXBUFF_LINES].set(NOSE_2);
			bitmasks[IDXBUFF_LINES].set(NOSE_3);
			bitmasks[IDXBUFF_LINES].set(NOSE_4);
			bitmasks[IDXBUFF_LINES].set(NOSE_5);
			bitmasks[IDXBUFF_LINES].set(NOSE_6);
			bitmasks[IDXBUFF_LINES].set(NOSE_7);
			bitmasks[IDXBUFF_LINES].set(NOSE_8);
			bitmasks[IDXBUFF_LINES].set(NOSE_9);

			bitmasks[IDXBUFF_HULL].set(JAW_1);
			bitmasks[IDXBUFF_HULL].set(JAW_2);
			bitmasks[IDXBUFF_HULL].set(JAW_3);
			bitmasks[IDXBUFF_HULL].set(JAW_4);
			bitmasks[IDXBUFF_HULL].set(JAW_5);
			bitmasks[IDXBUFF_HULL].set(JAW_6);
			bitmasks[IDXBUFF_HULL].set(JAW_7);
			bitmasks[IDXBUFF_HULL].set(JAW_8);
			bitmasks[IDXBUFF_HULL].set(JAW_9);
			bitmasks[IDXBUFF_HULL].set(JAW_10);
			bitmasks[IDXBUFF_HULL].set(JAW_11);
			bitmasks[IDXBUFF_HULL].set(JAW_12);
			bitmasks[IDXBUFF_HULL].set(JAW_13);
			bitmasks[IDXBUFF_HULL].set(JAW_14);
			bitmasks[IDXBUFF_HULL].set(JAW_15);
			bitmasks[IDXBUFF_HULL].set(JAW_16);
			bitmasks[IDXBUFF_HULL].set(JAW_17);
			bitmasks[IDXBUFF_HULL].set(HEAD_1);
			bitmasks[IDXBUFF_HULL].set(HEAD_2);
			bitmasks[IDXBUFF_HULL].set(HEAD_3);
			bitmasks[IDXBUFF_HULL].set(HEAD_4);
			bitmasks[IDXBUFF_HULL].set(HEAD_5);
			bitmasks[IDXBUFF_HULL].set(HEAD_6);
			bitmasks[IDXBUFF_HULL].set(HEAD_7);
			bitmasks[IDXBUFF_HULL].set(HEAD_8);
			bitmasks[IDXBUFF_HULL].set(HEAD_9);
			bitmasks[IDXBUFF_HULL].set(HEAD_10);
			bitmasks[IDXBUFF_HULL].set(HEAD_11);
			bitmasks[IDXBUFF_HULL].set(HULL_POINT);
		}
		return bitmasks;
	}

	TriangulationResult::BitmaskTable TriangulationResult::bitmasks;
	

	TriangulationResult::TriangulationResult() : vertexBuffer(nullptr),
		buildLines(false), autoBGRemoval(false), cartoonMode(false) {
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
		points = std::move(other.points);
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
