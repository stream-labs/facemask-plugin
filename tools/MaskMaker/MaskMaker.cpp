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
#include "command_tweak.h"



using namespace std;



void command_create(Args& args) {

	// create new json
	json j = args.createNewJson();

	// write it out
	args.writeJson(j);

	cout << args.filename << " created." << endl;
}

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
			// don't prefix lights. they must be named light0, light1, ...
			if (k.find("light") == std::string::npos) {
				k = n + k;
			}
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
				if (!Utils::is_default_resource(mat))
					j["resources"][k]["material"] = n + mat;
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
			string k = n + it.key();
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
				if (r.find("light") == std::string::npos) {
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
					if (rrr.find("light") == std::string::npos) {
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

int CheckNodeNames(aiNode* node, int count) {
	if (!node)
		return count;
	if (node->mName.length == 0) {
		char temp[256];
		snprintf(temp, sizeof(temp), "node%d", count++);
		node->mName = temp;
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		count = CheckNodeNames(node->mChildren[i], count);
	}
	return count;
}

int LightNumber(const aiScene* scene, const aiNode* node) {
	for (unsigned int i = 0; i < scene->mNumLights; i++) {
		if (strcmp(scene->mLights[i]->mName.C_Str(), node->mName.C_Str()) == 0) {
			return i;
		}
	}
	return -1;
}

bool HasMeshes(aiNode* node) {
	if (node->mNumMeshes > 0)
		return true;
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		if (HasMeshes(node->mChildren[i]))
			return true;
	}
	return false;
}


bool HasLight(const aiScene* scene, aiNode* node) {
	for (unsigned int i = 0; i < scene->mNumLights; i++) {
		if (strcmp(scene->mLights[i]->mName.C_Str(), node->mName.C_Str()) == 0) {
			return true;
		}
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		if (HasLight(scene, node->mChildren[i]))
			return true;
	}
	return false;
}

void RemovePostRotationNodes(aiNode* node) {
	if (!node)
		return;

	// is this one of those shitty nodes?
	string nodeName = node->mName.C_Str();
	if (nodeName.find("PostRotation") != std::string::npos) {

		// we should have 1 child
		if (node->mNumChildren != 1)
			throw std::logic_error("bad assumption");

		// remove ourselves from the heirarchy
		for (unsigned int i = 0; i < node->mParent->mNumChildren; i++) {
			if (node->mParent->mChildren[i] == node) {
				node->mParent->mChildren[i] = node->mChildren[0];
				break;
			}
		}
		node->mChildren[0]->mParent = node->mParent;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		RemovePostRotationNodes(node->mChildren[i]);
	}
}

void AddNodes(const aiScene* scene, aiNode* node, json* parts) {
	if (!node)
		return;

	string nodeName = node->mName.C_Str();
	if (nodeName != "root") {
		json part;
		if (node->mParent) {
			string parentName = node->mParent->mName.C_Str();
			if (parentName == "root") {
				// we need to see if this node should really be
				// parented to "root", or parented to "world"
				// - if there are no meshes, make it world
				if (!HasMeshes(node) && HasLight(scene, node)) {
					parentName = "world";
				}
			}
			if (parentName.length() > 0)
				part["parent"] = parentName;
		}

		aiVector3D pos, scl;
		aiQuaterniont<float> rot;
		node->mTransformation.Decompose(scl, rot, pos);

		json p;
		p["x"] = pos.x;
		p["y"] = -pos.y; // flip y
		p["z"] = pos.z;
		part["position"] = p;

		json r;
		r["x"] = rot.x;
		r["y"] = -rot.y; // flip y
		r["z"] = rot.z;
		r["w"] = -rot.w; // flip rot
		part["qrotation"] = r;

		json s;
		s["x"] = scl.x;
		s["y"] = scl.y;
		s["z"] = scl.z;
		part["scale"] = s;

		// add mesh resources
		json rez;
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			char temp[256];
			snprintf(temp, sizeof(temp), "%d", i);
			string r = scene->mMeshes[node->mMeshes[i]]->mName.C_Str();
			r += "Model";
			rez[temp] = r;
		}

		// we might be a light transform
		int lightNum = LightNumber(scene, node);
		if (lightNum >= 0) {
			char temp1[256];
			snprintf(temp1, sizeof(temp1), "%d", node->mNumMeshes);
			char temp2[256];
			snprintf(temp2, sizeof(temp2), "light%d", lightNum);
			rez[temp1] = temp2;
		}

		if (node->mNumMeshes > 0 || lightNum >= 0)
			part["resources"] = rez;

		// add the part
		(*parts)[node->mName.C_Str()] = part;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		AddNodes(scene, node->mChildren[i], parts);
	}
}

