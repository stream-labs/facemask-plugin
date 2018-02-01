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

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )

#include "FaceDetector.hpp"
#include "OBSRenderer.hpp"
#include "DLibImageWrapper.hpp"

#include <libobs/obs-module.h>

#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <vector>

// yep...this is what we're doin
#include <libobs/util/threaded-memcpy.c>


#pragma warning( pop )

#define FOCAL_LENGTH_FACTOR		(1.0f)
#define POSE_RESET_INTERVAL		(30)


using namespace dlib;
using namespace std;


namespace smll {

	FaceDetector::FaceDetector(const char* predictorFilename)
		: m_captureStage(nullptr)
		, m_detectStage(nullptr)
		, m_trackingStage(nullptr)
		, m_stageSize(0)
		, m_stageType(SFT_UNDEFINED)
		, m_memcpyEnv(nullptr)
		, m_timeout(0)
        , m_trackingTimeout(0)
        , m_detectionTimeout(0)
		, m_trackingFaceIndex(0) {
		// Load face detection and pose estimation models.
		m_detector = get_frontal_face_detector();
		deserialize(predictorFilename) >> m_predictor;

		m_memcpyEnv = init_threaded_memcpy_pool(0);
	}

	FaceDetector::~FaceDetector() {
		obs_enter_graphics();
		if (m_detectStage)
			gs_stagesurface_destroy(m_detectStage);
		if (m_trackingStage)
			gs_stagesurface_destroy(m_trackingStage);
		if (m_captureStage)
			gs_stagesurface_destroy(m_captureStage);
		obs_leave_graphics();

		if (m_stageWrapper.data)
			delete[] m_stageWrapper.data;
		if (m_memcpyEnv)
			destroy_threaded_memcpy_pool(m_memcpyEnv);
	}

