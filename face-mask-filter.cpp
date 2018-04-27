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

#include "face-mask-filter.h"
#include "strings.h"
#include "plugin.h"
#include "utils.h"

#include <smll/Face.hpp>
#include <smll/Config.hpp>
#include <smll/landmarks.hpp>

#include <Shlwapi.h>
#include <memory>
#include <string>
#include <map>
#include <locale>
#include <codecvt>
#include <tchar.h>

// Windows AV run time stuff
#include <avrt.h>

#include "mask-resource-image.h"
#include "mask-resource-mesh.h"
#include "mask-resource-morph.h"
#include "mask-resource-effect.h"

 // yep...this is what we're doin
#include <libobs/util/threaded-memcpy.c>


// how many frames before we consider a face "lost"
#define NUM_FRAMES_TO_LOSE_FACE			(30)

// use threaded memcpy (not sure if this actually helps
// due to windows thread priorities)
#define USE_THREADED_MEMCPY				(true)

// if we aren't using threaded memcpy, use fast memcpy?
#define USE_FAST_MEMCPY					(false)

// if we aren't using threaded memcpy, use Intel IPP copy?
#define USE_IPP_MEMCPY					(false)



// whether to run landmark detection/solvepnp/morph on main thread
#define STUFF_ON_MAIN_THREAD			(false)

// Windows MMCSS thread task name
//
// see registry: Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\...
//
#define MM_THREAD_TASK_NAME				"DisplayPostProcessing"

// Maximum for number of masks loaded in demo mode
#define DEMO_MODE_MAX_MASKS				(400)

static float FOVA(float aspect) {
	// field of view angle matched to focal length for solvePNP
	return 56.0f / aspect;
}

static const float_t NEAR_Z = 1.0f;
static const float_t FAR_Z = 15000.0f;

// for rewind button
bool Plugin::FaceMaskFilter::Instance::request_rewind = false;

// Filter Wrapper
Plugin::FaceMaskFilter::FaceMaskFilter() {
	std::memset(&filter, 0, sizeof(obs_source_info));
	filter.id = "face_mask_filter";
	filter.type = OBS_SOURCE_TYPE_FILTER;
	filter.output_flags = OBS_SOURCE_VIDEO;

	filter.get_name = get_name;
	filter.create = create;
	filter.destroy = destroy;
	filter.get_width = Instance::get_width;
	filter.get_height = Instance::get_height;
	filter.get_defaults = Instance::get_defaults;
	filter.get_properties = Instance::get_properties;
	filter.update = Instance::update;
	filter.activate = Instance::activate;
	filter.deactivate = Instance::deactivate;
	filter.show = Instance::show;
	filter.hide = Instance::hide;
	filter.video_tick = Instance::video_tick;
	filter.video_render = Instance::video_render;

	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	obs_register_source(&filter);
}

Plugin::FaceMaskFilter::~FaceMaskFilter() {

}

const char * Plugin::FaceMaskFilter::get_name(void *) {
	return "Face Mask Filter";
}

void * Plugin::FaceMaskFilter::create(obs_data_t *data, obs_source_t *source) {
	return new Instance(data, source);
}

void Plugin::FaceMaskFilter::destroy(void *ptr) {
	delete reinterpret_cast<Instance*>(ptr);
}

// ------------------------------------------------------------------------- //
// Filter Instance
// ------------------------------------------------------------------------- //

Plugin::FaceMaskFilter::Instance::Instance(obs_data_t *data, obs_source_t *source)
	: source(source), baseWidth(640), baseHeight(480), isActive(true), isVisible(true),
	isDisabled(false), videoTicked(true), taskHandle(NULL), memcpyEnv(nullptr), detectStage(nullptr),
	maskDataShutdown(false), maskJsonFilename(nullptr), maskData(nullptr),
	demoModeOn(false), demoModeMaskJustChanged(false), demoModeMaskChanged(false), 
	demoCurrentMask(0), demoModeInterval(0.0f), demoModeDelay(0.0f), demoModeElapsed(0.0f), 
	demoModeInDelay(false), demoModeGenPreviews(false),	demoModeSavingFrames(false), drawMask(true),	
	drawFaces(false), drawMorphTris(false), drawFDRect(false), filterPreviewMode(false), 
	autoBGRemoval(false), cartoonMode(false), testingStage(nullptr) {

	PLOG_DEBUG("<%" PRIXPTR "> Initializing...", this);

	if (USE_THREADED_MEMCPY)
		memcpyEnv = init_threaded_memcpy_pool(0);

	obs_enter_graphics();
	sourceRenderTarget = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	detectTexRender = gs_texrender_create(GS_R8, GS_ZS_NONE);
	drawTexRender = gs_texrender_create(GS_RGBA, GS_Z32F); // has depth buffer
	obs_leave_graphics();

	// Make the smll stuff
	smllFaceDetector = new smll::FaceDetector();
	smllRenderer = new smll::OBSRenderer(); 

	//FONTDEMO
	//smllFont1 = new smll::OBSFont("c:/Windows/Fonts/graffonti.3d.drop.[fontvir.us].ttf", 100);

	// set our mm thread task
	if (!taskHandle) {
		DWORD taskIndex = 0;
		taskHandle = AvSetMmThreadCharacteristics(TEXT(MM_THREAD_TASK_NAME), &taskIndex);
		if (taskHandle == NULL) {
			blog(LOG_DEBUG, "[FaceMask] Failed to set MM thread characteristics");
		}
	}

	// get list of masks
	char* maskname = obs_module_file(kFileDefaultJson);
	std::string maskPath = Utils::dirname(maskname);
	maskJsonList = Utils::ListFolder(maskPath, "*.json");
	bfree(maskname);

	// make sure default is first
	std::string defmask = kFileDefaultJson;
	Utils::find_and_replace(defmask, "masks/", "");
	maskJsonList.insert(maskJsonList.begin(), defmask);
	for (int i = 1; i < maskJsonList.size(); i++) {
		if (maskJsonList[i] == defmask) {
			maskJsonList.erase(maskJsonList.begin() + i);
			break;
		}
	}

	// initialize face detection thread data
	{
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.shutdown = false;
		detection.frameIndex = -1;
		detection.facesIndex = -1;
		detection.thread = std::thread(StaticThreadMain, this);
		for (int i = 0; i < ThreadData::BUFFER_SIZE; i++) {
			detection.frames[i].active = false;
		}
	}
	
	// start mask data loading thread
	maskDataThread = std::thread(StaticMaskDataThreadMain, this);

	this->update(data);

	//
	// DEBUG: Print out openCV build information
	//
	//std::string cvBuildInfo = cv::getBuildInformation();
	//blog(LOG_DEBUG, "OpenCV Build Info\n-----------------\n%s", cvBuildInfo.c_str());
	//

	PLOG_DEBUG("<%" PRIXPTR "> Initialized.", this);
}

