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

#pragma once
#include "mask-resource.h"
#include "mask-instance-data.h"
#include <string>
#include <vector>
#include <memory>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs-data.h>
	#include <libobs/graphics/vec3.h>
	#include <libobs/graphics/matrix4.h>
	#include <libobs/graphics/quat.h>
	#pragma warning( pop )
}

namespace Mask {

	class MaskData;

	// Part : scene graph node
	// - keeps a local & global transform
	// - can be euler or quaternion rotations
	struct Part {
		Part(std::shared_ptr<Part> p_parent,
			std::shared_ptr<Resource::IBase> p_resource) :
			parent(p_parent), mask(nullptr),
			localdirty(true), dirty(true), isquat(false) {
			vec3_zero(&position);
			vec3_zero(&rotation);
			vec3_set(&scale, 1, 1, 1);
			quat_identity(&qrotation);
			if (p_resource)
				resources.push_back(p_resource);
			matrix4_identity(&local);
			matrix4_identity(&global);
		}
		Part(std::shared_ptr<Resource::IBase> p_resource) :
			Part(nullptr, p_resource) {}
		Part(std::shared_ptr<Part> p_parent) : Part(p_parent, nullptr) {}
		Part() : Part(nullptr, nullptr) {}

		std::vector<std::shared_ptr<Resource::IBase>> resources;
		std::shared_ptr<Part> parent;
		MaskData* mask;
		std::size_t hash_id;
		vec3 position, scale, rotation;
		quat qrotation;

		// Internal
		matrix4 local, global;
		bool localdirty;
		bool dirty;
		bool isquat;
	};

	namespace Resource {
		class Animation;
	}

	class MaskData {
		public:
		MaskData();
		virtual ~MaskData();

		void Clear();
		void Load(std::string file);
		//void Save(std::string file);

		void AddResource(std::string name, std::shared_ptr<Resource::IBase> resource);
		std::shared_ptr<Resource::IBase> GetResource(std::string name);
		std::shared_ptr<Resource::IBase> RemoveResource(std::string name);

		void AddPart(std::string name, std::shared_ptr<Part> part);
		std::shared_ptr<Part> GetPart(std::string name);
		std::shared_ptr<Part> RemovePart(std::string name);

		void Tick(float time);
		void Render(bool depthOnly = false);

		// global instance datas
		MaskInstanceDatas	instanceDatas;

		private:
		std::shared_ptr<Part> LoadPart(std::string name, obs_data_t* data);
		static void PartCalcMatrix(std::shared_ptr<Mask::Part> part);

		struct {
			std::string name;
			std::string description;
			std::string author;
			std::string website;
		} m_metaData;
		std::map<std::string, std::shared_ptr<Resource::IBase>> m_resources;
		std::map<std::string, std::shared_ptr<Part>> m_parts;
		std::map<std::string, std::shared_ptr<Resource::Animation>> m_animations;
		obs_data_t* m_data;
		std::shared_ptr<Mask::Part> m_partWorld;
	};
}