	void FaceDetector::DetectFaces(const OBSTexture& capture, 
		const OBSTexture& detect, const OBSTexture& track) {
		// nothing is staged
		m_stageType = SFT_UNDEFINED;

		// paranoid
		if (m_faces.length == 0)
			InvalidatePoses();

		// do nothing at all timeout
		if (m_timeout > 0) {
			m_timeout--;
			return;
		}
        // face detection timeout
        if (m_detectionTimeout > 0) {
            m_detectionTimeout--;
        }
        // tracking timeouut
        if (m_trackingTimeout > 0) {
            m_trackingTimeout--;
        }

        // better check if the camera res has changed on us
        if ((capture.width != m_capture.width) || 
			(capture.height != m_capture.height) ||
			(track.width != m_track.width) ||
			(track.height != m_track.height)) {
            // forget whatever we thought were faces
            m_faces.length = 0;
			InvalidatePoses();
        }

		// save frames
		m_capture = capture;
		m_track = track;
		m_detect = detect;
        
        // what are we doing here
        bool doTracking = (m_faces.length > 0) &&
			Config::singleton().get_bool(CONFIG_BOOL_TRACKING_ENABLE) &&
			(m_trackingTimeout == 0);
        bool doFaceDetection = 
			Config::singleton().get_bool(CONFIG_BOOL_FACE_DETECT_ENABLE) && 
			(m_detectionTimeout == 0);
        
		// TRACK faces 
		//
		if (doTracking) {

            UpdateObjectTracking();

            // Detect landmarks
            DetectLandmarks();
            
            // Is Tracking is still good?
            if (m_faces.length > 0) {
				// next face for tracking time-slicing
				m_trackingFaceIndex = (m_trackingFaceIndex + 1) % m_faces.length;
                
                // tracking frequency
                m_trackingTimeout = 
					Config::singleton().get_int(CONFIG_INT_TRACKING_FREQUNCY);
			} else {
                // Tracking is bum, do face detect next time
				InvalidatePoses();
				m_timeout = 0;
                m_detectionTimeout = 0;
                m_trackingFaceIndex = 0;
            }
            
            // If tracking is good, we're done
            //
            // If tracking is no good, we STILL don't want to detect faces on
            // the same frame, so bail and go next time
            return;
		}

		// Do FACIAL DETECTION
        bool startTracking = false;
        if (doFaceDetection) {
            DoFaceDetection();
            m_detectionTimeout = 
				Config::singleton().get_int(
					CONFIG_INT_FACE_DETECT_RECHECK_FREQUENCY);
            startTracking = true;
        } else {
            // if face detection is disabled, we'll never have any faces to
            // track or predict on unless we make a fixed rect to start with
            //
			if (m_faces.length == 0 &&
				!Config::singleton().get_int(CONFIG_BOOL_FACE_DETECT_ENABLE)) {
                // set up a rect we assume the face is in
				m_faces.length = 1;
                int halfwidth = (int)((float)m_capture.width * 
					Config::singleton().get_double(
						CONFIG_DOUBLE_FIXED_RECT_WIDTH)) / 2;
                int x = (m_capture.width / 2) + 
					(int)((float)(m_capture.width / 2) * 
						Config::singleton().get_double(
							CONFIG_DOUBLE_FIXED_RECT_X));
                int y = (m_capture.height / 2) + 
					(int)((float)(m_capture.height / 2) * 
						Config::singleton().get_double(
							CONFIG_DOUBLE_FIXED_RECT_Y));
                
                m_faces[0].m_bounds.set_left(x - halfwidth);
                m_faces[0].m_bounds.set_right(x + halfwidth);
                m_faces[0].m_bounds.set_bottom(y + halfwidth);
                m_faces[0].m_bounds.set_top(y - halfwidth);
                startTracking = true;
            }
        }
        
		// Start Object Tracking
        if (startTracking && 
			Config::singleton().get_bool(CONFIG_BOOL_TRACKING_ENABLE)) {
            StartObjectTracking();
        }
        
        // detect landmarks
        DetectLandmarks();

		if (m_faces.length == 0) {
            // no faces found...set the do nothing timeout and 
			// ensure face detection next frame
            m_timeout = Config::singleton().get_int(
				CONFIG_INT_FACE_DETECT_FREQUENCY);
            m_detectionTimeout = 0;
		}
	}

	void FaceDetector::MakeTriangulation(const MorphData& morphData, 
		TriangulationResult& result) {

		// clear buffers
		result.DestroyBuffers();

		// need valid morph data
		if (!morphData.IsValid())
			return;

		// only 1 face supported
		if (m_faces.length == 0)
			return;
		Face& face = m_faces[0];

		// save capture width and height
		float width = (float)CaptureWidth();
		float height = (float)CaptureHeight();

		// list of points for triangulation
		std::vector<cv::Point2f> points;

		// add facial landmark points
		dlib::point* facePoints = face.m_points;
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			points.push_back(cv::Point2f((float)facePoints[i].x(), 
				(float)facePoints[i].y()));
		}
		
		// add extra points
		std::vector<cv::Point2f> extrapoints;
		extrapoints.push_back(cv::Point2f(0, 0));
		extrapoints.push_back(cv::Point2f(width, 0));
		extrapoints.push_back(cv::Point2f(width, height));
		extrapoints.push_back(cv::Point2f(0, height));
		// no need to add extra border points...in case we do
		//Subdivide(extrapoints);
		//Subdivide(extrapoints);
		//Subdivide(extrapoints);
		//Subdivide(extrapoints);
		points.insert(points.end(), extrapoints.begin(), extrapoints.end());
		
		// copy points into warped points
		std::vector<cv::Point2f> warpedpoints = points;
		
