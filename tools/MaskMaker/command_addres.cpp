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
#include "command_addres.h"


void command_addres(Args& args) {
	// load json file
	json j = args.loadJsonFile(args.filename);
	args.jptr = &j;

	// see if they set the type
	string resType = args.value("type");

	// and the resource name
	string name = args.value("name");

	json o;

	// Materials
	if (resType == "material") {
		// get required values
		string effect = args.value("effect");
		if (effect.length() == 0) {
			cout << "You must specify an effect for a material resource." << endl;
			return;
		}

		// make sure the effect exists
		if (!Utils::resource_exists(j, effect, "effect")) {
			cout << "Cannot find effect " << effect << "." << endl;
			return;
		}

		o = args.createMaterial(args.getMaterialParams(), effect);

		// make a unique name if they didn't specify one
		name = args.uniqueResourceName(name, resType);
	}

	// Emitters
	else if (resType == "emitter") {
		// get required values
		string model = args.value("model");
		if (model.length() == 0) {
			cout << "You must specify an model for a emitter resource." << endl;
			return;
		}

		// make sure the effect exists
		if (!Utils::resource_exists(j, model, "model")) {
			cout << "Cannot find model " << model << "." << endl;
			return;
		}

		// make a unique name if they didn't specify one
		name = args.uniqueResourceName(name, resType);

		// we need to find the part that we are going to be parented to
		string partName = args.value("part");
		// iterate parts
		json parts = j["parts"];
		for (auto it = parts.begin(); it != parts.end(); it++) {
			string k = it.key();
			if (k == partName) {
				json resources = j["parts"][k]["resources"];
				int idx = 0;
				char temp[64];
				snprintf(temp, sizeof(temp), "%d", idx);
				while (resources.find(temp) != resources.end()) {
					idx++;
					snprintf(temp, sizeof(temp), "%d", idx);
				}
				resources[temp] = name;
				j["parts"][k]["resources"] = resources;
			}
		}

		// set json object values
		o["type"] = resType;
		o["model"] = model;
		o["lifetime"] = args.floatValue("lifetime");
		o["scale-start"] = args.floatValue("scale-start");
		o["scale-end"] = args.floatValue("scale-end");
		o["alpha-start"] = args.floatValue("alpha-start");
		o["alpha-end"] = args.floatValue("alpha-end");
		o["num-particles"] = args.intValue("num-particles");
		o["world-space"] = args.boolValue("world-space");
		o["inverse-rate"] = args.boolValue("inverse-rate");
		o["z-sort-offset"] = args.floatValue("z-sort-offset");

		// these values *could* have min/max values
		if (args.haveValue("rate-min") && args.haveValue("rate-max")) {
			o["rate-min"] = args.floatValue("rate-min");
			o["rate-max"] = args.floatValue("rate-max");
		}
		else {
			o["rate"] = args.floatValue("rate");
		}
		if (args.haveValue("friction-min") && args.haveValue("friction-max")) {
			o["friction-min"] = args.floatValue("friction-min");
			o["friction-max"] = args.floatValue("friction-max");
		}
		else {
			o["friction"] = args.floatValue("friction");
		}
		if (args.haveValue("force-min") && args.haveValue("force-max")) {
			o["force-min"] = args.makeFloatArray(args.value("force-min"));
			o["force-max"] = args.makeFloatArray(args.value("force-max"));
		}
		else {
			o["force"] = args.makeFloatArray(args.value("force"));
		}
		if (args.haveValue("initial-velocity-min") && args.haveValue("initial-velocity-max")) {
			o["initial-velocity-min"] = args.makeFloatArray(args.value("initial-velocity-min"));
			o["initial-velocity-max"] = args.makeFloatArray(args.value("initial-velocity-max"));
		}
		else {
			o["initial-velocity"] = args.makeFloatArray(args.value("initial-velocity"));
		}
	}

	// Sequences
	else if (resType == "sequence") {
		// get required values
		string image = args.value("image");
		if (image.length() == 0) {
			cout << "You must specify an image for a sequence resource." << endl;
			return;
		}

		// make sure the image exists
		if (!Utils::resource_exists(j, image, "image")) {
			cout << "Cannot find image " << image << "." << endl;
			return;
		}

		// make a unique name if they didn't specify one
		name = args.uniqueResourceName(name, resType);

		// we want all references to this image to now point to this sequence

		// iterate resources 
		json res = j["resources"];
		for (auto it = res.begin(); it != res.end(); it++) {
			string k = it.key();
			string tp = j["resources"][k]["type"];
			// fix material references
			if (tp == "material") {
				json parms = j["resources"][k]["parameters"];
				for (auto pit = parms.begin(); pit != parms.end(); pit++) {
					if (pit.value()["type"] == "texture" &&
						pit.value()["value"] == image) {
						// replace with us
						j["resources"][k]["parameters"][pit.key()]["type"] = "sequence";
						j["resources"][k]["parameters"][pit.key()]["value"] = name;
					}
				}
			}
		}

		// set up json object
		o["type"] = resType;
		o["image"] = image;
		o["rows"] = args.intValue("rows");
		o["cols"] = args.intValue("cols");
		o["first"] = args.intValue("first");
		o["last"] = args.intValue("last");
		o["rate"] = args.floatValue("rate");
		o["delay"] = args.floatValue("delay");
		o["mode"] = args.value("mode");
		o["random-start"] = args.boolValue("random-start");
	}

	// Models
	else if (resType == "model") {

		string mesh = args.value("mesh");
		string material = args.value("material");

		// make sure the resources exist
		if (!Utils::resource_exists(j, mesh, "mesh")) {
			cout << "Cannot find mesh " << mesh << "." << endl;
			return;
		}
		if (!Utils::resource_exists(j, material, "material")) {
			cout << "Cannot find material " << material << "." << endl;
			return;
		}

		// set json object values
		o["type"] = resType;
		o["mesh"] = mesh;
		o["material"] = material;

		// make a unique name if they didn't specify one
		name = args.uniqueResourceName(name, resType);
	}

	// Other resources
	else {
		// get resource filename
		string resFile = args.value("file");
		if (resFile.length() == 0) {
			cout << "You must specify a file with addres." << endl;
			return;
		}

		// turn it into a resource json object
		o = args.createResourceFromFile(resFile, resType);

		// make a resource name if need be
		if (name.length() == 0) {
			name = Utils::get_filename(resFile);
		}
	}

	// add it to our json
	j["resources"][name] = o;

	cout << "Added " << resType << " resource " << name << endl;

	// write it out
	args.writeJson(j);
}

