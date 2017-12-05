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
#include "Face.hpp"

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#include <libobs/obs-module.h>
#pragma warning( pop )

namespace smll {

	
	
Face::Face() 
	: m_trackingX(0)
	, m_trackingY(0)
	, m_poseResetCounter(0)
	, m_trackingScale(1.0)
	, m_poseInitialized(false) {
	ResetPose();
}


Face::~Face() {
}

Face& Face::operator=(const Face& f) {
	m_bounds = f.m_bounds;
	m_trackingX = f.m_trackingX;
	m_trackingY = f.m_trackingY;
	m_trackingScale = f.m_trackingScale;
	for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
		m_points[i] = f.m_points[i];
	}
	m_poseInitialized = f.m_poseInitialized;
	f.m_cvTranslation.copyTo(m_cvTranslation);
	f.m_cvRotation.copyTo(m_cvRotation);

	return *this;
}

void Face::copy2DDataTo(Face& f) {
	f.m_bounds = m_bounds;
	for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
		f.m_points[i] = m_points[i];
	}
}

void Face::ResetPose() {
	m_poseResetCounter = 0;
	m_poseInitialized = false;
	m_cvTranslation = cv::Mat::zeros(3, 1, CV_64F);
	m_cvRotation = cv::Mat::zeros(3, 1, CV_64F);
}



} // smll namespace
