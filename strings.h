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

static const char* const kFileDefaultJson = "masks/glasses_Slabs_01.head.json";
static const char* const kFileShapePredictor =
	"shape_predictor_68_face_landmarks.dat";

// I know, this is gross, but temporary 
static const char* const kFileJsonOption1 = kFileDefaultJson;
static const char* const kFileJsonOption2 = "masks/arrow_02.head.json";
static const char* const kFileJsonOption3 = "masks/bunnyMask_02_teeth.json";
static const char* const kFileJsonOption4 = "masks/crown_typeA_env_sparkle.head.json";
static const char* const kFileJsonOption5 = "masks/crown_typeB_01.head.json";
static const char* const kFileJsonOption6 = "masks/crown_typeC.head.json";
static const char* const kFileJsonOption7 = "masks/deerAntlers_white_1.json";
static const char* const kFileJsonOption8 = "masks/deerEars_1.json";
static const char* const kFileJsonOption9 = "masks/deerMask_whiteAntlers_1.json";
static const char* const kFileJsonOption10 = "masks/duckBill_01.json";
static const char* const kFileJsonOption11 = "masks/fatty_01_emit.json";
static const char* const kFileJsonOption12 = "masks/flowerCrown_3.head.json";
static const char* const kFileJsonOption13 = "masks/FruitHat_1.head.json";
static const char* const kFileJsonOption14 = "masks/glassesRainbow_01.head.json";
static const char* const kFileJsonOption15 = "masks/glasses_Lashes_anim_01.head.json";
static const char* const kFileJsonOption16 = "masks/glasses_superStar_01_rasta.head.json";
static const char* const kFileJsonOption17 = "masks/glasses_typeC_dollar_anim_01.head.json";
static const char* const kFileJsonOption18 = "masks/horns_typeA_bwS_fire.head.json";
static const char* const kFileJsonOption19 = "masks/Hypno_eyes_1.json";
static const char* const kFileJsonOption20 = "masks/mask_typeA_sa_textures.json";
static const char* const kFileJsonOption21 = "masks/megaEye_head_1.json";
static const char* const kFileJsonOption22 = "masks/mustache_typeA.json";
static const char* const kFileJsonOption23 = "masks/pooHat_01_anim_01_emit.head.json";
static const char* const kFileJsonOption24 = "masks/rainbowVomit_1.json";
static const char* const kFileJsonOption25 = "masks/toastHead_hot.head.json";
static const char* const kFileJsonOption26 = "masks/unicorn_horn_1.json";
static const char* const kFileJsonOption27 = "masks/wigManga_01_green.head.json";

static const char* const kSettingsJsonOption1 = "Streamlabs";
static const char* const kSettingsJsonOption2 = "Arrow";
static const char* const kSettingsJsonOption3 = "Bunny";
static const char* const kSettingsJsonOption4 = "Crown 1";
static const char* const kSettingsJsonOption5 = "Crown 2";
static const char* const kSettingsJsonOption6 = "Crown 3";
static const char* const kSettingsJsonOption7 = "Deer Antlers";
static const char* const kSettingsJsonOption8 = "Deer Ears";
static const char* const kSettingsJsonOption9 = "Deer Mask";
static const char* const kSettingsJsonOption10 = "Duck Bill";
static const char* const kSettingsJsonOption11 = "Fatty";
static const char* const kSettingsJsonOption12 = "Flower Crown";
static const char* const kSettingsJsonOption13 = "Fruit Hat";
static const char* const kSettingsJsonOption14 = "Rainbow Glasses";
static const char* const kSettingsJsonOption15 = "Lash Glasses";
static const char* const kSettingsJsonOption16 = "Superstar Glasses";
static const char* const kSettingsJsonOption17 = "Dollah Glasses";
static const char* const kSettingsJsonOption18 = "Fire Horns";
static const char* const kSettingsJsonOption19 = "Hypno Eyes";
static const char* const kSettingsJsonOption20 = "Wrestler";
static const char* const kSettingsJsonOption21 = "Mega Eye";
static const char* const kSettingsJsonOption22 = "Moustache";
static const char* const kSettingsJsonOption23 = "Splat Hat";
static const char* const kSettingsJsonOption24 = "Rainbows";
static const char* const kSettingsJsonOption25 = "Toasted";
static const char* const kSettingsJsonOption26 = "Unicorn";
static const char* const kSettingsJsonOption27 = "Manga Hair";

static const char* const kSettingsPerformanceOption1 = "CPU Heavy";
static const char* const kSettingsPerformanceOption2 = "Moderate GPU (Recommended)";
static const char* const kSettingsPerformanceOption3 = "More GPU";
static const char* const kSettingsPerformanceOption4 = "GPU Heavy";



