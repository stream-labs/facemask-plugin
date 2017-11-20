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
#include "mask.h"
#include <string>
#include <vector>
#include <stack>
#include <map>
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

	// base class for per-part instance data
	// resources can subclass and add them to the 
	// instanceDatas map 
	struct InstanceData {
		virtual ~InstanceData() {}
	};

	// instance datas found in mask's instanceDatas
	// - these will always be present

	// Alpha
	// - global alpha value 
	static const size_t AlphaInstanceDataId = 1;
	struct AlphaInstanceData : public InstanceData {
		float alpha;
		AlphaInstanceData() : alpha(1.0f) {}
	};

	// MaskInstanceDatas
	// - manages per-instance data in the part/resource heirarchy
	// - since resources may be referenced in multiple parts or 
	//   multiple other resources, we create a unique "path" name
	//   as a key into instance datas.
	// - for example, say a sequence has instance data. Its name
	//   (or key into the instance data map) would look like:
	//     <part name><model name><material name><sequence name>
	//   thus producing unique instance data objects for each 
	//   unique occurrance
	// - some resources, like particle emitters, can create multiple
	//   instances of a resource by pushing and popping instance
	//   names before calling update on their child resources.
	class MaskInstanceDatas {
	public:

		MaskInstanceDatas() : m_currentId(0) {}

		template<typename IDType>
		std::shared_ptr<IDType> GetData() {
			return GetData<IDType>(CurrentId());
		}

		template<typename IDType>
		std::shared_ptr<IDType> GetData(std::size_t the_id) {
			// create instance data if we need to.
			auto it = m_instanceDatas.find(the_id);
			if (it == m_instanceDatas.end()) {
				std::shared_ptr<IDType> instData =
					std::make_shared<IDType>();
				m_instanceDatas[the_id] = instData;
				return instData;
			}
			else {
				return std::dynamic_pointer_cast<IDType>
					(it->second);
			}
		}

		template<typename IDType>
		std::shared_ptr<IDType> FindDataDontCreate(std::size_t the_id) {
			// return nullptr if not found
			auto it = m_instanceDatas.find(the_id);
			if (it == m_instanceDatas.end()) {
				return nullptr;
			}
			else {
				return std::dynamic_pointer_cast<IDType>
					(it->second);
			}
		}

		void Push(std::size_t the_id) {
			hash_combine(the_id);
			m_stack.push_back(the_id);
		}

		void Pop() {
			m_stack.pop_back();
			m_currentId = 0;
			for (size_t i = 0; i < m_stack.size(); i++) {
				hash_combine(m_stack[i]);
			}
		}

		std::size_t CurrentId() {
			return m_currentId;
		}


	protected:
		std::size_t m_currentId;
		std::vector<size_t> m_stack;
		std::map<size_t, std::shared_ptr<InstanceData>> m_instanceDatas;

		inline void hash_combine(std::size_t val) {
			m_currentId ^= val + 0x9e3779b9 + (m_currentId << 6) + (m_currentId >> 2);
		}
	};
}
