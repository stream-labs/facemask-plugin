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

#include "Common.hpp"
#include "landmarks.hpp"

extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/graphics/vec3.h>
#pragma warning( pop )
}
#include <array>
#include <bitset>

namespace smll {

	typedef std::array<vec3, NUM_MORPH_LANDMARKS> DeltaList;

	class MorphData
	{
	public:

		MorphData();

		const DeltaList&			GetDeltas() const;
		std::vector<cv::Point3f>	GetCVDeltas() const;
		DeltaList&					GetDeltasAndStamp();
		const LandmarkBitmask&		GetBitmask();

		void				Stamp();
		
		// be sure to call after changing deltas
		void				UpdateBitmask();

		void				Invalidate();
		bool				IsValid() const;
		bool				IsNewerThan(const MorphData & other) const;

	private:
		DeltaList			m_deltas;
		TimeStamp			m_timestamp;
		LandmarkBitmask		m_bitmask;
		bool				m_dirty;
	};

}

