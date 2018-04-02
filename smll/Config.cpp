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
#include "Config.hpp"

#include <opencv2/opencv.hpp>

namespace smll {

	// Static strings for config setting descriptions
	static const char* const CONFIG_BOOL_TOGGLE_SETTINGS_DESC =
		"Show Advanced Settings";
	static const char* const CONFIG_FLOAT_SMOOTHING_FACTOR_DESC =
		"Smoothing Factor";
	static const char* const CONFIG_INT_FACE_DETECT_WIDTH_DESC = 
		"Face Detect Frame Width";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH_DESC =
		"Face Detect Crop Width";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT_DESC =
		"Face Detect Crop Height";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_X_DESC =
		"Face Detect Crop X";
	static const char* const CONFIG_DOUBLE_FACE_DETECT_CROP_Y_DESC =
		"Face Detect Crop Y";
	static const char* const CONFIG_INT_FACE_DETECT_FREQUENCY_DESC =
		"Face Detection Frequency";
	static const char* const CONFIG_INT_FACE_DETECT_RECHECK_FREQUENCY_DESC =
		"Face Detection Recheck Frequency";
	static const char* const CONFIG_INT_TRACKING_FREQUNCY_DESC = 
		"Tracking Frequency";
	static const char* const CONFIG_DOUBLE_TRACKING_THRESHOLD_DESC =
		"Tracking Confidence Threshold";
	static const char* const CONFIG_BOOL_MAKE_CAPTURE_COPY_DESC =
		"Make copy of capture texture";
	static const char* const CONFIG_BOOL_MAKE_DETECT_COPY_DESC = 
		"Make copy of detection texture";
	static const char* const CONFIG_BOOL_MAKE_TRACK_COPY_DESC =
		"Make copy of tracking texture";
	static const char* const CONFIG_BOOL_KALMAN_ENABLE_DESC =
		"Enable Kalman Filtering";
	static const char* const CONFIG_INT_SPEED_LIMIT_DESC =
		"Face Detection Speed Limit (in ms)";

	static const char* const CONFIG_BOOL_IN_TEST_MODE_DESC =
		"Enable Testing Mode";

	// show our "advanced" settings
	static bool g_showSettings = false;
    
	bool onSettingsToggle(obs_properties_t *props, obs_property_t *property, 
		obs_data_t *settings) {
		UNUSED_PARAMETER(props);
		UNUSED_PARAMETER(property);
		UNUSED_PARAMETER(settings);

		g_showSettings = obs_data_get_bool(settings,
			CONFIG_BOOL_TOGGLE_SETTINGS);

		return true;
	}

	static Config g_config;

	Config::Config()
        : m_data(nullptr) {
		//
		// ---- Add All Parameters Here ----
		//
		AddParam(CONFIG_BOOL_TOGGLE_SETTINGS, 
			CONFIG_BOOL_TOGGLE_SETTINGS_DESC, false);

		AddParam(CONFIG_BOOL_KALMAN_ENABLE,
			CONFIG_BOOL_KALMAN_ENABLE_DESC, true);
		AddParam(CONFIG_FLOAT_SMOOTHING_FACTOR,
			CONFIG_FLOAT_SMOOTHING_FACTOR_DESC, 1.0, 0.0, 10.0, 0.1);

		AddParam(CONFIG_INT_FACE_DETECT_WIDTH, 
			CONFIG_INT_FACE_DETECT_WIDTH_DESC, 480, 120, 1200, 20);

		AddParam(CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH, 
			CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH_DESC, 0.6, 0.0, 1.0, 0.01);
		AddParam(CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT, 
			CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT_DESC, 0.8, 0.0, 1.0, 0.01);
		AddParam(CONFIG_DOUBLE_FACE_DETECT_CROP_X, 
			CONFIG_DOUBLE_FACE_DETECT_CROP_X_DESC, 0.0, -1.0, 1.0, 0.01);
		AddParam(CONFIG_DOUBLE_FACE_DETECT_CROP_Y, 
			CONFIG_DOUBLE_FACE_DETECT_CROP_Y_DESC, 0.0, -1.0, 1.0, 0.01);

		AddParam(CONFIG_INT_FACE_DETECT_FREQUENCY, 
			CONFIG_INT_FACE_DETECT_FREQUENCY_DESC, 5, 0, 600, 1);
		AddParam(CONFIG_INT_FACE_DETECT_RECHECK_FREQUENCY,
			CONFIG_INT_FACE_DETECT_RECHECK_FREQUENCY_DESC, 30, 0, 600, 1);

		AddParam(CONFIG_INT_TRACKING_FREQUNCY, 
			CONFIG_INT_TRACKING_FREQUNCY_DESC, 3, 0, 600, 1);

		AddParam(CONFIG_DOUBLE_TRACKING_THRESHOLD,
			CONFIG_DOUBLE_TRACKING_THRESHOLD_DESC, 8.0, 1.0, 100.0, 1.0);

		AddParam(CONFIG_INT_SPEED_LIMIT,
			CONFIG_INT_SPEED_LIMIT_DESC, 24, 0, 33 * 16, 1);

		AddParam(CONFIG_BOOL_IN_TEST_MODE, CONFIG_BOOL_IN_TEST_MODE_DESC, false);

		m_data = obs_data_create();
		set_defaults(m_data);
	}