Plugin::FaceMaskFilter::Instance::~Instance() {
	PLOG_DEBUG("<%" PRIXPTR "> Finalizing...", this);

	// kill the thread
	maskDataShutdown = true;
	PLOG_DEBUG("<%" PRIXPTR "> Stopping worker Threads...", this);
	{
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.shutdown = true;
	}
	// wait for them to die
	detection.thread.join();
	maskDataThread.join();
	PLOG_DEBUG("<%" PRIXPTR "> Worker Thread stopped.", this);

	obs_enter_graphics();
	gs_texrender_destroy(sourceRenderTarget);
	gs_texrender_destroy(drawTexRender);
	gs_texrender_destroy(detectTexRender);
	for (int i = 0; i < ThreadData::BUFFER_SIZE; i++) {
		if (detection.frames[i].capture.texture) {
			gs_texture_destroy(detection.frames[i].capture.texture);
		}
	}
	if (testingStage)
		gs_stagesurface_destroy(testingStage);
	if (detectStage)
		gs_stagesurface_destroy(detectStage);
	maskData = nullptr;
	obs_leave_graphics();

	delete smllFaceDetector;
	delete smllRenderer;

	//FONTDEMO
	//delete smllFont1;

	if (memcpyEnv)
		destroy_threaded_memcpy_pool(memcpyEnv);

	if (taskHandle != NULL) {
		AvRevertMmThreadCharacteristics(taskHandle);
	}

	PLOG_DEBUG("<%" PRIXPTR "> Finalized.", this);
}

uint32_t Plugin::FaceMaskFilter::Instance::get_width(void *ptr) {
	if (ptr == nullptr)
		return 0;
	return reinterpret_cast<Instance*>(ptr)->get_width();
}

uint32_t Plugin::FaceMaskFilter::Instance::get_width() {
	return obs_source_get_base_width(obs_filter_get_target(source));
}

uint32_t Plugin::FaceMaskFilter::Instance::get_height(void *ptr) {
	if (ptr == nullptr)
		return 0;
	return reinterpret_cast<Instance*>(ptr)->get_height();
}

uint32_t Plugin::FaceMaskFilter::Instance::get_height() {
	return obs_source_get_base_height(obs_filter_get_target(source));
}

void Plugin::FaceMaskFilter::Instance::get_defaults(obs_data_t *data) {
	// default params

	obs_data_set_default_bool(data, P_DEACTIVATE, false);

#ifdef PUBLIC_RELEASE	
	obs_data_set_default_int(data, P_MASK, 0);
#else
	char* jsonName = obs_module_file(kFileDefaultJson);
	obs_data_set_default_string(data, P_MASK, jsonName);
	bfree(jsonName);
#endif

	obs_data_set_default_bool(data, P_CARTOON, false);
	obs_data_set_default_bool(data, P_BGREMOVAL, false);

	obs_data_set_default_bool(data, P_GENTHUMBS, false);

	obs_data_set_default_bool(data, P_DRAWMASK, true);
	obs_data_set_default_bool(data, P_DRAWFACEDATA, false);
	obs_data_set_default_bool(data, P_DRAWMORPHTRIS, false);
	obs_data_set_default_bool(data, P_DRAWCROPRECT, false);

	obs_data_set_default_bool(data, P_DEMOMODEON, false);
	obs_data_set_default_double(data, P_DEMOINTERVAL, 5.0f);
	obs_data_set_default_double(data, P_DEMODELAY, 3.0f);

#if !defined(PUBLIC_RELEASE)
	// default advanced params
	smll::Config::singleton().set_defaults(data);
#endif
}


obs_properties_t * Plugin::FaceMaskFilter::Instance::get_properties(void *ptr) {
	obs_properties_t* props = obs_properties_create();

	// If OBS gave us a source (internally called "context"),
	// we can use that here.
	if (ptr != nullptr)
		reinterpret_cast<Instance*>(ptr)->get_properties(props);

	return props;
}

void Plugin::FaceMaskFilter::Instance::get_properties(obs_properties_t *props) {
	// Source-specific properties can be added through this.
	obs_property_t* p = nullptr;

	char* jsonName = obs_module_file(kFileDefaultJson);
	std::string maskDir = Utils::dirname(jsonName);
	bfree(jsonName);


#if defined(PUBLIC_RELEASE)
	// mask
	p = obs_properties_add_list(props, P_MASK, P_TRANSLATE(P_MASK),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	for (int i = 0; i < maskJsonList.size(); i++) {
		std::string n = maskJsonList[i];
		Utils::find_and_replace(n, ".json", "");
		obs_property_list_add_int(p, n.c_str(), i);
	}

#else
	// mask
	p = obs_properties_add_path(props, P_MASK, P_TRANSLATE(P_MASK),
		obs_path_type::OBS_PATH_FILE,
		"Face Mask JSON (*.json)", maskDir.c_str());
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK)));

#endif

	p = obs_properties_add_bool(props, P_BGREMOVAL, P_TRANSLATE(P_BGREMOVAL));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_BGREMOVAL)));

	p = obs_properties_add_bool(props, P_CARTOON, P_TRANSLATE(P_CARTOON));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_CARTOON)));

#if !defined(PUBLIC_RELEASE)

	// disable the plugin
	obs_properties_add_bool(props, P_DEACTIVATE, P_TRANSLATE(P_DEACTIVATE));

	// rewind button
	obs_properties_add_button(props, P_REWIND, P_TRANSLATE(P_REWIND), 
		rewind_clicked);

	// Demo mode
	obs_properties_add_bool(props, P_DEMOMODEON, P_TRANSLATE(P_DEMOMODEON));
	obs_properties_add_text(props, P_DEMOFOLDER, P_TRANSLATE(P_DEMOFOLDER), 
		obs_text_type::OBS_TEXT_DEFAULT);
	obs_properties_add_float_slider(props, P_DEMOINTERVAL,
		P_TRANSLATE(P_DEMOINTERVAL), 1.0f, 60.0f, 1.0f);
	obs_properties_add_float_slider(props, P_DEMODELAY,
		P_TRANSLATE(P_DEMODELAY), 1.0f, 60.0f, 1.0f);

	obs_properties_add_bool(props, P_GENTHUMBS,	P_TRANSLATE(P_GENTHUMBS));

	// debug drawing flags
	obs_properties_add_bool(props, P_DRAWMASK,
		P_TRANSLATE(P_DRAWMASK));
	obs_properties_add_bool(props, P_DRAWFACEDATA,
		P_TRANSLATE(P_DRAWFACEDATA));
	obs_properties_add_bool(props, P_DRAWMORPHTRIS,
		P_TRANSLATE(P_DRAWMORPHTRIS));
	obs_properties_add_bool(props, P_DRAWCROPRECT,
		P_TRANSLATE(P_DRAWCROPRECT));

	// add advanced configuration params
	smll::Config::singleton().get_properties(props);

