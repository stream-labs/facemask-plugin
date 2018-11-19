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
#include <smll/TestingPipe.hpp>
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

#include "mask/mask-resource-image.h"
#include "mask/mask-resource-mesh.h"
#include "mask/mask-resource-morph.h"
#include "mask/mask-resource-effect.h"

//
// SYSTEM MEMCPY STILL SEEMS FASTEST
//
// - the following memcpy options are typically set to false
//
#define USE_FAST_MEMCPY					(false)
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

// Fade time for masks when no intro/outro
#define MASK_FADE_TIME					(1.0f / 3.0f)

// Big enough
#define BIG_FLOAT					    (100000.0f)

static float FOVA(float aspect) {
	// field of view angle matched to focal length for solvePNP
	return 56.0f / aspect;
}

static const float_t NEAR_Z = 1.0f;
static const float_t FAR_Z = 15000.0f;

bool gs_rect_equal(const gs_rect& a, const gs_rect& b) {
	if (a.x != b.x || a.y != b.y || a.cx != b.cx || a.cy != b.cy) {
		return false;
	}
	return true;
}

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
	: request_rewind(false), 
	source(source), canvasWidth(0), canvasHeight(0), baseWidth(640), baseHeight(480),
	isActive(true), isVisible(true), videoTicked(true),
	taskHandle(NULL), detectStage(nullptr),	maskDataShutdown(false), 
	introFilename(nullptr),	outroFilename(nullptr),	alertActivate(true), alertDoIntro(false),
	alertDoOutro(false), alertDuration(10.0f),
	alertElapsedTime(BIG_FLOAT), alertTriggered(false), alertShown(false), alertsLoaded(false),
	demoModeOn(false), demoCurrentMask(0),
	demoModeInDelay(false), demoModeGenPreviews(false),	demoModeSavingFrames(false), 
	drawMask(true),	drawAlert(false), drawFaces(false), drawMorphTris(false), drawFDRect(false), 
	filterPreviewMode(false), autoBGRemoval(false), cartoonMode(false), testingStage(nullptr), testMode(false), custom_effect(nullptr){

	PLOG_DEBUG("<%" PRIXPTR "> Initializing...", this);

	obs_enter_graphics();
	sourceRenderTarget = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	detectTexRender = gs_texrender_create(GS_R8, GS_ZS_NONE);
	drawTexRender = gs_texrender_create(GS_RGBA, GS_Z32F); // has depth buffer
	alertTexRender = gs_texrender_create(GS_RGBA, GS_Z32F); // has depth buffer
	obs_leave_graphics();

	// Make the smll stuff
	smllFaceDetector = new smll::FaceDetector();
	smllRenderer = new smll::OBSRenderer(); 

	// set our mm thread task
	if (!taskHandle) {
		DWORD taskIndex = 0;
		taskHandle = AvSetMmThreadCharacteristics(TEXT(MM_THREAD_TASK_NAME), &taskIndex);
		if (taskHandle == NULL) {
			blog(LOG_DEBUG, "[FaceMask] Failed to set MM thread characteristics");
		}
	}

	// initialize face detection thread data
	{
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.shutdown = false;
		detection.facesIndex = -1;
		detection.thread = std::thread(StaticThreadMain, this);
		clearFramesActiveStatus();
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
	smll::TestingPipe* T = nullptr;

	if (testMode) {
		T = &smll::TestingPipe::singleton();
	}
	
	// kill the thread
	if (T) {
		T->SendString("stopping threads");
	}
	{
		std::unique_lock<std::mutex> lock(maskDataMutex);
		maskDataShutdown = true;
	}
	PLOG_DEBUG("<%" PRIXPTR "> Stopping worker Threads...", this);
	{
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.shutdown = true;
	}
	// wait for them to die
	detection.thread.join();
	maskDataThread.join();
	if (T) {
		T->SendString("threads stopped");
	}
	PLOG_DEBUG("<%" PRIXPTR "> Worker Thread stopped.", this);

	obs_enter_graphics();
	gs_texrender_destroy(sourceRenderTarget);
	gs_texrender_destroy(drawTexRender);
	gs_texrender_destroy(alertTexRender);
	gs_texrender_destroy(detectTexRender);
	if (detection.frame.capture.texture) {
		gs_texture_destroy(detection.frame.capture.texture);
	}
	if (testingStage)
		gs_stagesurface_destroy(testingStage);
	if (detectStage)
		gs_stagesurface_destroy(detectStage);
	maskData = nullptr;
	obs_leave_graphics();

	delete smllFaceDetector;
	delete smllRenderer;

	if (taskHandle != NULL) {
		AvRevertMmThreadCharacteristics(taskHandle);
	}

	PLOG_DEBUG("<%" PRIXPTR "> Finalized.", this);
	if (T) {
		T->SendString("filter destroyed");
		T->ClosePipe();
	}
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

	obs_data_set_default_bool(data, P_DEACTIVATE, false);

	char* defMaskFolder = (char*)obs_module_config_path(NULL);
	obs_data_set_default_string(data, P_MASKFOLDER, defMaskFolder);

	obs_data_set_default_string(data, P_MASK, kDefaultMask);
	obs_data_set_default_string(data, P_MASK_BROWSE, kDefaultMask);
	obs_data_set_default_string(data, P_ALERT_INTRO, kDefaultIntro);
	obs_data_set_default_string(data, P_ALERT_OUTRO, kDefaultOutro);

	bfree(defMaskFolder);

	// ALERTS
	obs_data_set_default_bool(data, P_ALERT_ACTIVATE, false);
	obs_data_set_default_double(data, P_ALERT_DURATION, 10.0f);
	obs_data_set_default_bool(data, P_ALERT_DOINTRO, false);
	obs_data_set_default_bool(data, P_ALERT_DOOUTRO, false);

	obs_data_set_default_bool(data, P_CARTOON, false);
	obs_data_set_default_bool(data, P_BGREMOVAL, false);
	obs_data_set_default_bool(data, P_TEST_MODE, false);

	obs_data_set_default_bool(data, P_GENTHUMBS, false);
	obs_data_set_default_bool(data, P_DRAWMASK, false);
	obs_data_set_default_bool(data, P_DRAWALERT, false);
	obs_data_set_default_bool(data, P_DRAWFACEDATA, false);
	obs_data_set_default_bool(data, P_DRAWMORPHTRIS, false);
	obs_data_set_default_bool(data, P_DRAWCROPRECT, false);

	obs_data_set_default_bool(data, P_DEMOMODEON, false);

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

static void add_bool_property(obs_properties_t *props, const char* name) {
	obs_property_t* p = obs_properties_add_bool(props, name, P_TRANSLATE(name));
	std::string n = name; n += ".Description";
	obs_property_set_long_description(p, P_TRANSLATE(n.c_str()));
}

static void add_dummy_property(obs_properties_t *props) {
	obs_property_t* p = obs_properties_add_bool(props, "  ", "  ");
	obs_property_set_visible(p, false);
}

static obs_property_t *add_int_list_property(obs_properties_t *props, const char* name) {
	obs_property_t* p = obs_properties_add_list(props, name, P_TRANSLATE(name), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	std::string n = name; n += ".Description";
	obs_property_set_long_description(p, P_TRANSLATE(n.c_str()));
	return p;
}

static void add_text_property(obs_properties_t *props, const char* name) {
	obs_property_t* p = p = obs_properties_add_text(props, name, P_TRANSLATE(name),
		obs_text_type::OBS_TEXT_DEFAULT);
	std::string n = name; n += ".Description";
	obs_property_set_long_description(p, P_TRANSLATE(n.c_str()));
}

static void add_json_file_property(obs_properties_t *props, const char* name,
	const char* folder) {
	char* defFolder = obs_module_file(folder);
	obs_property_t* p = obs_properties_add_path(props, name, P_TRANSLATE(name),
		obs_path_type::OBS_PATH_FILE,
		"Face Mask JSON (*.json)", defFolder);
	std::string n = name; n += ".Description";
	obs_property_set_long_description(p, P_TRANSLATE(n.c_str()));
	bfree(defFolder);
}

static void add_folder_property(obs_properties_t *props, const char* name,
	const char* folder) {
	obs_property_t* p = obs_properties_add_path(props, name, P_TRANSLATE(name),
		obs_path_type::OBS_PATH_DIRECTORY,
		NULL, folder);
	std::string n = name; n += ".Description";
	obs_property_set_long_description(p, P_TRANSLATE(n.c_str()));
}

static void add_float_slider(obs_properties_t *props, const char* name, float min, float max, float step) {
	obs_property_t* p = obs_properties_add_float_slider(props, name,
		P_TRANSLATE(name), min, max, step);
	std::string n = name; n += ".Description";
	obs_property_set_long_description(p, P_TRANSLATE(n.c_str()));
}


void Plugin::FaceMaskFilter::Instance::get_properties(obs_properties_t *props) {
#if !defined(PUBLIC_RELEASE)
	// mask 
	add_json_file_property(props, P_MASK_BROWSE, NULL);

	// ALERT PROPERTIES
	add_bool_property(props, P_ALERT_ACTIVATE);
	add_float_slider(props, P_ALERT_DURATION, 10.0f, 60.0f, 0.1f);
	add_bool_property(props, P_ALERT_DOINTRO);
	add_bool_property(props, P_ALERT_DOOUTRO);

	add_bool_property(props, P_TEST_MODE);

	// force mask/alert drawing
	add_bool_property(props, P_DRAWMASK);
	add_bool_property(props, P_DRAWALERT);

	// anti-aliasing
	obs_property_t *list = add_int_list_property(props, P_ANTI_ALIASING);
	obs_property_list_add_int(list, P_TRANSLATE(P_NO_ANTI_ALIASING), NO_ANTI_ALIASING);
	obs_property_list_add_int(list, P_TRANSLATE(P_SSAA_ANTI_ALIASING), SSAA_ANTI_ALIASING);
	obs_property_list_add_int(list, P_TRANSLATE(P_FXAA_ANTI_ALIASING), FXAA_ANTI_ALIASING);

	// bg removal
	add_bool_property(props, P_BGREMOVAL);

	// cartoon mode
	add_bool_property(props, P_CARTOON);

	// disable the plugin
	add_bool_property(props, P_DEACTIVATE);

	// rewind button
	obs_properties_add_button(props, P_REWIND, P_TRANSLATE(P_REWIND), 
		rewind_clicked);

	// Demo mode
	add_bool_property(props, P_DEMOMODEON);
	add_text_property(props, P_DEMOFOLDER);

	add_bool_property(props, P_GENTHUMBS);

	// debug drawing flags
	add_bool_property(props, P_DRAWFACEDATA);
	add_bool_property(props, P_DRAWMORPHTRIS);
	add_bool_property(props, P_DRAWCROPRECT);

	// add advanced configuration params
	smll::Config::singleton().get_properties(props);
#else
	//for fixing empty properties bug for endless loading
	add_dummy_property(props);
#endif
}

bool Plugin::FaceMaskFilter::Instance::rewind_clicked(obs_properties_t *pr, obs_property_t *p, void *ptr) {
	if (ptr == nullptr)
		return false;
	return reinterpret_cast<Instance*>(ptr)->rewind_clicked(pr, p);
}

bool Plugin::FaceMaskFilter::Instance::rewind_clicked(obs_properties_t *pr, obs_property_t *p) {
	UNUSED_PARAMETER(pr);
	UNUSED_PARAMETER(p);
	this->request_rewind = true;
	return true;
}

void Plugin::FaceMaskFilter::Instance::update(void *ptr, obs_data_t *data) {
	if (ptr == nullptr)
		return;
	return reinterpret_cast<Instance*>(ptr)->update(data);
}

void Plugin::FaceMaskFilter::Instance::update(obs_data_t *data) {

#if !defined(PUBLIC_RELEASE)
	// update advanced properties
	smll::Config::singleton().update_properties(data);
#endif

	// mask file names
	std::replace(maskFolder.begin(), maskFolder.end(), '/', '\\');

	if (!maskFolder.empty()) {
		char lastChar = maskFolder.back();
		//If slash at the end, remove it
		if (lastChar == '\\') {
			maskFolder.pop_back();
		}
	}

	std::string newMaskFilePath = (char*)obs_data_get_string(data, P_MASK_BROWSE);
	std::replace(newMaskFilePath.begin(), newMaskFilePath.end(), '/', '\\');
	std::string newMaskInternal = (char*)obs_data_get_string(data, P_MASK);
	//if mask internal changed
	if (newMaskInternal != maskInternal) {
		maskInternal = newMaskInternal;
		maskFilename = newMaskInternal;
		maskFolder = obs_data_get_string(data, P_MASKFOLDER);
	}
	//if mask file with path changed
	if(newMaskFilePath != maskFilePath) {
		std::size_t found = newMaskFilePath.find_last_of("\\");
		if (found == string::npos) {
			maskFolder == "";
			maskFilename = newMaskFilePath;
		}
		else {
			maskFolder = newMaskFilePath.substr(0, found);
			maskFilename = newMaskFilePath.substr(found + 1);
		}
		maskFilePath = newMaskFilePath;
	}

	// Flags
	autoBGRemoval = obs_data_get_bool(data, P_BGREMOVAL);
	cartoonMode = obs_data_get_bool(data, P_CARTOON);
	testMode = obs_data_get_bool(data, P_TEST_MODE);

	// Anti-aliasing
	antialiasing_method = obs_data_get_int(data, P_ANTI_ALIASING);

	// Alerts
	bool lastAlertActivate = alertActivate;
	alertActivate = obs_data_get_bool(data, P_ALERT_ACTIVATE);
	alertTriggered = (!lastAlertActivate && alertActivate);
	alertDuration = (float)obs_data_get_double(data, P_ALERT_DURATION);
	alertDoIntro = obs_data_get_bool(data, P_ALERT_DOINTRO);
	alertDoOutro = obs_data_get_bool(data, P_ALERT_DOOUTRO);
	introFilename = (char*)obs_data_get_string(data, P_ALERT_INTRO);
	outroFilename = (char*)obs_data_get_string(data, P_ALERT_OUTRO);
	alertShowDelay = 0; //Default value

	// demo mode
	demoModeOn = obs_data_get_bool(data, P_DEMOMODEON);
	demoModeFolder = obs_data_get_string(data, P_DEMOFOLDER);
	demoModeGenPreviews = obs_data_get_bool(data, P_GENTHUMBS);

	// update our param values
	drawMask = obs_data_get_bool(data, P_DRAWMASK);
	if (alertsLoaded) {
		drawAlert = obs_data_get_bool(data, P_DRAWALERT);
	}
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

	if (!isVisible || !isActive) {
		// *** SKIP TICK ***
		return;
	}

	// ----- GET FACES FROM OTHER THREAD -----
	updateFaces();

	// Lock mask datas mutex
	std::unique_lock<std::mutex> masklock(maskDataMutex, std::try_to_lock);
	if (!masklock.owns_lock()) {
		// *** SKIP TICK ***
		return;
	}

	// Figure out what's going on
	bool introActive = false;
	bool outroActive = false;
	float maskActiveTime = 0.0f;
	float alertOnTime = MASK_FADE_TIME;
	if (alertDoIntro && introData) {
		alertOnTime = introData->GetIntroDuration();
		maskActiveTime = introData->GetIntroDuration() - 
			introData->GetIntroFadeTime();
		if (alertElapsedTime <= introData->GetIntroDuration())
			introActive = true;
	}
	float maskInactiveTime = alertDuration;
	float alertOffTime = alertDuration - MASK_FADE_TIME;
	if (alertDoOutro && outroData) {
		maskInactiveTime -= outroData->GetIntroDuration() -
			outroData->GetIntroFadeTime();
		alertOffTime = alertDuration - outroData->GetIntroDuration();
		if (alertElapsedTime >= (alertDuration - outroData->GetIntroDuration()))
			outroActive = true;
	}
	alertOnTime += alertShowDelay;
	bool maskActive = (alertElapsedTime >= maskActiveTime &&
		alertElapsedTime <= maskInactiveTime);
	if (drawMask)
		maskActive = true;

	// get the right mask data
	Mask::MaskData* mdat = maskData.get();
	if (demoModeOn) {
		if (demoCurrentMask >= 0 && demoCurrentMask < demoMaskDatas.size()) {
			mdat = demoMaskDatas[demoCurrentMask].get();
		}
	}

	// Alert triggered?
	if (alertTriggered) {
		alertElapsedTime = 0.0f;
		// rewind everything
		if (mdat)
			mdat->Rewind();
		if (introData)
			introData->Rewind();
		if (outroData)
			outroData->Rewind();
		alertTriggered = false;
		alertShown = false;
	}

	// mask active?
	if (maskActive) {
		if (mdat) {
			// rewind
			if (request_rewind) {
				mdat->Rewind();
				request_rewind = false;
			}
			// tick main mask
			mdat->Tick(timeDelta);
		}
	}

	// Tick the alerts
	if (alertsLoaded) {
		alertElapsedTime += timeDelta;
	}

	// Tick the intro/outro
	if (introActive) {
		introData->Tick(timeDelta);
	}
	if (outroActive) {
		outroData->Tick(timeDelta);
	}
}

/*
 * Sets frames active status to false
 */
void Plugin::FaceMaskFilter::Instance::clearFramesActiveStatus() {
	detection.frame.active = false;
}

void Plugin::FaceMaskFilter::Instance::video_render(void *ptr,
	gs_effect_t *effect) {
	if (ptr == nullptr)
		return;
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

void Plugin::FaceMaskFilter::Instance::video_render(gs_effect_t *effect) {

	// Skip rendering if inactive or invisible.
	if (!isActive || !isVisible || 
		// or if the alert is done
		(!drawMask && alertElapsedTime > alertDuration)) {
		// reset the buffer
		std::unique_lock<std::mutex> lock(detection.mutex);
		detection.facesIndex = -1;
		// reset the detected faces
		clearFramesActiveStatus();
		smllFaceDetector->ResetFaces();
		faces.length = 0;
		// make sure file loads still happen
		checkForMaskUnloading();
		// *** SKIP ***
		obs_source_skip_video_filter(source);
		return;
	}

	// Grab parent and target source.
	obs_source_t *parent = obs_filter_get_parent(source);
	obs_source_t *target = obs_filter_get_target(source);
	if ((parent == NULL) || (target == NULL)) {
		// *** SKIP ***
		obs_source_skip_video_filter(source);
		return;
	}

	// Target base width and height.
	baseWidth = obs_source_get_base_width(target);
	baseHeight = obs_source_get_base_height(target);
	if ((baseWidth <= 0) || (baseHeight <= 0)) {
		// *** SKIP ***
		obs_source_skip_video_filter(source);
		return;
	}

	// Effects
	gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Render source frame to a texture
	gs_texture* sourceTexture = RenderSourceTexture(effect ? effect : defaultEffect);
	if (sourceTexture == NULL) {
		// *** SKIP ***
		obs_source_skip_video_filter(source);
		return;
	}

	// smll needs a "viewport" to draw
	smllRenderer->SetViewport(baseWidth, baseHeight);

	// ----- SEND FRAME TO FACE DETECTION THREAD -----
	SendSourceTextureToThread(sourceTexture);

	// Get mask data mutex
	std::unique_lock<std::mutex> masklock(maskDataMutex, std::try_to_lock);
	if (!masklock.owns_lock()) {
		// *** SKIP RENDERING ***
		obs_source_skip_video_filter(source);
		return;
	}

	// ----- DRAW -----

	// OBS rendering state
	gs_blend_state_push();
	setupRenderingState();

	// get mask data to draw
	Mask::MaskData* mask_data = maskData.get();
	if (demoModeOn && demoMaskDatas.size() > 0) {
		if (demoCurrentMask >= 0 &&
			demoCurrentMask < demoMaskDatas.size()) {
			mask_data = demoMaskDatas[demoCurrentMask].get();
		}
	}

	// set up alphas
	bool introActive = false;
	bool outroActive = false;
	float maskAlpha = 1.0f;
	if (alertDoIntro && introData) {
		float t1 = introData->GetIntroDuration() -
			introData->GetIntroFadeTime();
		float t2 = introData->GetIntroDuration();
		if (alertElapsedTime < t1)
			maskAlpha = 0.0f;
		else if (alertElapsedTime < t2)
			maskAlpha = Utils::hermite((alertElapsedTime - t1) / (t2 - t1), 0.0f, 1.0f);
		if (alertElapsedTime <= introData->GetIntroDuration())
			introActive = true;
	}
	else {
		if (alertElapsedTime < MASK_FADE_TIME)
			maskAlpha = Utils::hermite(alertElapsedTime / MASK_FADE_TIME, 0.0f, 1.0f);
	}
	float outroDuration = MASK_FADE_TIME;
	if (alertDoOutro && outroData) {
		outroDuration = outroData->GetIntroDuration();
		float t1 = alertDuration - outroData->GetIntroDuration();
		float t2 = t1 + outroData->GetIntroFadeTime();
		if (alertElapsedTime > t2)
			maskAlpha = 0.0f;
		else if (alertElapsedTime > t1)
			maskAlpha = Utils::hermite((alertElapsedTime - t1) / (t2 - t1), 1.0f, 0.0f);
		if (alertElapsedTime < alertDuration && 
			alertElapsedTime >= (alertDuration - outroData->GetIntroDuration()))
			outroActive = true;
	}
	else {
		float t = alertDuration - MASK_FADE_TIME;
		if (alertElapsedTime > alertDuration)
			maskAlpha = 0.0f;
		else if (alertElapsedTime > t)
			maskAlpha = Utils::hermite((alertElapsedTime - t) / MASK_FADE_TIME, 1.0f, 0.0f);
	}
	if (drawMask)
		maskAlpha = 1.0f;
	if (mask_data) {
		mask_data->SetGlobalAlpha(maskAlpha);
	}

	// Draw always current frame to be up to date, even it's processing is delayd
	gs_texture_t* vidTex = sourceTexture;

	// some reasons triangulation should be destroyed
	if (!mask_data || faces.length == 0) {
		triangulation.DestroyBuffers();
	}

	// flags
	bool genThumbs = mask_data && demoModeOn && demoModeGenPreviews && demoModeSavingFrames;

	// Draw the source video
	gs_enable_depth_test(false);
	gs_set_cull_mode(GS_NEITHER);
	while (gs_effect_loop(defaultEffect, "Draw")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
			"image"), vidTex);
		gs_draw_sprite(vidTex, 0, baseWidth, baseHeight);
	}

	// Get current method to use for anti-aliasing
	if (antialiasing_method == NO_ANTI_ALIASING ||
		antialiasing_method == FXAA_ANTI_ALIASING)
		m_scale_rate = 1;
	else
		m_scale_rate = SSAA_UPSAMPLE_FACTOR;


	// render mask to texture
	gs_texture* mask_tex = nullptr;
	if (faces.length > 0) {
		// only render once per video tick
		if (videoTicked) {

			//init start pose for static masks
			for (int i = 0; i < faces.length; i++) {
				faces[i].InitStartPose();
			}

			// draw mask to texture
			gs_texrender_reset(drawTexRender);
			if (gs_texrender_begin(drawTexRender, baseWidth*m_scale_rate, baseHeight*m_scale_rate)) {

				// clear
				vec4 black, thumbbg;
				vec4_zero(&black);
				float vv = (float)0x9a / 255.0f;
				vec4_set(&thumbbg, vv, vv, vv, 1.0f);
				gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 1.0f, 0);

				if (mask_data) {

					// Swap texture with video texture :P
					// TODO: we'd prefer to have the mask drawn on top
					// TODO: this should be a feature!!
					//std::shared_ptr<Mask::Resource::Image> img = std::dynamic_pointer_cast<Mask::Resource::Image>
					//	(mask_data->GetResource("diffuse-1"));
					//if (img)
					//	img->SwapTexture(vidTex);

					// Check here for no morph
					if (!mask_data->GetMorph()) {
						triangulation.DestroyBuffers();
					}

					// Draw depth-only stuff
					for (int i = 0; i < faces.length; i++) {
						gs_matrix_push();
						setFaceTransform(faces[i].pose);
						drawMaskData(mask_data, true, false, false);
						drawMaskData(mask_data, true, true, false);
						setFaceTransform(faces[i].pose, true);
						drawMaskData(mask_data, true, false, true);
						drawMaskData(mask_data, true, true, true);
						gs_matrix_pop();
					}

					// if we are generating thumbs
					if (genThumbs) {
						// clear the color buffer (leaving depth info there)
						gs_clear(GS_CLEAR_COLOR, &thumbbg, 1.0f, 0);
					}
					else {
						// clear the color buffer (leaving depth info there)
						gs_clear(GS_CLEAR_COLOR, &black, 1.0f, 0);
					}

					// draw video to the mask texture?
					if (genThumbs || mask_data->DrawVideoWithMask()) {
						// setup transform state
						gs_viewport_push();
						gs_projection_push();
						gs_matrix_push();
						gs_set_viewport(0, 0, baseWidth, baseHeight);
						gs_ortho(0, (float)baseWidth, 0, (float)baseHeight, -1, 1);
						gs_matrix_identity();

						// Draw the source video
						if (mask_data && (autoBGRemoval || cartoonMode || demoModeOn)) {
							triangulation.autoBGRemoval = autoBGRemoval;
							triangulation.cartoonMode = cartoonMode;
							mask_data->RenderMorphVideo(vidTex, baseWidth, baseHeight, triangulation);
						}

						// restore transform state
						gs_matrix_pop();
						gs_viewport_pop();
						gs_projection_pop();
					}

					// Draw regular stuff
					for (int i = 0; i < faces.length; i++) {
						if (maskAlpha > 0.0f) {
							gs_matrix_push();
							setFaceTransform(faces[i].startPose);
							drawMaskData(mask_data, false, true, false);
							setFaceTransform(faces[i].startPose, true);
							drawMaskData(mask_data, false, true, true);
							setFaceTransform(faces[i].pose);
							drawMaskData(mask_data, false, false, false);
							setFaceTransform(faces[i].pose, true);
							drawMaskData(mask_data, false, false, true);
							gs_matrix_pop();
						}
					}

					if (introActive || outroActive)
						gs_clear(GS_CLEAR_DEPTH, &black, 1.0f, 0);

					for (int i = 0; i < faces.length; i++) {
						if (introActive) {
							gs_matrix_push();
							setFaceTransform(faces[i].pose, true);
							drawMaskData(introData.get(), false, false, true);
							gs_matrix_pop();
						}
						if (outroActive) {
							gs_matrix_push();
							setFaceTransform(faces[i].pose, true);
							drawMaskData(outroData.get(), false, false, true);
							gs_matrix_pop();
						}
					}
				}

				// draw face detection data
				if (drawFaces)
					smllRenderer->DrawFaces(faces);

				gs_texrender_end(drawTexRender);
			}
		}

		mask_tex = gs_texrender_get_texture(drawTexRender);

	}

	if (testMode) {
		if (faces.length > 0) {

			dlib::point pos = faces[0].GetPosition();
			smll::ThreeDPose pose = faces[0].pose;

			if (!testingStage) {
				testingStage = gs_stagesurface_create(baseWidth, baseHeight, GS_RGBA);
			}
			gs_stage_texture(testingStage, mask_tex);
			uint8_t *data; uint32_t linesize;
			if (gs_stagesurface_map(testingStage, &data, &linesize)) {

				uint8_t* pixel = data + (pos.y() * linesize) + (pos.x() * 4);
				uint8_t red = *pixel++;
				uint8_t green = *pixel++;
				uint8_t blue = *pixel++;
				uint8_t alpha = *pixel++;

				char buf[128];
				snprintf(buf, sizeof(buf), "detected pixel %d,%d,%d,%d",
					(int)red, (int)green, (int)blue, (int)alpha);
				smll::TestingPipe::singleton().SendString(buf);

				snprintf(buf, sizeof(buf), "Pose Translations %d,%d,%d",
					(int)pose.translation[0], (int)pose.translation[1], (int)pose.translation[2]);
				smll::TestingPipe::singleton().SendString(buf);

				snprintf(buf, sizeof(buf), "Mask %s", maskFilename.c_str());
				smll::TestingPipe::singleton().SendString(buf);

				gs_stagesurface_unmap(testingStage);
			}
		}
	}

	// SPRITE DRAWING - draw rendered stuff as sprites

	// set up for sprite rendering
	gs_set_cull_mode(GS_NEITHER);
	gs_enable_blending(true);
	gs_enable_depth_test(false);
	gs_enable_color(true, true, true, true);
	gs_blend_function(gs_blend_type::GS_BLEND_SRCALPHA,
		gs_blend_type::GS_BLEND_INVSRCALPHA);

	// Draw the source video:
	// - if we are not drawing the video with the mask, then we need
	//   to draw video now.
	if (!mask_data ||
		(!mask_data->DrawVideoWithMask() &&
			!genThumbs)) {

		// Draw the source video
		if (mask_data) {
			triangulation.autoBGRemoval = autoBGRemoval;
			triangulation.cartoonMode = cartoonMode;
			mask_data->RenderMorphVideo(vidTex, baseWidth, baseHeight, triangulation);
		}
		else {
			// Draw the source video
			while (gs_effect_loop(defaultEffect, "Draw")) {
				gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
					"image"), vidTex);
				gs_draw_sprite(vidTex, 0, baseWidth, baseHeight);
			}
		}
	}

	// TEST DRAW EFFECT
	if (custom_effect == nullptr) {
		char* f = obs_module_file("effects/aa.effect");
		char* errorMessage = nullptr;
		custom_effect = gs_effect_create_from_file(f, &errorMessage);
		if (custom_effect) {
			gs_effect_set_float(gs_effect_get_param_by_name(custom_effect, "inv_width"), 1.0f / (baseWidth*m_scale_rate));
			gs_effect_set_float(gs_effect_get_param_by_name(custom_effect, "inv_height"), 1.0f / (baseHeight*m_scale_rate));
		}
		if (f) {
			bfree(f);
		}
	}

	// Draw the rendered Mask
	if (mask_tex && custom_effect) {
		gs_effect_set_int(gs_effect_get_param_by_name(custom_effect, "antialiasing_method"), antialiasing_method);

		while (gs_effect_loop(custom_effect, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(custom_effect,
				"image"), mask_tex);
			gs_draw_sprite(mask_tex, 0, baseWidth, baseHeight);
		}
	}
	

	// draw face detection data
	if (drawFaces)
		smllRenderer->DrawFaces(faces);

	// draw crop rectangles
	drawCropRects(baseWidth, baseHeight);

	// demo mode render stuff
	demoModeRender(vidTex, mask_tex, mask_data);

	// restore rendering state
	gs_blend_state_pop();

	// since we are on the gpu right now anyway, here is 
	// a good spot to unload mask data if we need to.
	checkForMaskUnloading();

	videoTicked = false;
}

