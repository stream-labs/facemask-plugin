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
#include "stdafx.h"
#include "args.h"

// STB : single-file public domain (or MIT licensed) libraries for C/C++ 
// https://github.com/nothings/stb
// you must define these for stb calls to work
// - enable image library and resizing library
#define STB_IMAGE_IMPLEMENTATION 
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"

// AVIR : Advanced image resizer library
// https://github.com/avaneev/avir
// (in stdafx.h)
// - used to generate nice color-corrected super-sampled mipmaps

using namespace std;

Args::Args(int argc, char** argv) 
	: jptr(nullptr), failed(true) {
	if (argc < 3) {
		usage();
		return;
	}

	// init json names
	initJsonNamesAndValues();

	command = argv[1];

	if (command != "inspect")
		filename = argv[argc - 1];

	if (command == "merge") {
		for (int i = 2; i < (argc - 1); i++) {
			string s = argv[i];
			if (s.find("=") != string::npos) {
				vector<string> pair = Utils::split(argv[i], '=');
				if (pair.size() > 1)
					kvpairs[pair[0]] = pair[1];
				else
					kvpairs[pair[0]] = "";
			}
			else {
				files.push_back(argv[i]);
			}
		}
	}
	else {
		int arg_count = (command == "inspect") ? argc : argc - 1;
		for (int i = 2; i < arg_count; i++) {
			vector<string> pair = Utils::split(argv[i], '=');
			if (pair.size() > 1)
				kvpairs[pair[0]] = pair[1];
			else
				kvpairs[pair[0]] = "";
		}
	}
	failed = false;
}

void Args::usage() {
	cout << endl;
	cout << "Mask Maker - tool for making facemask json files" << endl;
	cout << endl;
	cout << "usage:" << endl;
	cout << endl;
	cout << "  maskmaker.exe <command> [key=value ...] <file.json>" << endl;
	cout << endl;
	cout << "commands:" << endl;
	cout << endl;
	cout << "  create  -  creates a new facemask json file" << endl;
	cout << "  addres  -  adds a resource" << endl;
	cout << "  addpart -  adds a part" << endl;
	cout << "  merge   -  merges jsons" << endl;
	cout << "  import  -  imports an FBX file and creates a json" << endl;
	cout << "  morphimport,mi  -  imports morph FBX files and creates a json" << endl;
	cout << "  tweak   -  tweak (set) values in the json." << endl;
	cout << endl;
	cout << "example:" << endl;
	cout << endl;
	cout << "  maskmaker.exe create author=\"Joe Blow\" description=\"My lame helmet\" helmet.json" << endl;
	cout << "  maskmaker.exe addres file=helmet.obj helmet.json" << endl;
	cout << "  maskmaker.exe addres file=helmet.png name=\"helmet image\" helmet.json" << endl;
	cout << "  maskmaker.exe addres file=phong.effect helmet.json" << endl;
	cout << "  maskmaker.exe addres type=material helmet.json" << endl;
	cout << "  maskmaker.exe addpart name=helmet helmet.json" << endl;
	cout << endl;
}

void Args::print() {
	cout << "command: " << command << endl;
	cout << "file: " << filename << endl;
	for (auto it = kvpairs.begin(); it != kvpairs.end(); it++) {
		cout << "kvpair: " << it->first << " = " << it->second << endl;
	}
}