#endif
}

bool Plugin::FaceMaskFilter::Instance::rewind_clicked(obs_properties_t *pr, obs_property_t *p, void *data) {
	UNUSED_PARAMETER(p);
	UNUSED_PARAMETER(pr);
	UNUSED_PARAMETER(data);

	Plugin::FaceMaskFilter::Instance::request_rewind = true;

	return true;
}


void Plugin::FaceMaskFilter::Instance::update(void *ptr, obs_data_t *data) {
	if (ptr == nullptr)
		return;
	return reinterpret_cast<Instance*>(ptr)->update(data);
}

void Plugin::FaceMaskFilter::Instance::update(obs_data_t *data) {

	{
		std::unique_lock<std::mutex> lock(maskDataMutex, std::try_to_lock);
		if (lock.owns_lock()) {
#ifdef PUBLIC_RELEASE
			long long idx = obs_data_get_int(data, P_MASK);
			maskJsonFilename = maskJsonList[idx % maskJsonList.size()].c_str();
#else
			maskJsonFilename = (char*)obs_data_get_string(data, P_MASK);
#endif
		}
	}

#if !defined(PUBLIC_RELEASE)
	// update advanced properties
	smll::Config::singleton().update_properties(data);
#endif

	// disabled flag
	isDisabled = obs_data_get_bool(data, P_DEACTIVATE);
	if (isDisabled) {
		// reset the buffer
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.frameIndex = -1;
		detection.facesIndex = -1;
	}

	autoBGRemoval = obs_data_get_bool(data, P_BGREMOVAL);
	cartoonMode = obs_data_get_bool(data, P_CARTOON);

	// demo mode
	demoModeOn = obs_data_get_bool(data, P_DEMOMODEON);
	demoModeFolder = obs_data_get_string(data, P_DEMOFOLDER);
	demoModeInterval = (float)obs_data_get_double(data, P_DEMOINTERVAL);
	demoModeDelay = (float)obs_data_get_double(data, P_DEMODELAY);
	demoModeGenPreviews = obs_data_get_bool(data, P_GENTHUMBS);

	// update our param values
	drawMask = obs_data_get_bool(data, P_DRAWMASK);
	drawFaces = obs_data_get_bool(data, P_DRAWFACEDATA);
	drawMorphTris = obs_data_get_bool(data, P_DRAWMORPHTRIS);
	drawFDRect = obs_data_get_bool(data, P_DRAWCROPRECT);
}

void Plugin::FaceMaskFilter::Instance::activate(void *ptr) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->activate();
}

void Plugin::FaceMaskFilter::Instance::activate() {
	PLOG_DEBUG("<%" PRIXPTR "> Activating...", this);
	isActive = true;
}

void Plugin::FaceMaskFilter::Instance::deactivate(void *ptr) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->deactivate();
}

void Plugin::FaceMaskFilter::Instance::deactivate() {
	PLOG_DEBUG("<%" PRIXPTR "> Deactivating...", this);
	isActive = false;
}

void Plugin::FaceMaskFilter::Instance::show(void *ptr) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->show();
}

void Plugin::FaceMaskFilter::Instance::show() {
	PLOG_DEBUG("<%" PRIXPTR "> Show...", this);
	isVisible = true;
}

void Plugin::FaceMaskFilter::Instance::hide(void *ptr) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->hide();
}

void Plugin::FaceMaskFilter::Instance::hide() {
	PLOG_DEBUG("<%" PRIXPTR "> Hide...", this);
	isVisible = false;
	{
		// reset the buffer
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.frameIndex = -1;
		detection.facesIndex = -1;
	}
}

void Plugin::FaceMaskFilter::Instance::video_tick(void *ptr, float timeDelta) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->video_tick(timeDelta);
}

void Plugin::FaceMaskFilter::Instance::video_tick(float timeDelta) {

	videoTicked = true;

	// ----- GET FACES FROM OTHER THREAD -----
	updateFaces();

	// demo mode stuff
	demoModeUpdate(timeDelta);

	// Tick the mask data
	std::unique_lock<std::mutex> masklock(maskDataMutex, std::try_to_lock);
	if (masklock.owns_lock()) {
		// get the right mask data
		Mask::MaskData* mdat = maskData.get();
		if (demoModeOn && !demoModeInDelay) {
			if (demoCurrentMask >= 0 && demoCurrentMask < demoMaskDatas.size())
				mdat = demoMaskDatas[demoCurrentMask].get();
		}
		if (mdat) {
			if (Plugin::FaceMaskFilter::Instance::request_rewind) {
				mdat->RewindAnimations();
				Plugin::FaceMaskFilter::Instance::request_rewind = false;
			}

			mdat->Tick(timeDelta);
		}
	}
}

void Plugin::FaceMaskFilter::Instance::video_render(void *ptr,
	gs_effect_t *effect) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

