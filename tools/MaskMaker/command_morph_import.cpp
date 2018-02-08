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
#include "command_morph_import.h"
#include "command_import.h"


static string g_locator_name = "landmark";

extern void RemovePostRotationNodes(aiNode* node);

// 68 detected points + 11 head points = 79
#define NUM_LANDMARK_POINTS		(79)


void GetLandmarkPoints(const aiScene* scene, aiNode* node, aiVector3D* points) {
	if (!node)
		return;

	// see if this is one of the landmark point locators
	string nodeName = node->mName.C_Str();
	if (nodeName.substr(0, g_locator_name.size()) == g_locator_name) {
		// decompose to get position
		aiVector3D pos, scl;
		aiQuaterniont<float> rot;
		node->mTransformation.Decompose(scl, rot, pos);

		// get point index
		int idx = atoi(nodeName.substr(g_locator_name.size()).c_str());
		if (idx >= 0 && idx < NUM_LANDMARK_POINTS) {
			points[idx] = pos;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		GetLandmarkPoints(scene, node->mChildren[i], points);
	}
}




void command_morph_import(Args& args) {

	// get rest pose filename
	string restFile = args.value("restfile");
	if (restFile.length() == 0) {
		cout << "You must specify a rest pose file (restfile) with morph_import." << endl;
		return;
	}

	// get morph pose filename
	string poseFile = args.value("posefile");
	if (poseFile.length() == 0) {
		cout << "You must specify a morph pose file (posefile) with morph_import." << endl;
		return;
	}

	cout << "// Importing morph from rest: '" << restFile << "' and pose: '" << poseFile << "' ..." << endl;

	// make new json
	json j = args.createNewJson();
	j["description"] = "MaskMaker morph_import of " + Utils::get_filename(poseFile) + "." + Utils::get_extension(poseFile);
	args.jptr = &j;

	unsigned int impFlags =
		aiProcess_TransformUVCoords |
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType;

	aiVector3D rest_points[NUM_LANDMARK_POINTS];
	aiVector3D pose_points[NUM_LANDMARK_POINTS];
	for (int i = 0; i < NUM_LANDMARK_POINTS; i++) {
		rest_points[i].x = 0.0f;
		rest_points[i].y = 0.0f;
		rest_points[i].z = 0.0f;
		pose_points[i].x = 0.0f;
		pose_points[i].y = 0.0f;
		pose_points[i].z = 0.0f;
	}

	// ASSIMP : Import the rest file
	{
		Assimp::Importer importer;
		const aiScene* rest_scene = importer.ReadFile(restFile, impFlags);

		// If import failed, report it
		if (!rest_scene) {
			cout << "Assimp is unable to import '" << restFile << "'." << endl;
			return;
		}

		// Fix shitty nodes
		RemovePostRotationNodes(rest_scene->mRootNode);

		// Get rest pose landmark point positions
		GetLandmarkPoints(rest_scene, rest_scene->mRootNode, rest_points);
	}
	// ASSIMP : Import the pose file
	Assimp::Importer importer;
	const aiScene* pose_scene = importer.ReadFile(poseFile, impFlags);

	// If import failed, report it
	if (!pose_scene) {
		cout << "Assimp is unable to import '" << poseFile << "'." << endl;
		return;
	}

	// Fix shitty nodes
	RemovePostRotationNodes(pose_scene->mRootNode);

	// Get rest pose landmark point positions
	GetLandmarkPoints(pose_scene, pose_scene->mRootNode, pose_points);

	// iterate points
	json delts;
	char temp[64];
	for (int i = 0; i < NUM_LANDMARK_POINTS; i++) {
		// delta from rest pose
		aiVector3D delta = pose_points[i] - rest_points[i];

		// MEL command to do morph
		//
		//cout << "float $pos_x = `getAttr landmark" << i << ".translateX`;" << endl;
		//cout << "float $pos_y = `getAttr landmark" << i << ".translateY`;" << endl;
		//cout << "float $pos_z = `getAttr landmark" << i << ".translateZ`;" << endl;
		//cout << "setAttr landmark" << i << ".translateX " << "($pos_x + " << delta.x << ");" << endl;
		//cout << "setAttr landmark" << i << ".translateY " << "($pos_y + " << delta.y << ");" << endl;
		//cout << "setAttr landmark" << i << ".translateZ " << "($pos_z + " << delta.z << ");" << endl;

		json d;
		d["x"] = delta.x;
		d["y"] = -delta.y;
		d["z"] = -delta.z;

		snprintf(temp, sizeof(temp), "%d", i);
		delts[temp] = d;
	}

	// make the morph resource
	json morph;
	morph["type"] = "morph";
	morph["deltas"] = delts;

	// add it to the resources
	json rez;
	string morphResourceName = Utils::get_filename(poseFile);
	rez[morphResourceName] = morph;

	// add animations
	ImportAnimations(args, pose_scene, rez, true, rest_points);

	// add resources to json
	j["resources"] = rez;

	// make a part for the morph
	json part;
	part["parent"] = "root";

	json p;
	p["x"] = 0.0f;
	p["y"] = 0.0f;
	p["z"] = 0.0f;
	part["position"] = p;

	json r;
	r["x"] = 1.0f;
	r["y"] = 0.0f;
	r["z"] = 0.0f;
	r["w"] = 0.0f;
	part["qrotation"] = r;

	json s;
	s["x"] = 1.0f;
	s["y"] = 1.0f;
	s["z"] = 1.0f;
	part["scale"] = s;

	json partrez;
	partrez["0"] = morphResourceName;
	part["resources"] = partrez;

	// add the part
	json parts;
	parts[morphResourceName] = part;
	j["parts"] = parts;

	// write it out
	args.writeJson(j);
	cout << "// Done!" << endl << endl;
}