aiNode* FindNode(aiNode* node, const string& name) {
	if (!node)
		return nullptr;
	if (name == node->mName.C_Str())
		return node;
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		aiNode* n = FindNode(node->mChildren[i], name);
		if (n)
			return n;
	}
	return nullptr;
}


string getMaterialTexture(aiMaterial* mtl, aiTextureType tt) {
	int nt = mtl->GetTextureCount(tt);
	string p = "";
	if (nt > 0) {
		aiString path;
		mtl->GetTexture(tt, 0, &path);
		p = path.C_Str();
	}
	return p;
}

string getTextureName(aiTextureType tt) {
	switch (tt) {
	case aiTextureType_AMBIENT:
		return "ambient";
	case aiTextureType_DIFFUSE:
		return "diffuse";
	case aiTextureType_SPECULAR:
		return "specular";
	case aiTextureType_EMISSIVE:
		return "emissive";
	case aiTextureType_HEIGHT:
	case aiTextureType_NORMALS:
		return "normal";
	case aiTextureType_LIGHTMAP:
	case aiTextureType_REFLECTION:
		return "reflect";
	}
	return "";
}

string lightTypeToString(aiLightSourceType t) {
	switch (t) {
	case aiLightSource_DIRECTIONAL:
		return "directional";
	case aiLightSource_POINT:
		return "point";
	case aiLightSource_SPOT:
		return "spot";
	case aiLightSource_AMBIENT:
		return "ambient";
	case aiLightSource_AREA:
		return "area";
	}
	return "";
}

string AnimBehaviourToString(aiAnimBehaviour b) {
	if (b == aiAnimBehaviour_DEFAULT)
		return "repeat";
	if (b == aiAnimBehaviour_CONSTANT)
		return "constant";
	if (b == aiAnimBehaviour_LINEAR)
		return "linear";
	if (b == aiAnimBehaviour_REPEAT)
		return "repeat";
	return "repeat";
}


#define GETTEXTURE(_TEXTYPE_) {\
imgfile = getMaterialTexture(scene->mMaterials[i], _TEXTYPE_);\
if (imgfile.length() > 0) {\
snprintf(temp, sizeof(temp), "%s-%d", getTextureName(_TEXTYPE_).c_str(), i);\
textureFiles[temp] = imgfile;\
}}

#define SETTEXPARAM(_TEXTYPE_) {\
imgfile = getMaterialTexture(scene->mMaterials[i], _TEXTYPE_);\
if (imgfile.length() > 0) {\
string paramName = getTextureName(_TEXTYPE_);\
snprintf(temp, sizeof(temp), "%s-%d", paramName.c_str(), i);\
textureFiles[temp] = imgfile;\
json parm;\
parm["type"] = "texture";\
parm["value"] = temp;\
params[paramName + "Tex"] = parm;\
parm["type"] = "integer";\
parm["value"] = 1;\
params[paramName + "Map"] = parm;\
}}

