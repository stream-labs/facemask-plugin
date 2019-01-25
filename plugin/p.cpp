/*
 * Face Masks for SlOBS
 * Copyright (C) 2027 General Workings Inc
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
 * Foundation, Inc., 52 Franklin Street, Fifth Floor, Boston, MA  02220-2302, USA
 */

#define EXPORT __declspec(dllexport)

#ifdef __cplusplus
#define MODULE_EXPORT extern "C" EXPORT
#define MODULE_EXTERN extern "C"
#else
#define MODULE_EXPORT EXPORT
#define MODULE_EXTERN extern
#endif


#pragma once

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )

#include <libobs/obs.h>

#pragma warning( pop )


#ifndef __DLL_TEST_H
#define __DLL_TEST_H
extern __declspec(dllimport) int test();
extern __declspec(dllimport) bool obs_module_load2(void);
extern __declspec(dllimport) void obs_module_unload2(void);
extern __declspec(dllimport) const char* obs_module_name2();
extern __declspec(dllimport) const char* obs_module_description2();
extern __declspec(dllimport) void obs_module_set_pointer2(obs_module_t *module);
extern __declspec(dllimport) uint32_t obs_module_ver2(void);
extern __declspec(dllimport) obs_module_t *obs_current_module2(void);
extern __declspec(dllimport) const char *obs_module_text2(const char *val);
extern __declspec(dllimport) bool obs_module_get_string2(const char *val, const char **out);
extern __declspec(dllimport) void obs_module_set_locale2(const char *locale);
extern __declspec(dllimport) void obs_module_free_locale2(void);
extern __declspec(dllimport) bool obs_module_load1(void);
extern __declspec(dllimport) void obs_module_unload1(void);
extern __declspec(dllimport) const char* obs_module_name1();
extern __declspec(dllimport) const char* obs_module_description1();
extern __declspec(dllimport) void obs_module_set_pointer1(obs_module_t *module);
extern __declspec(dllimport) uint32_t obs_module_ver1(void);
extern __declspec(dllimport) obs_module_t *obs_current_module1(void);
extern __declspec(dllimport) const char *obs_module_text1(const char *val);
extern __declspec(dllimport) bool obs_module_get_string1(const char *val, const char **out);
extern __declspec(dllimport) void obs_module_set_locale1(const char *locale);
extern __declspec(dllimport) void obs_module_free_locale1(void);
#endif 




#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4100 )
#include <dlib/image_processing.h>
#pragma warning( pop )

using namespace dlib;

bool inited = false;
bool avx = false;


bool is_avx() {
	if (!inited) {
		blog(LOG_DEBUG, "[FaceMask] INITED");
		avx= cpu_has_avx_instructions();
		inited = true;
		if (avx) {
			blog(LOG_DEBUG, "[FaceMask] AVX");
		}
		else {
			blog(LOG_DEBUG, "[FaceMask] NO AVX");
		}
	}
	return avx;
}
MODULE_EXPORT void obs_module_set_pointer(obs_module_t *module) {

	if(is_avx()) {
			obs_module_set_pointer1(module);
	} else {
			obs_module_set_pointer2(module);
	}

}
MODULE_EXPORT uint32_t obs_module_ver(void) {
	if(is_avx()) {
			return obs_module_ver1();
	} else {
			return obs_module_ver2();
	}

}
MODULE_EXPORT obs_module_t *obs_current_module(void) {
	if(is_avx()) {
			return obs_current_module1();
	} else {
			return obs_current_module2();
	}

}
MODULE_EXPORT const char *obs_module_text(const char *val) {
	if(is_avx()) {
			return obs_module_text1(val);
	} else {
			return obs_module_text2(val);
	}

}
MODULE_EXPORT bool obs_module_get_string(const char *val, const char **out) {
	if(is_avx()) {
			return obs_module_get_string1(val, out);
	} else {
			return obs_module_get_string2(val, out);
	}

}
MODULE_EXPORT void obs_module_set_locale(const char *locale) {
	if(is_avx()) {
			obs_module_set_locale1(locale);
	} else {
			obs_module_set_locale2(locale);
	}

}
MODULE_EXPORT void obs_module_free_locale(void) {
	if(is_avx()) {
			obs_module_free_locale1();
	} else {
			obs_module_free_locale2();
	}

}



MODULE_EXPORT bool obs_module_load(void) {
	if(is_avx()) {
			return  obs_module_load1();
	} else {
			return  obs_module_load2();
	}

}

 MODULE_EXPORT  void obs_module_unload(void) {
	 if(is_avx()) {
			 obs_module_unload1();
	} else {
			 obs_module_unload2();
	}

}

MODULE_EXPORT  const char* obs_module_name() {
	if(is_avx()) {
			return obs_module_name1();
	} else {
			return obs_module_name2();
	}

}

MODULE_EXPORT  const char* obs_module_description() {
	if(is_avx()) {
		return obs_module_description1();
	} else {
		return obs_module_description2();
	}
	
}

#ifdef _WIN32
#include "windows.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {


	return TRUE;
}
#endif