	Config::~Config() {
		obs_data_release(m_data);
	}

	Config& Config::singleton()	{
		return g_config;
	}

	void Config::set_defaults(obs_data_t* data)	{
		for (std::map<std::string,ParamInfo>::iterator it = m_params.begin(); 
			it != m_params.end(); it++)	{
			switch (it->second.type) {
			case PARAM_TYPE_BOOL:
				obs_data_set_default_bool(data, it->first.c_str(), 
					m_params[it->first].defaultValue != 0.0);
				break;
			case PARAM_TYPE_INT:
				obs_data_set_default_int(data, it->first.c_str(), 
					(int)m_params[it->first].defaultValue);
				break;
			case PARAM_TYPE_DOUBLE:
				obs_data_set_default_double(data, it->first.c_str(),
					m_params[it->first].defaultValue);
				break;
			default:
				break;
			}
		}
	}

	void Config::get_properties(obs_properties_t* props) {
		for (auto it = m_paramNames.begin(); it != m_paramNames.end(); it++) {
			// skip hidden params
			if (std::find(m_hiddenParams.begin(), 
				m_hiddenParams.end(), *it) != m_hiddenParams.end())	{
				continue;
			}

			ParamInfo& pinfo = m_params[*it];

			switch (pinfo.type)	{
			case PARAM_TYPE_BOOL: {
				obs_property_t* p = obs_properties_add_bool(props, 
					it->c_str(), pinfo.description);
				if (*it == CONFIG_BOOL_TOGGLE_SETTINGS) {
					obs_property_set_modified_callback(p, onSettingsToggle);
					if (!g_showSettings)
						return;
				}
				break; 
			}
			case PARAM_TYPE_INT:
				obs_properties_add_int_slider(props, it->c_str(), 
					pinfo.description, (int)pinfo.min, (int)pinfo.max, 
					(int)pinfo.step);
				break;
			case PARAM_TYPE_DOUBLE:
				obs_properties_add_float_slider(props, it->c_str(),
					pinfo.description, pinfo.min, pinfo.max, pinfo.step);
				break;
			default:
				break;
			}
		}
	}

	void Config::update_properties(obs_data_t* data) {
		// - it would be nice to just do this, however, libobs makes no 
		//   guarantees on the validity of these values, such as they should 
		//   be between min/max and lie on a step. 
		// - So, unfortunately, we need to validate these values
		obs_data_apply(m_data, data);

		g_showSettings = get_bool(CONFIG_BOOL_TOGGLE_SETTINGS);

		// iterate params, and clamp int/double to their min/max
		//
		for (auto it = m_params.begin(); it != m_params.end(); it++) {
			if (it->second.type == PARAM_TYPE_INT) {
				int v = get_int(it->first.c_str());
				v = std::max<int>((int)it->second.min, v);
				v = std::min<int>((int)it->second.max, v);
				set_int(it->first.c_str(), v);
			}
			else if (it->second.type == PARAM_TYPE_DOUBLE) {
				double v = get_double(it->first.c_str());
				v = std::max<double>(it->second.min, v);
				v = std::min<double>(it->second.max, v);
				set_double(it->first.c_str(), v);
			}
		}
	}


	void Config::AddParam(const char* name, const char* description, 
		bool defaultValue)
	{
		AddParam(name, description, (double)defaultValue, 0.0, 0.0, 0.0);
		m_params[name].type = PARAM_TYPE_BOOL;
	}

	void Config::AddParam(const char* name, const char* description, 
		int defaultValue, int min, int max, int step)
	{
		AddParam(name, description, (double)defaultValue, (double)min, 
			(double)max, (double)step);
		m_params[name].type = PARAM_TYPE_INT;
	}

	void Config::AddParam(const char* name, const char* description, 
		double defaultValue, double min, double max, double step)
	{
		ParamInfo info;
		info.name = name;
		info.description = description;
		info.defaultValue = defaultValue;
		info.min = min;
		info.max = max;
		info.step = step;

		m_params[name] = info;
		m_params[name].type = PARAM_TYPE_DOUBLE;

		m_paramNames.push_back(name);
	}

} // smll namespace