void Plugin::FaceMaskFilter::Instance::checkForMaskUnloading() {
	// Check for file/folder changes
	if (currentMaskFilename != maskFilename) {
		maskData = nullptr;
	}
	if (introFilename && currentIntroFilename != introFilename) {
		introData = nullptr;
	}
	if (outroFilename && currentOutroFilename != outroFilename) {
		outroData = nullptr;
	}

	if (currentMaskFolder != maskFolder) {
		maskData = nullptr;
		introData = nullptr;
		outroData = nullptr;
	}
}

void Plugin::FaceMaskFilter::Instance::demoModeRender(gs_texture* vidTex, gs_texture* maskTex, 
	Mask::MaskData* mask_data) {

	// generate previews?
	if (demoModeOn && videoTicked && demoModeGenPreviews && demoMaskDatas.size() > 0) {

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
				mask_data->Rewind();
			}
			else if (isRed) {
				// done
				WritePreviewFrames();
				demoModeSavingFrames = false;
				//increase mask number, change mask
				demoCurrentMask = (demoCurrentMask + 1) % demoMaskDatas.size();
				demoModeInDelay = false;
			}
			else {
				PreviewFrame pf(maskTex, baseWidth, baseHeight);
				previewFrames.emplace_back(pf);
			}
		}
		else if (isRed) {
			if (demoModeInDelay) {
				// ready to go
				mask_data->Rewind();
				demoModeSavingFrames = true;
				demoModeInDelay = false;
			}
		}
		else {
			//wait one cycle
			demoModeInDelay = true;
		}
	}
}

