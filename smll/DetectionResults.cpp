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



// how many frames before we consider a face "lost"
#define NUM_FRAMES_TO_LOSE_FACE			(5)


namespace smll {

	static void CheckForPoseFlip(double* r, double* t) {

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
			double x = sin1 * cos2 * r[0] + sin1 * sin2 * r[1];
			double y = sin1 * cos2 * r[1] + sin1 * sin2 * r[0];
			double z = sin1 * cos2 * r[2] + cos1 * sin2;

			double sina = sin(a / 2.0);
			if (sina < 0.0001)
				sina = 1.0;
			r[0] = x / sina;
			r[1] = y / sina;
			r[2] = z / sina;
			r[3] = a;
		}
	}

	ThreeDPose::ThreeDPose() {
		ResetPose();
	}

	void ThreeDPose::SetPose(cv::Mat cvRot, cv::Mat cvTrs) {
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
	}

	cv::Mat ThreeDPose::GetCVRotation() const {
		cv::Mat m = cv::Mat::zeros(3, 1, CV_64F);
		m.at<double>(0, 0) = rotation[0] * rotation[3];
		m.at<double>(1, 0) = rotation[1] * rotation[3];
		m.at<double>(2, 0) = rotation[2] * rotation[3];
		return m;
	}

	cv::Mat ThreeDPose::GetCVTranslation() const {
		cv::Mat m = cv::Mat::zeros(3, 1, CV_64F);
		m.at<double>(0, 0) = translation[0];
		m.at<double>(1, 0) = translation[1];
		m.at<double>(2, 0) = translation[2];
		return m;
	}

	void ThreeDPose::CopyPoseFrom(const ThreeDPose& r) {
		for (int i = 0; i < 3; i++) {
			this->translation[i] = r.translation[i];
			this->rotation[i] = r.rotation[i];
		}
		this->rotation[3] = r.rotation[3];
	}

	double ThreeDPose::DistanceTo(const ThreeDPose& r) const
	{
		double x = r.translation[0] - translation[0];
		double y = r.translation[1] - translation[1];
		double z = r.translation[2] - translation[2];
		return sqrt(x*x + y*y + z*z);
	}

	bool ThreeDPose::PoseValid() {
		if (translation[2] < 0.0f)
			return false;
		double v = 0.0f;
		for (int i = 0; i < 3; i++) {
			v += translation[i] * translation[i];
		}
		if (v < 1.0 || v > 10000.0)
			return false;
		
		return true;
	}

	void ThreeDPose::ResetPose() {
		rotation[0] = 0.0;
		rotation[1] = 1.0;
		rotation[2] = 0.0;
		rotation[3] = 0.0;
		translation[0] = 0.0;
		translation[1] = 0.0;
		translation[2] = 0.0;
	}




	DetectionResults::DetectionResults() 
		: sarray<DetectionResult, MAX_FACES>() {

	}

	void DetectionResults::CorrelateAndUpdateFrom(DetectionResults& other) {

		DetectionResults& faces = *this;

		// no faces lost, maybe some gained
		if (faces.length <= other.length) {
			// clear matched flags
			for (int i = 0; i < other.length; i++) {
				other[i].matched = false;
			}
			// match our faces to the new ones
			for (int i = 0; i < faces.length; i++) {
				// find closest
				int closest = other.findClosest(faces[i]);

				// smooth new face into ours
				faces[i].UpdateResultsFrom(other[closest]);
				faces[i].numFramesLost = 0;
				other[closest].matched = true;
			}
			// now check for new faces
			for (int i = 0; i < other.length; i++) {
				if (!other[i].matched) {
					// copy new face
					faces[faces.length] = other[i];
					faces[faces.length].numFramesLost = 0;
					other[i].matched = true;
					faces.length++;
				}
			}
		}

		// faces were lost
		else {
			// clear matched flags
			for (int i = 0; i < faces.length; i++) {
				faces[i].matched = false;
			}
			// match new faces to ours
			for (int i = 0; i < other.length; i++) {
				// find closest
				int closest = faces.findClosest(other[i]);

				// smooth new face into ours
				faces[closest].UpdateResultsFrom(other[i]);
				faces[closest].numFramesLost = 0;
				faces[closest].matched = true;
			}
			// now we need check lost faces
			for (int i = 0; i < faces.length; i++) {
				if (!faces[i].matched) {
					// wait some number of frames until we actually lose the face
					faces[i].numFramesLost++;
					if (faces[i].numFramesLost > NUM_FRAMES_TO_LOSE_FACE) {
						// remove face
						for (int j = i; j < (faces.length - 1); j++) {
							faces[j] = faces[j + 1];
						}
						faces.length--;
					}
					else {
						faces[i].UpdateResults();
					}
				}
			}
		}
	}

	int DetectionResults::findClosest(const smll::DetectionResult& result) {

		DetectionResults& results = *this;

		// find closest
		int closest = -1;
		double min = DBL_MAX;
		for (int j = 0; j < results.length; j++) {
			if (!results[j].matched) {
				double d = result.DistanceTo(results[j]);
				if (d < min) {
					closest = j;
					min = d;
				}
			}
		}
		return closest;
	}


	DetectionResult::DetectionResult() 
		: matched(false), numFramesLost(0), kalmanFiltersInitialized(false), initedStartPose(false) {

	}

	DetectionResult::~DetectionResult() {
	}

	DetectionResult& DetectionResult::operator=(const DetectionResult& r) {
		bounds = r.bounds;
		pose.CopyPoseFrom(r.pose);
		
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			landmarks68[i] = r.landmarks68[i];
		}
		CheckForPoseFlip(pose.rotation, pose.translation);

		kalmanFiltersInitialized = false;
		initedStartPose = false;
		return *this;
	}

	DetectionResult& DetectionResult::operator=(const Face& f) {
		bounds = f.m_bounds;
		return *this;
	}

	void DetectionResult::SetPose(const ThreeDPose& p) {
		pose.CopyPoseFrom(p);
		kalmanFiltersInitialized = false;
	}

	void DetectionResult::SetPose(cv::Mat cvRot, cv::Mat cvTrs) {
		pose.SetPose(cvRot, cvTrs);
		kalmanFiltersInitialized = false;
	}

	cv::Mat DetectionResult::GetCVRotation() const {
		return pose.GetCVRotation();
	}

	cv::Mat DetectionResult::GetCVTranslation() const {
		return pose.GetCVTranslation();
	}

	double DetectionResult::DistanceTo(const DetectionResult& r) const {
		return pose.DistanceTo(r.pose);
	}

	void DetectionResult::CopyPoseFrom(const DetectionResult& r) {
		pose.CopyPoseFrom(r.pose);
	}

	bool DetectionResult::PoseValid() {
		return pose.PoseValid();
	}

	void DetectionResult::ResetPose() {
		pose.ResetPose();
	}

	void DetectionResult::InitStartPose() {
		if (!initedStartPose) {
			startPose.rotation[0] = 0.0;
			startPose.rotation[1] = 0.0;
			startPose.rotation[2] = 0.0;
			startPose.rotation[3] = 0.0;
			startPose.translation[0] = 0.0;
			startPose.translation[1] = 4;  // default y
			startPose.translation[2] = 34; // default Z
			initedStartPose = true;
		}
	}

	void DetectionResult::UpdateResultsFrom(const DetectionResult& r) {

		int i;
		for (i = 0; i < 3; i++)	{
			ntx[i] = r.pose.translation[i];
			nrot[i] = r.pose.rotation[i];
		}
		//additional  for nrot 
		nrot[i] = r.pose.rotation[i];

		dlib::rectangle bnd = r.bounds;
		CheckForPoseFlip(nrot, ntx);

		// copy values
		bounds = bnd;

		if (!kalmanFiltersInitialized) {
			*this = r;
			InitKalmanFilters();
		}

		UpdateResults();

		for (int j = 0; j < smll::NUM_FACIAL_LANDMARKS; j++) {
			landmarks68[j] = r.landmarks68[j];
		}
	}

	void DetectionResult::UpdateResults() {
		if (!kalmanFiltersInitialized) {
			return;
		}

		// kalman filtering enabled?
		if (Config::singleton().get_bool(CONFIG_BOOL_KALMAN_ENABLE)) {
			
			// update smoothing factor
			double smoothing = Config::singleton().get_double(CONFIG_FLOAT_SMOOTHING_FACTOR);
			for (int i = 0; i < KF_NUM_FILTERS; i++) {
				kalmanFilters[i].SetMeasurementNoiseCovariance(smoothing);
			}

			// update the kalman filters
			pose.translation[0] = kalmanFilters[KF_TRANS_X].Update(ntx[0]);
			pose.translation[1] = kalmanFilters[KF_TRANS_Y].Update(ntx[1]);
			pose.translation[2] = kalmanFilters[KF_TRANS_Z].Update(ntx[2]);
			pose.rotation[0] = kalmanFilters[KF_ROT_X].Update(nrot[0]);
			pose.rotation[1] = kalmanFilters[KF_ROT_Y].Update(nrot[1]);
			pose.rotation[2] = kalmanFilters[KF_ROT_Z].Update(nrot[2]);
			pose.rotation[3] = kalmanFilters[KF_ROT_A].Update(nrot[3]);
		}
		else {
			pose.translation[0] = ntx[0];
			pose.translation[1] = ntx[1];
			pose.translation[2] = ntx[2];
			pose.rotation[0]	= nrot[0];
			pose.rotation[1]	= nrot[1];
			pose.rotation[2]	= nrot[2];
			pose.rotation[3]	= nrot[3];
		}

	}


	void DetectionResult::InitKalmanFilters() {
		if (Config::singleton().get_bool(CONFIG_BOOL_KALMAN_ENABLE)) {
			kalmanFilters[KF_TRANS_X].Init(pose.translation[0]);
			kalmanFilters[KF_TRANS_Y].Init(pose.translation[1]);
			kalmanFilters[KF_TRANS_Z].Init(pose.translation[2]);
			kalmanFilters[KF_ROT_X].Init(pose.rotation[0]);
			kalmanFilters[KF_ROT_Y].Init(pose.rotation[1]);
			kalmanFilters[KF_ROT_Z].Init(pose.rotation[2]);
			kalmanFilters[KF_ROT_A].Init(pose.rotation[3]);
			kalmanFiltersInitialized = true;
		}
	}

}