void command_import(Args& args) {

	// get filename
	string resFile = args.value("file");
	if (resFile.length() == 0) {
		cout << "You must specify a file with import." << endl;
		return;
	}

	cout << "Importing '" << resFile << "'..." << endl;

	// make new json
	json j = args.createNewJson();
	j["description"] = "MaskMaker import of " + Utils::get_filename(resFile) + "." + Utils::get_extension(resFile);
	args.jptr = &j;

	// ASSIMP : Import the scene from whatever file it is (dae/fbx/...)
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(resFile,
		aiProcess_TransformUVCoords |
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!scene)	{
		cout << "Assimp is unable to import '"<<resFile<<"'." << endl;
		return;
	}

	// Get a list of all the textures
	map<string, string> textureFiles;
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		char temp[256];
		string imgfile;
		GETTEXTURE(aiTextureType_AMBIENT);
		GETTEXTURE(aiTextureType_DIFFUSE);
		GETTEXTURE(aiTextureType_SPECULAR);
		GETTEXTURE(aiTextureType_EMISSIVE);
		GETTEXTURE(aiTextureType_HEIGHT);
		GETTEXTURE(aiTextureType_NORMALS);
		GETTEXTURE(aiTextureType_LIGHTMAP);
		GETTEXTURE(aiTextureType_REFLECTION);
	}

	json rez;

	// Add all the meshes
	struct GSVertex {
		float px, py, pz, pw;	// note: vec3 in obs has 4 values
		float nx, ny, nz, nw;
		float tx, ty, tz, tw;
		float u, v, w, ww;
		float u1, v1, w1, ww1;
		float u2, v2, w2, ww2;
		float u3, v3, w3, ww3;
		float u4, v4, w4, ww4;
		float u5, v5, w5, ww5;
		float u6, v6, w6, ww6;
		float u7, v7, w7, ww7;
		uint32_t color;

		char pad[12];			// pad to 16-byte boundary

		GSVertex() {
			memset(this, 0, sizeof(GSVertex));
		}
	};
	cout << "Importing " << scene->mNumMeshes << " meshes..." << endl;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		// sometimes meshes don't have names
		if (scene->mMeshes[i]->mName.length == 0) {
			char temp[256];
			snprintf(temp, sizeof(temp), "mesh%d", i);
			scene->mMeshes[i]->mName = temp;
		}
		aiMesh* mesh = scene->mMeshes[i];

		if (!mesh->mTangents) {
			cout << "*** MESH HAS NO TANGENTS! NORMAL MAPPING WILL BE SCREWED! ***" << endl;
		}
		if (!mesh->mTextureCoords[0]) {
			cout << "*** MESH HAS NO TEXTURE COORDINATES! HOPE YOU AREN'T TEXTURE MAPPING! ***" << endl;
		}

		// is this a skinned mesh?
		if (mesh->mNumBones > 0) {



		}
		else {
			// Create vertex list
			GSVertex* vertices = new GSVertex[mesh->mNumVertices];
			for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
				vertices[j].px = mesh->mVertices[j].x;
				vertices[j].py = -mesh->mVertices[j].y;
				vertices[j].pz = mesh->mVertices[j].z;
				vertices[j].nx = mesh->mNormals[j].x;
				vertices[j].ny = -mesh->mNormals[j].y;
				vertices[j].nz = mesh->mNormals[j].z;
				if (mesh->mTangents) {
					vertices[j].tx = mesh->mTangents[j].x;
					vertices[j].ty = mesh->mTangents[j].y;
					vertices[j].tz = mesh->mTangents[j].z;
				}
				if (mesh->mTextureCoords[0]) {
					vertices[j].u = mesh->mTextureCoords[0][j].x;
					vertices[j].v = 1.0f - mesh->mTextureCoords[0][j].y;
				}
			}

			string vertexDataBase64 =
				base64_encodeZ((uint8_t*)vertices, sizeof(GSVertex) * mesh->mNumVertices);

			unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
			int indIdx = 0;
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				aiFace& face = mesh->mFaces[j];
				assert(face.mNumIndices == 3);
				indices[indIdx++] = face.mIndices[0];
				indices[indIdx++] = face.mIndices[1];
				indices[indIdx++] = face.mIndices[2];
			}

			string indexDataBase64 =
				base64_encodeZ((uint8_t*)indices, sizeof(unsigned int) * indIdx);

			json o;
			o["type"] = "mesh";
			o["vertex-buffer"] = vertexDataBase64;
			o["index-buffer"] = indexDataBase64;

			rez[mesh->mName.C_Str()] = o;

			delete[] indices;
			delete[] vertices;
		}
	}

	// Add all the textures
	int count = 0;
	cout << "Importing textures..." << endl;
	map<string, bool> textureHasAlpha;
	for (auto it = textureFiles.begin(); it != textureFiles.end(); it++, count++) {
		json o = args.createImageResourceFromFile(it->second);
		textureHasAlpha[it->first] = args.lastImageHadAlpha;
		if (args.lastImageHadAlpha)
			cout << it->first << " " << " has alpha" << endl;
		rez[it->first] = o;
	}
	cout << "Imported " << count << " textures." << endl;

	// Add all the materials
	count = 0;
	cout << "Importing " << scene->mNumMaterials << " materials..." << endl;
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {

		aiMaterial* mtl = scene->mMaterials[i];

		char temp[256];
		json params;
		string imgfile;

		for (unsigned int j = 0; j < mtl->mNumProperties; j++) {
			aiMaterialProperty* pp = mtl->mProperties[j];
		}

		float reflectivity;
		mtl->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
		float opacity;
		mtl->Get(AI_MATKEY_OPACITY, opacity);
		float strength;
		mtl->Get(AI_MATKEY_SHININESS_STRENGTH, strength);
		float shininess;
		mtl->Get(AI_MATKEY_SHININESS, shininess);

		// Texture params
		SETTEXPARAM(aiTextureType_AMBIENT);
		SETTEXPARAM(aiTextureType_DIFFUSE);
		SETTEXPARAM(aiTextureType_SPECULAR);
		SETTEXPARAM(aiTextureType_EMISSIVE);
		SETTEXPARAM(aiTextureType_HEIGHT);
		SETTEXPARAM(aiTextureType_NORMALS);
		SETTEXPARAM(aiTextureType_LIGHTMAP);
		SETTEXPARAM(aiTextureType_REFLECTION);

		// Opaque flag, set based on diffuse texture
		bool opaque = true;
		imgfile = getMaterialTexture(scene->mMaterials[i], aiTextureType_DIFFUSE);
		if (imgfile.length() > 0) {
			string paramName = getTextureName(aiTextureType_DIFFUSE);
			snprintf(temp, sizeof(temp), "%s-%d", paramName.c_str(), i);
			opaque = !textureHasAlpha[temp];
		}

		// Only set colors if textures aren't set
		int namb = mtl->GetTextureCount(aiTextureType_AMBIENT);
		int ndff = mtl->GetTextureCount(aiTextureType_DIFFUSE);
		int nspc = mtl->GetTextureCount(aiTextureType_SPECULAR);
		int nemm = mtl->GetTextureCount(aiTextureType_EMISSIVE);

		// Color Params
		aiColor4D dcolor = aiColor4D(0.8f, 0.8f, 0.8f, 1.0f);
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &dcolor) &&
			ndff == 0) {
			json parm;
			json cc;
			cc["x"] = dcolor.r;
			cc["y"] = dcolor.g;
			cc["z"] = dcolor.b;
			cc["w"] = dcolor.a;
			parm["type"] = "float4";
			parm["value"] = cc;
			params["diffuseColor"] = parm;
		}
		aiColor4D scolor = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &scolor) &&
			nspc == 0) {
			json parm;
			json cc;
			cc["x"] = scolor.r;
			cc["y"] = scolor.g;
			cc["z"] = scolor.b;
			cc["w"] = scolor.a;
			parm["type"] = "float4";
			parm["value"] = cc;
			params["specularColor"] = parm;
		}
		aiColor4D acolor = aiColor4D(0.2f, 0.2f, 0.2f, 1.0f);
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &acolor) &&
			namb == 0) {
			json parm;
			json cc;
			cc["x"] = acolor.r;
			cc["y"] = acolor.g;
			cc["z"] = acolor.b;
			cc["w"] = acolor.a;
			parm["type"] = "float4";
			parm["value"] = cc;
			params["ambientColor"] = parm;
		}
		aiColor4D ecolor = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ecolor) &&
			nemm == 0) {
			json parm;
			json cc;
			cc["x"] = ecolor.r;
			cc["y"] = ecolor.g;
			cc["z"] = ecolor.b;
			cc["w"] = ecolor.a;
			parm["type"] = "float4";
			parm["value"] = cc;
			params["emissiveColor"] = parm;
		}

		// Culling
		int two_sided = 0;
		if (AI_SUCCESS == aiGetMaterialInteger(mtl, AI_MATKEY_TWOSIDED, &two_sided) && two_sided) {
			args.kvpairs["culling"] = "neither";
		}

		int mode[10];
		unsigned int max = 10;
		aiGetMaterialIntegerArray(mtl, AI_MATKEY_UVWSRC_DIFFUSE(0), mode, &max);
		
		max = 10;
		aiGetMaterialIntegerArray(mtl, AI_MATKEY_UVWSRC_HEIGHT(0), mode, &max);

		// Shininess
		{
			float shininess = 8;
			aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess);

			// nope
			//
			//float strength = 1;
			//if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength)) {
			//	shininess *= strength;
			//}

			json parm;
			parm["type"] = "float";
			parm["value"] = shininess;
			params["shininess"] = parm;
		}
		
		// make the material
		string effect = "effectPhong";
		snprintf(temp, sizeof(temp), "material%d", count++);
		rez[temp] = args.createMaterial(params, effect);
		rez[temp]["opaque"] = opaque;
	}

	// Add models
	cout << "Importing " << scene->mNumMeshes << " models..." << endl;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];

		char temp[256];
		snprintf(temp, sizeof(temp), "material%d", mesh->mMaterialIndex);

		// model
		json o;
		o["type"] = "model";
		o["mesh"] = mesh->mName.C_Str();
		o["material"] = temp;
		
		snprintf(temp, sizeof(temp), "%sModel", mesh->mName.C_Str());
		rez[temp] = o;
	}

	// Add lights
	cout << "Importing " << scene->mNumLights << " lights..." << endl;
	for (unsigned int i = 0; i < scene->mNumLights; i++) {
		aiLight* light = scene->mLights[i];

		// light type
		json o;
		o["type"] = "light";
		o["light-type"] = lightTypeToString(light->mType);

		// position/attenuation factors (not valid for directional lights)
		if (light->mType != aiLightSource_DIRECTIONAL &&
			light->mType != aiLightSource_AMBIENT) {
			json p;
			p["x"] = light->mPosition.x;
			p["y"] = light->mPosition.y;
			p["z"] = light->mPosition.z;
			o["position"] = p;

			o["att0"] = light->mAttenuationConstant;
			o["att1"] = light->mAttenuationLinear;
			o["att2"] = light->mAttenuationQuadratic;
		}

		// direction/up (not valid for point lights)
		if (light->mType != aiLightSource_POINT &&
			light->mType != aiLightSource_AMBIENT) {

			// wow. awesome.
			if (light->mDirection.SquareLength() < 0.0001f) {
				light->mDirection.z = 1.0;
			}
			json p;
			p["x"] = light->mDirection.x;
			p["y"] = light->mDirection.y;
			p["z"] = light->mDirection.z;
			o["direction"] = p;

			// seriously assimp? why even bother?
			if (light->mUp.SquareLength() < 0.0001f) {
				light->mUp.y = 1.0f;
			}
			p["x"] = light->mUp.x;
			p["y"] = light->mUp.y;
			p["z"] = light->mUp.z;
			o["up"] = p;
		}

		// NOTE: for some reason light colors out of assimp are
		//       in percentages [0,100]

		// color : ambient
		json c;
		c["x"] = light->mColorAmbient.r / 100.0f;
		c["y"] = light->mColorAmbient.g / 100.0f;
		c["z"] = light->mColorAmbient.b / 100.0f;
		o["ambient"] = c;

		// color : diffuse
		c["x"] = light->mColorDiffuse.r / 100.0f;
		c["y"] = light->mColorDiffuse.g / 100.0f;
		c["z"] = light->mColorDiffuse.b / 100.0f;
		o["diffuse"] = c;

		// color : specular
		c["x"] = light->mColorSpecular.r / 100.0f;
		c["y"] = light->mColorSpecular.g / 100.0f;
		c["z"] = light->mColorSpecular.b / 100.0f;
		o["specular"] = c;

		// spot angles (only valid for spot lights)
		if (light->mType == aiLightSource_SPOT) {

			o["inner-angle"] = light->mAngleInnerCone;
			o["outer-angle"] = light->mAngleOuterCone;
		}

		// area (only valid for area lights)
		if (light->mType == aiLightSource_AREA) {
			json v;
			v["x"] = light->mSize.x;
			v["y"] = light->mSize.y;
			o["area-size"] = v;
		}

		// we rename all the lights 
		char temp[256];
		snprintf(temp, sizeof(temp), "light%d", i);
		rez[temp] = o;
	}

	// Add animations
	cout << "Adding animations..." << endl;
	for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* anim = scene->mAnimations[i];

		char temp[256];

		if (anim->mName.length == 0) {
			snprintf(temp, sizeof(temp), "animation%d", i);
			anim->mName = temp;
		}

		json janim;
		janim["type"] = "animation";
		janim["duration"] = anim->mDuration;
		janim["fps"] = anim->mTicksPerSecond;

		int jchanCount = 0;
		json jchannels;

		// add node anim channels
		for (unsigned int j = 0; j < anim->mNumChannels; j++) {
			aiNodeAnim* chan = anim->mChannels[j];

			// get the node
			aiNode* node = FindNode(scene->mRootNode, chan->mNodeName.C_Str());
			// and transform
			aiVector3D pos, scl;
			aiQuaterniont<float> rot;
			node->mTransformation.Decompose(scl, rot, pos);

			// Position
			if (chan->mNumPositionKeys > 0) {
				// get the key values
				std::vector<float> xkeys;
				std::vector<float> ykeys;
				std::vector<float> zkeys;
				bool xch, ych, zch;
				xch = ych = zch = false;
				for (unsigned int k = 0; k < chan->mNumPositionKeys; k++) {
					const aiVectorKey& key = chan->mPositionKeys[k];
					xkeys.emplace_back((float)key.mValue.x);
					ykeys.emplace_back(-(float)key.mValue.y); // flip y
					zkeys.emplace_back((float)key.mValue.z);
					if (FLT_NEQ((float)key.mValue.x, pos.x))
						xch = true;
					if (FLT_NEQ((float)key.mValue.y, pos.y))
						ych = true;
					if (FLT_NEQ((float)key.mValue.z, pos.z))
						zch = true;
				}
				// only add non-static channels
				if (xch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-pos-x";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)xkeys.data(), 
						(unsigned int)(sizeof(float) * xkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (ych) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-pos-y";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)ykeys.data(), 
						(unsigned int)(sizeof(float) * ykeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (zch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-pos-z";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)zkeys.data(), 
						(unsigned int)(sizeof(float) * zkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
			}
			// Rotation
			if (chan->mNumRotationKeys > 0) {
				// get the key values
				std::vector<float> xkeys;
				std::vector<float> ykeys;
				std::vector<float> zkeys;
				std::vector<float> wkeys;
				bool xch, ych, zch, wch;
				xch = ych = zch = wch = false;
				for (unsigned int k = 0; k < chan->mNumRotationKeys; k++) {
					const aiQuatKey& key = chan->mRotationKeys[k];
					xkeys.emplace_back((float)key.mValue.x);
					ykeys.emplace_back(-(float)key.mValue.y); // flip y
					zkeys.emplace_back((float)key.mValue.z);
					wkeys.emplace_back(-(float)key.mValue.w); // flip rot
					if (FLT_NEQ((float)key.mValue.x, rot.x))
						xch = true;
					if (FLT_NEQ((float)key.mValue.y, rot.y))
						ych = true;
					if (FLT_NEQ((float)key.mValue.z, rot.z))
						zch = true;
					if (FLT_NEQ((float)key.mValue.w, rot.w))
						wch = true;
				}
				// only add non-static channels
				if (xch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-qrot-x";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)xkeys.data(), 
						(unsigned int)(sizeof(float) * xkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (ych) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-qrot-y";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)ykeys.data(), 
						(unsigned int)(sizeof(float) * ykeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (zch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-qrot-z";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)zkeys.data(), 
						(unsigned int)(sizeof(float) * zkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (wch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-qrot-w";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)wkeys.data(), 
						(unsigned int)(sizeof(float) * wkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
			}
			if (chan->mNumScalingKeys > 0) {
				// get the key values
				std::vector<float> xkeys;
				std::vector<float> ykeys;
				std::vector<float> zkeys;
				bool xch, ych, zch;
				xch = ych = zch = false;
				for (unsigned int k = 0; k < chan->mNumScalingKeys; k++) {
					const aiVectorKey& key = chan->mScalingKeys[k];
					snprintf(temp, sizeof(temp), "%d", k);
					xkeys.emplace_back((float)key.mValue.x);
					ykeys.emplace_back((float)key.mValue.y);
					zkeys.emplace_back((float)key.mValue.z);
					if (FLT_NEQ((float)key.mValue.x, scl.x))
						xch = true;
					if (FLT_NEQ((float)key.mValue.y, scl.y))
						ych = true;
					if (FLT_NEQ((float)key.mValue.z, scl.z))
						zch = true;
				}
				// only add non-static channels
				if (xch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-scl-x";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)xkeys.data(), 
						(unsigned int)(sizeof(float) * xkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (ych) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-scl-y";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)ykeys.data(), 
						(unsigned int)(sizeof(float) * ykeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
				if (zch) {
					json jchan;
					jchan["name"] = chan->mNodeName.C_Str();
					jchan["type"] = "part-scl-z";
					jchan["pre-state"] = AnimBehaviourToString(chan->mPreState);
					jchan["post-state"] = AnimBehaviourToString(chan->mPostState);
					jchan["values"] = base64_encodeZ((uint8_t*)zkeys.data(), 
						(unsigned int)(sizeof(float) * zkeys.size()));
					snprintf(temp, sizeof(temp), "%d", jchanCount++);
					jchannels[temp] = jchan;
				}
			}
		}

		janim["channels"] = jchannels;
		rez[anim->mName.C_Str()] = janim;
	}

	j["resources"] = rez;

	// Make sure the nodes have names
	count = 0;
	CheckNodeNames(scene->mRootNode, count);

	// Make their root the facemask root
	scene->mRootNode->mName = "root";

	// hrm
	RemovePostRotationNodes(scene->mRootNode);

	// Add parts
	json parts;
	AddNodes(scene, scene->mRootNode, &parts);
	j["parts"] = parts;

	// write it out
	args.writeJson(j);
	cout << "Done!" << endl << endl;
}


int main(int argc, char** argv) {

	// parse arguments
	Args args(argc, argv);
	if (args.failed)
		return -1;

	// run command
	if (args.command == "create")
		command_create(args);
	else if (args.command == "addres")
		command_addres(args);
	else if (args.command == "addpart")
		command_addpart(args);
	else if (args.command == "merge")
		command_merge(args);
	else if (args.command == "import")
		command_import(args);
	else if (args.command == "tweak")
		command_tweak(args);

	//getchar();
    return 0;
}