void Plugin::FaceMaskFilter::Instance::video_render(gs_effect_t *effect) {

	// Skip rendering if inactive or invisible.
	if (!isActive || !isVisible || isDisabled) {
		obs_source_skip_video_filter(source);
		return;
	}

	// Grab parent and target source.
	obs_source_t *parent = obs_filter_get_parent(source);
	obs_source_t *target = obs_filter_get_target(source);
	if ((parent == NULL) || (target == NULL)) {
		// Early-exit if we have no parent or target (invalid state).
		obs_source_skip_video_filter(source);
		return;
	}

	// Target base width and height.
	baseWidth = obs_source_get_base_width(target);
	baseHeight = obs_source_get_base_height(target);
	if ((baseWidth <= 0) || (baseHeight <= 0)) {
		// Target is not ready yet or an invalid state happened.
		obs_source_skip_video_filter(source);
		return;
	}

	// Effects
	gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Render source frame to a texture
	gs_texture* sourceTexture = RenderSourceTexture(effect ? effect : defaultEffect);
	if (sourceTexture == NULL) {
		// Failed to grab a usable texture, so skip.
		obs_source_skip_video_filter(source);
		return;
	}

	// smll needs a "viewport" to draw
	smllRenderer->SetViewport(baseWidth, baseHeight);

	// ----- SEND FRAME TO FACE DETECTION THREAD -----
	SendSourceTextureToThread(sourceTexture);

	// ----- DRAW -----

	// start
	smllRenderer->DrawBegin();

	// Set up sampler state
	// Note: We need to wrap for morphing
	gs_sampler_info sinfo;
	sinfo.address_u = GS_ADDRESS_WRAP;
	sinfo.address_v = GS_ADDRESS_WRAP;
	sinfo.address_w = GS_ADDRESS_CLAMP;
	sinfo.filter = GS_FILTER_LINEAR;
	sinfo.border_color = 0;
	sinfo.max_anisotropy = 0;
	gs_samplerstate_t* ss = gs_samplerstate_create(&sinfo);
	gs_load_samplerstate(ss, 0);
	gs_samplerstate_destroy(ss);

	// mask to draw
	Mask::MaskData* mask_data = maskData.get();
	if (demoModeOn && demoMaskDatas.size() > 0) {
		if (demoCurrentMask >= 0 &&
			demoCurrentMask < demoMaskDatas.size()) {
			mask_data = demoMaskDatas[demoCurrentMask].get();
		}
	}
	
	// Select the video frame to draw
	// - since we are already caching frames of video for the
	//   face detection thread to consume, we can likely find
	//   the frame of video that matches the timestamp of the
	//   current detection data.
	gs_texture_t* vidTex = FindCachedFrame(timestamp);
	if (vidTex == nullptr) {
		vidTex = sourceTexture;
	}

	// draw face detection data
	if (drawFaces)
		smllRenderer->DrawFaces(faces);

	gs_enable_depth_test(true);
	gs_depth_function(GS_LESS);

	// draw crop rectangles
	drawCropRects(baseWidth, baseHeight);

	// some reasons triangulation should be destroyed
	if (!mask_data || faces.length == 0) {
		triangulation.DestroyBuffers();
	}

	// Draw the source video, if we aren't going to later
	if (!autoBGRemoval && 
		(faces.length == 0 || !drawMask || !mask_data || !videoTicked)) {
		gs_enable_depth_test(false);
		gs_set_cull_mode(GS_NEITHER);
		while (gs_effect_loop(defaultEffect, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
				"image"), vidTex);
			gs_draw_sprite(vidTex, 0, baseWidth, baseHeight);
		}
	}

	// ready?
	gs_texture* mask_tex = nullptr;
	if (faces.length > 0 && !demoModeInDelay) {

		// only render once per video tick
		if (videoTicked) {

			// draw stuff to texture
			gs_texrender_reset(drawTexRender);
			texRenderBegin(baseWidth, baseHeight);
			if (gs_texrender_begin(drawTexRender, baseWidth, baseHeight)) {

				// clear
				vec4 black, thumbbg;
				vec4_zero(&black);
				float vv = (float)0x9a / 255.0f;
				vec4_set(&thumbbg, vv, vv, vv, 1.0f);
				gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 1.0f, 0);

				if (drawMask && mask_data) {
					std::unique_lock<std::mutex> lock(maskDataMutex, std::try_to_lock);
					if (lock.owns_lock()) {
						// Check here for no morph
						if (!mask_data->GetMorph()) {
							triangulation.DestroyBuffers();
						}

						// Draw depth-only stuff
						for (int i = 0; i < faces.length; i++) {
							drawMaskData(faces[i], mask_data, true);
						}

						// if we are generating thumbs
						if (mask_data && demoModeOn && demoModeMaskChanged && demoModeGenPreviews && demoModeSavingFrames) {
							// clear the color buffer (leaving depth info there)
							gs_clear(GS_CLEAR_COLOR, &thumbbg, 1.0f, 0);
						}
						else {
							// clear the color buffer (leaving depth info there)
							gs_clear(GS_CLEAR_COLOR, &black, 1.0f, 0);
						}

						// Draw the source video
						if (mask_data && !demoModeInDelay) {
							triangulation.autoBGRemoval = autoBGRemoval;
							triangulation.cartoonMode = cartoonMode;
							mask_data->RenderMorphVideo(vidTex, baseWidth, baseHeight, triangulation);
						}
						else {
							// Draw the source video
							gs_enable_depth_test(false);
							gs_set_cull_mode(GS_NEITHER);
							while (gs_effect_loop(defaultEffect, "Draw")) {
								gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
									"image"), vidTex);
								gs_draw_sprite(vidTex, 0, baseWidth, baseHeight);
							}
						}

						// Draw regular stuff
						for (int i = 0; i < faces.length; i++) {
							drawMaskData(faces[i], mask_data, false);
						}
					}
				}

				// draw face detection data
				if (drawFaces)
					smllRenderer->DrawFaces(faces);

				gs_texrender_end(drawTexRender);
			}
			texRenderEnd();
		}

		mask_tex = gs_texrender_get_texture(drawTexRender);
	}

	if (mask_tex) {
		// draw the rendering on top of the video
		gs_matrix_push();
		gs_blend_state_push();
		gs_set_cull_mode(GS_NEITHER);
		gs_enable_blending(true);
		gs_enable_color(true, true, true, true);
		gs_blend_function(gs_blend_type::GS_BLEND_SRCALPHA, gs_blend_type::GS_BLEND_INVSRCALPHA);
		while (gs_effect_loop(defaultEffect, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
				"image"), mask_tex);
			gs_draw_sprite(mask_tex, 0, baseWidth, baseHeight);
		}
		gs_blend_state_pop();
		gs_matrix_pop();
	}

	//FONTDEMO
	//smllFont1->RenderText("this is a test of FreeType", 150, 150);

	// end
	smllRenderer->DrawEnd();

	// demo mode render stuff
	demoModeRender(vidTex, mask_tex, mask_data);

	// 1 frame only
	demoModeMaskJustChanged = false;
	videoTicked = false;
}

void Plugin::FaceMaskFilter::Instance::demoModeUpdate(float timeDelta) {
	// demo mode : switch masks/delay
	if (demoModeOn && demoMaskDatas.size()) {
		demoModeElapsed += timeDelta;
		if (demoModeInDelay && (demoModeElapsed > demoModeDelay)) {
			demoModeInDelay = false;
			demoModeElapsed -= demoModeDelay;
		}
		else if (!demoModeInDelay && (demoModeElapsed > demoModeInterval)) {
			demoCurrentMask = (demoCurrentMask + 1) % demoMaskDatas.size();
			demoModeMaskChanged = true;
			demoModeMaskJustChanged = true;
			demoModeSavingFrames = false;
			demoModeElapsed -= demoModeInterval;
			demoModeInDelay = true;
		}
	}
}

