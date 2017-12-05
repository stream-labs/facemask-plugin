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
#ifndef __SMLL_FACE_HPP__
#define __SMLL_FACE_HPP__

#include <stdexcept>

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#include <dlib/image_processing.h>
#include <opencv2/opencv.hpp>
#pragma warning( pop )

#include "landmarks.hpp"
#include "sarray.hpp"

namespace smll {

	class FaceDetector;

	class Face
	{
	public:

		Face();
		Face(const Face& f) { *this = f; }
		~Face();
		Face& operator=(const Face& f);

		void copy2DDataTo(Face& f);

		inline dlib::rectangle&	GetBounds() { return m_bounds; }
		inline dlib::point*		GetPoints() { return m_points; }
		inline dlib::point&		GetPoint(uint32_t i) {
			if (i >= NUM_FACIAL_LANDMARKS)
				throw std::invalid_argument("face point index out of range");
			return m_points[i];
		}
		inline const dlib::rectangle&	GetBounds() const { return m_bounds; }
		inline const dlib::point*		GetPoints() const { return m_points; }
		inline const dlib::point&		GetPoint(uint32_t i) const {
			if (i >= NUM_FACIAL_LANDMARKS)
				throw std::invalid_argument("face point index out of range");
			return m_points[i];
		}

		inline bool PoseInitialized() const { return m_poseInitialized; }

		inline dlib::point GetPosition() {
			long x = (int)(m_bounds.right() + m_bounds.left()) / 2;
			long y = (int)(m_bounds.top() + m_bounds.bottom()) / 2;
			return dlib::point(x, y);
		}

		const cv::Mat&	GetCVRotation() const { return m_cvRotation; }
		const cv::Mat&	GetCVTranslation() const { return m_cvTranslation; }

		int IncPoseResetCounter() { return ++m_poseResetCounter; }
		int GetPoseResetCounter() { return m_poseResetCounter; }
		void ResetPoseResetCounter() { m_poseResetCounter = 0; }
		void SetPoseResetCounter(int c) { m_poseResetCounter = c; }

	private:
		dlib::rectangle				m_bounds;
		int							m_trackingX;
		int							m_trackingY;
		int							m_poseResetCounter;
		double						m_trackingScale;
		dlib::point					m_points[NUM_FACIAL_LANDMARKS];
		dlib::correlation_tracker	m_tracker;

		// 3d pose estimation (cv::solvePnP)
		bool						m_poseInitialized;
		cv::Mat						m_cvTranslation;
		cv::Mat						m_cvRotation;

		// allow private access to the Face Detector class
		friend class FaceDetector;

		void					ResetPose();
		inline void				UpdateTransform() {
			SetRotation(m_cvRotation);
			SetTranslation(m_cvTranslation);
		}
		void					SetRotation(cv::Mat m);
		void					SetTranslation(cv::Mat m);
		bool					CheckForFlip();

		template <typename image_type> void
			StartTracking(const image_type& image, float scale, int x, int y) {
			m_trackingX = x;
			m_trackingY = y;
			m_trackingScale = scale;
			double invscale = 1.0 / scale;
			double left = ((double)m_bounds.left() * invscale - x);
			double right = ((double)m_bounds.right() * invscale - x);
			double top = ((double)m_bounds.top() * invscale - y);
			double bottom = ((double)m_bounds.bottom() * invscale - y);
			dlib::drectangle r(left, top, right, bottom);
			m_tracker.start_track(image, r);
		}
		template <typename image_type> double
			UpdateTracking(const image_type& image) {
			double confidence = m_tracker.update(image);
			dlib::drectangle r = m_tracker.get_position();
			m_bounds.set_left((long)((r.left() + m_trackingX) * m_trackingScale));
			m_bounds.set_right((long)((r.right() + m_trackingX) * m_trackingScale));
			m_bounds.set_top((long)((r.top() + m_trackingY) * m_trackingScale));
			m_bounds.set_bottom((long)((r.bottom() + m_trackingY) * m_trackingScale));
			return confidence;
		}
	};

	// Just to keep memory clean, statically allocate faces
	const int		MAX_FACES = 1;
	typedef sarray<Face, MAX_FACES> Faces;

} // smll namespace
#endif // __SMLL_FACE_HPP__

