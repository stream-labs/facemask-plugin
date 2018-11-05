/*
* Face Masks for SlOBS
* smll - streamlabs machine learning library
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
#ifndef __SMLL_OBS_RENDERER_HPP__
#define __SMLL_OBS_RENDERER_HPP__

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#include <libobs/graphics/graphics.h>
#pragma warning( pop )


#include "OBSTexture.hpp"
#include "OBSFont.hpp"
#include "ImageWrapper.hpp"
#include "FaceDetector.hpp"
#include "DetectionResults.hpp"

namespace smll {


	class OBSRenderer 
	{
	public:

		OBSRenderer();
		~OBSRenderer();

		void	SetDrawColor(unsigned char red, 
							 unsigned char green, 
							 unsigned char blue, 
							 unsigned char alpha = 255);
		void	SetViewport(int w, int h);

		int		LoadTexture(const ImageWrapper& image);
		int		LoadTexture(const char* filename);
		void	UnLoadTexture(int texture);

		void	BeginVertexBuffer();
		int		EndVertexBuffer();
		int		LoadVertexBufferFromOBJ(const char* filename);
		gs_vertbuffer_t* GetVertexBuffer(int which);
		void    DestroyVertexBufffer(int which);

		void	DrawFaces(const DetectionResults& faces);
		void	DrawLandmarks(const dlib::point* points, uint8_t r, 
			uint8_t g, uint8_t b);
		void	DrawRect(const dlib::rectangle& r, int width = 1);

		void    DrawGlasses(const DetectionResult& face, int texture);
		void    DrawMask(const DetectionResult& face, int texture);
		void    Draw2DMask(const DetectionResult& face, int texture);
		void	DrawModel(const DetectionResult& face, int vertexbuffer, int texture);
		
		static gs_color_format		SMLLToOBS(smll::ImageType t);
		static smll::ImageType		OBSToSMLL(gs_color_format f);
		static uint32_t				MakeColor(uint8_t r, uint8_t g, 
			uint8_t b, uint8_t a=255);
		void    SpriteTexRender(gs_texture_t* texture, 
			gs_texrender_t* texrender, int width, int height);

		void						SetTransform(const DetectionResult& face);

		// Text rendering
		gs_texture*		RenderTextToTexture(const std::vector<std::wstring>& lines,
			int tex_width, int tex_height, OBSFont* font);

	private:

		// gl texture names
		std::vector<gs_texture*>	m_textures;

		// viewport dimensions
		int							m_viewportWidth;
		int							m_viewportHeight;

		gs_effect_t*				m_phongEffect;
		gs_sampler_state*			m_samplerState;
		std::vector<gs_vertbuffer_t*>	m_vertexBuffers;
		gs_effect_t*				m_colorConversion;

		gs_texrender_t*				drawTexRender;

		//color vector
		struct vec4					veccol;
		void						drawLines(const dlib::point* points,
			int start, int end, bool closed = false);
		void						drawLine(const dlib::point* points,
			int start, int end);
	};


} // smll namespace
#endif // __SMLL_OBS_RENDERER_HPP__

