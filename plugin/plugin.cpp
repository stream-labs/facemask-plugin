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

#include "plugin.h"
#include "face-mask-filter.h"

static Plugin::FaceMaskFilter* g_faceMaskFilter;

OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Streamlabs");
OBS_MODULE_USE_DEFAULT_LOCALE("obs-facemask-plugin", "en-US");



MODULE_EXPORT bool obs_module_load(void) {
	g_faceMaskFilter = new Plugin::FaceMaskFilter();
	return true;
}


MODULE_EXPORT void obs_module_unload(void) {
	delete g_faceMaskFilter;
}

MODULE_EXPORT const char* obs_module_name() {
	static const char pluginName[] = "Face Mask";
	return pluginName;
}

MODULE_EXPORT const char* obs_module_description() {
	static const char pluginDescription[] = 
		"This plugin adds a Face Mask Filter, which overlays shapes onto a source.";
	return pluginDescription;
}

#ifdef _WIN32
#include "windows.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
	return TRUE;
}
#endif
