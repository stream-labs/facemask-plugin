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

#include <windows.h>

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


#define FACEMASK_AVX		(L"facemask_AVX.dll")
#define FACEMASK_NO_AVX		(L"facemask_NO_AVX.dll")

using namespace dlib;

typedef bool(*obs_module_load_fcn)(void);
typedef void(*obs_module_unload_fcn)(void);
typedef const char* (*obs_module_name_fcn)();
typedef const char* (*obs_module_description_fcn)();
typedef void(*obs_module_set_pointer_fcn)(obs_module_t *module);
typedef uint32_t(*obs_module_ver_fcn)(void);
typedef obs_module_t *(*obs_current_module_fcn)(void);
typedef const char *(*obs_module_text_fcn)(const char *val);
typedef bool(*obs_module_get_string_fcn)(const char *val, const char **out);
typedef void(*obs_module_set_locale_fcn)(const char *locale);
typedef void(*obs_module_free_locale_fcn)(void);


static bool loaded = false;
static bool avx = false;
static HINSTANCE hGetProcIDDLL = NULL;

bool is_avx() {

	avx= cpu_has_avx_instructions();

	if (avx) {
		blog(LOG_DEBUG, "[FaceMask] AVX");
	}
	else {
		blog(LOG_DEBUG, "[FaceMask] NO AVX");
	}
	return avx;
}


void load_dll() {
	if (loaded) {
		return;
	}

	loaded = true;
	blog(LOG_DEBUG, "[FaceMask]  INITED");
	LPCWSTR dllName = is_avx() ? FACEMASK_AVX : FACEMASK_NO_AVX;
	hGetProcIDDLL = LoadLibrary(dllName);

	if (!hGetProcIDDLL) {
		int err = GetLastError();
		blog(LOG_DEBUG, "[FaceMask] DLL can not loaded. error code: %d", err);
	}

}

MODULE_EXPORT void obs_module_set_pointer(obs_module_t *module) {
	load_dll();
	obs_module_set_pointer_fcn fcn = (obs_module_set_pointer_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_set_pointer");

	if (!fcn) {
		return;
	}

	fcn(module);
}

MODULE_EXPORT uint32_t obs_module_ver(void) {
	load_dll();
	obs_module_ver_fcn fcn = (obs_module_ver_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_ver");

	if (!fcn) {
		return 0;
	}

	return fcn();
}

MODULE_EXPORT obs_module_t *obs_current_module(void) {
	load_dll();
	obs_current_module_fcn fcn = (obs_current_module_fcn)GetProcAddress(hGetProcIDDLL, "obs_current_module");

	if (!fcn) {
		return NULL;
	}

	return fcn();
}

MODULE_EXPORT const char *obs_module_text(const char *val) {
	load_dll();
	obs_module_text_fcn fcn = (obs_module_text_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_text");

	if (!fcn) {
		return NULL;
	}

	return fcn(val);
}

MODULE_EXPORT bool obs_module_get_string(const char *val, const char **out) {
	load_dll();
	obs_module_get_string_fcn fcn = (obs_module_get_string_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_get_string");

	if (!fcn) {
		return FALSE;
	}

	return fcn(val, out);
}
MODULE_EXPORT void obs_module_set_locale(const char *locale) {
	load_dll();
	obs_module_set_locale_fcn fcn = (obs_module_set_locale_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_set_locale");

	if (!fcn) {
		return;
	}

	fcn(locale);
}
MODULE_EXPORT void obs_module_free_locale(void) {
	load_dll();
	obs_module_free_locale_fcn fcn = (obs_module_free_locale_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_free_locale");

	if (!fcn) {
		return;
	}

	fcn();
}



MODULE_EXPORT bool obs_module_load(void) {
	load_dll();
	obs_module_load_fcn fcn = (obs_module_load_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_load_inner");

	if (!fcn) {
		return FALSE;
	}

	return fcn();
}

MODULE_EXPORT  void obs_module_unload(void) {
	load_dll();
	obs_module_unload_fcn fcn = (obs_module_unload_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_unload");

	if (!fcn) {
		return;
	}

	fcn();
}

MODULE_EXPORT  const char* obs_module_name() {
	load_dll();
	obs_module_name_fcn fcn = (obs_module_name_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_name");

	if (!fcn) {
		return NULL;
	}

	return fcn();
}

MODULE_EXPORT  const char* obs_module_description() {
	load_dll();
	obs_module_description_fcn fcn = (obs_module_description_fcn)GetProcAddress(hGetProcIDDLL, "obs_module_description");

	if (!fcn) {
		return NULL;
	}

	return fcn();
}

#ifdef _WIN32
#include "windows.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
	load_dll();
	return TRUE;
}
#endif
