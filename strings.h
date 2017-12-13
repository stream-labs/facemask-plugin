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

static const char* const kSettingsEmpty = "";

static const char* const kFileDefaultJson = "masks/glasses_Slabs_01.head.json";
static const char* const kFileShapePredictor =
	"shape_predictor_68_face_landmarks.dat";


// I know, this is gross, but temporary 
static const char* const kFileJsonOption1 = kFileDefaultJson;
static const char* const kFileJsonOption2 = "masks/Beret2.head.json";
static const char* const kFileJsonOption3 = "masks/Beret_5 + head.json";
static const char* const kFileJsonOption4 = "masks/bugoutEyes_1.json";
static const char* const kFileJsonOption5 = "masks/bunnyMask_01_pink.json";
static const char* const kFileJsonOption6 = "masks/cigar_1.json";
static const char* const kFileJsonOption7 = "masks/crown_typeC.head.json";
static const char* const kFileJsonOption8 = "masks/doggyEars_typeA.json";
static const char* const kFileJsonOption9 = "masks/Ears1.head.json";
static const char* const kFileJsonOption10 = "masks/Ears2.head.json";
static const char* const kFileJsonOption11 = "masks/Ears3.head.json";
static const char* const kFileJsonOption12 = "masks/eyesMask_ver3.json";
static const char* const kFileJsonOption13 = "masks/flowerCrown_2.head.json";
static const char* const kFileJsonOption14 = "masks/GagFace_1.json";
static const char* const kFileJsonOption15 = "masks/glassesHearts_1 + head.json";
static const char* const kFileJsonOption16 = "masks/glasses_Lashes_white_01.head.json";
static const char* const kFileJsonOption17 = "masks/glasses_superStar_01.head.json";
static const char* const kFileJsonOption18 = "masks/googly_eyes_hugePupils_03.json";
static const char* const kFileJsonOption19 = "masks/horns_typeA.head.json";
static const char* const kFileJsonOption20 = "masks/juicyLips_03_pinkMega_emit.json";
static const char* const kFileJsonOption21 = "masks/Lashes.json";
static const char* const kFileJsonOption22 = "masks/megaEye_head_2.json";
static const char* const kFileJsonOption23 = "masks/mustache_typeB.json";
static const char* const kFileJsonOption24 = "masks/pigEars_02.head.json";
static const char* const kFileJsonOption25 = "masks/pigHead_01.json";
static const char* const kFileJsonOption26 = "masks/SheriffHat_1.head.json";
static const char* const kFileJsonOption27 = "masks/Tiara_gold + head.json";

static const char* const kSettingsJsonOption1 = "Streamlabs";
static const char* const kSettingsJsonOption2 = "Beret 2";
static const char* const kSettingsJsonOption3 = "Beret 5";
static const char* const kSettingsJsonOption4 = "Bugout Eyes";
static const char* const kSettingsJsonOption5 = "Pink Bunny Mask";
static const char* const kSettingsJsonOption6 = "Cigar";
static const char* const kSettingsJsonOption7 = "Crown of Antlers";
static const char* const kSettingsJsonOption8 = "Doggy Ears";
static const char* const kSettingsJsonOption9 = "Ears 1";
static const char* const kSettingsJsonOption10 = "Ears 2";
static const char* const kSettingsJsonOption11 = "Ears 3";
static const char* const kSettingsJsonOption12 = "Eyes Mask";
static const char* const kSettingsJsonOption13 = "Flower Crown 2";
static const char* const kSettingsJsonOption14 = "Gag Face";
static const char* const kSettingsJsonOption15 = "Heart Glasses";
static const char* const kSettingsJsonOption16 = "Lashes Glasses";
static const char* const kSettingsJsonOption17 = "Superstar Glasses";
static const char* const kSettingsJsonOption18 = "Big Pupils";
static const char* const kSettingsJsonOption19 = "Horns";
static const char* const kSettingsJsonOption20 = "Juicy Pink Lips";
static const char* const kSettingsJsonOption21 = "Lashes";
static const char* const kSettingsJsonOption22 = "Mega Eye Head";
static const char* const kSettingsJsonOption23 = "Mustache";
static const char* const kSettingsJsonOption24 = "Pig Ears";
static const char* const kSettingsJsonOption25 = "Pig Head";
static const char* const kSettingsJsonOption26 = "Sherriff Hat";
static const char* const kSettingsJsonOption27 = "Tiara";

