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
#include "command_merge.h"



void command_merge(Args& args) {

	// merge all files into new json
	json j = args.createNewJson();
	for (int i = 0; i < args.files.size(); i++) {
		// load json file
		json jm = args.loadJsonFile(args.files[i]);

		// prefix
		string n = Utils::get_filename(args.files[i]) + "_";

		// merge resources
		json res = jm["resources"];
		for (auto it = res.begin(); it != res.end(); it++) {
			string k = it.key();

			cout << "key before " << k << endl;

			// don't prefix lights. they must be named light0, light1, ...
			if (k.substr(0, 5) != "light" &&
				k != "depth_head_mat" && k != "depth_head_mdl") {
				k = n + k;
			}

			cout << "key after " << k << endl;

			// add the resource to the new json
			j["resources"][k] = it.value();

			// now fix references by type
			string tp = j["resources"][k]["type"];

			// fix model references
			if (tp == "model") {
				string msh = j["resources"][k]["mesh"];
				string mat = j["resources"][k]["material"];
				if (!Utils::is_default_resource(msh))
					j["resources"][k]["mesh"] = n + msh;
				if (!Utils::is_default_resource(mat) && mat != "depth_head_mat")
					j["resources"][k]["material"] = n + mat;
			}
			// fix skinned model references
			else if (tp == "skinned-model") {
				string mat = j["resources"][k]["material"];
				if (!Utils::is_default_resource(mat))
					j["resources"][k]["material"] = n + mat;
				json bones = j["resources"][k]["bones"];
				for (auto pit = bones.begin(); pit != bones.end(); pit++) {
					string tt = pit.value()["name"];
					j["resources"][k]["bones"][pit.key()]["name"] = n + tt;
				}
				json skins = j["resources"][k]["skins"];
				for (auto pit = skins.begin(); pit != skins.end(); pit++) {
					string msh = pit.value()["mesh"];
					if (!Utils::is_default_resource(msh))
						j["resources"][k]["skins"][pit.key()]["mesh"] = n + msh;
				}
			}
			// fix emitter references
			else if (tp == "emitter") {
				string mdl = j["resources"][k]["model"];
				if (!Utils::is_default_resource(mdl))
					j["resources"][k]["model"] = n + mdl;
			}
			// fix sequence references
			else if (tp == "sequence") {
				string img = j["resources"][k]["image"];
				if (!Utils::is_default_resource(img))
					j["resources"][k]["image"] = n + img;
			}
			// fix material references
			else if (tp == "material") {
				string eff = j["resources"][k]["effect"];
				if (!Utils::is_default_resource(eff))
					j["resources"][k]["effect"] = n + eff;
				json parms = j["resources"][k]["parameters"];
				for (auto pit = parms.begin(); pit != parms.end(); pit++) {
					if (pit.value()["type"] == "texture" || pit.value()["type"] == "sequence") {
						string tt = pit.value()["value"];
						if (!Utils::is_default_resource(tt))
							j["resources"][k]["parameters"][pit.key()]["value"] = n + tt;
					}
				}
			}
			// fix animation references
			else if (tp == "animation") {
				json channels = j["resources"][k]["channels"];
				for (auto chit = channels.begin(); chit != channels.end(); chit++) {
					string tt = chit.value()["name"];
					j["resources"][k]["channels"][chit.key()]["name"] = n + tt;
				}
			}
		}

		// merge parts
		json pts = jm["parts"];
		for (auto it = pts.begin(); it != pts.end(); it++) {
			string k = it.key();
			if (it.key().find("directionalLight") == string::npos &&
				it.key().find("pointLight") == string::npos &&
				it.key() != "depth_head") {
				k = n + k;
			}

			j["parts"][k] = it.value();
			json rezs;
			int rezIdx = 0;

			string parent = j["parts"][k]["parent"];
			if (parent.length() > 0 &&
				parent != "root" &&
				parent != "world") {
				j["parts"][k]["parent"] = n + parent;
			}

			// old single resource
			if (j["parts"][k].find("resource") != j["parts"][k].end()) {
				string r = j["parts"][k]["resource"];
				if (r.substr(0, 5) != "light" &&
					r != "depth_head_mat" && r != "depth_head_mdl") {
					r = n + r;
				}
				j["parts"][k]["resource"] = r;
			}
			// new multiple resources
			else if (j["parts"][k].find("resources") != j["parts"][k].end()) {
				json rr = j["parts"][k]["resources"];
				for (auto rit = rr.begin(); rit != rr.end(); rit++) {
					string rrr = rit.value();
					char temp[128];
					snprintf(temp, sizeof(temp), "%d", rezIdx++);
					if (rrr.substr(0, 5) != "light" &&
						rrr != "depth_head_mat" && rrr != "depth_head_mdl") {
						rrr = n + rrr;
					}
					j["parts"][k]["resources"][rit.key()] = rrr;
				}
			}
		}
	}

	string outfilename = "";
	if (args.filename == "auto") {
		for (unsigned int i = 0; i < args.files.size(); i++) {
			if (i != 0)
				outfilename = outfilename + "+";
			outfilename = outfilename + Utils::get_filename(args.files[i]);
		}
		outfilename = outfilename + ".json";
	}
	else {
		outfilename = args.filename;
	}

	cout << "Merged all files into " << outfilename << endl;

	// write it out
	args.writeJson(j, outfilename);
}

