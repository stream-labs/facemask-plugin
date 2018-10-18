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
	, m_trackingScale(1.0) {
	
}


Face::~Face() {
}

Face& Face::operator=(const Face& f) {
	m_bounds = f.m_bounds;
	m_trackingX = f.m_trackingX;
	m_trackingY = f.m_trackingY;
	m_trackingScale = f.m_trackingScale;
	return *this;
}


} // smll namespace
