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
#include "gs/gs-vertexbuffer.h"
#include "gs/gs-indexbuffer.h"

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

			vec4 GetCenter() { return m_center; }

			bool GetScreenExtents(gs_rect* r, int screen_width, int screen_height, float trsZ);

		private:
			void LoadObj(std::string file);

		protected:
			std::shared_ptr<GS::VertexBuffer>	m_VertexBuffer;
			std::shared_ptr<GS::IndexBuffer>	m_IndexBuffer;
			vec4								m_center;

			// cached in GetScreenExtents
			std::shared_ptr<Mask::Part>			m_part;

			// for delayed gs creation
			std::string				m_tempFile;
			uint8_t*				m_rawVertices;
			uint8_t*				m_rawIndices;
			int						m_numIndices;

			vec3 CalculateTangent(const GS::Vertex& v1,
				const GS::Vertex& v2, const GS::Vertex& v3);
		};
	}
}
