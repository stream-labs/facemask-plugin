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
static const char* const kSettingsDrawFDCropRect = "drawFDCropRect";
static const char* const kSettingsDrawTRCropRect = "drawTRCropRect";

static const char* const kSettingsDemoMode = "demoMode";
static const char* const kSettingsDemoFolder = "demoFolder";
static const char* const kSettingsDemoInterval = "demoInterval";
static const char* const kSettingsDemoDelay = "demoDelay";

static const char* const kSettingsPerformance = "perfSetting";


// setting descriptions
static const char* const kSettingsDeactivatedDesc = "Deactivate the filter";
static const char* const kSettingsDrawMaskDesc = "Draw the face mask";

static const char* const kSettingsDrawFacesDesc = "Draw Face Landmark Data";
static const char* const kSettingsDrawFDCropRectDesc = 
	"Draw Face Detect Crop Rectangle";
static const char* const kSettingsDrawTRCropRectDesc =
	"Draw Tracking Crop Rectangle";

static const char* const kSettingsDemoModeDesc = "Demo Mode";
static const char* const kSettingsDemoFolderDesc = "Demo Mode Folder";
static const char* const kSettingsDemoIntervalDesc = "Demo Mode Interval (seconds)";
static const char* const kSettingsDemoDelayDesc = "Demo Mode Delay Between Masks (seconds)";

static const char* const kSettingsPerformanceDesc = "Performance Setting";

static const char* const kSettingsEmpty = "";

static const char* const kFileDefaultJson = "masks/mask1.json";
static const char* const kFileShapePredictor =
	"shape_predictor_68_face_landmarks.dat";

static const char* const kFileJsonOption1 = kFileDefaultJson;
static const char* const kFileJsonOption2 = "masks/mask2.json";
static const char* const kFileJsonOption3 = "masks/mask3.json";
static const char* const kFileJsonOption4 = "masks/mask4.json";
static const char* const kFileJsonOption5 = "masks/mask5.json";
static const char* const kFileJsonOption6 = "masks/mask6.json";
static const char* const kFileJsonOption7 = "masks/mask7.json";
static const char* const kFileJsonOption8 = "masks/mask8.json";
static const char* const kFileJsonOption9 = "masks/mask9.json";
static const char* const kFileJsonOption10 = "masks/mask10.json";
static const char* const kSettingsJsonOption1 = "Mask 1";
static const char* const kSettingsJsonOption2 = "Mask 2";
static const char* const kSettingsJsonOption3 = "Mask 3";
static const char* const kSettingsJsonOption4 = "Mask 4";
static const char* const kSettingsJsonOption5 = "Mask 5";
static const char* const kSettingsJsonOption6 = "Mask 6";
static const char* const kSettingsJsonOption7 = "Mask 7";
static const char* const kSettingsJsonOption8 = "Mask 8";
static const char* const kSettingsJsonOption9 = "Mask 9";
static const char* const kSettingsJsonOption10 = "Mask 10";

static const char* const kSettingsPerformanceOption1 = "CPU Heavy";
static const char* const kSettingsPerformanceOption2 = "Moderate GPU (Recommended)";
static const char* const kSettingsPerformanceOption3 = "More GPU";
static const char* const kSettingsPerformanceOption4 = "GPU Heavy";