string Args::default_value(string key) {
	if (command == "create" ||
		command == "merge" ||
		command == "morphimport" ||
		command == "import") {
		if (key == "name")
			return Utils::get_filename(filename);
		if (key == "description")
			return "Streamlabs facemask description";
		if (key == "author")
			return "Streamlabs";
		if (key == "license")
			return "Copyright 2017 - General Workings Inc. - All rights reserved.";
		if (key == "website")
			return "http://streamlabs.com/";
		if (key == "version")
			return "1";
		if (key == "tier")
			return "1";
		if (key == "texture_max")
			return "256";
		if (key == "is_intro")
			return "false";
		if (key == "intro_fade_time")
			return "0.3333";
		if (key == "intro_duration")
			return "2.1333";
		if (key == "draw_video_with_mask")
			return "false";
		if (key == "modtime") {
			long long modtime = std::chrono::duration_cast<std::chrono::seconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();
			char temp[256];
			snprintf(temp, sizeof(temp), "%lld", modtime);
			return temp;
		}
	}
	if (command == "addres" ||
		command == "morphimport" ||
		command == "import") {
		if (key == "filter")
			return "min-mag-linear-mip-point";
		//return "min-linear-mag-point-mip-linear";
		if (key == "technique")
			return "Draw";
		if (key == "texture")
			return Utils::find_resource(*jptr, "image");
		if (key == "effect")
			return Utils::find_resource(*jptr, "effect");
		if (key == "material")
			return Utils::find_resource(*jptr, "material");
		if (key == "mesh")
			return Utils::find_resource(*jptr, "mesh");
		if (key == "model")
			return Utils::find_resource(*jptr, "model");
		if (key == "mode")
			return "repeat";
		if (key == "rate")
			return "4";
		if (key == "u-wrap")
			return "clamp";
		if (key == "v-wrap")
			return "clamp";
		if (key == "w-wrap")
			return "clamp";
		if (key == "culling")
			return "back";
		if (key == "depth-test")
			return "less";
		if (key == "depth-only")
			return "false";
		if (key == "lifetime")
			return "3";
		if (key == "friction")
			return "0.9";
		if (key == "force")
			return "0,100,0";
		if (key == "initial-velocity")
			return "0,0,5000";
		if (key == "num-particles")
			return "1000";
		if (key == "random-start")
			return "false";
		if (key == "part")
			return "emitter1";
		if (key == "scale-start")
			return "1.0";
		if (key == "scale-end")
			return "1.0";
		if (key == "alpha-start")
			return "1.0";
		if (key == "alpha-end")
			return "1.0";
		if (key == "delay")
			return "0.0";
		if (key == "opaque")
			return "true";
		if (key == "world-space")
			return "true";
		if (key == "inverse-rate")
			return "false";
		if (key == "z-sort-offset")
			return "0";
		if (key == "alpha-write")
			return "true";
	}
	if (command == "addpart") {
		if (key == "parent")
			return "root";
		if (key == "position")
			return "0,0,0";
		if (key == "rotation")
			return "0,0,0";
		if (key == "scale")
			return "1,1,1";
		if (key == "type")
			return "model";
		if (key == "model")
			return Utils::find_resource(*jptr, "model");
		if (key == "sound")
			return Utils::find_resource(*jptr, "sound");
	}

	return "";
}

bool Args::haveValue(string key) {
	return kvpairs.find(key) != kvpairs.end();
}

string Args::value(string key) {
	if (kvpairs.find(key) == kvpairs.end()) {
		return default_value(key);
	}
	return kvpairs[key];
}

float Args::floatValue(string key) {
	return (float)atof(value(key).c_str());
}

int Args::intValue(string key) {
	return atoi(value(key).c_str());
}

long Args::longValue(string key) {
	return atol(value(key).c_str());
}

long long Args::longlongValue(string key) {
	return atoll(value(key).c_str());
}

bool Args::boolValue(string v) {
	return (value(v) == "true" ||
		value(v) == "True" ||
		value(v) == "TRUE" ||
		value(v) == "yes" ||
		value(v) == "Yes" ||
		value(v) == "YES" ||
		value(v) == "1");
}

json Args::makeNumberArray(string v, bool isFloat, int sidx) {
	vector<string> vals = Utils::split(v, ',');
	json vvv;
	char k[2] = "x";
	for (int i = sidx; i < vals.size(); i++) {
		if (isFloat)
			vvv[k] = (float)atof(vals[i].c_str());
		else
			vvv[k] = atoi(vals[i].c_str());
		if (k[0] == 'z')
			k[0] = 'w';
		else if (k[0] == 'w')
			k[0] = 'a';
		else
			k[0]++;
	}
	return vvv;
}

