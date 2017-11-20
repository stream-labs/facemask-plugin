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
		: matched(false), numFramesLost(0) {
		rotation[0] = 1.0;
		rotation[1] = 0.0;
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
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			landmarks[i] = r.landmarks[i];
		}
		CheckForPoseFlip(rotation, translation);

		InitKalmanFilters();

		return *this;
	}

	DetectionResult& DetectionResult::operator=(const Face& f) {
		// bounds
		bounds = f.GetBounds();

		// landmarks
		const dlib::point* fp = f.GetPoints();
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			landmarks[i] = fp[i];
		}

		// rotation
		// - openCV uses a scaled vector for rotation, so
		//   we convert it to an axis-angle rotation for
		//   easy use with obs/opengl/whatever
		const cv::Mat& m = f.GetCVRotation();
		double angle = sqrt(m.dot(m));
		double dividor = angle;
		if (angle < 0.001) {
			rotation[0] = 1.0;
			rotation[1] = 0.0;
			rotation[2] = 0.0;
			rotation[3] = 0.0;
		}
		else {
			rotation[0] = m.at<double>(0, 0) / dividor;
			rotation[1] = m.at<double>(1, 0) / dividor;
			rotation[2] = m.at<double>(2, 0) / dividor;
			rotation[3] = angle;
		}

		// translation
		const cv::Mat& t = f.GetCVTranslation();
		translation[0] = t.at<double>(0, 0);
		translation[1] = t.at<double>(1, 0);
		translation[2] = t.at<double>(2, 0);

		CheckForPoseFlip(rotation, translation);

		InitKalmanFilters();

		return *this;
	}

	double DetectionResult::DistanceTo(const DetectionResult& r) const
	{
		double x = r.translation[0] - translation[0];
		double y = r.translation[1] - translation[1];
		double z = r.translation[2] - translation[2];
		return sqrt(x*x + y*y + z*z);
	}

	void DetectionResult::UpdateResults(const DetectionResult& r, 
		double smoothingAmount) {

		double ntx[3] = { r.translation[0], r.translation[1], r.translation[2] };
		double nrot[4] = { r.rotation[0], r.rotation[1], r.rotation[2], r.rotation[3] };
		dlib::rectangle bnd = r.bounds;

		CheckForPoseFlip(nrot, ntx);

		if (Config::singleton().get_bool(CONFIG_BOOL_KALMAN_ENABLE)) {

			// update the kalman filters
			//bnd.set_right((long)kalmanFilters[KF_BOUNDS_RIGHT].Update(bnd.right()));
			//bnd.set_bottom((long)kalmanFilters[KF_BOUNDS_BOTTOM].Update(bnd.bottom()));
			//bnd.set_left((long)kalmanFilters[KF_BOUNDS_LEFT].Update(bnd.left()));
			//bnd.set_top((long)kalmanFilters[KF_BOUNDS_TOP].Update(bnd.top()));
			ntx[0] = kalmanFilters[KF_TRANS_X].Update(ntx[0]);
			ntx[1] = kalmanFilters[KF_TRANS_Y].Update(ntx[1]);
			ntx[2] = kalmanFilters[KF_TRANS_Z].Update(ntx[2]);
			nrot[0] = kalmanFilters[KF_ROT_X].Update(nrot[0]);
			nrot[1] = kalmanFilters[KF_ROT_Y].Update(nrot[1]);
			nrot[2] = kalmanFilters[KF_ROT_Z].Update(nrot[2]);
			nrot[3] = kalmanFilters[KF_ROT_A].Update(nrot[3]);
		}

		if (smoothingAmount > 0.0f) {
			// use smoothing to update current value
			lerp3(translation, ntx, translation, smoothingAmount);
			lerp4(rotation, nrot, rotation, smoothingAmount);
			lerp(bounds, bnd, bounds, smoothingAmount);
			for (int i = 0; i < smll::NUM_FACIAL_LANDMARKS; i++) {
				lerp(landmarks[i], r.landmarks[i], landmarks[i], smoothingAmount);
			}
		}
		else {
			// just copy values
			translation[0] = ntx[0];
			translation[1] = ntx[1];
			translation[2] = ntx[2];
			rotation[0] = nrot[0];
			rotation[1] = nrot[1];
			rotation[2] = nrot[2];
			bounds = bnd;
			for (int i = 0; i < smll::NUM_FACIAL_LANDMARKS; i++) {
				landmarks[i] = r.landmarks[i];
			}
		}
	}

	void DetectionResult::CheckForPoseFlip(double* r, double* t) {
		// check for pose flip
		if (t[2] < 0.0)
		{
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

	void DetectionResult::lerp(const dlib::rectangle& a, const dlib::rectangle& b,
		dlib::rectangle& c, double t) {
		c.set_top((long)lerp((double)a.top(), (double)b.top(), t));
		c.set_bottom((long)lerp((double)a.bottom(), (double)b.bottom(), t));
		c.set_left((long)lerp((double)a.left(), (double)b.left(), t));
		c.set_right((long)lerp((double)a.right(), (double)b.right(), t));
	}

	void DetectionResult::lerp3(const double* __restrict a,
		const double* __restrict b, double* __restrict c, double t) {
		c[0] = lerp(a[0], b[0], t);
		c[1] = lerp(a[1], b[1], t);
		c[2] = lerp(a[2], b[2], t);
	}

	void DetectionResult::lerp4(const double* __restrict a,
		const double* __restrict b, double* __restrict c, double t) {
		c[0] = lerp(a[0], b[0], t);
		c[1] = lerp(a[1], b[1], t);
		c[2] = lerp(a[2], b[2], t);
		c[3] = lerp(a[3], b[3], t);
	}

	void DetectionResult::slerp(const double* __restrict a,
		const double* __restrict b, double* __restrict c, double t) {
		double omega = acos((a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]));
		double sin_o = sin(omega);
		double sin_mt = sin((1.0 - t) * omega);
		double sin_t = sin(t * omega);

		if (abs(sin_o) < 0.1) {
			lerp3(a, b, c, t);
		}
		else {
			sin_mt = sin_mt / sin_o;
			sin_t = sin_t / sin_o;
			c[0] = (sin_t * a[0]) + (sin_mt * b[0]);
			c[1] = (sin_t * a[1]) + (sin_mt * b[1]);
			c[2] = (sin_t * a[2]) + (sin_mt * b[2]);
		}
	}

	void DetectionResult::slerpalerp(const double* __restrict a,
		const double* __restrict b, double* __restrict c, double t) {
		slerp(a, b, c, t);
		c[3] = lerp(a[3], b[3], t);
	}

	void DetectionResult::InitKalmanFilters() {
		if (Config::singleton().get_bool(CONFIG_BOOL_KALMAN_ENABLE)) {
			kalmanFilters[KF_BOUNDS_RIGHT].Init(bounds.right());
			kalmanFilters[KF_BOUNDS_BOTTOM].Init(bounds.bottom());
			kalmanFilters[KF_BOUNDS_LEFT].Init(bounds.left());
			kalmanFilters[KF_BOUNDS_TOP].Init(bounds.top());
			kalmanFilters[KF_TRANS_X].Init(translation[0]);
			kalmanFilters[KF_TRANS_Y].Init(translation[1]);
			kalmanFilters[KF_TRANS_Z].Init(translation[2]);
			kalmanFilters[KF_ROT_X].Init(rotation[0]);
			kalmanFilters[KF_ROT_Y].Init(rotation[1]);
			kalmanFilters[KF_ROT_Z].Init(rotation[2]);
			kalmanFilters[KF_ROT_A].Init(rotation[3]);
		}
	}

}
