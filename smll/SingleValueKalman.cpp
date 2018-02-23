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

#include "SingleValueKalman.hpp"

namespace smll {


	SingleValueKalman::SingleValueKalman() {

		// This is taken from the kalman-test.cpp
		// provided with the KalmanFilter by
		//  Hayk Martirosyan
		//

		static const int n = 3; // Number of states
		static const int m = 1; // Number of measurements

		double dt = 1.0 / 10; // Time step (todo: this should be dynamic)

		dlib::matrix<double, n, n> A; // System dynamics matrix
		dlib::matrix<double, m, n> C; // Output matrix
		dlib::matrix<double, n, n> Q; // Process noise covariance
		dlib::matrix<double, m, m> R; // Measurement noise covariance
		dlib::matrix<double, n, n> P; // Estimate error covariance

		// Discrete LTI projectile motion, measuring position only
		A = 1, dt, 0,
			0, 1, dt,
			0, 0, 1;
		C = 1, 0, 0;

		// Reasonable covariance matrices
		Q =	.05, .05, .0,
			.05, .05, .0,
			.0, .0, .0;
		P =	.1, .1, .1,
			.1, 10000, 10,
			.1, 10, 100;

		// R largely determines the "smoothing factor"
		// under 0 is low, 1 is good, 5 is high, 10 is too much
		// - I keep this at 1 for the facemask plugin
		// - I set this to 4 for generating previews
		R = 4;

		m_kf = new KalmanFilter<double, m, n>(dt, A, C, Q, R, P);
	}

	SingleValueKalman::~SingleValueKalman() {

		delete m_kf;
	}

	void SingleValueKalman::Init(double val) {

		dlib::matrix<double, 3, 1> x0;
		x0 = val, 0, 0;
		m_kf->init(0, x0);
	}

	double SingleValueKalman::Update(double val) {

		dlib::matrix<double, 1, 1> y;
		y = val;
		m_kf->update(y);
		return m_kf->state()(0, 0);
	}




}