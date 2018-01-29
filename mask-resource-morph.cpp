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

static const char* const S_MESH = "mesh";
static const char* const S_MATERIAL = "material";


Mask::Resource::Morph::Morph(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {
	UNUSED_PARAMETER(data);
}

Mask::Resource::Morph::~Morph() {}

Mask::Resource::Type Mask::Resource::Morph::GetType() {
	return Mask::Resource::Type::Model;
}

void Mask::Resource::Morph::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(time);

	part->mask->instanceDatas.Push(m_id);
	// update
	part->mask->instanceDatas.Pop();
}

void Mask::Resource::Morph::Render(Mask::Part* part) {
	part->mask->instanceDatas.Push(m_id);
	// update
	part->mask->instanceDatas.Pop();
}

bool Mask::Resource::Morph::IsDepthOnly() {
	return false;
}
