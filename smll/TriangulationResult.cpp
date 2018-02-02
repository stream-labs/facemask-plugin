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

	TriangulationResult::TriangulationResult() : triangulationVB(nullptr),
		triangulationIB(nullptr), linesIB(nullptr), buildLines(false) {
	}

	TriangulationResult::~TriangulationResult() {
		DestroyBuffers();
	}

	void TriangulationResult::DestroyBuffers() {
		obs_enter_graphics();
		if (triangulationVB)
			gs_vertexbuffer_destroy(triangulationVB);
		if (triangulationIB)
			gs_indexbuffer_destroy(triangulationIB);
		if (linesIB)
			gs_indexbuffer_destroy(linesIB);
		obs_leave_graphics();
		triangulationVB = nullptr;
		triangulationIB = nullptr;
		linesIB = nullptr;
	}

	void TriangulationResult::TakeBuffersFrom(TriangulationResult& other) {

		/*
		DestroyBuffers();
		if (other.triangulationVB) {
			triangulationVB = other.triangulationVB;
			other.triangulationVB = nullptr;
		}
		if (other.triangulationIB) {
			triangulationIB = other.triangulationIB;
			other.triangulationIB = nullptr;
		}
		if (other.linesIB) {
			linesIB = other.linesIB;
			other.linesIB = nullptr;
		}*/
		
		if (other.triangulationVB) {
			if (triangulationVB) {
				obs_enter_graphics();
				gs_vertexbuffer_destroy(triangulationVB);
				obs_leave_graphics();
			}
			triangulationVB = other.triangulationVB;
			other.triangulationVB = nullptr;
		}
		if (other.triangulationIB) {
			if (triangulationIB) {
				obs_enter_graphics();
				gs_indexbuffer_destroy(triangulationIB);
				obs_leave_graphics();
			}
			triangulationIB = other.triangulationIB;
			other.triangulationIB = nullptr;
		}
		if (other.linesIB) {
			if (linesIB) {
				obs_enter_graphics();
				gs_indexbuffer_destroy(linesIB);
				obs_leave_graphics();
			}
			linesIB = other.linesIB;
			other.linesIB = nullptr;
		}
	}

}
