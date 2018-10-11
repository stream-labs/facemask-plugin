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
#include "smll/OBSFont.hpp"
#include "smll/OBSRenderer.hpp"
#include "smll/DetectionResults.hpp"
#include "smll/TriangulationResult.hpp"
#include "smll/MorphData.hpp"
#include "smll/TextShaper.hpp"


#include "mask/mask.h"

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

			// callbacks
			static bool rewind_clicked(obs_properties_t *pr, obs_property_t *p, void *data);
			bool rewind_clicked(obs_properties_t *pr, obs_property_t *p);
			bool request_rewind;


		protected:
			// face detection thread
			static int32_t StaticThreadMain(Instance*);
			int32_t LocalThreadMain();

			// mask data loading thread
			static int32_t StaticMaskDataThreadMain(Instance*);
			int32_t LocalMaskDataThreadMain();

			// misc functions
			Mask::MaskData*	LoadMask(std::string filename);
			void LoadDemo();
			void drawCropRects(int width, int height);
			void updateFaces();
			void setFaceTransform(const smll::ThreeDPose& pose,
				bool billboard = false);
			void setupRenderingState();
			void getCanvasInfo();
			void drawMaskData(Mask::MaskData*	maskData, bool depthOnly, 
				bool staticOnly, bool rotationDisable);
			gs_texture* RenderSourceTexture(gs_effect_t* effect);
			bool SendSourceTextureToThread(gs_texture* sourceTexture);
			void clearFramesActiveStatus();

		private:
			// Filter State
			obs_source_t*	source;
			gs_rect			sourceViewport;
			int32_t			canvasWidth, canvasHeight;
			int32_t			baseWidth, baseHeight;
			bool			isActive;
			bool			isVisible;
			bool			videoTicked;
			HANDLE			taskHandle;

			// Face detector
			smll::FaceDetector*		smllFaceDetector;
			smll::OBSRenderer*		smllRenderer;

			// Fonts
			smll::TextShaper*		smllTextShaper;
			smll::OBSFont*			smllFont;

			// Texture rendering & staging
			gs_texrender_t*		sourceRenderTarget;
			gs_texrender_t*		drawTexRender;
			gs_texrender_t*		alertTexRender;
			gs_texrender_t*		detectTexRender;
			gs_stagesurf_t*		detectStage;

			// mask filenames
			std::string			maskFolder;
			std::string			currentMaskFolder;
			std::string			maskFilename;
			std::string			maskFilePath;
			std::string			maskInternal;
			std::string			currentMaskFilename;
			const char*			introFilename;
			std::string			currentIntroFilename;
			const char*			outroFilename;
			std::string			currentOutroFilename;

			void	checkForMaskUnloading();

			// alert params
			bool				alertActivate;
			bool				alertDoIntro;
			bool				alertDoOutro;
			std::string			alertText;
			std::string			donorName;
			float				alertDuration;
			float				donorNameDuration;
			float				alertOffsetBig;
			float				alertOffsetSmall;
			float				alertMinSize;
			float				alertMaxSize;
			float				alertShowDelay;

			// mask data loading thread
			bool				maskDataShutdown;
			std::thread			maskDataThread;
			std::mutex			maskDataMutex;
			std::unique_ptr<Mask::MaskData>	maskData;
			std::unique_ptr<Mask::MaskData>	introData;
			std::unique_ptr<Mask::MaskData>	outroData;

			// alert location
			enum AlertLocation {
				LEFT_BOTTOM,
				LEFT_TOP,
				RIGHT_BOTTOM,
				RIGHT_TOP,

				NUM_ALERT_LOCATIONS
			};

			// alert data
			std::string			renderedAlertText;
			AlertLocation		currentAlertLocation;
			std::unique_ptr<Mask::MaskData>	alertMaskDatas[AlertLocation::NUM_ALERT_LOCATIONS];
			gs_texture*			alertTextTexture;
			float				alertTranslation;
			float				alertAspectRatio;
			float				alertElapsedTime;
			float				alertAnimationDuration;
			bool				alertTriggered;
			bool				alertShown;
			bool				alertsLoaded;
			gs_rect				alertViewport;
			vec2				smoothCenter;

			// demo mode
			bool				demoModeOn;
			std::string			demoModeFolder;
			int					demoCurrentMask;
			bool				demoModeInDelay;
			bool				demoModeGenPreviews;
			bool				demoModeSavingFrames;
			std::vector<std::unique_ptr<Mask::MaskData>>	demoMaskDatas;
			std::vector<std::string> demoMaskFilenames;

			void demoModeRender(gs_texture* vidTex, 
				gs_texture* maskTex, Mask::MaskData* mask_data);

			// For writing thumbnails
			struct PreviewFrame {
				gs_texture_t*	vidtex;

				PreviewFrame(gs_texture_t* v, int w, int h);
				PreviewFrame(const PreviewFrame& other);
				PreviewFrame& operator=(const PreviewFrame& other);
				~PreviewFrame();
			};
			std::vector<PreviewFrame>	previewFrames;
			void WritePreviewFrames();
			void WriteTextureToFile(gs_texture* tex, std::string filename);

			// our current face detection results
			smll::DetectionResults		faces;
			smll::TriangulationResult	triangulation;
			TimeStamp					timestamp;

			// flags
			bool				drawMask;
			bool				drawAlert;
			bool				drawFaces;
			bool				drawMorphTris;
			bool				drawFDRect;
			bool				filterPreviewMode;
			bool				autoBGRemoval;
			bool				cartoonMode;

			// for testing/thumbs/writing textures to files
			gs_stagesurf_t*		testingStage;

			// find a cached video frame
			int					FindCachedFrameIndex(const TimeStamp& ts);
			gs_texture_t*		FindCachedFrame(const TimeStamp& ts);

			// Detection
			struct ThreadData {

				static const int BUFFER_SIZE = 8;

				std::thread thread;
				std::mutex mutex;
				bool shutdown;

				// frames circular buffer (video_render()'s thread -> detection thread)
				struct CachedFrame {
					smll::OBSTexture	capture;
					smll::ImageWrapper	detect;
					smll::MorphData     morphData;
					std::mutex			mutex;
					TimeStamp			timestamp;
					bool				active;
				};
				int frameIndex;
				std::array<struct CachedFrame, BUFFER_SIZE> frames;

				// faces circular buffer (detection thread -> video_tick()'s thread)
				struct CachedResult {
					smll::DetectionResults		detectionResults;
					smll::TriangulationResult	triangulationResults;
					std::mutex					mutex;
					TimeStamp					timestamp;
				};
				int facesIndex;
				std::array<struct CachedResult, BUFFER_SIZE> faces;

			} detection;

		};
	};
}
