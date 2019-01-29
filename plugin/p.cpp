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

#include <windows.h>

#ifndef __DLL_TEST_H
#define __DLL_TEST_H

typedef bool(*obs_module_load2)(void);
typedef void(*obs_module_unload2)(void);
typedef const char* (*obs_module_name2)();
typedef const char* (*obs_module_description2)();
typedef void(*obs_module_set_pointer2)(obs_module_t *module);
typedef uint32_t(*obs_module_ver2)(void);
typedef obs_module_t *  (*obs_current_module2)(void);
typedef const char *  (*obs_module_text2)(const char *val);
typedef bool(*obs_module_get_string2)(const char *val, const char **out);
typedef void(*obs_module_set_locale2)(const char *locale);
typedef void(*obs_module_free_locale2)(void);
typedef bool(  *obs_module_load1)(void);
typedef void(  *obs_module_unload1)(void);
typedef const char* (  *obs_module_name1)();
typedef const char* (  *obs_module_description1)();
typedef void(  *obs_module_set_pointer1)(obs_module_t *module);
typedef uint32_t(  *obs_module_ver1)(void);
typedef obs_module_t *(  *obs_current_module1)(void);
typedef const char *(  *obs_module_text1)(const char *val);
typedef bool(  *obs_module_get_string1)(const char *val, const char **out);
typedef void(  *obs_module_set_locale1)(const char *locale);
typedef void(  *obs_module_free_locale1)(void);
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

HINSTANCE hGetProcIDDLL = NULL;
bool is_avx() {


	if (hGetProcIDDLL) {
		return TRUE;
	}

	int err = GetLastError();
	std::cout << err + "a" << std::endl;
	hGetProcIDDLL = LoadLibrary(L"C:/Users/glaba/Documents/dev/facemask-plugin/build/RelWithDebInfo/gen/facemask-gen.dll");
	err = GetLastError();
	std::cout << err + "a" << std::endl;
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
	is_avx();
	obs_module_set_pointer1 funci = (obs_module_set_pointer1)GetProcAddress(hGetProcIDDLL, "obs_module_set_pointer1");
	int err = GetLastError();
	std::cout << err + "a" << std::endl;
	
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	funci(module);

}
MODULE_EXPORT uint32_t obs_module_ver(void) {
	is_avx();
	obs_module_ver1 funci = (obs_module_ver1)GetProcAddress(hGetProcIDDLL, "obs_module_ver1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci();
}
MODULE_EXPORT obs_module_t *obs_current_module(void) {
	is_avx();
	obs_current_module1 funci = (obs_current_module1)GetProcAddress(hGetProcIDDLL, "obs_current_module1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci();

}
MODULE_EXPORT const char *obs_module_text(const char *val) {
	is_avx();
	obs_module_text1 funci = (obs_module_text1)GetProcAddress(hGetProcIDDLL, "obs_module_text1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci(val);

}
MODULE_EXPORT bool obs_module_get_string(const char *val, const char **out) {
	is_avx();
	obs_module_get_string1 funci = (obs_module_get_string1)GetProcAddress(hGetProcIDDLL, "obs_module_get_string1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci(val, out);

}
MODULE_EXPORT void obs_module_set_locale(const char *locale) {
	is_avx();
	obs_module_set_locale1 funci = (obs_module_set_locale1)GetProcAddress(hGetProcIDDLL, "obs_module_set_locale1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	funci(locale);

}
MODULE_EXPORT void obs_module_free_locale(void) {
	is_avx();
	obs_module_free_locale1 funci = (obs_module_free_locale1)GetProcAddress(hGetProcIDDLL, "obs_module_free_locale1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	funci();

}



MODULE_EXPORT bool obs_module_load(void) {
	is_avx();
	obs_module_load1 funci = (obs_module_load1)GetProcAddress(hGetProcIDDLL, "obs_module_unload1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci();

}

 MODULE_EXPORT  void obs_module_unload(void) {
	 is_avx();
	 obs_module_unload1 funci = (obs_module_unload1)GetProcAddress(hGetProcIDDLL, "obs_module_unload1");
	 if (!funci) {
		 std::cout << "could not locate the function" << std::endl;
	 }

	 funci();

}

MODULE_EXPORT  const char* obs_module_name() {
	is_avx();
	obs_module_name1 funci = (obs_module_name1)GetProcAddress(hGetProcIDDLL, "obs_module_name1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci();

}

MODULE_EXPORT  const char* obs_module_description() {
	is_avx();
	obs_module_description1 funci = (obs_module_description1)GetProcAddress(hGetProcIDDLL, "obs_module_description1");
	if (!funci) {
		std::cout << "could not locate the function" << std::endl;
	}

	return funci();
}

#ifdef _WIN32
#include "windows.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
	is_avx();

	return TRUE;
}
#endif