		// Camera internals
		// Approximate focal length.
		double focal_length = (double)width * FOCAL_LENGTH_FACTOR;
		cv::Point2d center = cv::Point2d(width / 2, height / 2);
		cv::Mat camera_matrix =
			(cv::Mat_<double>(3, 3) <<
				focal_length, 0, center.x, 0, focal_length, center.y, 0, 0, 1);
		// We assume no lens distortion
		cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type);

		// project the deltas
		const cv::Mat& rot = face.GetCVRotation();
		cv::Mat trx = face.GetCVTranslation();
		trx.at<double>(0, 0) = 0.0; // clear x & y
		trx.at<double>(1, 0) = 0.0;
		std::vector<cv::Point3f> deltas = morphData.GetCVDeltas();
		std::vector<cv::Point2f> projectedDeltas;
		cv::projectPoints(deltas, rot, trx, camera_matrix, dist_coeffs, projectedDeltas);

		// apply the morph deltas
		cv::Point2f c(width / 2, height / 2);
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			// offset from center			
			warpedpoints[i] += projectedDeltas[i] - c;
		}

		// create the openCV Subdiv2D object
		cv::Rect rect(0, 0, CaptureWidth() + 1, CaptureHeight() + 1);
		cv::Subdiv2D subdiv(rect);

		// add our points to subdiv2d
		// save a map: subdiv2d vtx id -> index into our points
		std::map<int,int> vtxMap;
		for (int i = 0; i < points.size(); i++) {
			cv::Point2f& p = points[i];
			if (rect.contains(p)) {
				// note: this crashes if you insert a point outside the rect.
				int vid = subdiv.insert(p);
				cv::Point2f k = subdiv.getVertex(vid);
				vtxMap[vid] = i;
			}
		}

		// make the vertex buffer using order from subdiv2d
		obs_enter_graphics();
		gs_render_start(true);
		int nv = subdiv.getNumVertices();
		for (int i = 0; i < nv; i++) {
			cv::Point2f p, uv;
			if (i < 4) {
				// first 4 points (the corners) get sent out to
				// bloody nowheresville (3*max dimension beyond borders)
				// odd, but hey
				p = subdiv.getVertex(i);
				if (p.x < 0) p.x = 0;
				if (p.y < 0) p.y = 0;
				if (p.x > width) p.x = width;
				if (p.y > height) p.y = height;
				uv = p;
			}
			else {
				// position from warped points
				// uv from original points
				p = warpedpoints[vtxMap[i]];
				uv = points[vtxMap[i]];
			}
			// add point and uv
			gs_texcoord(uv.x / width, uv.y / height, 0);
			gs_vertex2f(p.x, p.y);
		}
		result.triangulationVB = gs_render_save();
		obs_leave_graphics();

		// get triangulation
		std::vector<cv::Vec3i>	triangleList;
		subdiv.getTriangleIndexList(triangleList);

		// make lines
		if (result.buildLines) {
			// convert to lines
			std::vector<uint32_t> linesList;
			for (auto t : triangleList) {

				int i0 = t[0];
				int i1 = t[1];
				int i2 = t[2];
				linesList.push_back(i0);
				linesList.push_back(i1);
				linesList.push_back(i1);
				linesList.push_back(i2);
				linesList.push_back(i2);
				linesList.push_back(i0);
			}

			// make index buffer
			obs_enter_graphics();
			result.linesIB = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG,
				linesList.data(), linesList.size(), GS_DYNAMIC);
			obs_leave_graphics();
		}

		// make index buffer
		obs_enter_graphics();
		result.triangulationIB = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG,
			triangleList.data(), triangleList.size() * 3, GS_DYNAMIC);
		obs_leave_graphics();
	}

	void FaceDetector::Subdivide(std::vector<cv::Point2f>& points) {
		for (unsigned int i = 0; i < points.size(); i++) {
			int i2 = (i + 1) % points.size();
			points.insert(points.begin()+i2, cv::Point2f(
				(points[i].x + points[i2].x) / 2.0f,
				(points[i].y + points[i2].y) / 2.0f));
			i++;
		}
	}

	void FaceDetector::CatmullRomSmooth(std::vector<cv::Point2f>& points, int steps) {
		if (points.size() < 3)
			return;

		float dt = 1.0f / (float)steps;

		std::vector<cv::Point2f> spline;
		float x, y;
		size_t i0, i1, i2, i3;
		size_t count = points.size() - 1;
		for (unsigned int i = 0; i <= count; i++) {
			if (i == 0) {
				i0 = i;
				i1 = i;
				i2 = i + 1;
				i3 = i + 2;
			}
			else if (i == count) {
				i0 = count - 2;
				i1 = count - 1;
				i2 = count;
				i3 = count;
			}
			else {
				i0 = i - 1;
				i1 = i;
				i2 = i + 1;
				i3 = i + 2;
			}
			const cv::Point2f& p0 = points[i0];
			const cv::Point2f& p1 = points[i1];
			const cv::Point2f& p2 = points[i2];
			const cv::Point2f& p3 = points[i3];

			// TODO: we have more than a couple of SSE-enabled math libraries on tap
			//       we should be using one here
			//       ie) dlib/openCV/libOBS
			//
			for (float t = 0.0f; t <= 1.0f; t += dt) {
				float t2 = t * t;
				float t3 = t2 * t;

				x = 0.5f * 
					((2.0f * p1.x) +
					 (p2.x - p0.x) * t +
					 (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
					 (3.0f * p1.x - p0.x - 3.0f * p2.x + p3.x) * t3);
					
				y = 0.5f *
					((2.0f * p1.y) +
					 (p2.y - p0.y) * t +
					 (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
					 (3.0f * p1.y - p0.y - 3.0f * p2.y + p3.y) * t3);

				if (!(spline.size() && spline[spline.size() - 1].x == x && spline[spline.size() - 1].y == y)) {
					spline.push_back(cv::Point2f(x, y));
				}
			}
		}
	}

	void FaceDetector::ScaleMorph(std::vector<cv::Point2f>& points,
		std::vector<int> indices, cv::Point2f& center, cv::Point2f& scale) {
		for (auto i : indices) {
			points[i].x = (points[i].x - center.x) * scale.x + center.x;
			points[i].y = (points[i].y - center.y) * scale.y + center.y;
		}
	}
    
	void FaceDetector::InvalidatePoses() {
		for (int i = 0; i < m_faces.length; i++) {
			m_faces[i].ResetPose();
		}
	}

    void FaceDetector::DoFaceDetection() {
		bool copyTex = Config::singleton().get_bool(
			CONFIG_BOOL_MAKE_DETECT_COPY);
		if (copyTex)
			StageAndCopyTexture(SFT_DETECT);
		else
			StageTexture(SFT_DETECT);

		int ww = (int)((float)m_stageWork.w * 
			Config::singleton().get_double(
				CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH));
		int hh = (int)((float)m_stageWork.h * 
			Config::singleton().get_double(
				CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT));
		int xx = (int)((float)(m_stageWork.w / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_FACE_DETECT_CROP_X)) + 
			(m_stageWork.w / 2);
		int yy = (int)((float)(m_stageWork.h / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_FACE_DETECT_CROP_Y)) + 
			(m_stageWork.h / 2);

		// need to transform back to capture size
		int offsetX = xx - (ww / 2);
		int offsetY = yy - (hh / 2);
		float scale = (float)m_capture.width / m_detect.width;

		char* cropdata = m_stageWork.data +
			(m_stageWork.getStride() * offsetY) +
			(m_stageWork.getNumElems() * offsetX);

        // detect faces
		// NOTE : WE ASSUME A LUMA IMAGE HERE
        dlib_image_wrapper<unsigned char> fdimg(cropdata, 
			ww, hh, m_stageWork.getStride());
        std::vector<rectangle> faces = m_detector(fdimg);

		// only consider the face detection results if:
        //
        // - tracking is disabled (so we have to)
        // - we currently have no faces
        // - face detection just found some faces
        //
        // otherwise, we are tracking faces, and the tracking is still trusted, so don't trust
        // the FD results
        //
        if (!Config::singleton().get_bool(CONFIG_BOOL_TRACKING_ENABLE) ||
			(m_faces.length == 0) || (faces.size() > 0)) {
            // clamp to max faces
			m_faces.length = (int)faces.size();
            if (m_faces.length > MAX_FACES)
				m_faces.length = MAX_FACES;

            // copy rects into our faces, start tracking
            for (int i = 0; i < m_faces.length; i++) {
                // scale rectangle up to video frame size
				m_faces[i].m_bounds.set_left((long)((float)(faces[i].left() +
					offsetX) * scale));
                m_faces[i].m_bounds.set_right((long)((float)(faces[i].right() +
					offsetX) * scale));
                m_faces[i].m_bounds.set_top((long)((float)(faces[i].top() +
					offsetY) * scale));
                m_faces[i].m_bounds.set_bottom((long)((float)(faces[i].bottom() +
					offsetY) * scale));
            }
        }

		if (!copyTex)
			UnstageTexture(SFT_DETECT);
    }
    
        
    void FaceDetector::StartObjectTracking() {
		bool copyTex = Config::singleton().get_bool(CONFIG_BOOL_MAKE_TRACK_COPY);
		if (copyTex)
			StageAndCopyTexture(SFT_TRACK);
		else
			StageTexture(SFT_TRACK);

		int ww = (int)((float)m_stageWork.w *
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_WIDTH));
		int hh = (int)((float)m_stageWork.h * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_HEIGHT));
		int xx = (int)((float)(m_stageWork.w / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_X)) +
			(m_stageWork.w / 2);
		int yy = (int)((float)(m_stageWork.h / 2) *
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_Y)) + 
			(m_stageWork.h / 2);

		// need to transform back to capture size
		int offsetX = xx - (ww / 2);
		int offsetY = yy - (hh / 2);
		float scale = (float)m_capture.width / m_track.width;

		char* cropdata = m_stageWork.data +
			(m_stageWork.getStride() * offsetY) + (m_stageWork.getNumElems() * 
				offsetX);

        // wrap up our image
        dlib_image_wrapper<unsigned char> trimg(cropdata, ww, hh, 
			m_stageWork.getStride());
        
        // copy rects into our faces, start tracking
        for (int i = 0; i < m_faces.length; ++i) {
            m_faces[i].StartTracking(trimg, scale, offsetX, offsetY);
        }

		if (!copyTex)
			UnstageTexture(SFT_TRACK);
	}
    
    
    void FaceDetector::UpdateObjectTracking() {
		bool copyTex = Config::singleton().get_bool(CONFIG_BOOL_MAKE_TRACK_COPY);
		if (copyTex)
			StageAndCopyTexture(SFT_TRACK);
		else
			StageTexture(SFT_TRACK);

		int ww = (int)((float)m_stageWork.w * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_WIDTH));
		int hh = (int)((float)m_stageWork.h *
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_HEIGHT));
		int xx = (int)((float)(m_stageWork.w / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_X)) +
			(m_stageWork.w / 2);
		int yy = (int)((float)(m_stageWork.h / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_Y)) +
			(m_stageWork.h / 2);

		// need to transform back to capture size
		int offsetX = xx - (ww / 2);
		int offsetY = yy - (hh / 2);

		char* cropdata = m_stageWork.data +
			(m_stageWork.getStride() * offsetY) +
			(m_stageWork.getNumElems() * offsetX);

		// wrap up our image
        dlib_image_wrapper<unsigned char> trimg(cropdata, ww, hh,
			m_stageWork.getStride());
        
        for (int i = 0; i < m_faces.length; i++) {
            // time-slice face tracking (only track 1 face per frame)
            if (i == m_trackingFaceIndex) {
                double confidence = m_faces[i].UpdateTracking(trimg);
                if (confidence < Config::singleton().get_double(
					CONFIG_DOUBLE_TRACKING_THRESHOLD)) {
                    // lost confidence in tracking. time to detect faces again.
                    // BAIL
					m_faces.length = 0;
                    break;
                }
            }
        }

		if (!copyTex) {
			UnstageTexture(SFT_TRACK);
		}
	}
    
    
    void FaceDetector::DetectLandmarks()
    {
		bool copyTex = Config::singleton().get_bool(
			CONFIG_BOOL_MAKE_CAPTURE_COPY);
		if (copyTex)
			StageAndCopyTexture(SFT_CAPTURE);

		if (Config::singleton().get_bool(CONFIG_BOOL_LANDMARKS_ENABLE)) {
			// detect landmarks
			for (int f = 0; f < m_faces.length; f++) {
				if (!copyTex)
					StageTexture(SFT_CAPTURE);

				// Detect features on full-size frame
				full_object_detection d;
				if (m_stageWork.type == IMAGETYPE_BGR) {
					dlib_image_wrapper<bgr_pixel> fcimg(m_stageWork.data, 
						m_stageWork.w, m_stageWork.h, m_stageWork.getStride());
					d = m_predictor(fcimg, m_faces[f].GetBounds());
				}
				else if (m_stageWork.type == IMAGETYPE_RGB)	{
					dlib_image_wrapper<rgb_pixel> fcimg(m_stageWork.data,
						m_stageWork.w, m_stageWork.h, m_stageWork.getStride());
					d = m_predictor(fcimg, m_faces[f].GetBounds());
				}
				else if (m_stageWork.type == IMAGETYPE_RGBA) {
					dlib_image_wrapper<rgb_alpha_pixel> fcimg(m_stageWork.data,
						m_stageWork.w, m_stageWork.h, m_stageWork.getStride());
					d = m_predictor(fcimg, m_faces[f].GetBounds());
				}
				else if (m_stageWork.type == IMAGETYPE_LUMA) {
					dlib_image_wrapper<unsigned char> fcimg(m_stageWork.data, 
						m_stageWork.w, m_stageWork.h, m_stageWork.getStride());
					d = m_predictor(fcimg, m_faces[f].GetBounds());
				}
				else {
					throw std::invalid_argument(
						"bad image type for face detection - handle better");
				}

				if (!copyTex)
					UnstageTexture(SFT_CAPTURE);

				// Save the face
				if (d.num_parts() != NUM_FACIAL_LANDMARKS)
					throw std::invalid_argument(
						"shape predictor got wrong number of landmarks");

				for (int j = 0; j < NUM_FACIAL_LANDMARKS; j++) {
					m_faces[f].m_points[j] = point(d.part(j).x(), d.part(j).y());
				}
			}

			// Do 3D Pose Estimation
			if (Config::singleton().get_bool(CONFIG_BOOL_POSES_ENALBLE))
				DoPoseEstimation();
		}

	}


	void FaceDetector::DoPoseEstimation()
	{
		// get config vars
		int pnpMethod = Config::singleton().get_int(
			CONFIG_INT_SOLVEPNP_ALGORITHM);
		bool copyTex = Config::singleton().get_bool(
			CONFIG_BOOL_MAKE_CAPTURE_COPY);

		// build set of model points to use for solving 3D pose
		// note: we use these points because they move the least
		std::vector<int> model_indices;
		model_indices.push_back(EYE_CENTER);
		model_indices.push_back(LEFT_INNER_EYE_CORNER);
		model_indices.push_back(RIGHT_INNER_EYE_CORNER);
		model_indices.push_back(NOSE_TIP);
		if (pnpMethod != cv::SOLVEPNP_P3P &&
			pnpMethod != cv::SOLVEPNP_AP3P)
		{
			model_indices.push_back(NOSE_2);
			model_indices.push_back(NOSE_3);
		}
		std::vector<cv::Point3d> model_points = GetLandmarkPoints(model_indices);

		// Camera internals
		// Approximate focal length.
		double focal_length = (double)m_stageWork.w * FOCAL_LENGTH_FACTOR; 
		cv::Point2d center = cv::Point2d(m_stageWork.w / 2, m_stageWork.h / 2);
		cv::Mat camera_matrix = 
			(cv::Mat_<double>(3, 3) << 
				focal_length, 0, center.x, 0, focal_length, center.y, 0, 0, 1);
		// We assume no lens distortion
		cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type);

		int numIterations = Config::singleton().get_int(
			CONFIG_INT_SOLVEPNP_ITERATIONS);

		for (int i = 0; i < m_faces.length; i++) {
			point* p = m_faces[i].m_points;

			// 2D image points. 
			std::vector<cv::Point2d> image_points;
			for (int j = 0; j < model_indices.size(); j++) {
				int idx = model_indices[j];
				image_points.push_back(cv::Point2d(p[idx].x(), p[idx].y()));
			}

			if (m_faces[i].IncPoseResetCounter() > POSE_RESET_INTERVAL) {
				m_faces[i].ResetPose();
			}

			if (!copyTex)
				StageTexture(SFT_CAPTURE);

			// Solve for pose
			if (pnpMethod == PNP_RANSAC) {
				cv::solvePnPRansac(model_points, image_points,
					camera_matrix, dist_coeffs,
					m_faces[i].m_cvRotation, m_faces[i].m_cvTranslation,
					m_faces[i].m_poseInitialized,
					numIterations);
			}
			else {
				cv::solvePnP(model_points, image_points,
					camera_matrix, dist_coeffs,
					m_faces[i].m_cvRotation, m_faces[i].m_cvTranslation,
					m_faces[i].m_poseInitialized,
					pnpMethod);
			}
			m_faces[i].m_poseInitialized = true;

			if (!copyTex)
				UnstageTexture(SFT_CAPTURE);

			// check for solvePnp result flip
			// - make sure it doesn't use these results for next iteration
			bool flipped = (m_faces[i].m_cvTranslation.at<double>(2, 0) < 0.0);
			if (flipped) {
				// this will ensure it gets reset before next calculation
				m_faces[i].SetPoseResetCounter(POSE_RESET_INTERVAL+1);
			}
		}
	}

	void FaceDetector::StageAndCopyTexture(SourceFrameType sft) {
		// for all the early returns coming up next
		m_stageWork = m_stageWrapper;

		// dont re-stage
		if (m_stageType == sft)
			return;

		// get pointers to the right stage and texture objects
		gs_stagesurf_t** __restrict stage = nullptr;
		OBSTexture* __restrict tex = nullptr;
		switch (sft)
		{
		case SFT_CAPTURE:
			// dont re-stage
			if (m_stageType == SFT_DETECT &&
				m_detect.texture == m_capture.texture)
				return;
			if (m_stageType == SFT_TRACK &&
				m_track.texture == m_capture.texture)
				return;
			stage = &m_captureStage;
			tex = &m_capture;
			break;
		case SFT_DETECT:
			// dont re-stage
			if (m_stageType == SFT_CAPTURE &&
				m_capture.texture == m_detect.texture)
				return;
			if (m_stageType == SFT_TRACK &&
				m_track.texture == m_detect.texture)
				return;
			stage = &m_detectStage;
			tex = &m_detect;
			break;
		case SFT_TRACK:
			// dont re-stage
			if (m_stageType == SFT_CAPTURE &&
				m_capture.texture == m_track.texture)
				return;
			if (m_stageType == SFT_DETECT &&
				m_detect.texture == m_track.texture)
				return;
			stage = &m_trackingStage;
			tex = &m_track;
			break;
		default:
			return;
		}

		// enter graphics context - don't stay here long!
		obs_enter_graphics();

		// need to stage the surface so we can read from it
		// (re)alloc the stage surface if necessary
		if (*stage == nullptr ||
			(int)gs_stagesurface_get_width(*stage) != tex->width ||
			(int)gs_stagesurface_get_height(*stage) != tex->height) {
			if (*stage)
				gs_stagesurface_destroy(*stage);
			*stage = gs_stagesurface_create(tex->width, tex->height,
				gs_texture_get_color_format(tex->texture));
		}
		gs_stage_texture(*stage, tex->texture);

		// mapping the stage surface 
		uint8_t *data; uint32_t linesize;
		if (gs_stagesurface_map(*stage, &data, &linesize)) {

			// make sure our space is big enough. 
			int texSize = tex->height * linesize;
			if (m_stageSize < texSize) {
				if (m_stageWrapper.data)
					delete[] m_stageWrapper.data;
				m_stageWrapper.data = new char[texSize];
				m_stageSize = texSize;
			}

			// copy texture data
			if (Config::singleton().get_bool(CONFIG_BOOL_USE_THREADED_MEMCPY))
				threaded_memcpy(m_stageWrapper.data, data, texSize, m_memcpyEnv);
			else
				std::memcpy(m_stageWrapper.data, data, texSize);

			// Wrap the staged texture data
			m_stageType = sft;
			m_stageWrapper.w = tex->width;
			m_stageWrapper.h = tex->height;
			m_stageWrapper.stride = linesize;
			m_stageWrapper.type =
				OBSRenderer::OBSToSMLL(
					gs_texture_get_color_format(tex->texture));

			m_stageWork = m_stageWrapper;
		}
		else {
			blog(LOG_DEBUG, "unable to stage texture!!! bad news!");
			m_stageWork = ImageWrapper();
		}
	
		// unstage the surface and leave graphics context
		gs_stagesurface_unmap(*stage);
		obs_leave_graphics();
	}

	void FaceDetector::StageTexture(SourceFrameType sft) {
		// get pointers to the right stage and texture objects
		gs_stagesurf_t** __restrict stage = nullptr;
		OBSTexture* __restrict tex = nullptr;
		switch (sft) {
		case SFT_CAPTURE:
			stage = &m_captureStage;
			tex = &m_capture;
			break;
		case SFT_DETECT:
			stage = &m_detectStage;
			tex = &m_detect;
			break;
		case SFT_TRACK:
			stage = &m_trackingStage;
			tex = &m_track;
			break;
		default:
			return;
		}

		// enter graphics context - don't stay here long!
		obs_enter_graphics();

		// need to stage the surface so we can read from it
		// (re)alloc the stage surface if necessary
		if (*stage == nullptr ||
			(int)gs_stagesurface_get_width(*stage) != tex->width ||
			(int)gs_stagesurface_get_height(*stage) != tex->height) {
			if (*stage)
				gs_stagesurface_destroy(*stage);
			*stage = gs_stagesurface_create(tex->width, tex->height,
				gs_texture_get_color_format(tex->texture));
		}
		gs_stage_texture(*stage, tex->texture);

		// mapping the stage surface 
		uint8_t *data; uint32_t linesize;
		if (gs_stagesurface_map(*stage, &data, &linesize)) {

			// Wrap the staged texture data
			m_stageWork.w = tex->width;
			m_stageWork.h = tex->height;
			m_stageWork.stride = linesize;
			m_stageWork.type = OBSRenderer::OBSToSMLL(
				gs_texture_get_color_format(tex->texture));
			m_stageWork.data = (char*)data;
		}
		else {
			blog(LOG_DEBUG, "unable to stage texture!!! bad news!");
			m_stageWork = ImageWrapper();
		}
	}

	void FaceDetector::UnstageTexture(SourceFrameType sft) {
		// get pointers to the right stage and texture objects
		gs_stagesurf_t* stage = nullptr;
		switch (sft) {
		case SFT_CAPTURE:
			stage = m_captureStage;
			break;
		case SFT_DETECT:
			stage = m_detectStage;
			break;
		case SFT_TRACK:
			stage = m_trackingStage;
			break;
		default:
			return;
		}

		// unstage the surface and leave graphics context
		gs_stagesurface_unmap(stage);
		obs_leave_graphics();
	}
	
} // smll namespace





