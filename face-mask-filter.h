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
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include "smll/FaceDetector.hpp"
#include "smll/OBSRenderer.hpp"
#include "smll/DetectionResults.hpp"

#include "mask.h"

extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs-source.h>
	#include <libobs/graphics/matrix4.h>
	#pragma warning( pop )
}

using namespace std;

namespace Plugin {
	class FaceMaskFilter {
		public:
		FaceMaskFilter();
		~FaceMaskFilter();

		static const char *get_name(void *);
		static void *create(obs_data_t *, obs_source_t *);
		static void destroy(void *);

		private:
		obs_source_info filter;

		class Instance {
			public:
			Instance(obs_data_t *, obs_source_t *);
			~Instance();

			// OBS API
			static uint32_t get_width(void *);
			uint32_t get_width();
			static uint32_t get_height(void *);
			uint32_t get_height();
			static void get_defaults(obs_data_t *);
			static obs_properties_t *get_properties(void *);
			void get_properties(obs_properties_t *pr);
			static bool properties_modified(obs_properties_t *, obs_property_t *, obs_data_t *);
			static void update(void *, obs_data_t *);
			void update(obs_data_t *);
			static void activate(void *);
			void activate();
			static void deactivate(void *);
			void deactivate();
			static void show(void *);
			void show();
			static void hide(void *);
			void hide();
			static void video_tick(void *, float);
			void video_tick(float);
			static void video_render(void *, gs_effect_t *);
			void video_render(gs_effect_t *);

			protected:
			static int32_t StaticThreadMain(Instance*);
			int32_t LocalThreadMain();

			static int32_t StaticMaskDataThreadMain(Instance*);
			int32_t LocalMaskDataThreadMain();

			Mask::MaskData*	LoadMask(std::string filename);
			void LoadDemo();

			void texRenderBegin(int width, int height);
			void texRenderEnd();

			void drawCropRects(int width, int height);

			void updateFaces();

			void drawMaskData(const smll::DetectionResult& face,
				Mask::MaskData*	maskData, bool depthOnly);

			private:
			// Filter State
			obs_source_t *m_source;
			int32_t m_baseWidth, m_baseHeight;
			bool m_isActive;
			bool m_isVisible;
			bool m_isDisabled;

			// Options

			// Face detector
			smll::FaceDetector*		smllFaceDetector;
			smll::OBSRenderer*		smllRenderer;

			gs_texrender_t	   *m_sourceRenderTarget;
			gs_texrender_t	   *drawTexRender;
			gs_texrender_t	   *detectTexRender;
			gs_texrender_t	   *trackTexRender;

			char*				maskJsonFilename;
			std::string			currentMaskJsonFilename;

			bool				maskDataShutdown;
			std::thread			maskDataThread;
			std::mutex			maskDataMutex;
			std::unique_ptr<Mask::MaskData>	maskData;

			// demo mode
			bool				demoModeOn;
			std::string			demoModeFolder;
			int					demoCurrentMask;
			float				demoModeElapsed;
			float				demoModeInterval;
			float				demoModeDelay;
			bool				demoModeInDelay;
			std::vector<std::unique_ptr<Mask::MaskData>>	demoMaskDatas;

			// our current version of detected faces
			smll::DetectionResults	faces;

			// our current triangulation
			gs_vertbuffer_t*		triangulationVB;
			gs_indexbuffer_t*		triangulationIB;

			long long			frameCounter;

			bool				drawMask;
			bool				drawFaces;
			bool				drawFDRect;
			bool				drawTRRect;
			bool				filterPreviewMode;

			int					performanceSetting;

			// for testing
			gs_stagesurf_t*		testingStage;

			// Detection
			struct ThreadData {

				static const int BUFFER_SIZE = 4;

				std::thread thread;
				std::mutex mutex;
				bool shutdown;

				// frames circular buffer (main thread -> detection thread)
				struct CachedFrame {
					smll::OBSTexture	capture;
					smll::OBSTexture	detect;
					smll::OBSTexture	track;
					std::mutex			mutex;
					long long			frame;
				};
				int frameIndex;
				std::array<struct CachedFrame, BUFFER_SIZE> frames;

				// faces circular buffer (detection thread -> main thread)
				struct CachedResult {
					smll::DetectionResults	detectionResults;
					gs_vertbuffer_t*		triangulationVB;
					gs_indexbuffer_t*		triangulationIB;
					CachedResult() : triangulationVB(nullptr), 
						triangulationIB(nullptr) {}
				};
				int facesIndex;
				std::array<struct CachedResult, BUFFER_SIZE> faces;

			} detection;

		};
	};
}