void Plugin::FaceMaskFilter::Instance::demoModeRender(gs_texture* vidTex, gs_texture* maskTex, 
	Mask::MaskData* mask_data) {

	// generate previews?
	if (demoModeOn && !demoModeInDelay && videoTicked &&
		demoModeMaskChanged && demoModeGenPreviews) {

		// get frame color
		if (!testingStage) {
			testingStage = gs_stagesurface_create(baseWidth, baseHeight, GS_RGBA);
		}
		gs_stage_texture(testingStage, vidTex);
		uint8_t *data; uint32_t linesize;
		bool isRed = false;
		if (gs_stagesurface_map(testingStage, &data, &linesize)) {
			uint8_t red = *data++;
			uint8_t green = *data++;
			uint8_t blue = *data++;
			if (red > 252 && green < 3 && blue < 3) {
				isRed = true;
			}
			gs_stagesurface_unmap(testingStage);
		}

		if (demoModeSavingFrames) {
			// sometimes the red frame is 2 frames
			if (isRed && previewFrames.size() < 1) {
				mask_data->RewindAnimations();
			}
			else if (isRed) {
				// done
				WritePreviewFrames();
				demoModeMaskChanged = false;
				demoModeSavingFrames = false;
			}
			else {
				PreviewFrame pf(maskTex, baseWidth, baseHeight);
				previewFrames.emplace_back(pf);
			}
		}
		else if (isRed) {
			// ready to go
			mask_data->RewindAnimations();
			demoModeSavingFrames = true;
		}
	}
}

gs_texture_t* Plugin::FaceMaskFilter::Instance::FindCachedFrame(const TimeStamp& ts) {
	int i = FindCachedFrameIndex(ts);
	if (i > 1)
		return detection.frames[i].capture.texture;
	return nullptr;
}

int Plugin::FaceMaskFilter::Instance::FindCachedFrameIndex(const TimeStamp& ts) {
	// Look for a cached video frame with the closest timestamp

	// Return one that matches first
	for (int i = 0; i < ThreadData::BUFFER_SIZE; i++) {
		if (detection.frames[i].timestamp == ts &&
			detection.frames[i].capture.texture != nullptr &&
			detection.frames[i].capture.width == baseWidth &&
			detection.frames[i].capture.height == baseHeight) {
			return i;
		}
	}
	
	// Now look for a valid frame with the closest timestamp
	long long tst = TIMESTAMP_AS_LL(ts);
	long long diff = 0;
	int nearest = -1;
	for (int i = 0; i < ThreadData::BUFFER_SIZE; i++) {
		if (detection.frames[i].capture.texture != nullptr &&
			detection.frames[i].capture.width > 0 &&
			detection.frames[i].capture.height > 0) {
			if (diff == 0) {
				nearest = i;
				diff = UNSIGNED_DIFF(TIMESTAMP_AS_LL(detection.frames[i].timestamp), tst);
			}
			else {
				long long d = UNSIGNED_DIFF(TIMESTAMP_AS_LL(detection.frames[i].timestamp), tst);
				if (d < diff) {
					diff = d;
					nearest = i;
				}
			}
		}
	}

	// Return what we got
	return nearest;
}



bool Plugin::FaceMaskFilter::Instance::SendSourceTextureToThread(gs_texture* sourceTexture) {

	// only if first render after video tick
	if (!videoTicked)
		return false;

	// timestamp for this frame
	TimeStamp sourceTimestamp = NEW_TIMESTAMP;
	bool frameSent = false;

	// Get the index
	int fidx = detection.frameIndex;
	if (fidx < 0)
		fidx = 0;

	// if there's already an active frame, bail
	int last_idx = (fidx - 1 + ThreadData::BUFFER_SIZE) % ThreadData::BUFFER_SIZE;
	if (detection.frames[last_idx].active)
		return false;

	// lock current frame
	{
		std::unique_lock<std::mutex> lock(detection.frames[fidx].mutex,
			std::try_to_lock);
		if (lock.owns_lock()) {
			frameSent = true;

			smll::OBSTexture& capture = detection.frames[fidx].capture;
			smll::ImageWrapper& detect = detection.frames[fidx].detect;

			detection.frames[fidx].active = true;
			detection.frames[fidx].timestamp = sourceTimestamp;

			// (re) allocate capture texture if necessary
			if (capture.width != baseWidth ||
				capture.height != baseHeight) {
				capture.width = baseWidth;
				capture.height = baseHeight;
				if (capture.texture)
					gs_texture_destroy(capture.texture);
				gs_color_format fmt = gs_texture_get_color_format(sourceTexture);
				capture.texture = gs_texture_create(baseWidth, baseHeight, fmt, 1, 0, 0);
			}

			// copy capture texture
			gs_copy_texture(capture.texture, sourceTexture);

			// detect texture dimensions
			smll::OBSTexture detectTex;
			detectTex.width = smll::Config::singleton().get_int(
				smll::CONFIG_INT_FACE_DETECT_WIDTH);
			detectTex.height = (int)((float)detectTex.width *
				(float)baseHeight / (float)baseWidth);

			// render the detect texture
			smllRenderer->SpriteTexRender(capture.texture,
				detectTexRender, detectTex.width, detectTex.height);
			detectTex.texture = gs_texrender_get_texture(detectTexRender);

			// stage and copy
			if (detectStage == nullptr ||
				(int)gs_stagesurface_get_width(detectStage) != detectTex.width ||
				(int)gs_stagesurface_get_height(detectStage) != detectTex.height) {
				if (detectStage)
					gs_stagesurface_destroy(detectStage);
				detectStage = gs_stagesurface_create(detectTex.width, detectTex.height,
					gs_texture_get_color_format(detectTex.texture));
			}
			gs_stage_texture(detectStage, detectTex.texture);
			{
				uint8_t *data; uint32_t linesize;
				if (gs_stagesurface_map(detectStage, &data, &linesize)) {
					// (re) allocate detect image buffer if necessary
					if (detect.w != detectTex.width ||
						detect.h != detectTex.height ||
						detect.stride != (int)linesize) {

						detect.w = detectTex.width;
						detect.h = detectTex.height;
						detect.stride = linesize;
						detect.type = smll::OBSRenderer::OBSToSMLL(gs_texture_get_color_format(detectTex.texture));
						detect.AlignedAlloc();
					}

					if (memcpyEnv)
						threaded_memcpy(detect.data, data, detect.getSize(), memcpyEnv);
					else if (USE_FAST_MEMCPY)
						Utils::fastMemcpy(detect.data, data, detect.getSize());
					else if (USE_IPP_MEMCPY) {
						smll::ImageWrapper src(detect.w, detect.h, detect.stride, detect.type, (char*)data);
						src.CopyTo(detect);
					}
					else
						memcpy(detect.data, data, detect.getSize());
					gs_stagesurface_unmap(detectStage);
				}
			}

			// get the right mask data
			Mask::MaskData* mdat = maskData.get();
			if (demoModeOn && !demoModeInDelay) {
				if (demoCurrentMask >= 0 && demoCurrentMask < demoMaskDatas.size())
					mdat = demoMaskDatas[demoCurrentMask].get();
			}

			// ask mask for a morph resource
			Mask::Resource::Morph* morph = nullptr;
			if (mdat) {
				morph = mdat->GetMorph();
			}

			// (possibly) update morph buffer
			if (morph) {
				if (morph->GetMorphData().IsNewerThan(detection.frames[fidx].morphData)) {
					detection.frames[fidx].morphData = morph->GetMorphData();
				}
			}
			else {
				// Make sure current is invalid
				detection.frames[fidx].morphData.Invalidate();
			}
		}
	}

	// Advance frame index if we copied a frame
	if (frameSent) {
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.frameIndex = (fidx + 1) % ThreadData::BUFFER_SIZE;
	}

	return frameSent;
}

