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
#include "json.hpp"
#include "utils.h"

// Facemask data version
#define FACEMASK_JSON_VERSION	(1)

using namespace std;

class Args {
public:

	json* jptr;
	string command;
	string filename;
	map<string, string> kvpairs;
	vector<string> files;
	map<string, vector<string>> jsonKeyNames;
	bool failed;
	bool lastImageHadAlpha;

	Args(int argc, char** argv);


	void usage();
	void print();
	string default_value(string key);
	bool haveValue(string key);
	string value(string key);
	float floatValue(string key);
	int intValue(string key);
	bool boolValue(string v);
	long longValue(string key);
	long long longlongValue(string key);
	json makeNumberArray(string v, bool isFloat, int sidx = 0);
	json makeFloatArray(string v, int sidx = 0);
	json makeIntArray(string v, int sidx = 0);
	json makeMatrix(string v, int sidx = 0);
	json getMaterialParams();
	bool fileExists(string fname);
	json loadJsonFile(string fname);
	void initJsonNamesAndValues();
	json createNewJson();
	void writeJson(const json& j);
	void writeJson(const json& j, string toFile);
	string uniqueResourceName(string name, string resType);
	string getFullResourcePath(string resFile);
	json createResourceFromFile(string resFile, string resType = string(""));
	json createImageResourceFromFile(string resFile, bool wantMips = true);
	json createMaterial(json params, string effect);

};