json Args::makeFloatArray(string v, int sidx) {
	return makeNumberArray(v, true, sidx);
}

json Args::makeIntArray(string v, int sidx) {
	return makeNumberArray(v, false, sidx);
}

json Args::makeMatrix(string v, int sidx) {
	// TODO : make this work properly
	return makeNumberArray(v, true, sidx);
}


json Args::getMaterialParams() {
	json params;
	vector<string> mat_params = { "name", "type", "effect", "technique",
		"u-wrap", "v-wrap", "w-wrap", "culling", "depth-test", "depth-only",
		"filter", "opaque", "alpha-write" };
	for (auto it = kvpairs.begin(); it != kvpairs.end(); it++) {
		if (std::find(mat_params.begin(), mat_params.end(), it->first) == mat_params.end()) {

			json parm;
			string parmName = it->first;
			vector<string> bits = Utils::split(it->second, ',');
			if (bits.size() < 2) {
				cout << "Malformed material parameter '" << it->first << "'. skipping." << endl;
				continue;
			}
			string parmType = bits[0];
			parm["type"] = parmType;
			if (parmType == "texture") {

				string image = bits[1];
				if (!Utils::resource_exists(*jptr, image, "image")) {
					cout << "Cannot find image '" << image << "'. skipping." << endl;
					continue;
				}

				parm["value"] = image;
			}
			else if (parmType == "sequence") {

				string sequence = bits[1];
				if (!Utils::resource_exists(*jptr, sequence, "sequence")) {
					cout << "Cannot find sequence '" << sequence << "'. skipping." << endl;
					continue;
				}

				parm["value"] = sequence;
			}
			else if (parmType.find("float") != string::npos) {
				parm["value"] = makeFloatArray(it->second, 1);
			}
			else if (parmType.find("int") != string::npos) {
				parm["value"] = makeIntArray(it->second, 1);
			}
			else if (parmType == "matrix") {
				cout << "matrix param types don't quite work yet." << endl;
				parm["value"] = makeMatrix(it->second, 1);
			}

			params[parmName] = parm;
		}
	}
	return params;
}

bool Args::fileExists(string fname) {
	fstream f(fname.c_str(), ios::in);
	bool good = f.good();
	f.close();
	return good;
}

json Args::loadJsonFile(string fname) {
	// load json file
	json j;
	fstream f(fname.c_str(), ios::in);
	if (!f.good()) {
		cout << "Cannot load json file '" << fname << "'." << endl;
		return j;
	}
	f >> j;
	f.close();
	return j;
}

void Args::initJsonNamesAndValues() {
	vector<string> create_keys = {
		"name",
		"uuid",
		"tier",
		"description",
		"author",
		"tags",
		"category",
		"license",
		"website",
		"is_intro",
		"intro_fade_time",
		"intro_duration",
		"modtime",
		"draw_video_with_mask"
	};
	jsonKeyNames["create"] = create_keys;
}

json Args::createNewJson() {
	json j;

	vector<string>& jsonKeys = jsonKeyNames["create"];
	for (int i = 0; i < jsonKeys.size(); i++) {
		string k = jsonKeys[i];
		if (k == "intro_fade_time" || k == "intro_duration")
			j[k] = floatValue(k);
		else if (k == "tier")
			j[k] = intValue(k);
		else if (k == "modtime")
			j[k] = longlongValue(k);
		else if (k == "is_intro")
			j[k] = boolValue(k);
		else if (k == "draw_video_with_mask")
			j[k] = boolValue(k);
		else
			j[k] = value(k);
	}

	j["version"] = FACEMASK_JSON_VERSION;

	j["resources"] = {};
	j["parts"] = {};

	return j;
}

void Args::outputJson(const json& j) {
	std::cout << j.dump(4) << endl;
}

void Args::writeJson(const json& j) {
	writeJson(j, filename);
}