bool Plugin::FaceMaskFilter::Instance::SendSourceTextureToThread(gs_texture* sourceTexture) {

	// only if first render after video tick
	if (!videoTicked)
		return false;

	// timestamp for this frame
	TimeStamp sourceTimestamp = NEW_TIMESTAMP;
	bool frameSent = false;

	// if there's already an active frame, bail
	if (detection.frame.active)
		return false;

	// lock current frame
	{
		std::unique_lock<std::mutex> lock(detection.frame.mutex,
			std::try_to_lock);
		if (lock.owns_lock()) {
			frameSent = true;

			smll::OBSTexture& capture = detection.frame.capture;
			smll::ImageWrapper& detect = detection.frame.detect;

			detection.frame.active = true;
			detection.frame.timestamp = sourceTimestamp;

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

					if (USE_FAST_MEMCPY)
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
				if (morph->GetMorphData().IsNewerThan(detection.frame.morphData) || demoModeOn) {
					detection.frame.morphData = morph->GetMorphData();
				}
			}
			else {
				// Make sure current is invalid
				detection.frame.morphData.Invalidate();
			}
		}
	}

	// Advance frame index if we copied a frame
	if (frameSent) {
		std::unique_lock<std::mutex> lock(detection.mutex);
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

void Plugin::FaceMaskFilter::Instance::setupRenderingState() {

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

	// set up initial rendering state
	gs_enable_stencil_test(false);
	gs_enable_depth_test(false);
	gs_depth_function(GS_ALWAYS);
	gs_set_cull_mode(GS_NEITHER);
	gs_enable_color(true, true, true, true);
	gs_enable_blending(true);
	gs_blend_function(gs_blend_type::GS_BLEND_SRCALPHA,
		gs_blend_type::GS_BLEND_INVSRCALPHA);
}


void Plugin::FaceMaskFilter::Instance::setFaceTransform(const smll::ThreeDPose& pose,
	bool billboard) {

	gs_matrix_identity();
	gs_matrix_translate3f((float)pose.translation[0],
		(float)pose.translation[1], (float)-pose.translation[2]);
	if (!billboard) {
		gs_matrix_rotaa4f((float)pose.rotation[0], (float)pose.rotation[1],
			(float)-pose.rotation[2], (float)-pose.rotation[3]);
	}
}


void Plugin::FaceMaskFilter::Instance::drawMaskData(Mask::MaskData*	_maskData, 
	bool depthOnly, bool staticOnly, bool rotationDisable){

	gs_viewport_push();
	gs_projection_push();

	uint32_t w = baseWidth*m_scale_rate;
	uint32_t h = baseHeight*m_scale_rate;

	gs_set_viewport(0, 0, w, h);
	gs_enable_depth_test(true);
	gs_depth_function(GS_LESS);

	float aspect = (float)w / (float)h;
	gs_perspective(FOVA(aspect), aspect, NEAR_Z, FAR_Z);

	_maskData->Render(depthOnly, staticOnly, rotationDisable);

	gs_projection_pop();
	gs_viewport_pop();
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
		{
			std::unique_lock<std::mutex> lock(detection.mutex);
			shutdown = detection.shutdown;
		}
		if (shutdown) break;
		if (!detection.frame.active) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		smll::DetectionResults detect_results;

		bool skipped = false;
		{
			std::unique_lock<std::mutex> lock(detection.frame.mutex);

			// check to see if we are detecting the same frame as last time
			if (lastTimestamp == detection.frame.timestamp) {
				// same frame, skip
				skipped = true;
			}
			else {
				// new frame - do the face detection
				smllFaceDetector->DetectFaces(detection.frame.detect, detection.frame.capture, detect_results);
				if (!STUFF_ON_MAIN_THREAD) {
					// Now do the landmark detection & pose estimation
					smllFaceDetector->DetectLandmarks(detection.frame.capture, detect_results);
					smllFaceDetector->DoPoseEstimation(detect_results);
				}

				lastTimestamp = detection.frame.timestamp;
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
				std::unique_lock<std::mutex> framelock(detection.frame.mutex);

				// Make the triangulation
				detection.faces[face_idx].triangulationResults.buildLines = drawMorphTris;
				smllFaceDetector->MakeTriangulation(detection.frame.morphData,
					detect_results, detection.faces[face_idx].triangulationResults);

				detection.frame.active = false;
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

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

	alertsLoaded = true;

	// Loading loop
	bool lastDemoMode = false; 
	while (true) {
		{
			std::unique_lock<std::mutex> lock(maskDataMutex, std::try_to_lock);
			if (lock.owns_lock()) {
				if (maskDataShutdown) {
					break;
				}
				// time to load mask?
				if ((maskData == nullptr) &&
					maskFilename.length() > 0) {
					// save current
					currentMaskFilename = maskFilename;
					currentMaskFolder = maskFolder;
					// mask filename
					std::string maskFn = currentMaskFolder + "\\" + currentMaskFilename;
					// load mask
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
					maskData = std::unique_ptr<Mask::MaskData>(LoadMask(maskFn));
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
				}

				// time to load intro?
				if ((introData == nullptr) &&
					introFilename && introFilename[0]) {
					// save current
					currentIntroFilename = introFilename;
					currentMaskFolder = maskFolder;
					// mask filename
					std::string maskFn = currentMaskFolder + "\\" + currentIntroFilename;
					// load mask
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
					introData = std::unique_ptr<Mask::MaskData>(LoadMask(maskFn));
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
				}

				// time to load outro?
				if ((outroData == nullptr) && 
					outroFilename && outroFilename[0]) {
					// save current
					currentOutroFilename = outroFilename;
					currentMaskFolder = maskFolder;
					// mask filename
					std::string maskFn = currentMaskFolder + "\\" + currentOutroFilename;
					// load mask
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
					outroData = std::unique_ptr<Mask::MaskData>(LoadMask(maskFn));
					SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
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
			addMask = (::PathFileExists(Utils::ConvertStringToWstring(gifname).c_str()) != TRUE);

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
	demoModeSavingFrames = false;
}


Mask::MaskData*
Plugin::FaceMaskFilter::Instance::LoadMask(std::string filename) {

	PLOG_INFO("Loading mask json '%s'...", filename.c_str());

	// new mask data
	Mask::MaskData* mdat = new Mask::MaskData();

	// load the json
	try {
		mdat->Load(filename);
		PLOG_INFO("Loading mask '%s' successful!", filename.c_str());
	}
	catch (...) {
		PLOG_ERROR("Failed to load mask %s.", filename.c_str());
	}

	return mdat;
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

			// TEST MODE ONLY : output for testing
			if (testMode) {
				char b[128];
				snprintf(b, sizeof(b), "%d faces detected", newFaces.length);
				smll::TestingPipe::singleton().SendString(b);
				for (int i = 0; i < newFaces.length; i++) {

					dlib::point pos = newFaces[i].GetPosition();
					snprintf(b, sizeof(b), "face detected at %ld,%ld", pos.x(), pos.y());
					smll::TestingPipe::singleton().SendString(b);
				}
			}

			// new triangulation
			triangulation.TakeBuffersFrom(detection.faces[fidx].triangulationResults);
			if (!drawMorphTris) {
				triangulation.DestroyLineBuffer();
			}

			// new timestamp
			timestamp = NEW_TIMESTAMP;

			// update our results
			faces.CorrelateAndUpdateFrom(newFaces);
		}
	}
}

void Plugin::FaceMaskFilter::Instance::WritePreviewFrames() {

	if (demoMaskFilenames.size() == 0)
		return;

	obs_enter_graphics();

	// if the gif already exists, clean up and bail
	std::string gifname = demoMaskFilenames[demoCurrentMask].substr(0, demoMaskFilenames[demoCurrentMask].length() - 4) + "gif";
	if (::PathFileExists(Utils::ConvertStringToWstring(gifname).c_str()) == TRUE) {
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
	::CreateDirectory(Utils::ConvertStringToWstring(outFolder).c_str(), NULL);

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

void Plugin::FaceMaskFilter::Instance::WriteTextureToFile(gs_texture* tex, std::string filename) {
	cv::Mat vidf(baseWidth, baseHeight, CV_8UC4);

	if (!testingStage) {
		testingStage = gs_stagesurface_create(baseWidth, baseHeight, GS_RGBA);
	}

	// get vid tex
	gs_stage_texture(testingStage, tex);
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

	// wrap & write
	cv::imwrite(filename.c_str(), vidf);
}

Plugin::FaceMaskFilter::Instance::PreviewFrame::PreviewFrame(gs_texture_t* v, 
	int w, int h) {
	obs_enter_graphics();
	gs_color_format fmt = gs_texture_get_color_format(v);
	vidtex = gs_texture_create(w, h, fmt, 1, 0, 0);
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

