/*
* Face Masks for SlOBS
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

#include "mask-resource-morph.h"
#include "mask.h"
#include "exceptions.h"
#include "plugin.h"
#include "utils.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}
#include <sstream>

static const char* const S_DELTAS = "deltas";


Mask::Resource::Morph::Morph(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	for (int i = 0; i < smll::NUM_FACE_AREAS; i++) {
		faceIndexBuffers[i] = nullptr;
	}

	// Deltas list
	if (!obs_data_has_user_value(data, S_DELTAS)) {
		PLOG_ERROR("Morph '%s' has no deltas list.", name.c_str());
		throw std::logic_error("Morph has no deltas.");
	}
	obs_data_item_t* deltasItem = obs_data_item_byname(data, S_DELTAS);
	if (obs_data_item_gettype(deltasItem) != obs_data_type::OBS_DATA_OBJECT) {
		PLOG_ERROR("Bad deltas section in '%s'.", name.c_str());
		throw std::logic_error("Morph has bad deltas section.");
	}
	obs_data_t* deltasData = obs_data_item_get_obj(deltasItem);
	if (!deltasData) {
		PLOG_ERROR("Bad deltas section in '%s'.", name.c_str());
		throw std::logic_error("Morph has bad deltas section.");
	}
	 
	int numPoints = 0;
	smll::DeltaList& deltas = m_morphData.GetDeltasAndStamp();
	for (obs_data_item_t* itm = obs_data_first(deltasData); itm; obs_data_item_next(&itm)) {
		// use string key as index into array
		std::string nn = obs_data_item_get_name(itm);
		int idx = atoi(nn.c_str());

		// position
		vec3 position;
		obs_data_get_vec3(deltasData, nn.c_str(), &position);
		deltas[idx] = position;

		// sanity check
		numPoints++;
	}
	// sanity check
	if (numPoints > smll::NUM_FACIAL_LANDMARKS) {
		PLOG_ERROR("Bad deltas section in '%s'. Too many deltas.", name.c_str());
		throw std::logic_error("Morph has bad deltas section. Too many deltas.");
	}
}

Mask::Resource::Morph::~Morph() {
	obs_enter_graphics();
	for (int i = 0; i < smll::NUM_FACE_AREAS; i++) {
		if (faceIndexBuffers[i])
			gs_indexbuffer_destroy(faceIndexBuffers[i]);
	}
	obs_leave_graphics();
}

Mask::Resource::Type Mask::Resource::Morph::GetType() {
	return Mask::Resource::Type::Morph;
}

void Mask::Resource::Morph::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	UNUSED_PARAMETER(time);

	// TODO: update non-frame data here
}

void Mask::Resource::Morph::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);

	// TODO: move rendering in here
}

bool Mask::Resource::Morph::IsDepthOnly() {
	return false;
}

void Mask::Resource::Morph::SetAnimatableValue(float v,
	Mask::Resource::AnimationChannelType act) {
	// sanity
	if (act < MORPH_CHANNEL_FIRST || act >= MORPH_CHANNEL_LAST) {
		PLOG_ERROR("Bad channel sent to Morph::SetAnimatableValue");
		throw std::logic_error("Bad channel sent to Morph::SetAnimatableValue.");
	}

	// get indices into deltas
	int deltaIdx = (int)act - (int)MORPH_LANDMARK_0_X;
	int deltaMod = deltaIdx % 3;
	deltaIdx /= 3;

	// set new value
	smll::DeltaList& deltas = m_morphData.GetDeltasAndStamp();
	switch (deltaMod) {
	case 0:
		deltas[deltaIdx].x = v;
		break;
	case 1:
		deltas[deltaIdx].y = v;
		break;
	case 2:
		deltas[deltaIdx].z = v;
		break;
	}
}

void Mask::Resource::Morph::MakeFaceIndexBuffers() {
	// just check if one of them is null
	if (faceIndexBuffers[smll::FACE_AREA_EYE_LEFT] == nullptr) {
		obs_enter_graphics();
		for (int i = 0; i < smll::NUM_FACE_AREAS; i++) {
			const smll::FaceArea& fa = smll::GetFaceArea((smll::FaceAreaID)i);
			faceIndexBuffers[i] = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG,
				(void*)fa.mesh_indices.data(), fa.mesh_indices.size(), 0);
		}
		obs_leave_graphics();
	}
}

void Mask::Resource::Morph::RenderMorphVideo(gs_texture* vidtex,
	const smll::TriangulationResult& trires) {

	if (trires.vertexBuffer == nullptr)
		return;

	MakeFaceIndexBuffers();

	// Effects
	gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	gs_effect_t* solidEffect = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t* solidcolor = gs_effect_get_param_by_name(solidEffect, "color");

	struct vec4 veccol;

	// Draw the source video
	gs_enable_depth_test(false);
	gs_set_cull_mode(GS_NEITHER);

	// GREEN SCREEN
	while (gs_effect_loop(solidEffect, "Solid")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
			"image"), vidtex);
		gs_load_vertexbuffer(trires.vertexBuffer);

		vec4_from_rgba(&veccol, MAKE32COLOR(0, 255, 0, 255));
		gs_effect_set_vec4(solidcolor, &veccol);
		gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_BACKGROUND]);
		gs_draw(GS_TRIS, 0, 0);
		gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_HULL]);
		gs_draw(GS_TRIS, 0, 0);
	}


	while (gs_effect_loop(defaultEffect, "Draw")) {
//	while (gs_effect_loop(solidEffect, "Solid")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
			"image"), vidtex); 
			gs_load_vertexbuffer(trires.vertexBuffer);

			// bg
			vec4_from_rgba(&veccol, MAKE32COLOR(66, 134, 244, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			//gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_BACKGROUND]);
			//gs_draw(GS_TRIS, 0, 0);

			// hull
			vec4_from_rgba(&veccol, MAKE32COLOR(41, 118, 242, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			//gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_HULL]);
			//gs_draw(GS_TRIS, 0, 0);
			
			// face
			vec4_from_rgba(&veccol, MAKE32COLOR(255, 220, 177, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_FACE]);
			gs_draw(GS_TRIS, 0, 0);
			
			// eyes
			vec4_from_rgba(&veccol, MAKE32COLOR(255, 255, 255, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_EYE_LEFT]);
			gs_draw(GS_TRIS, 0, 0);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_EYE_RIGHT]);
			gs_draw(GS_TRIS, 0, 0);

			// eyebrows & nose
			vec4_from_rgba(&veccol, MAKE32COLOR(227, 161, 115, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_BROW_LEFT]);
			gs_draw(GS_TRIS, 0, 0);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_BROW_RIGHT]);
			gs_draw(GS_TRIS, 0, 0);			
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_NOSE]);
			gs_draw(GS_TRIS, 0, 0);

			// lips
			vec4_from_rgba(&veccol, MAKE32COLOR(255, 0, 0, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_MOUTH_LIPS_TOP]);
			gs_draw(GS_TRIS, 0, 0);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_MOUTH_LIPS_BOTTOM]);
			gs_draw(GS_TRIS, 0, 0);

			// mouth hole
			vec4_from_rgba(&veccol, MAKE32COLOR(0, 0, 0, 255));
			gs_effect_set_vec4(solidcolor, &veccol);
			gs_load_indexbuffer(faceIndexBuffers[smll::FACE_AREA_MOUTH_HOLE]);
			gs_draw(GS_TRIS, 0, 0);
	}

	// draw lines
	if (trires.indexBuffers[smll::TriangulationResult::IDXBUFF_LINES]) {
		gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t    *color = gs_effect_get_param_by_name(solid, "color");
		vec4_from_rgba(&veccol, MAKE32COLOR(0, 255, 0, 200));
		gs_effect_set_vec4(color, &veccol);
		while (gs_effect_loop(solid, "Solid")) {
			gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_LINES]);
			gs_load_vertexbuffer(trires.vertexBuffer);
			gs_draw(GS_LINES, 0, 0);
		}
		// and points
		vec4_from_rgba(&veccol, MAKE32COLOR(255, 0, 0, 200));
		gs_effect_set_vec4(color, &veccol);
		while (gs_effect_loop(solid, "Solid")) {
			gs_load_indexbuffer(trires.indexBuffers[smll::TriangulationResult::IDXBUFF_LINES]);
			gs_load_vertexbuffer(trires.vertexBuffer);
			gs_draw(GS_POINTS, 0, 0);
		}
	}
}