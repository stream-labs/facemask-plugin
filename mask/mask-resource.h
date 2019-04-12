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
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <thread>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs-module.h>
	#include <libobs/obs-data.h>
	#pragma warning( pop )
}

namespace Mask {
	class MaskData;
	class Part;

	namespace Resource {
		enum class Type : uint32_t {
			Image,
			Sequence,
			Effect,
			Material,
			Mesh,
			Model,
			SkinnedModel,
			Morph,
			Sound,
			Emitter,
			Light,
			Animation,
		};

		class Cache {
		public:
			// each cache pool will try to cap at this number
			static const size_t POOL_SIZE;

			struct CacheItem {
				// if a resource is used many times
				// we should try to keep it in the pool
				size_t use_count;
				// if we want a resource to leave the pool
				// the number of instances this resource is
				// actively used in will tell us if it is safe
				// for it to leave the pool.
				// e.g. if no one is using it, no one will
				// care. if one is using it, that
				// resource will take care of destructing
				// but if two are using it, it
				// is unsafe, as both of them will try to
				// destruct the same resource
				size_t active_count;
				void *resource;
				CacheItem():resource(nullptr) {}
				CacheItem(void *resource) :resource(resource) {
					active_count = use_count = 1;
				}
			};
			// map < resource name, cache item >
			using CachePool = std::map<std::string, CacheItem>;

			enum class CacheableType {
				Texture,
				Effect,
				OBSData
			};
			using PermanentResource = std::pair<CacheableType, void*>;
			 
			bool add(CacheableType resource_type, std::string name, void *resource);
			bool add_permanent(CacheableType resource_type, std::string name, void *resource);

			void load(CacheableType resource_type, std::string name, void **resource_ptr);
			void load_permanent(std::string name, void **resource_ptr);

			void destruct_by_type(void *resource, CacheableType resource_type);
			void try_destroy_resource(std::string name, void *resource, CacheableType resource_type);
			void destroy();
		private:
			std::map<CacheableType, CachePool> pool_map;
			std::map<std::string, PermanentResource> permanent_cache;
		};

		class IBase {
		public:
			using CacheableType = Cache::CacheableType;
			static std::shared_ptr<IBase> Load(Mask::MaskData* parent, std::string name, obs_data_t* data, Cache *cache);
			static std::shared_ptr<IBase> LoadDefault(Mask::MaskData* parent, std::string name, Cache *cache);

		protected:
			IBase(Mask::MaskData* parent, std::string name);

		public:
			virtual ~IBase();

			virtual Type GetType() = 0;
			std::string GetName() {	return m_name; }
			size_t GetId() { return m_id; }
			Mask::MaskData* GetParent() { return m_parent; }
			virtual void Update(Mask::Part* part, float time) = 0;
			virtual void Render(Mask::Part* part) = 0;
			virtual bool IsDepthOnly() { return false; }
			virtual bool IsStatic() { return false; }
			virtual bool IsRotationDisabled() { return false; }

		protected:
			Mask::MaskData* m_parent;
			std::string m_name;
			std::size_t m_id;
		};
	}
}
