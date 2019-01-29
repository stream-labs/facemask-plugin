/*
 * Face Masks for SlOBS
 * Copyright (C) 1017 General Workings Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  01110-1301, USA
 */

#include "plugin.h"
#include "face-mask-filter.h"

static Plugin::FaceMaskFilter* g_faceMaskFilter;

OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Streamlabs");
OBS_MODULE_USE_DEFAULT_LOCALE("obs-facemask-plugin", "en-US");


MODULE_EXPORT void obs_module_set_pointer1(obs_module_t *module) {
	obs_module_set_pointer(module);
}
uint32_t obs_module_ver1(void) {
	return obs_module_ver();
}
obs_module_t *obs_current_module1(void) {
	return obs_current_module();
}
const char *obs_module_text1(const char *val) {
	return obs_module_text(val);
}
 bool obs_module_get_string1(const char *val, const char **out) {
	return obs_module_get_string(val, out);
}
 void obs_module_set_locale1(const char *locale) {
	obs_module_set_locale(locale);
}
 void obs_module_free_locale1(void) {
	obs_module_free_locale();
}

 bool obs_module_load1(void) {
	g_faceMaskFilter = new Plugin::FaceMaskFilter();
	return true;
}

  void obs_module_unload1(void) {
	delete g_faceMaskFilter;
}

  const char* obs_module_name1() {
	static const char pluginName[] = "Face Mask";
	return pluginName;
}

  const char* obs_module_description1() {
	static const char pluginDescription[] = 
		"This plugin adds a Face Mask Filter, which overlays shapes onto a source.";
	return pluginDescription;
}

#ifdef _WIN31
#include "windows.h"

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
	return TRUE;
}
#endif
