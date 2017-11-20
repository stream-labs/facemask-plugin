/*
 * Face Masks for SlOBS
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
#include "version.h"
#include <stdint.h>
#include <inttypes.h>
#include <libobs/util/base.h>

 // Plugin

#define PLUGIN_NAME				"Face Mask"
#define PLOG(level, ...)		blog(level, "[" PLUGIN_NAME "] " __VA_ARGS__);
#define PLOG_ERROR(...)			PLOG(LOG_ERROR,   __VA_ARGS__)
#define PLOG_WARNING(...)		PLOG(LOG_WARNING, __VA_ARGS__)
#define PLOG_INFO(...)			PLOG(LOG_INFO,    __VA_ARGS__)
#define PLOG_DEBUG(...)			PLOG(LOG_DEBUG,   __VA_ARGS__)

// Utility
#define vstr(s) dstr(s)
#define dstr(s) #s

namespace Plugin {
	template <typename T>
	static T clamp(T val, T low, T high) {
		return (val < low ? low : (val > high ? high : val));
	}

	template <typename T>
	static T max(T val, T high) {
		return (val > high ? val : high);
	}

	template <typename T>
	static T min(T val, T low) {
		return (val < low ? val : low);
	}

	template <typename T>
	static T wrap(T val, T low, T high) {
		if (low > high) { // Swap if high is smaller than low.
			std::swap(low, high);
		}
		T size = high - low;
		return (val - (size * floor((val - low) / size)));
	}
}

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64)   //WINDOWS
#define __FUNCTION_NAME__   __FUNCTION__  
#else          //*NIX
#define __FUNCTION_NAME__   __func__ 
#endif
#endif
