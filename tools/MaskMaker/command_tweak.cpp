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
#include "stdafx.h"
#include "utils.h"
#include "command_tweak.h"

bool is_int(string v) {
	char* p;
	long c = strtol(v.c_str(), &p, 10);
	if (*p)
		return false;
	return true;
}

bool is_float(string v) {
	char* p;
	double c = strtod(v.c_str(), &p);
	if (*p)
		return false;
	return true;
}

bool is_vector(string v) {
	vector<string> bits = Utils::split(v, ',');
	return (bits.size() > 1);
}

bool is_bool(string v) {
	if (v == "true")
		return true;
	if (v == "True")
		return true;
	if (v == "TRUE")
		return true;
	if (v == "false")
		return true;
	if (v == "False")
		return true;
	if (v == "FALSE")
		return true;
	return false;
}

bool atobool(string v) {
	if (v == "true")
		return true;
	if (v == "True")
		return true;
	if (v == "TRUE")
		return true;
	return false;
}

vector<vector<string>> expand_tweak_path(const json& j, const vector<string>& path) {
	vector<vector<string>> results;

	// look for wild cards
	bool expanded = false;
	json jpath = j;
	for (int i = 0; i < path.size(); i++) {
		if (path[i] == "*") {
			expanded = true;
			for (json::iterator it = jpath.begin(); it != jpath.end(); it++) {
				vector<string> epath = path;
				epath[i] = it.key();
				results.push_back(epath);
			}
		}
		else {
			jpath = jpath[path[i]];
		}
	}

	if (!expanded) {
		results.push_back(path);
	}

	return results;
}

void command_tweak(Args& args) {

	// load json file
	json j = args.loadJsonFile(args.filename);
	args.jptr = &j;

	for (auto it = args.kvpairs.begin(); it != args.kvpairs.end(); it++) {
		vector<string> path = Utils::split(it->first, '.');

		// convert values from strings to bool/int/float
		json jval;
		if (is_int(it->second))
			jval = atoi(it->second.c_str());
		else if (is_float(it->second))
			jval = atof(it->second.c_str());
		else if (is_bool(it->second))
			jval = atobool(it->second.c_str());
		else if (is_vector(it->second)) {
			vector<string> bits = Utils::split(it->second, ',');
			vector<string> keys = { "x","y","z","w" };
			for (int i = 0; i < bits.size(); i++) {
				jval[keys[i]] = atof(bits[i].c_str());
			}
		}
		else
			jval = it->second;

		// expand path
		vector<vector<string>> expanded_paths = expand_tweak_path(j, path);

		for (int i = 0; i < expanded_paths.size(); i++) {
			vector<string>& epath = expanded_paths[i];

			// apply the tweak to the specified path
			switch (epath.size()) {
			case 1:
				j[epath[0]] = jval;
				break;
			case 2:
				j[epath[0]][epath[1]] = jval;
				break;
			case 3:
				j[epath[0]][epath[1]][epath[2]] = jval;
				break;
			case 4:
				j[epath[0]][epath[1]][epath[2]][epath[3]] = jval;
				break;
			case 5:
				j[epath[0]][epath[1]][epath[2]][epath[3]][epath[4]] = jval;
				break;
			case 6:
				j[epath[0]][epath[1]][epath[2]][epath[3]][epath[4]][epath[5]] = jval;
				break;
			case 7:
				j[epath[0]][epath[1]][epath[2]][epath[3]][epath[4]][epath[5]][epath[6]] = jval;
				break;
			case 8:
				j[epath[0]][epath[1]][epath[2]][epath[3]][epath[4]][epath[5]][epath[6]][epath[7]] = jval;
				break;
			case 9:
				j[epath[0]][epath[1]][epath[2]][epath[3]][epath[4]][epath[5]][epath[6]][epath[7]][epath[8]] = jval;
				break;
			case 10:
				j[epath[0]][epath[1]][epath[2]][epath[3]][epath[4]][epath[5]][epath[6]][epath[7]][epath[8]][epath[9]] = jval;
				break;
			default:
				assert(0);
				break;
			}
		}
	}

	// write it out
	args.writeJson(j);
	cout << "Done!" << endl << endl;
}



