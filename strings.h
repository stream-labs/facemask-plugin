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

// translation macros
// see data/locale/en-US.ini
//
#define P_TRANSLATE(x)			obs_module_text(x)
#define P_DESC(x)				x ".Description"

// Properties
#define P_MASK					"Mask"
#define P_DEACTIVATE			"deactivated"
#define P_DRAWMASK				"drawmask"
#define P_DRAWFACEDATA			"drawfaces"
#define P_DRAWMORPHTRIS			"drawmorphtris"
#define P_DRAWCROPRECT			"drawFDCropRect"
#define P_DEMOMODEON			"demoMode"
#define P_DEMOFOLDER			"demoFolder"
#define P_DEMOINTERVAL			"demoInterval"
#define P_DEMODELAY				"demoDelay"
#define P_BGREMOVAL				"greenscreen"
#define P_GENTHUMBS				"genpreviews"
#define P_REWIND				"rewindanims"
#define P_CARTOON				"cartoonMode"

// Other static strings
static const char* const kFileDefaultJson = "masks/No Mask.json";
static const char* const kFileAlertJson = "alerts/alert.json";
static const char* const kFontAlertTTF = "fonts/back_issues.ttf";

