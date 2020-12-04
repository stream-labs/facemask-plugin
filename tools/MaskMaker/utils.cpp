/*
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
#include "utils.h"
#include <Windows.h>
#include <string>
#include <fstream>
#include "base64.h"


static const vector<string> g_defaultResources = {
	"imageNull",
	"imageWhite",
	"imageBlack",
	"imageRed",
	"imageGreen",
	"imageBlue",
	"imageYellow",
	"imageMagenta",
	"imageCyan",

	"meshTriangle",
	"meshQuad",
	"meshCube",
	"meshSphere",
	"meshCylinder",
	"meshPyramid",
	"meshTorus",
	"meshCone",
	"meshHead",

	"effectDefault",
	"effectPhong",
	"PBR"
};

namespace Utils {

	const char* GetTempPath() {
		static TCHAR tbuff[256];
		::GetTempPath(256, tbuff);
#ifdef UNICODE
		static char buff[256];
		wcstombs(buff, (wchar_t*)tbuff, wcslen((wchar_t*)tbuff) + 1);
#else
		char* buff = tbuff;
#endif
		return buff;
	}

	const char* GetTempFileName() {
		TCHAR dbuff[256];
		static TCHAR tbuff[256];
		::GetTempPath(256, dbuff);
		::GetTempFileName(dbuff, nullptr, 0, tbuff);
#ifdef UNICODE
		static char buff[256];
		wcstombs(buff, (wchar_t*)tbuff, wcslen((wchar_t*)tbuff) + 1);
#else
		char* buff = tbuff;
#endif
		return buff;
	}

	const char* Base64ToTempFile(std::string base64String) {
		std::vector<uint8_t> decoded;
		base64_decode(base64String, decoded);
		const char* fn = GetTempFileName();
		std::fstream f(fn, std::ios::out | std::ios::binary);
		f.write((char*)decoded.data(), decoded.size());
		f.close();
		return fn;
	}

	void DeleteTempFile(std::string filename) {
		::DeleteFile(filename.c_str());
	}


	// ------------------------------------------------------------------------------
	// https://stackoverflow.com/questions/236129/most-elegant-way-to-split-a-string
	//
	template<typename Out>
	void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}
	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}



	string get_extension(string filename) {
		vector<string> parts = split(filename, '.');
		string ext = parts[parts.size() - 1];
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext;
	}

	string get_filename(string filename) {
		vector<string> parts = split(filename, '\\');
		string f = parts[parts.size() - 1];
		parts = split(f, '.');
		return parts[0];
	}

	string get_filename_ext(string filename) {
		vector<string> parts = split(filename, '\\');
		return parts[parts.size() - 1];
	}

	string get_dirname(string filename) {
		vector<string> parts = split(filename, '\\');
		string d;
		for (int i = 0; i < (parts.size() - 1); i++)
			d += parts[i] + "\\";
		return d;
	}

	string get_resource_type(string filename) {
		string ext = get_extension(filename);

		if (ext == "png")
			return "image";
		if (ext == "jpg")
			return "image";
		if (ext == "obj")
			return "mesh";
		if (ext == "wav")
			return "sound";
		if (ext == "aiff")
			return "sound";
		if (ext == "mp3")
			return "sound";
		if (ext == "effect")
			return "effect";
		return "binary";
	}


	string find_resource(const json& j, string type) {
		for (auto it = j["resources"].begin(); it != j["resources"].end(); it++) {
			if (it.value()["type"] == type) {
				return it.key();
			}
		}
		return "";
	}

	bool is_default_resource(string name) {
		bool is_default = (find(g_defaultResources.begin(), g_defaultResources.end(), name) != g_defaultResources.end());

		string templates_path = "templates.json";

		// read resource file
		fstream ff(templates_path, ios::in);
		if (!ff.good()) {
			ff.open(templates_path, ios::in);
			if (!ff.good()) {
				return is_default;
			}
		}
		json j;
		ff >> j;
		ff.close();

		// Check IBLs in templates as well
		json ibls = j["ibl"];

		for (auto it = ibls.begin(); it != ibls.end(); it++)
		{
			auto val = it.value();
			is_default = is_default || (name == val["specular"]);
			is_default = is_default || (name == val["diffuse"]);
			is_default = is_default || (name == val["brdf"]);
		}

		return is_default;
	}

	bool resource_exists(const json& j, string name, string type) {
		if (j["resources"].find(name) != j["resources"].end()) {
			if (j["resources"][name]["type"] == type) {
				return true;
			}
		}
		return is_default_resource(name);
	}

	bool resource_exists(const json& j, string name) {
		if (j["resources"].find(name) != j["resources"].end()) {
			return true;
		}
		return is_default_resource(name);
	}

	bool part_exists(const json& j, string name) {
		if (j["parts"].find(name) != j["parts"].end()) {
			return true;
		}
		return false;
	}

	void add_no_dupe(const string& s, vector<string>& a) {
		if (std::find(a.begin(), a.end(), s) == a.end())
			a.push_back(s);
	}

	string join(const vector<string>& a, string j) {
		string r;
		for (int i = 0; i < a.size(); i++) {
			if (i > 0)
				r += j;
			r += a[i];
		}
		return r;
	}


}
