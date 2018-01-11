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
#include "command_addpart.h"



void command_addpart(Args& args) {
	// load json file
	json j = args.loadJsonFile(args.filename);
	args.jptr = &j;


	// get addpart values
	string name = args.value("name");
	string parent = args.value("parent");
	json position = args.makeFloatArray(args.value("position"));
	json rotation = args.makeFloatArray(args.value("rotation"));
	json scale = args.makeFloatArray(args.value("scale"));
	string resource = args.value("resource");

	// make sure the resource exists
	if (resource.length() > 0) {
		if (!Utils::resource_exists(j, resource)) {
			cout << "Cannot find resource " << resource << "." << endl;
			return;
		}
	}

	// make a unique name if they didn't specify one
	int count = 1;
	char bb[128];
	while (name.length() == 0) {
		snprintf(bb, sizeof(bb), "part%d", count);
		name = bb;
		if (Utils::part_exists(j, name)) {
			count++;
			name = "";
		}
	}

	// build our part object
	json o;
	o["parent"] = parent;
	o["position"] = position;
	o["rotation"] = rotation;
	o["scale"] = scale;
	if (resource.length() > 0)
		o["resource"] = resource;

	// add it to our json
	j["parts"][name] = o;

	cout << "Added part: " << name << endl;

	// write it out
	args.writeJson(j);
}
