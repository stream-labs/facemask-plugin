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

	cout << "Importing morph from rest: '" << restFile << "' and pose: '" << poseFile << "' ..." << endl;

	// make new json
	json j = args.createNewJson();
	j["description"] = "MaskMaker morph_import of " + Utils::get_filename(poseFile) + "." + Utils::get_extension(poseFile);
	args.jptr = &j;

	// ASSIMP : Import the rest and pose files
	Assimp::Importer importer;
	unsigned int impFlags = 
		aiProcess_TransformUVCoords |
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType;
	const aiScene* rest_scene = importer.ReadFile(restFile, impFlags);
	const aiScene* pose_scene = importer.ReadFile(poseFile, impFlags);

	// If either import failed, report it
	if (!rest_scene) {
		cout << "Assimp is unable to import '" << restFile << "'." << endl;
		return;
	}
	if (!pose_scene) {
		cout << "Assimp is unable to import '" << poseFile << "'." << endl;
		return;
	}

}
