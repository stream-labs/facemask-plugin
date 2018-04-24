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
#include "DetectionResults.hpp"
#include "Config.hpp"


namespace smll {

	DetectionResult::DetectionResult() 
		: matched(false), numFramesLost(0), kalmanFiltersInitialized(false) {
		rotation[0] = 0.0;
		rotation[1] = 1.0;
		rotation[2] = 0.0;
		rotation[3] = 0.0;
		translation[0] = 0.0;
		translation[1] = 0.0;
		translation[2] = 0.0;
	}

	DetectionResult::~DetectionResult() {
	}

	DetectionResult& DetectionResult::operator=(const DetectionResult& r) {
		bounds = r.bounds;
		rotation[0] = r.rotation[0];
		rotation[1] = r.rotation[1];
		rotation[2] = r.rotation[2];
		rotation[3] = r.rotation[3];
		translation[0] = r.translation[0];
		translation[1] = r.translation[1];
		translation[2] = r.translation[2];
		for (int i = 0; i < FIVE_LANDMARK_NUM_LANDMARKS; i++) {
			landmarks5[i] = r.landmarks5[i];
		}
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			landmarks68[i] = r.landmarks68[i];
		}
		CheckForPoseFlip(rotation, translation);

		kalmanFiltersInitialized = false;