gs_texture* Plugin::FaceMaskFilter::Instance::RenderSourceTexture(gs_effect_t* effect) {

	// Render previous Filters to texture.
	gs_texrender_reset(sourceRenderTarget);
	if (gs_texrender_begin(sourceRenderTarget, baseWidth, baseHeight)) {
		if (obs_source_process_filter_begin(source, GS_RGBA,
			OBS_NO_DIRECT_RENDERING)) {
			gs_blend_state_push();
			gs_projection_push();

			gs_ortho(0, (float)baseWidth, 0, (float)baseHeight, -1, 1);
			gs_set_cull_mode(GS_NEITHER);
			gs_reset_blend_state();
			gs_blend_function(gs_blend_type::GS_BLEND_ONE, gs_blend_type::GS_BLEND_ZERO);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_enable_color(true, true, true, true);

			vec4 empty;
			vec4_zero(&empty);
			gs_clear(GS_CLEAR_COLOR, &empty, 0, 0);

			obs_source_process_filter_end(source,
				effect, baseWidth, baseHeight);

			gs_projection_pop();
			gs_blend_state_pop();
		}
		gs_texrender_end(sourceRenderTarget);
	}
	return gs_texrender_get_texture(sourceRenderTarget);
}




void Plugin::FaceMaskFilter::Instance::drawMaskData(const smll::DetectionResult& face,
	Mask::MaskData*	_maskData, bool depthOnly) {

	gs_projection_push();
	float aspect = (float)baseWidth / (float)baseHeight;
	gs_perspective(FOVA(aspect), aspect, NEAR_Z, FAR_Z);

	gs_enable_stencil_test(false);

	gs_matrix_push();
	gs_matrix_identity();
	gs_matrix_translate3f((float)face.translation[0],
 		(float)face.translation[1], (float)-face.translation[2]);
	if (!_maskData->IsIntroAnimation()) {
		gs_matrix_rotaa4f((float)face.rotation[0], (float)face.rotation[1],
			(float)-face.rotation[2], (float)-face.rotation[3]);
	}
	_maskData->Render(depthOnly);

	gs_matrix_pop();
	gs_projection_pop();
}


int32_t Plugin::FaceMaskFilter::Instance::StaticThreadMain(Instance *ptr) {
	return ptr->LocalThreadMain();
}

