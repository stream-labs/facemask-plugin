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
#ifndef __SMLL_CONFIG_HPP__
#define __SMLL_CONFIG_HPP__

#pragma once

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )

#include <libobs/obs-module.h>
#include <libobs/obs-data.h>

#pragma warning( pop )


#include <mutex>
#include <map>
#include <vector>


// param types
#define PARAM_TYPE_UNDEFINED	(-1)
#define PARAM_TYPE_INT			(0)
#define PARAM_TYPE_BOOL			(1) 
#define PARAM_TYPE_DOUBLE		(2)

// Some macros to keep things clean below
#define CONFIG_GET(TYPE) 		{	lock();  \
TYPE v = (TYPE)obs_data_get_##TYPE(m_data, name); unlock(); return v; }
#define CONFIG_SET(TYPE,VALUE)	{	lock();  \
obs_data_set_##TYPE(m_data, name, VALUE); unlock(); }


namespace smll {

	// Toggle Flag
	static const char* const CONFIG_BOOL_TOGGLE_SETTINGS = 
		"toggleFDSettings";

	// Smoothing factor
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR1 = "smoothingFactor1";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR2 = "smoothingFactor2";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR3 = "smoothingFactor3";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR4 = "smoothingFactor4";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR5 = "smoothingFactor5";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR6 = "smoothingFactor6";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR7 = "smoothingFactor7";

	static const char* const CONFIG_FLOAT_DIFF_FACTOR1 = "diffFactor1";
	static const char* const CONFIG_FLOAT_DIFF_FACTOR2 = "diffFactor2";
	static const char* const CONFIG_FLOAT_DIFF_FACTOR3 = "diffFactor3";
	static const char* const CONFIG_FLOAT_DIFF_FACTOR4 = "diffFactor4";
	static const char* const CONFIG_FLOAT_DIFF_FACTOR5 = "diffFactor5";
	static const char* const CONFIG_FLOAT_DIFF_FACTOR6 = "diffFactor6";
	static const char* const CONFIG_FLOAT_DIFF_FACTOR7 = "diffFactor7";
	static const char* const CONFIG_DLIB = "DLIB";
	static const char* const CONFIG_OF = "OF";
	// Face Detection Vars ----------

	// Scale Width
	static const char* const CONFIG_INT_FACE_DETECT_WIDTH = 
		"faceDetectWidth";

	// Cropping
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH =
		"faceDetectCropWidth";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT =
		"faceDetectCropHeight";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_X = 
		"faceDetectCropX";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_Y = 
		"faceDetectCropY";

	// Execution Frequencies
	static const char* const CONFIG_INT_FACE_DETECT_FREQUENCY = 
		"faceDetectFrequency";
	static const char* const CONFIG_INT_FACE_DETECT_RECHECK_FREQUENCY = 
		"faceDetectRecheckFrequency";

	// Tracking Vars ----------

	// NOTE: tracking now uses same image dimensions as face
	//       detection.

	// Execution Frequency
	static const char* const CONFIG_INT_TRACKING_FREQUNCY =
		"trackingFrequency";

	// Threshold
	static const char* const CONFIG_DOUBLE_TRACKING_THRESHOLD =
		"trackingThreshold";

	// Speed Limit
	static const char* const CONFIG_INT_SPEED_LIMIT = 
		"detectSpeedLimit";

	// Kalman filtering
	static const char* const CONFIG_BOOL_KALMAN_ENABLE =
		"kalmanFilteringEnable";

	class Config
	{
	public:
		Config();
		~Config();

		static Config& singleton();

		// Helper Get methods
		inline bool			get_bool(const char* name) 
			CONFIG_GET(bool);
		inline int			get_int(const char* name) 
			CONFIG_GET(int);
		inline double		get_double(const char* name) 
			CONFIG_GET(double);

		// Helper Set methods
		inline void			set_bool(const char* name, bool v)
			CONFIG_SET(bool,v);
		inline void			set_int(const char* name, int v)
			CONFIG_SET(int,v);
		inline void			set_double(const char* name, double v)
			CONFIG_SET(double,v);

		// manual access
		inline obs_data_t*		lock() { m_mutex.lock(); return m_data; }
		inline void				unlock() { m_mutex.unlock(); }

		// stuff
		void				set_defaults(obs_data_t* data);
		void				get_properties(obs_properties_t* props);
		void				update_properties(obs_data_t* data);

	private:

		struct ParamInfo
		{
			const char* name;
			double		defaultValue;
			double		min;
			double		max;
			double		step;
			int			type;
		};

		void AddParam(const char* name, bool defaultValue);
		void AddParam(const char* name, int defaultValue, int min, int max, int step);
		void AddParam(const char* name, double defaultValue, double min, double max, double step);

		std::vector<std::string> m_paramNames; //  need this to preserve order
		std::map<std::string, ParamInfo> m_params;

		std::mutex		m_mutex; // ensure thread safety
		obs_data_t*		m_data;  // all vars stored here

		std::vector<std::string> m_hiddenParams; // hide these from UI
	};



} // smll namespace

#endif // __SMLL_CONFIG_HPP__