void Args::writeJson(const json& j, string toFile) {
	fstream fff(toFile.c_str(), ios::out);
	fff << j.dump(4) << endl;
	fff.close();
}

string Args::uniqueResourceName(string name, string resType) {
	int count = 1;
	char bb[128];
	while (name.length() == 0) {
		snprintf(bb, sizeof(bb), "%s%d", resType.c_str(), count);
		name = bb;
		if (Utils::resource_exists(*jptr, name, resType)) {
			count++;
			name = "";
		}
	}
	return name;
}

string Args::getFullResourcePath(string resFile) {
	string fullPath = resFile;

	// read resource file
	fstream ff(fullPath, ios::in | ios::binary);
	if (!ff.good()) {
		// try adding the source folder
		string dirname = Utils::get_dirname(value("file"));
		fullPath = dirname + resFile;
		ff.open(fullPath, ios::in | ios::binary);
		if (!ff.good()) {
			// try bare filename.ext in source folder
			fullPath = dirname + Utils::get_filename_ext(resFile);
			ff.open(fullPath, ios::in | ios::binary);
			if (!ff.good()) {
				return resFile;
			}
		}
	}
	ff.close();
	return fullPath;
}

json Args::createResourceFromFile(string resFile, string resType) {
	json o;

	lastImageHadAlpha = false;

	// read resource file
	fstream ff(resFile, ios::in | ios::binary);
	if (!ff.good()) {
		// try adding the source folder
		string dirname = Utils::get_dirname(value("file"));
		ff.open(dirname + resFile, ios::in | ios::binary);
		if (!ff.good()) {
			// try bare filename.ext in source folder
			ff.open(dirname + Utils::get_filename_ext(resFile), ios::in | ios::binary);
			if (!ff.good()) {
				// ok, we tried...
				cout << "Can't load the specified resource file: " << resFile << endl;
				return o;
			}
		}
	}
	ff.seekg(0, ff.end);
	streampos dataLen = ff.tellg();
	ff.seekg(0, ff.beg);
	uint8_t* buffer = new uint8_t[dataLen];
	ff.read((char*)buffer, dataLen);
	ff.close();

	// get resource type
	if (resType.length() < 1)
		resType = Utils::get_resource_type(resFile);

	// convert data to base64
	string fileDataBase64 = base64_encodeZ(buffer, (unsigned int)dataLen);
	delete[] buffer;

	o["type"] = resType;
	o["data"] = fileDataBase64;

	return o;
}