int32_t Plugin::FaceMaskFilter::Instance::LocalThreadMain() {

	HANDLE hTask = NULL;
	DWORD taskIndex = 0;
	hTask = AvSetMmThreadCharacteristics(TEXT(MM_THREAD_TASK_NAME), &taskIndex);
	if (hTask == NULL) {
		blog(LOG_DEBUG, "[FaceMask] Failed to set MM thread characteristics");
	}

	// run until we're shut down
	TimeStamp lastTimestamp;
	while (true) {

		auto frameStart = std::chrono::system_clock::now();

		// get the frame index
		bool shutdown;
		int frame_idx;
		{
			std::unique_lock<std::mutex> lock(detection.mutex);
			frame_idx = detection.frameIndex;
			shutdown = detection.shutdown;
		}
		if (shutdown) break;
		if (frame_idx < 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		// the read index is always right behind the write
		frame_idx = (frame_idx + ThreadData::BUFFER_SIZE - 1) % ThreadData::BUFFER_SIZE;

		smll::DetectionResults detect_results;

		bool skipped = false;
		{
			std::unique_lock<std::mutex> lock(detection.frames[frame_idx].mutex);

			// check to see if we are detecting the same frame as last time
			if (lastTimestamp == detection.frames[frame_idx].timestamp) {
				// same frame, skip
				skipped = true;
			}
			else {
				// new frame - do the face detection
				smllFaceDetector->DetectFaces(detection.frames[frame_idx].detect, detection.frames[frame_idx].capture, detect_results);
				if (!STUFF_ON_MAIN_THREAD) {
					// Now do the landmark detection & pose estimation
					smllFaceDetector->DetectLandmarks(detection.frames[frame_idx].capture, detect_results);
					smllFaceDetector->DoPoseEstimation(detect_results);
				}

				lastTimestamp = detection.frames[frame_idx].timestamp;
			}
		}

		if (skipped) {
			// sleep for 1ms and continue
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		// get the index into the faces buffer
		int face_idx;
		{
			std::unique_lock<std::mutex> lock(detection.mutex);
			face_idx = detection.facesIndex;
		}
		if (face_idx < 0)
			face_idx = 0;

		{
			std::unique_lock<std::mutex> facelock(detection.faces[face_idx].mutex);

			// pass on timestamp to results
			detection.faces[face_idx].timestamp = lastTimestamp;

			if (!STUFF_ON_MAIN_THREAD) {
				std::unique_lock<std::mutex> framelock(detection.frames[frame_idx].mutex);

				// Make the triangulation
				detection.faces[face_idx].triangulationResults.buildLines = drawMorphTris;
				smllFaceDetector->MakeTriangulation(detection.frames[frame_idx].morphData,
					detect_results, detection.faces[face_idx].triangulationResults);

				detection.frames[frame_idx].active = false;
			}

			// Copy our detection results
			for (int i = 0; i < detect_results.length; i++) {
				detection.faces[face_idx].detectionResults[i] = detect_results[i];
			}
			detection.faces[face_idx].detectionResults.length = detect_results.length;
		}

		{
			std::unique_lock<std::mutex> lock(detection.mutex);

			// increment face buffer index
			detection.facesIndex = (face_idx + 1) % ThreadData::BUFFER_SIZE;
		}

		// don't go too fast and eat up all the cpu
		auto frameEnd = std::chrono::system_clock::now();
		auto elapsedMs =
			std::chrono::duration_cast<std::chrono::microseconds>
			(frameEnd - frameStart);
		long long speedLimit = smll::Config::singleton().get_int(
			smll::CONFIG_INT_SPEED_LIMIT) * 1000;
		long long sleepTime = max(speedLimit - elapsedMs.count(),
			(long long)0);
		if (sleepTime > 0)
			std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
	}

	if (hTask != NULL) {
		AvRevertMmThreadCharacteristics(hTask);
	}

	return 0;
}

int32_t Plugin::FaceMaskFilter::Instance::StaticMaskDataThreadMain(Instance *ptr) {
	return ptr->LocalMaskDataThreadMain();
}

int32_t Plugin::FaceMaskFilter::Instance::LocalMaskDataThreadMain() {

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);

	bool lastDemoMode = false; 
	while (!maskDataShutdown) {
		{
			std::unique_lock<std::mutex> lock(maskDataMutex, std::try_to_lock);
			if (lock.owns_lock()) {

				// time to load mask?
				if ((maskData == nullptr) &&
					maskJsonFilename && maskJsonFilename[0]) {

					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
					std::string maskFilename = maskJsonFilename;
#ifdef PUBLIC_RELEASE
					char* maskname = obs_module_file(kFileDefaultJson);
					std::string maskPath = Utils::dirname(maskname);
					bfree(maskname);
					maskFilename = maskPath + "/" + maskJsonFilename;
#endif

					// load mask
					maskData = std::unique_ptr<Mask::MaskData>(LoadMask(maskFilename));
					currentMaskJsonFilename = maskJsonFilename;
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
				}

				// mask filename changed?
				if (maskJsonFilename &&
					currentMaskJsonFilename != maskJsonFilename) {

					// unload mask
					obs_enter_graphics();
					maskData = nullptr;
					obs_leave_graphics();
				}

				// demo mode
				if (demoModeOn && !lastDemoMode) {
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
					LoadDemo();
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
				}
				else if (!demoModeOn && lastDemoMode) {
					obs_enter_graphics();
					demoMaskDatas.clear();
					demoMaskFilenames.clear();
					obs_leave_graphics();
				}
				lastDemoMode = demoModeOn;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}


	return 0;
}

void Plugin::FaceMaskFilter::Instance::LoadDemo() {

	blog(LOG_DEBUG, "loading demo folder %s", demoModeFolder.c_str());

	std::vector<std::string> files = Utils::ListFolderRecursive(demoModeFolder, "*.json");

	obs_enter_graphics();
	demoMaskDatas.clear();
	demoMaskFilenames.clear();
	obs_leave_graphics();

	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	for (int i = 0; i < files.size(); i++) {
		if (demoMaskDatas.size() == DEMO_MODE_MAX_MASKS)
			break;

		std::string fn = demoModeFolder + "\\" + files[i];
		bool addMask = true;
		if (demoModeGenPreviews) {
			std::string gifname = fn.substr(0, fn.length() - 4) + "gif";
			addMask = (::PathFileExists(gifname.c_str()) != TRUE);

			// don't do thumbs on these folders
			if (fn.find("\\heads\\") != std::string::npos)
				addMask = false;
			if (fn.find("\\facemask-plugin\\") != std::string::npos)
				addMask = false;
			if (fn.find("\\Releases\\") != std::string::npos)
				addMask = false;
			if (fn.find("\\releases\\") != std::string::npos)
				addMask = false;
		}
		if (addMask) {
			demoMaskDatas.push_back(std::unique_ptr<Mask::MaskData>(LoadMask(fn)));
			demoMaskFilenames.push_back(fn);
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	demoCurrentMask = 0;
	demoModeMaskChanged = true;
	demoModeSavingFrames = false;
}


Mask::MaskData*
Plugin::FaceMaskFilter::Instance::LoadMask(std::string filename) {

	obs_enter_graphics();
	PLOG_INFO("Loading mask '%s'...", filename.c_str());

	// new mask data
	Mask::MaskData* mdat = new Mask::MaskData();

	// load the json
	try {
		mdat->Load(filename);
		PLOG_INFO("Loading mask '%s' successful!", filename.c_str());
	}
	catch (...) {
		obs_leave_graphics();
		PLOG_ERROR("Failed to load mask %s.", filename.c_str());
	}
	obs_leave_graphics();

	return mdat;
}


void Plugin::FaceMaskFilter::Instance::texRenderBegin(int width, int height) {
	gs_matrix_push();
	gs_projection_push();
	gs_viewport_push();

	gs_matrix_identity();
	gs_set_viewport(0, 0, width, height);
	gs_ortho(0, (float)width, 0, (float)height, 0, 100);
}

void Plugin::FaceMaskFilter::Instance::texRenderEnd() {
	gs_matrix_pop();
	gs_viewport_pop();
	gs_projection_pop();
}

void Plugin::FaceMaskFilter::Instance::drawCropRects(int width, int height) {
	if (drawFDRect) {
		dlib::rectangle r;
		int x = (int)((float)(width / 2) *
			smll::Config::singleton().get_double(
				smll::CONFIG_DOUBLE_FACE_DETECT_CROP_X)) + (width / 2);
		int y = (int)((float)(height / 2) *
			smll::Config::singleton().get_double(
				smll::CONFIG_DOUBLE_FACE_DETECT_CROP_Y)) + (height / 2);
		int w = (int)((float)width *
			smll::Config::singleton().get_double(
				smll::CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH));
		int h = (int)((float)height *
			smll::Config::singleton().get_double(
				smll::CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT));

		// need to transform back to capture size
		x -= w / 2;
		y -= h / 2;

		r.set_top(y);
		r.set_bottom(y + h);
		r.set_left(x);
		r.set_right(x + w);
		smllRenderer->SetDrawColor(255, 0, 255);
		smllRenderer->DrawRect(r);
	}
}

int findClosest(const smll::DetectionResult& result, const smll::DetectionResults& results) {
	// find closest
	int closest = -1;
	double min = DBL_MAX;
	for (int j = 0; j < results.length; j++) {
		if (!results[j].matched) {
			double d = result.DistanceTo(results[j]);
			if (d < min) {
				closest = j;
				min = d;
			}
		}
	}
	return closest;
}

void Plugin::FaceMaskFilter::Instance::updateFaces() {


	// get the faces index from the other thread
	int fidx = -1;
	{
		std::unique_lock<std::mutex> lock(detection.mutex, std::try_to_lock);
		if (lock.owns_lock()) {
			fidx = detection.facesIndex;
		}
	}

	// other thread ready?
	if (fidx >= 0) {
		// read index is right behind the write index
		fidx = (fidx + ThreadData::BUFFER_SIZE - 1) % ThreadData::BUFFER_SIZE;

		std::unique_lock<std::mutex> lock(detection.faces[fidx].mutex, std::try_to_lock);
		if (lock.owns_lock()) {

			// new detected faces
			smll::DetectionResults& newFaces = detection.faces[fidx].detectionResults;

			if (STUFF_ON_MAIN_THREAD) {
				// Find the cached frame for these results
				int cfi = FindCachedFrameIndex(detection.faces[fidx].timestamp);
				if (cfi >= 0) {

					// Now do the landmark detection & pose estimation
					smllFaceDetector->DetectLandmarks(detection.frames[cfi].capture, detection.faces[fidx].detectionResults);
					smllFaceDetector->DoPoseEstimation(detection.faces[fidx].detectionResults);

					// Make the triangulation
					detection.faces[fidx].triangulationResults.buildLines = drawMorphTris;
					smllFaceDetector->MakeTriangulation(detection.frames[cfi].morphData,
						detection.faces[fidx].detectionResults, detection.faces[fidx].triangulationResults);

					detection.frames[fidx].active = false;
				}
			}

			// new triangulation
			triangulation.TakeBuffersFrom(detection.faces[fidx].triangulationResults);
			if (!drawMorphTris) {
				triangulation.DestroyLineBuffer();
			}

			// new timestamp
			timestamp = detection.faces[fidx].timestamp;

			// no faces lost, maybe some gained
			if (faces.length <= newFaces.length) {
				// clear matched flags
				for (int i = 0; i < newFaces.length; i++) {
					newFaces[i].matched = false;
				}

				// match our faces to the new ones
				for (int i = 0; i < faces.length; i++) {
					// find closest
					int closest = findClosest(faces[i], newFaces);

					// smooth new face into ours
					faces[i].UpdateResults(newFaces[closest]);
					faces[i].numFramesLost = 0;
					newFaces[closest].matched = true;
				}

				// now check for new faces
				for (int i = 0; i < newFaces.length; i++) {
					if (!newFaces[i].matched) {
						faces[faces.length] = newFaces[i];
						faces[faces.length].numFramesLost = 0;
						newFaces[i].matched = true;
						faces.length++;
					}
				}
			}

			// faces were lost
			else {

				// clear matched flags
				for (int i = 0; i < faces.length; i++) {
					faces[i].matched = false;
				}

				// match new faces to ours
				for (int i = 0; i < newFaces.length; i++) {

					// find closest
					int closest = findClosest(newFaces[i], faces);

					// smooth new face into ours
					faces[closest].UpdateResults(newFaces[i]);
					faces[closest].numFramesLost = 0;
					faces[closest].matched = true;
				}

				// now we need check lost faces
				for (int i = 0; i < faces.length; i++) {
					if (!faces[i].matched) {
						faces[i].numFramesLost++;
						if (faces[i].numFramesLost > NUM_FRAMES_TO_LOSE_FACE) {
							// remove face
							for (int j = i; j < (faces.length - 1); j++) {
								faces[j] = faces[j + 1];
							}
							faces.length--;
						}
					}
				}
			}
		}
	}
}

void Plugin::FaceMaskFilter::Instance::WritePreviewFrames() {

	obs_enter_graphics();

	// if the gif already exists, clean up and bail
	std::string gifname = demoMaskFilenames[demoCurrentMask].substr(0, demoMaskFilenames[demoCurrentMask].length() - 4) + "gif";
	if (::PathFileExists(gifname.c_str()) == TRUE) {
		for (int i = 0; i < previewFrames.size(); i++) {
			const PreviewFrame& frame = previewFrames[i];
			gs_texture_destroy(frame.vidtex);
		}
		previewFrames.clear();
		obs_leave_graphics();
		return;
	}

	// create output folder
	std::string outFolder = demoMaskFilenames[demoCurrentMask] + ".render";
	::CreateDirectory(outFolder.c_str(), NULL);

	// write out frames
	for (int i = 0; i < previewFrames.size(); i++) {
		const PreviewFrame& frame = previewFrames[i];

		// skip first frame for more seamless loop
		size_t last = previewFrames.size() - 2;
		if (i > 0 && i <= last) {
			cv::Mat vidf(baseWidth, baseHeight, CV_8UC4);

			if (!testingStage) {
				testingStage = gs_stagesurface_create(baseWidth, baseHeight, GS_RGBA);
			}

			// get vid tex
			gs_stage_texture(testingStage, frame.vidtex);
			uint8_t *data; uint32_t linesize;
			if (gs_stagesurface_map(testingStage, &data, &linesize)) {

				cv::Mat cvm = cv::Mat(baseHeight, baseWidth, CV_8UC4, data, linesize);
				cvm.copyTo(vidf);

				gs_stagesurface_unmap(testingStage);
			}

			// convert rgba -> bgra
			uint8_t* vpixel = vidf.data;
			for (int w = 0; w < baseWidth; w++)
				for (int h = 0; h < baseHeight; h++) {
					uint8_t red = vpixel[0];
					uint8_t blue = vpixel[2];
					vpixel[0] = blue;
					vpixel[2] = red;
					vpixel += 4;
				}

			// crop
			int offset = (baseWidth - baseHeight) * 2;
			cv::Mat cropf(baseHeight, baseHeight, CV_8UC4, vidf.data + offset, linesize);

			char temp[256];
			snprintf(temp, sizeof(temp), "frame%04d.png", i);
			std::string outFile = outFolder + "/" + temp;
			cv::imwrite(outFile.c_str(), cropf);

			// write out last frame again for thumbnail
			if (i == last) {
				outFile = outFolder + "/last_frame.png";
				cv::imwrite(outFile.c_str(), cropf);
			}
		}

		// kill frame data
		gs_texture_destroy(frame.vidtex);
	}

	previewFrames.clear();

	obs_leave_graphics();

	char* gifmaker = obs_module_file("gifmaker.bat");
	std::string cmd = gifmaker;
	Utils::find_and_replace(cmd, "/", "\\");
	Utils::find_and_replace(cmd, "Program Files", "\"Program Files\"");
	Utils::find_and_replace(cmd, "Streamlabs OBS", "\"Streamlabs OBS\"");
	cmd += " ";
	cmd += outFolder;
	::system(cmd.c_str());
	bfree(gifmaker);
}


Plugin::FaceMaskFilter::Instance::PreviewFrame::PreviewFrame(gs_texture_t* v, 
	int w, int h) {
	obs_enter_graphics();
	gs_color_format fmt = gs_texture_get_color_format(v);
	vidtex = gs_texture_create(w, h, fmt, 0, 0, 0);
	gs_copy_texture(vidtex, v);
	obs_leave_graphics();
}

Plugin::FaceMaskFilter::Instance::PreviewFrame::PreviewFrame(const PreviewFrame& other) {
	*this = other;
}

Plugin::FaceMaskFilter::Instance::PreviewFrame& 
Plugin::FaceMaskFilter::Instance::PreviewFrame::operator=(const PreviewFrame& other) {
	vidtex = other.vidtex;
	return *this;
}

Plugin::FaceMaskFilter::Instance::PreviewFrame::~PreviewFrame() {
}

