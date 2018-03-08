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
#include "command_depends.h"

#define PRINTTEXTURE(_TEXTYPE_) {\
imgfile = getMaterialTexture(scene->mMaterials[i], _TEXTYPE_);\
if (imgfile.length() > 0) {\
cout << imgfile << endl;\
}}

static string getMaterialTexture(aiMaterial* mtl, aiTextureType tt) {
	int nt = mtl->GetTextureCount(tt);
	string p = "";
	if (nt > 0) {
		aiString path;
		mtl->GetTexture(tt, 0, &path);
		p = path.C_Str();
	}
	return p;
}


void command_depends(Args& args) {

	// ASSIMP : Import the scene from whatever file it is (dae/fbx/...)
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(args.filename,
		aiProcess_TransformUVCoords |
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!scene) {
		cout << "Assimp is unable to import '" << args.filename << "'." << endl;
		return;
	}

	// Get a list of all the textures
	map<string, string> textureFiles;
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		string imgfile;
		PRINTTEXTURE(aiTextureType_AMBIENT);
		PRINTTEXTURE(aiTextureType_DIFFUSE);
		PRINTTEXTURE(aiTextureType_SPECULAR);
		PRINTTEXTURE(aiTextureType_EMISSIVE);
		PRINTTEXTURE(aiTextureType_HEIGHT);
		PRINTTEXTURE(aiTextureType_NORMALS);
		PRINTTEXTURE(aiTextureType_LIGHTMAP);
		PRINTTEXTURE(aiTextureType_REFLECTION);
	}
}