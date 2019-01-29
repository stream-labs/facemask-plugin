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
#include "p2.h"
#include "face-mask-filter.h"

static Plugin::FaceMaskFilter* g_faceMaskFilter;

OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Streamlabs");
OBS_MODULE_USE_DEFAULT_LOCALE("obs-facemask-plugin", "en-US");


__declspec(dllexport)  void obs_module_set_pointer2(obs_module_t *module) {
	obs_module_set_pointer(module);
}
__declspec(dllexport)  uint32_t obs_module_ver2(void) {
	return obs_module_ver();
}
__declspec(dllexport) obs_module_t *obs_current_module2(void) {
	return obs_current_module();
}
__declspec(dllexport) const char *obs_module_text2(const char *val) {
	return obs_module_text(val);
}
__declspec(dllexport) bool obs_module_get_string2(const char *val, const char **out) {
	return obs_module_get_string(val, out);
}
__declspec(dllexport) void obs_module_set_locale2(const char *locale) {
	obs_module_set_locale(locale);
}
__declspec(dllexport) void obs_module_free_locale2(void) {
	obs_module_free_locale();
}

__declspec(dllexport) bool obs_module_load2(void) {
	g_faceMaskFilter = new Plugin::FaceMaskFilter();
	return true;
}

__declspec(dllexport)  void obs_module_unload2(void) {
	delete g_faceMaskFilter;
}

__declspec(dllexport)  const char* obs_module_name2() {
	static const char pluginName[] = "Face Mask";
	return pluginName;
}

__declspec(dllexport)  const char* obs_module_description2() {
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