json Args::createCubemapResourceFromFiles(string resFileTemplate, int mipLevels) {
	size_t buff_size = resFileTemplate.size() * 2;
	char *buff = new char[buff_size];

	int width, height, bpp;
	json o;

	o["type"] = "image";
	o["bpp"] = 4;
	o["mip-levels"] = mipLevels;

	bool once = true;

	for (size_t side = 0; side < 6; side++)
	{
		for (size_t mip = 0; mip < mipLevels; mip++)
		{
			snprintf(buff, buff_size, resFileTemplate.c_str(), side, mip);
			string fullPath = getFullResourcePath(buff);
			bool is_hdr = stbi_is_hdr(fullPath.c_str());
			float * hdr_data = nullptr;
			unsigned char* rgba = nullptr;

			if (!is_hdr) {
				rgba = stbi_load(fullPath.c_str(), &width, &height, &bpp, 4);
			}
			else {
				hdr_data = stbi_loadf(fullPath.c_str(), &width, &height, &bpp, 3);
			}

			if (rgba == nullptr && hdr_data == nullptr) {
				cerr << "FAILED TO LOAD IMAGE!" << endl;
				cerr << " LOOKING FOR: " << fullPath << endl;
				assert(rgba || hdr_data);
			}

			string imageDataBase64;
			if (!is_hdr)
				imageDataBase64 = base64_encodeZ(rgba, width * height * 4);
			else
			{
				float *hdr_data_rgbaf = new float[width * height * 4];
				for (size_t i = 0; i < width*height; i++)
				{
					hdr_data_rgbaf[i * 4 + 0] = hdr_data[i * 3 + 0];
					hdr_data_rgbaf[i * 4 + 1] = hdr_data[i * 3 + 1];
					hdr_data_rgbaf[i * 4 + 2] = hdr_data[i * 3 + 2];
					hdr_data_rgbaf[i * 4 + 3] = 1.0;

				}
				imageDataBase64 = base64_encodeZ((uint8_t *)hdr_data_rgbaf, width * height * 4 * sizeof(float));
				/* unit test
				   TODO refactor to a separate file for tests
				vector<uint8_t>  res;
				base64_decodeZ(imageDataBase64, res);
				float *res_f = (float *)res.data();
				bool all_true = true;
				for (size_t i = 0; i < width*height*4; i++)
				{
					all_true == all_true && (res_f[i] == hdr_data_rgbaf[i]);
				}
				if(all_true)
					std::cout << "What we did, was correct and reversible." << std::endl;
				else
					std::cout << "What we did, was wrong and irreversible." << std::endl;
				*/
				delete[] hdr_data_rgbaf;
			}

			if (rgba)
				stbi_image_free(rgba);
			if (hdr_data)
				stbi_image_free(hdr_data);

			if (is_hdr)
				o["channel-format"] = "float32";

			if (once) {
				// assume input data is correct
				// and all sides have the same size
				once = false;
				o["width"] = width;
				o["height"] = height;
			}


			char key[64];
			snprintf(key, sizeof(key), "side-%d-mip-data-%d", side, mip);
			o[key] = imageDataBase64;

		}
	}
	delete[] buff;
	return o;
}

