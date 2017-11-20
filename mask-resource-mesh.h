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
#include "gs-vertexbuffer.h"
#include "gs-indexbuffer.h"

namespace Mask {
	namespace Resource {
		class Mesh : public IBase {
			public:
			Mesh(Mask::MaskData* parent, std::string name, std::string file);
			Mesh(Mask::MaskData* parent, std::string name, obs_data_t* data);
			virtual ~Mesh();

			virtual Type GetType() override;

			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			std::shared_ptr<GS::VertexBuffer> GetVertexBuffer() {
				return m_VertexBuffer;
			}

			private:
			void LoadObj(std::string file);

			protected:
			std::shared_ptr<GS::VertexBuffer> m_VertexBuffer;
			std::shared_ptr<GS::IndexBuffer> m_IndexBuffer;

			vec3 CalculateTangent(const GS::Vertex& v0,
				const GS::Vertex& v1, const GS::Vertex& v2);
		};
	}
}
