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
		// face detection/tracking
		dlib::rectangle		bounds;

		// facial landmarks (5 and 68)
		dlib::point			landmarks5[FIVE_LANDMARK_NUM_LANDMARKS];
		dlib::point			landmarks68[NUM_FACIAL_LANDMARKS];

		// 3D pose estimation
		double		translation[3];
		double		rotation[4];

		DetectionResult();
		DetectionResult(const DetectionResult& r) { *this = r; }
		~DetectionResult();
		DetectionResult& operator=(const DetectionResult& r);
		DetectionResult& operator=(const Face& f);

		void SetPose(cv::Mat cvRot, cv::Mat cvTrs);
		cv::Mat GetCVRotation() const;
		cv::Mat GetCVTranslation() const;

		void UpdateResults(const DetectionResult& r);

		double DistanceTo(const DetectionResult& r) const;

		inline dlib::point GetPosition() {
			long x = (int)(bounds.right() + bounds.left()) / 2;
			long y = (int)(bounds.top() + bounds.bottom()) / 2;
			return dlib::point(x, y);
		}

		bool matched;
		int numFramesLost;


	private:

		// kalman filters. 
		// see: 'An Introduction to the Kalman Filter' - Gary Bishop
		//      http://www.cs.unc.edu/~tracker/media/pdf/SIGGRAPH2001_CoursePack_08.pdf
		//
		enum FilterIndex : uint32_t {
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

		bool	kalmanFiltersInitialized;
		void InitKalmanFilters();

		// cv::solvePnp returns totally flipped results at times. 
		// we correct them here.
		static void						CheckForPoseFlip(double* r, double* t);
	};

	typedef sarray<DetectionResult, MAX_FACES> DetectionResults;
}