json Args::createImageResourceFromFile(string resFile, bool wantMips) {
	json o;

	// AVIR image resizer vars
	avir::CImageResizer<> ImageResizer(8, 0, avir::CImageResizerParamsUltra());
	avir::CImageResizer<> HDRImageResizer(32, 0, avir::CImageResizerParamsUltra());
	avir::CImageResizerVars vars;


	int width, height, bpp;

	cerr << "Loading image " << resFile << endl;
	string fullPath = getFullResourcePath(resFile);
	bool is_hdr = stbi_is_hdr(fullPath.c_str());
	float * hdr_data = nullptr;
	unsigned char* rgba = nullptr;

	if (!is_hdr) {
		vars.UseSRGBGamma = true;
		rgba = stbi_load(fullPath.c_str(), &width, &height, &bpp, 4);
	}
	else {
		hdr_data = stbi_loadf(fullPath.c_str(), &width, &height, &bpp, 3);
	}

	if (rgba == nullptr && hdr_data == nullptr) {
		cerr << "FAILED TO LOAD IMAGE!" << endl;
		cerr << " LOOKING FOR: " << fullPath << endl;
		assert(rgba || hdr_data);
	}

	lastImageHadAlpha = false;
	int count = 0;
	if (!is_hdr)
	{
		for (unsigned char* p = rgba; count < (width*height); p += 4, count++) {
			if ((int)p[3] < 255) {
				lastImageHadAlpha = true;
				break;
			}
		}
		if (lastImageHadAlpha)
			cerr << "IMAGE HAS ALPHA!!!!" << endl;
	}


	bool imageScaled = false;

	// during "inspect" we only want to see original tex size
	// also don't scale hdrs.
	if (command != "inspect" && !is_hdr)
	{
		int tex_size_limit = intValue("texture_max");

		// limit texture sizes
		int nWidth = width;
		int nHeight = height;
		if (width > height) {
			if (width > tex_size_limit) {
				nHeight = (int)((float)tex_size_limit * (float)height / (float)width);
				nWidth = tex_size_limit;
			}
		}
		else {
			if (height > tex_size_limit) {
				nWidth = (int)((float)tex_size_limit * (float)width / (float)height);
				nHeight = tex_size_limit;
			}
		}
		if (nWidth != width || nHeight != height) {
			imageScaled = true;
			unsigned char* mip = new unsigned char[nWidth * nHeight * 4];
			ImageResizer.resizeImage(rgba, width, height, 0,
				mip, nWidth, nHeight, 4, 0, &vars);
			stbi_image_free(rgba);
			rgba = mip;
			width = nWidth;
			height = nHeight;
		}
	}


	// convert data to base64
	string imageDataBase64;
	if(!is_hdr)
		imageDataBase64 = base64_encodeZ(rgba, width * height * 4);
	else
	{
		float *hdr_data_rgbaf = new float[width * height * 4];
		for (size_t i = 0; i < width*height; i++)
		{
			hdr_data_rgbaf[i * 4 + 0] = hdr_data[i * 3 + 0];
			hdr_data_rgbaf[i * 4 + 1] = hdr_data[i * 3 + 1];
			hdr_data_rgbaf[i * 4 + 2] = 0;// hdr_data[i * 3 + 2];
			hdr_data_rgbaf[i * 4 + 3] = 1.0;

		}
		imageDataBase64 = base64_encodeZ((uint8_t *)hdr_data_rgbaf, width * height * 4 * sizeof(float));
		delete[] hdr_data_rgbaf;
	}

	// possibly make mip maps, ignore hdrs for now, not sure if avir supports hdr
	bool makeMipmaps = IS_POWER_2(width) && IS_POWER_2(height) && !is_hdr;
	int mipLevels = 1;
	if (makeMipmaps && wantMips) {
		int w = width / 2;
		int h = height / 2;
		while (w > 2 && h > 2) {
			unsigned char* mip = new unsigned char[w * h * 4];
			//stbir_resize_uint8(rgba, width, height, 0,
			//	mip, w, h, 0, 4);
			if (!is_hdr)
			{
				unsigned char* mip = new unsigned char[w * h * 4];
				ImageResizer.resizeImage(rgba, width, height, 0,
					mip, w, h, 4, 0, &vars);
				string mipDataBase64 = base64_encodeZ(mip, w * h * 4);
				char key[64];
				snprintf(key, sizeof(key), "mip-data-%d", mipLevels);
				o[key] = mipDataBase64;

				delete[] mip;
			}
			else
			{
				/* disabled for now
				float* mip = new float[w * h * 4];
				// this requires testing, not sure if it works ok
				// as avir doesn't officially have support for hdr format
				HDRImageResizer.resizeImage(hdr_data, width, height, 0,
					mip, w, h, 3, 0, &vars);
				string mipDataBase64 = base64_encodeZ(mip, w * h * 3);
				char key[64];
				snprintf(key, sizeof(key), "mip-data-%d", mipLevels);
				o[key] = mipDataBase64;

				delete[] mip;
				*/
			}


			mipLevels++;
			w /= 2;
			h /= 2;
		}
	}

	// cleanup
	if (imageScaled) {
		delete[] rgba;
	}
	else {
		if (rgba)
			stbi_image_free(rgba);
		if (hdr_data)
			stbi_image_free(hdr_data);
	}
	if(is_hdr)
		o["channel-format"] = "float32";

	o["type"] = "image";
	o["width"] = width;
	o["height"] = height;
	o["bpp"] = 4;
	o["mip-levels"] = mipLevels;
	o["mip-data-0"] = imageDataBase64;

	return o;
}

json Args::createMaterial(json params, string effect) {
	json o;

	// set json object values
	o["type"] = "material";
	o["effect"] = effect;
	o["technique"] = this->value("technique");
	o["u-wrap"] = this->value("u-wrap");
	o["v-wrap"] = this->value("v-wrap");
	o["w-wrap"] = this->value("w-wrap");
	o["culling"] = this->value("culling");
	o["alpha-write"] = this->boolValue("alpha-write");
	o["depth-test"] = this->value("depth-test");
	o["depth-only"] = this->boolValue("depth-only");
	o["filter"] = this->value("filter");
	o["opaque"] = this->boolValue("opaque");
	o["parameters"] = params;

	return o;
}
