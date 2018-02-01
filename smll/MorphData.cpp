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
#include "MorphData.hpp"


namespace smll {

	MorphData::MorphData() 
		: m_timestamp(TimeStamp::min())
		, m_dirty(true) {
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			vec3_zero(m_deltas.data() + i);
		}
	}

	const DeltaList& MorphData::GetDeltas() const {
		return m_deltas;
	}

	std::vector<cv::Point3f> MorphData::GetCVDeltas() const {
		std::vector<cv::Point3f> r;
		for (unsigned int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			const vec3& v = m_deltas[i];
			r.emplace_back(cv::Point3f(v.x, v.y, v.z));
		}
		return r;
	}

	DeltaList& MorphData::GetDeltasAndStamp() {
		Stamp();
		return m_deltas;
	}

	void MorphData::Stamp() {
		m_timestamp = std::chrono::steady_clock::now();
		m_dirty = true;
	}

	static inline bool vec3_is_nonzero(const vec3& v) {
		if (((int)(v.x * 1000.0f) == 0) &&
			((int)(v.y * 1000.0f) == 0) &&
			((int)(v.z * 1000.0f) == 0))
			return false;
		return true;
	}

	void MorphData::UpdateBitmask() {
		m_bitmask.reset();
		for (unsigned int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			if (vec3_is_nonzero(m_deltas[i])) {
				m_bitmask.set(i);
			}
		}
		m_dirty = false;
	}

	const LandmarkBitmask& MorphData::GetBitmask() {
		if (m_dirty)
			UpdateBitmask();
		return m_bitmask;
	}

	void MorphData::Invalidate() {
		m_timestamp = TimeStamp::min();
	}

	bool MorphData::IsValid() const {
		return (m_timestamp > TimeStamp::min());
	}

	bool MorphData::IsNewerThan(const MorphData&other) const {
		return (m_timestamp > other.m_timestamp);
	}

}