		return *this;
	}

	DetectionResult& DetectionResult::operator=(const Face& f) {
		bounds = f.m_bounds;
		return *this;
	}

	void DetectionResult::SetPose(cv::Mat cvRot, cv::Mat cvTrs) {
		// rotation
		// - openCV uses a scaled vector for rotation, so
		//   we convert it to an axis-angle rotation for
		//   easy use with obs/opengl/whatever
		double angle = sqrt(cvRot.dot(cvRot));
		if (angle < 0.0001) {
			rotation[0] = 0.0;
			rotation[1] = 1.0;
			rotation[2] = 0.0;
			rotation[3] = 0.0;
		}
		else {
			rotation[0] = cvRot.at<double>(0, 0) / angle;
			rotation[1] = cvRot.at<double>(1, 0) / angle;
			rotation[2] = cvRot.at<double>(2, 0) / angle;
			rotation[3] = angle;
		}

		// translation
		translation[0] = cvTrs.at<double>(0, 0);
		translation[1] = cvTrs.at<double>(1, 0);
		translation[2] = cvTrs.at<double>(2, 0);

		CheckForPoseFlip(rotation, translation);

		kalmanFiltersInitialized = false;
	}

	cv::Mat DetectionResult::GetCVRotation() const {
		cv::Mat m = cv::Mat::zeros(3, 1, CV_64F);
		m.at<double>(0, 0) = rotation[0] * rotation[3];
		m.at<double>(1, 0) = rotation[1] * rotation[3];
		m.at<double>(2, 0) = rotation[2] * rotation[3];
		return m;
	}

	cv::Mat DetectionResult::GetCVTranslation() const {
		cv::Mat m = cv::Mat::zeros(3, 1, CV_64F);
		m.at<double>(0, 0) = translation[0];
		m.at<double>(1, 0) = translation[1];
		m.at<double>(2, 0) = translation[2];
		return m;
	}

	double DetectionResult::DistanceTo(const DetectionResult& r) const
	{
		double x = r.translation[0] - translation[0];
		double y = r.translation[1] - translation[1];
		double z = r.translation[2] - translation[2];
		return sqrt(x*x + y*y + z*z);
	}

	void DetectionResult::UpdateResults(const DetectionResult& r) {

		if (!kalmanFiltersInitialized) {
			*this = r;
			InitKalmanFilters();
		}

		double ntx[3] = { r.translation[0], r.translation[1], r.translation[2] };
		double nrot[4] = { r.rotation[0], r.rotation[1], r.rotation[2], r.rotation[3] };
		dlib::rectangle bnd = r.bounds;

		CheckForPoseFlip(nrot, ntx);

		// kalman filtering enabled?
		if (Config::singleton().get_bool(CONFIG_BOOL_KALMAN_ENABLE)) {
			
			// update smoothing factor
			double smoothing = Config::singleton().get_double(CONFIG_FLOAT_SMOOTHING_FACTOR);
			for (int i = 0; i < KF_NUM_FILTERS; i++) {
				kalmanFilters[i].SetMeasurementNoiseCovariance(smoothing);
			}

			// update the kalman filters
			ntx[0] = kalmanFilters[KF_TRANS_X].Update(ntx[0]);
			ntx[1] = kalmanFilters[KF_TRANS_Y].Update(ntx[1]);
			ntx[2] = kalmanFilters[KF_TRANS_Z].Update(ntx[2]);
			nrot[0] = kalmanFilters[KF_ROT_X].Update(nrot[0]);
			nrot[1] = kalmanFilters[KF_ROT_Y].Update(nrot[1]);
			nrot[2] = kalmanFilters[KF_ROT_Z].Update(nrot[2]);
			nrot[3] = kalmanFilters[KF_ROT_A].Update(nrot[3]);
		}

		// copy values
		bounds = bnd;
		for (int i = 0; i < FIVE_LANDMARK_NUM_LANDMARKS; i++) {
			landmarks5[i] = r.landmarks5[i];
		}
		for (int i = 0; i < smll::NUM_FACIAL_LANDMARKS; i++) {
			landmarks68[i] = r.landmarks68[i];
		}
		translation[0] = ntx[0];
		translation[1] = ntx[1];
		translation[2] = ntx[2];
		rotation[0] = nrot[0];
		rotation[1] = nrot[1];
		rotation[2] = nrot[2];
		rotation[3] = nrot[3];
	}

	void DetectionResult::CheckForPoseFlip(double* r, double* t) {
		// check for pose flip
		if (t[2] < 0.0)
		{
			//blog(LOG_DEBUG, "***** POSE FLIP *****");

			// flip translation
			t[0] = -t[0];
			t[1] = -t[1];
			t[2] = -t[2];
			
			// flip rotation
			// this is odd, but works...

			// flip z and the angle
			r[0] = r[0];
			r[1] = r[1];
			r[2] = -r[2];
			r[3] = -r[3];

			// rotate 180 around z
			// use Rodrigues formula, simplified for rotation around Z
			//
			double cos1 = cos(r[3] / 2.0);
			double sin1 = sin(r[3] / 2.0);
			double cos2 = cos(M_PI / 2.0);
			double sin2 = sin(M_PI / 2.0);

			double a = acos(cos1 * cos2 - sin1 * sin2 * r[2]) * 2.0;
			double x = sin1 * cos2 * r[0] +	sin1 * sin2 * r[1];
			double y = sin1 * cos2 * r[1] +	sin1 * sin2 * r[0];
			double z = sin1 * cos2 * r[2] +	cos1 * sin2;

			double sina = sin(a / 2.0);
			if (sina < 0.0001)
				sina = 1.0;
			r[0] = x / sina;
			r[1] = y / sina;
			r[2] = z / sina;
			r[3] = a;
		}
	}

	void DetectionResult::InitKalmanFilters() {
		if (Config::singleton().get_bool(CONFIG_BOOL_KALMAN_ENABLE)) {
			kalmanFilters[KF_TRANS_X].Init(translation[0]);
			kalmanFilters[KF_TRANS_Y].Init(translation[1]);
			kalmanFilters[KF_TRANS_Z].Init(translation[2]);
			kalmanFilters[KF_ROT_X].Init(rotation[0]);
			kalmanFilters[KF_ROT_Y].Init(rotation[1]);
			kalmanFilters[KF_ROT_Z].Init(rotation[2]);
			kalmanFilters[KF_ROT_A].Init(rotation[3]);
			kalmanFiltersInitialized = true;
		}
	}

}
