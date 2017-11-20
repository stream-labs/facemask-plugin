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
#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#include <dlib/image_processing.h>
#include <dlib/filtering.h>
#include <dlib/matrix.h>
#pragma warning( pop )

#include <vector>
#include <array>

#include "landmarks.hpp"
#include "Face.hpp"
#include "SingleValueKalman.hpp"

namespace smll {

	class DetectionResult
	{
	public:
		// 2D
		dlib::rectangle		bounds;
		dlib::point			landmarks[NUM_FACIAL_LANDMARKS];
		// 3D
		double		translation[3];
		double		rotation[4];

		DetectionResult();
		DetectionResult(const DetectionResult& r) { *this = r; }
		~DetectionResult();
		DetectionResult& operator=(const DetectionResult& r);
		DetectionResult& operator=(const Face& f);

		void UpdateResults(const DetectionResult& r, double smoothingAmount);

		double DistanceTo(const DetectionResult& r) const;

		inline dlib::point GetPosition() {
			long x = (int)(bounds.right() + bounds.left()) / 2;
			long y = (int)(bounds.top() + bounds.bottom()) / 2;
			return dlib::point(x, y);
		}

		bool matched;
		int numFramesLost;

		// Interpolation Methods
		static inline double lerp(double a, double b, double t) {
			return (a * (1.0 - t)) + (b * t);
		}
		static void lerp(const dlib::rectangle& a, const dlib::rectangle& b,
			dlib::rectangle& c, double t);
		static inline void lerp(const dlib::point& a, const dlib::point& b,
			dlib::point& c, double t) {
			c.x() = (long)lerp((double)a.x(), (double)b.x(), t);
			c.y() = (long)lerp((double)a.y(), (double)b.y(), t);
		}
		static void	lerp3(const double* __restrict a, const double* __restrict b,
			double* __restrict c, double t);
		static void	lerp4(const double* __restrict a, const double* __restrict b,
			double* __restrict c, double t);
		static void	slerp(const double* __restrict a, const double* __restrict b,
			double* __restrict c, double t);
		static void	slerpalerp(const double* __restrict a,
			const double* __restrict b, double* __restrict c, double t);



	private:

		// kalman filters. 
		// see: 'An Introduction to the Kalman Filter' - Gary Bishop
		//      http://www.cs.unc.edu/~tracker/media/pdf/SIGGRAPH2001_CoursePack_08.pdf
		//
		enum FilterIndex : uint32_t {
			KF_BOUNDS_LEFT,
			KF_BOUNDS_BOTTOM,
			KF_BOUNDS_RIGHT,
			KF_BOUNDS_TOP,
			KF_TRANS_X,
			KF_TRANS_Y,
			KF_TRANS_Z,
			KF_ROT_X,
			KF_ROT_Y,
			KF_ROT_Z,
			KF_ROT_A,

			KF_NUM_FILTERS
		};
		std::array<SingleValueKalman, KF_NUM_FILTERS> kalmanFilters;

		void InitKalmanFilters();

		// cv::solvePnp returns totally flipped results at times. 
		// we correct them here.
		static void						CheckForPoseFlip(double* r, double* t);
	};

	typedef sarray<DetectionResult, MAX_FACES> DetectionResults;
}

