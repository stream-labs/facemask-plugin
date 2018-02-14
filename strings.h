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

#define P_TRANSLATE(x)			obs_module_text(x)
#define P_DESC(x)				x ".Description"
// General Translation
#define P_NAME					"Name"
// Properties
#define P_MASK					"Mask"
/// Advanced
#define P_ADVANCED				"Advanced"
#define P_RESOLUTIONSCALE			"ResolutionScale"
/// Debug
#define P_DEBUG					"Debug"

// setting keys
static const char* const kSettingsDeactivated = "deactivated";
static const char* const kSettingsDrawMask = "drawmask";

static const char* const kSettingsDrawFaces = "drawfaces";
static const char* const kSettingsDrawMorphTris = "drawmorphtris";
static const char* const kSettingsDrawFDCropRect = "drawFDCropRect";
static const char* const kSettingsDrawTRCropRect = "drawTRCropRect";

static const char* const kSettingsDemoMode = "demoMode";
static const char* const kSettingsDemoFolder = "demoFolder";
static const char* const kSettingsDemoInterval = "demoInterval";
static const char* const kSettingsDemoDelay = "demoDelay";

static const char* const kSettingsPixomojo = "pixomojo";

// setting descriptions
static const char* const kSettingsDeactivatedDesc = "Deactivate the filter";
static const char* const kSettingsDrawMaskDesc = "Draw the face mask";

static const char* const kSettingsDrawFacesDesc = "Draw Face Landmark Data";
static const char* const kSettingsDrawMorphTrisDesc = "Draw Morph Wireframe";
static const char* const kSettingsDrawFDCropRectDesc =
	"Draw Face Detect Crop Rectangle";
static const char* const kSettingsDrawTRCropRectDesc =
	"Draw Tracking Crop Rectangle"; 

static const char* const kSettingsDemoModeDesc = "Demo Mode";
static const char* const kSettingsDemoFolderDesc = "Demo Mode Folder";
static const char* const kSettingsDemoIntervalDesc = "Demo Mode Interval (seconds)";
static const char* const kSettingsDemoDelayDesc = "Demo Mode Delay Between Masks (seconds)";

static const char* const kSettingsPixomojoDesc = "Get more face masks from pixomojo.com";



static const char* const kSettingsEmpty = "";

static const char* const kFileDefaultJson = "masks/Baseball Cap.json";
static const char* const kFileShapePredictor =
	"shape_predictor_68_face_landmarks.dat";
