/*
* Face Masks for SlOBS
* smll - streamlabs machine learning library
*
*
* Kalman filter implementation using dlib. Based on the implementation
* by Hayk Martirosyan:
*
*     https://github.com/hmartiro/kalman-cpp
*
* which, according to him, is based on the following
* introductory paper:
*
*     http://www.cs.unc.edu/~welch/media/pdf/kalman_intro.pdf
*
*
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
*
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
#include <dlib/matrix.h>
#pragma warning( pop )

namespace smll {

	template <
		typename T,
		long M,  // number of measurements
		long N   // number of states
	>
	class KalmanFilter {

	public:

		// Create a Kalman filter with the specified matrices.
		//   A - System dynamics matrix
		//   C - Output matrix
		//   Q - Process noise covariance
		//   R - Measurement noise covariance
		//   P - Estimate error covariance
		//
		KalmanFilter(
			double _dt,
			const dlib::matrix<T, N, N>& _A,
			const dlib::matrix<T, M, N>& _C,
			const dlib::matrix<T, N, N>& _Q,
			const dlib::matrix<T, M, M>& _R,
			const dlib::matrix<T, N, N>& _P)
			: A(_A), C(_C), Q(_Q), R(_R), P0(_P),
			t0(0), t(0), dt(_dt), initialized(false) {

			I = dlib::identity_matrix<double, N>();
		}

		// Initialize the filter with initial states as zero.
		void init() {
			x_hat = dlib::zeros_matrix<double, N, 1>();
			P = P0;
			t0 = 0;
			t = t0;
			initialized = true;
		}

		// Initialize the filter with a guess for initial states.
		void init(double _t0, const dlib::matrix<T, N, 1>& _x0) {
			x_hat = _x0;
			P = P0;
			this->t0 = _t0;
			t = _t0;
			initialized = true;
		}

		// Update the estimated state based on measured values. The
		// time step is assumed to remain constant.
		void update(const dlib::matrix<T, M, 1>& y) {

			if (!initialized)
				return;

			x_hat_new = A * x_hat;
			P = A * P * dlib::trans(A) + Q;
			K = P * dlib::trans(C) * dlib::inv(C * P * dlib::trans(C) + R);
			x_hat_new = x_hat_new + K * (y - C * x_hat_new);
			P = (I - K * C) * P;
			x_hat = x_hat_new;

			t += dt;
		}

		// Update the estimated state based on measured values,
		// using the given time step and dynamics matrix.
		void update(const dlib::matrix<T, M, 1>& y, 
			double dt, const dlib::matrix<T, N, N>& _A) {

			this->A = _A;
			this->dt = _dt;
			update(y);
		}

		// Return the current state and time.
		const dlib::matrix<T, N, 1>& state() { 
			return x_hat; 
		}
		double time() { 
			return t; 
		}

		// Accessor for Measurement noise covariance (R)
		// - akin to a smoothing factor
		void set_R(double r) {
			R = r;
		}

	private:
		// Matrices for computation
		dlib::matrix<T, N, N>	A; // System dynamics matrix
		dlib::matrix<T, M, N>	C; // Output matrix
		dlib::matrix<T, N, N>	Q; // Process noise covariance
		dlib::matrix<T, M, M>	R; // Measurement noise covariance
		dlib::matrix<T, N, N>	P; // Estimate error covariance
		dlib::matrix<T, N, 1>	K;
		dlib::matrix<T, N, N>	P0;

		// Initial and current time
		double t0, t;

		// Discrete time step
		double dt;

		// Is the filter initialized?
		bool initialized;

		// n-size identity
		dlib::matrix<T, N, N> I;

		// Estimated states
		dlib::matrix<T, N, 1> x_hat, x_hat_new;
	};
}

