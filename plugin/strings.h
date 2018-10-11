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
#define P_OPTION(x,n)			x ".Option" n

// Properties
#define P_MASK					"Mask"
#define P_MASK_BROWSE			"Browse to mask"
#define P_MASKFOLDER			"maskFolder"
#define P_DEACTIVATE			"deactivated"
#define P_DRAWMASK				"drawmask"
#define P_DRAWALERT				"drawAlert"
#define P_DRAWFACEDATA			"drawfaces"
#define P_DRAWMORPHTRIS			"drawmorphtris"
#define P_DRAWCROPRECT			"drawFDCropRect"
#define P_DEMOMODEON			"demoMode"
#define P_DEMOFOLDER			"demoFolder"
#define P_BGREMOVAL				"greenscreen"
#define P_GENTHUMBS				"genpreviews"
#define P_REWIND				"rewindanims"
#define P_CARTOON				"cartoonMode"
#define P_ALERT_TEXT			"alertText"
#define P_ALERT_DONOR_NAME	    "donorName"
#define P_ALERT_DURATION		"alertDuration"
#define P_ALERT_INTRO			"alertIntro"
#define P_ALERT_OUTRO			"alertOutro"
#define P_ALERT_DOINTRO			"alertDoIntro"
#define P_ALERT_DOOUTRO			"alertDoOutro"
#define P_ALERT_ACTIVATE		"alertActivate"
#define P_ALERT_OFFSET_BIG		"alertOffsetWhenBig"
#define P_ALERT_OFFSET_SMALL	"alertOffsetWhenSmall"
#define P_ALERT_MIN_SIZE		"alertMinSize"
#define P_ALERT_MAX_SIZE		"alertMaxSize"
#define P_ALERT_SHOW_DELAY		"alertShowDelay"

// Other static strings
static const char* const kDefaultMask = "b41214ab-8ef1-4842-924d-be113e2b5566.json";
static const char* const kDefaultIntro = "f99e23b1-0393-4a01-b4ec-369823528635.json";
static const char* const kDefaultOutro = "f99e23b1-0393-4a01-b4ec-369823528635.json";
static const char* const kDefaultMaskFolder = "masks";
static const char* const kDefaultAlertLT = "alerts/alert_LT.json";
static const char* const kDefaultAlertLB = "alerts/alert_LB.json";
static const char* const kDefaultAlertRT = "alerts/alert_RT.json";
static const char* const kDefaultAlertRB = "alerts/alert_RB.json";
static const char* const kFontAlertTTF     = "fonts/ComicRelief.ttf";
static const char* const kBaseFontAlertTTF = "fonts/CODE2000.ttf";


