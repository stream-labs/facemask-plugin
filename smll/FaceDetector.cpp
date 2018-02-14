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



#pragma warning( pop )

#define FOCAL_LENGTH_FACTOR		(1.0f)
#define POSE_RESET_INTERVAL		(30)
#define HULL_POINTS_SCALE		(1.25f)
// border points = 4 corners + subdivide
#define NUM_BORDER_POINTS		(4 * 2 * 2 * 2) 
#define NUM_BORDER_POINT_DIVS	(3)
// hull points = head + jaw + subdivide
#define NUM_HULL_POINTS			(28 * 2 * 2 * 2)
#define NUM_HULL_POINT_DIVS		(3)


using namespace dlib;
using namespace std;


namespace smll {

	FaceDetector::FaceDetector(const char* predictorFilename)
		: m_captureStage(nullptr)
		, m_stageSize(0)
		, m_timeout(0)
        , m_trackingTimeout(0)
        , m_detectionTimeout(0)
		, m_trackingFaceIndex(0)
		, m_camera_w(0)
		, m_camera_h(0) {
		// Load face detection and pose estimation models.
		m_detector = get_frontal_face_detector();
		deserialize(predictorFilename) >> m_predictor;
	}

	FaceDetector::~FaceDetector() {
		obs_enter_graphics();
		if (m_captureStage)
			gs_stagesurface_destroy(m_captureStage);
		obs_leave_graphics();
	}


	void FaceDetector::MakeVtxBitmaskLookup() {
		if (m_vtxBitmaskLookup.size() == 0) {
			for (int i = 0; i < NUM_MORPH_LANDMARKS; i++) {
				LandmarkBitmask b;
				b.set(i);
				m_vtxBitmaskLookup.push_back(b);
			}
			for (int i = 0; i < NUM_FACE_CONTOURS; i++) {
				const FaceContour& fc = GetFaceContour((FaceContourID)i);
				for (int j = 0; j < fc.num_smooth_points; j++) {
					m_vtxBitmaskLookup.push_back(fc.bitmask);
				}
			}
			LandmarkBitmask bp, hp;
			bp.set(BORDER_POINT);
			hp.set(HULL_POINT);
			for (int i = 0; i < NUM_BORDER_POINTS; i++) {
				m_vtxBitmaskLookup.push_back(bp);
			}
			for (int i = 0; i < NUM_HULL_POINTS; i++) {
				m_vtxBitmaskLookup.push_back(hp);
			}
		}
	}

	const cv::Mat& FaceDetector::GetCVCamMatrix() {
		SetCVCamera();
		return m_camera_matrix;
	}

	const cv::Mat& FaceDetector::GetCVDistCoeffs() {
		SetCVCamera();
		return m_dist_coeffs;
	}

	void FaceDetector::SetCVCamera() {
		int w = CaptureWidth();
		int h = CaptureHeight();

		if (m_camera_w != w || m_camera_h != h) {
			m_camera_w = w;
			m_camera_h = h;

			// Approximate focal length.
			float focal_length = (float)m_camera_w * FOCAL_LENGTH_FACTOR;
			cv::Point2f center = cv::Point2f(m_camera_w / 2.0f, m_camera_h / 2.0f);
			m_camera_matrix =
				(cv::Mat_<float>(3, 3) <<
					focal_length, 0,			center.x, 
					0,			  focal_length, center.y, 
					0,			  0,			1);
			// We assume no lens distortion
			m_dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<float>::type);
		}
	}


	void FaceDetector::DetectFaces(const OBSTexture& capture, 
		const ImageWrapper& detect, const ImageWrapper& track) {

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
			(detect.w != m_detect.w) ||
			(detect.h != m_detect.h) ||
			(track.w != m_track.w) ||
			(track.h != m_track.h)) {
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


	void FaceDetector::MakeTriangulation(MorphData& morphData, 
		TriangulationResult& result) {

		// clear buffers
		result.DestroyBuffers();

		// need valid morph data
		if (!morphData.IsValid())
			return;

		// make sure we have our bitmask lookup table
		MakeVtxBitmaskLookup();

		// only 1 face supported (for now)
		if (m_faces.length == 0)
			return;
		Face& face = m_faces[0];

		// get angle of face pose
		const cv::Mat& m = face.GetCVRotation();
		double angle = sqrt(m.dot(m));
		bool faceDeadOn = (angle < 0.1f);

		// save capture width and height
		float width = (float)CaptureWidth();
		float height = (float)CaptureHeight();

		// Project morph deltas to image space

		// project the deltas
		// TODO: only project non-zero deltas 
		//       (although, to be honest, with compiler opts and such, it
		//        would likely take longer to separate them out)
		const cv::Mat& rot = face.GetCVRotation();
		cv::Mat trx;
		face.GetCVTranslation().copyTo(trx); // make sure to copy!
		trx.at<double>(0, 0) = 0.0; // clear x & y, we'll center it
		trx.at<double>(1, 0) = 0.0;
		std::vector<cv::Point3f> deltas = morphData.GetCVDeltas();
		std::vector<cv::Point2f> projectedDeltas;
		cv::projectPoints(deltas, rot, trx, GetCVCamMatrix(), GetCVDistCoeffs(), projectedDeltas);

		// make a list of points for triangulation
		std::vector<cv::Point2f> points;

		// add facial landmark points
		dlib::point* facePoints = face.m_points;
		for (int i = 0; i < NUM_FACIAL_LANDMARKS; i++) {
			points.push_back(cv::Point2f((float)facePoints[i].x(),
				(float)facePoints[i].y()));
		}

		// add the head points
		AddHeadPoints(points, face);

		// Apply the morph deltas to points to create warpedpoints
		std::vector<cv::Point2f> warpedpoints = points;
		cv::Point2f c(width / 2, height / 2);
		for (int i = 0; i < NUM_MORPH_LANDMARKS; i++) {
			// bitmask tells us which deltas are non-zero
			if (morphData.GetBitmask()[i]) {
				// offset from center
				warpedpoints[i] += projectedDeltas[i] - c;
			}
		}

		// add smoothing points
		for (int i = 0; i < NUM_FACE_CONTOURS; i++) {
			const FaceContour& fc = GetFaceContour((FaceContourID)i);
			// smooth em out
			CatmullRomSmooth(points, fc.indices, NUM_SMOOTHING_STEPS);
			CatmullRomSmooth(warpedpoints, fc.indices, NUM_SMOOTHING_STEPS);
		}

		// add border points
		std::vector<cv::Point2f> borderpoints;
		// 4 corners
		borderpoints.push_back(cv::Point2f(0, 0));
		borderpoints.push_back(cv::Point2f(width, 0));
		borderpoints.push_back(cv::Point2f(width, height));
		borderpoints.push_back(cv::Point2f(0, height));
		// subdivide
		for (int i = 0; i < NUM_BORDER_POINT_DIVS; i++) {
			Subdivide(borderpoints);
		}
		points.insert(points.end(), borderpoints.begin(), borderpoints.end());
		warpedpoints.insert(warpedpoints.end(), borderpoints.begin(), borderpoints.end());

		// add hull points
		std::vector<cv::Point2f> hullpoints;
		MakeHullPoints(points, warpedpoints, hullpoints);
		points.reserve(points.size() + hullpoints.size());
		warpedpoints.reserve(warpedpoints.size() + hullpoints.size());
		for (int i = 0; i < hullpoints.size(); i++) {
			points.push_back(hullpoints[i]);
			warpedpoints.push_back(hullpoints[i]);
		}

		// make the vertex buffer 
		obs_enter_graphics();
		gs_render_start(true);
		size_t nv = points.size();
		for (int i = 0; i < nv; i++) {
			// position from warped points
			// uv from original points
			const cv::Point2f& p = warpedpoints[i];
			const cv::Point2f& uv = points[i];

			// add point and uv
			gs_texcoord(uv.x / width, uv.y / height, 0);
			gs_vertex2f(p.x, p.y);
		}
		result.vertexBuffer = gs_render_save();
		obs_leave_graphics();


		// Create Triangulation

		// create the openCV Subdiv2D object
		cv::Rect rect(0, 0, CaptureWidth() + 1, CaptureHeight() + 1);
		cv::Subdiv2D subdiv(rect);

		// add our points to subdiv2d
		// save a map: subdiv2d vtx id -> index into our points
		std::map<int, int> vtxMap;
		size_t nsmooth = GetFaceContour(FACE_CONTOUR_LAST).smooth_points_index +
			GetFaceContour(FACE_CONTOUR_LAST).num_smooth_points;
		LandmarkBitmask facebm = TriangulationResult::GetBitmasks()[TriangulationResult::IDXBUFF_FACE];
		if (faceDeadOn) {
			// don't bother doing boundary checks if face is dead-on
			facebm = facebm | TriangulationResult::GetBitmasks()[TriangulationResult::IDXBUFF_LINES];
		}
		for (int i = 0; i < points.size(); i++) {
			cv::Point2f& p = points[i];
			// only add points belonging to face, hull, border
			if ((i >= nsmooth || (m_vtxBitmaskLookup[i] & facebm).any())
				&& rect.contains(p)) {
				// note: this crashes if you insert a point outside the rect.
				try {
					int vid = subdiv.insert(p);
					vtxMap[vid] = i;
				}
				catch (const std::exception& e) {
					blog(LOG_DEBUG, "[FaceMask] ***** CAUGHT EXCEPTION CV::SUBDIV2D: %s", e.what());
					blog(LOG_DEBUG, "[FaceMask] ***** whilst adding point %d at (%f,%f)", i, p.x, p.y);
				}
			}
		}

		// selectively add eyebrows & nose points
		if (!faceDeadOn) {
			AddSelectivePoints(subdiv, points, warpedpoints, vtxMap);
		}

		// get triangulation
		std::vector<cv::Vec3i>	triangleList;
		subdiv.getTriangleIndexList(triangleList);

		// NOTE: openCV's subdiv2D class adds 4 points on initialization:
		//
		//       p0 = 0,0
		//       p1 = M,0
		//       p2 = 0,M
		//       p3 = -M,-M
		//
		// where M = max(W,H) * 3
		//
		// I assume this is to ensure the entire triangulation is contained 
		// in a triangle. Or something. I dunno.
		// Either way, since I add my own border points myself, these first
		// 4 vertices are crap to us, and all resulting triangles are also
		// crap.
		// We ignore these triangles, and use a vtxMap we created above
		// to re-index the triangle indices to our vtx list.
		//
		// Also, the subdiv2D object will merge vertices that are close enough
		// to each other. The vtxMap also fixes this, and we use the 
		// revVtxMap for area sorting later on.

		// re-index triangles and remove bad ones
		for (int i = 0; i < triangleList.size(); i++) {
			cv::Vec3i &t = triangleList[i];
			if (t[0] < 4 || t[1] < 4 || t[2] < 4) {
				// mark triangle as bad
				t[0] = 0;
				t[1] = 0;
				t[2] = 0;
			}
			else {
				// re-index
				t[0] = vtxMap[t[0]];
				t[1] = vtxMap[t[1]];
				t[2] = vtxMap[t[2]];
			}
		}

		// Make Index Buffers
		MakeAreaIndices(result, triangleList);
	}


	void FaceDetector::AddSelectivePoints(cv::Subdiv2D& subdiv,
		const std::vector<cv::Point2f>& points,
		const std::vector<cv::Point2f>& warpedpoints, std::map<int, int>& vtxMap) {

		bool turnedLeft = warpedpoints[NOSE_4].x < warpedpoints[NOSE_1].x;

		if (turnedLeft) {
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYEBROW_LEFT), points, warpedpoints, vtxMap, true);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYE_LEFT_TOP), points, warpedpoints, vtxMap, true);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYE_LEFT_BOTTOM), points, warpedpoints, vtxMap, true);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYE_LEFT_BOTTOM), points, warpedpoints, vtxMap, true);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_MOUTH_OUTER_TOP_LEFT), points, warpedpoints, vtxMap, true);

			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYEBROW_RIGHT), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYE_RIGHT_TOP), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYE_RIGHT_BOTTOM), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYE_RIGHT_BOTTOM), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_MOUTH_OUTER_TOP_RIGHT), points, vtxMap);
		}
		else {
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYEBROW_RIGHT), points, warpedpoints, vtxMap, false);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYE_RIGHT_TOP), points, warpedpoints, vtxMap, false);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYE_RIGHT_BOTTOM), points, warpedpoints, vtxMap, false);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_EYE_RIGHT_BOTTOM), points, warpedpoints, vtxMap, false);
			AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_MOUTH_OUTER_TOP_RIGHT), points, warpedpoints, vtxMap, false);

			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYEBROW_LEFT), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYE_LEFT_TOP), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYE_LEFT_BOTTOM), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_EYE_LEFT_BOTTOM), points, vtxMap);
			AddContour(subdiv, GetFaceContour(FACE_CONTOUR_MOUTH_OUTER_TOP_LEFT), points, vtxMap);
		}

		AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_NOSE_BRIDGE), points, warpedpoints, vtxMap, turnedLeft);
		AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_NOSE_BOTTOM), points, warpedpoints, vtxMap, turnedLeft);
		AddContourSelective(subdiv, GetFaceContour(FACE_CONTOUR_MOUTH_OUTER_BOTTOM), points, warpedpoints, vtxMap, turnedLeft);
	}

	void FaceDetector::AddContour(cv::Subdiv2D& subdiv, const FaceContour& fc, const std::vector<cv::Point2f>& points,
		std::map<int, int>& vtxMap) {

		// add points 
		for (int i = 0; i < fc.indices.size(); i++) {
			int vid = subdiv.insert(points[fc.indices[i]]);
			vtxMap[vid] = fc.indices[i];
		}
		int smoothidx = fc.smooth_points_index;
		for (int i = 0; i < fc.num_smooth_points; i++, smoothidx++) {
			int vid = subdiv.insert(points[smoothidx]);
			vtxMap[vid] = smoothidx;
		}
	}


	void FaceDetector::AddContourSelective(cv::Subdiv2D& subdiv, const FaceContour& fc,
		const std::vector<cv::Point2f>& points,
		const std::vector<cv::Point2f>& warpedpoints, std::map<int, int>& vtxMap, 
		bool checkLeft) {

		std::array<int, 15> lhead_points = { HEAD_6, HEAD_5, HEAD_4, HEAD_3, HEAD_2, 
			HEAD_1, JAW_1, JAW_2, JAW_3, JAW_4, JAW_5, JAW_6, JAW_7, JAW_8, JAW_9};
		std::array<int, 15> rhead_points = { HEAD_6, HEAD_7, HEAD_8, HEAD_9, HEAD_10, 
			HEAD_11, JAW_17, JAW_16, JAW_15, JAW_14, JAW_13, JAW_12, JAW_11, JAW_10,
			JAW_9};


		// find min/max y of contour points
		float miny = warpedpoints[fc.indices[0]].y;
		float maxy = warpedpoints[fc.indices[0]].y;
		for (int i = 1; i < fc.indices.size(); i++) {
			const cv::Point2f& p = warpedpoints[fc.indices[i]];
			if (p.y < miny)
				miny = p.y;
			if (p.y > maxy)
				maxy = p.y;
		}

		// find hi/low points on sides of face
		std::array<int, 15>& headpoints = checkLeft ? lhead_points : rhead_points;
		int lop = 0;
		int hip = (int)headpoints.size() - 1;
		for (int i = 1; i < headpoints.size(); i++) {
			if (warpedpoints[headpoints[i]].y < miny)
				lop = i;
			else
				break;
		}
		for (int i = (int)headpoints.size() - 2; i >= 0; i--) {
			if (warpedpoints[headpoints[i]].y > maxy)
				hip = i;
			else
				break;
		}

		// hi low points
		const cv::Point2f& hi = warpedpoints[headpoints[hip]];
		const cv::Point2f& lo = warpedpoints[headpoints[lop]];

		float m = 1.0f;
		if (!checkLeft)
			m = -1.0f;

		// add points if they are inside the outside line
		for (int i = 0; i < fc.indices.size(); i++) {
			const cv::Point2f& p1 = warpedpoints[fc.indices[i]];
			float d = m * ((p1.x - lo.x) * (hi.y - lo.y) - (p1.y - lo.y) * (hi.x - lo.x));
			if (d > 10.0f) {
				int vid = subdiv.insert(points[fc.indices[i]]);
				vtxMap[vid] = fc.indices[i];
			}
			else
				break;
		}
		int smoothidx = fc.smooth_points_index;
		for (int i = 0; i < fc.num_smooth_points; i++, smoothidx++) {
			const cv::Point2f& p1 = warpedpoints[smoothidx];
			float d = m * ((p1.x - lo.x) * (hi.y - lo.y) - (p1.y - lo.y) * (hi.x - lo.x));
			if (d > 10.0f) {
				int vid = subdiv.insert(points[smoothidx]);
				vtxMap[vid] = smoothidx;
			}
			else
				break;
		}
	}


	void FaceDetector::AddHeadPoints(std::vector<cv::Point2f>& points, const Face& face) {

		points.reserve(points.size() + HP_NUM_HEAD_POINTS);

		// get the head points
		std::vector<cv::Point3f> headpoints = GetAllHeadPoints();

		// project all the head points
		const cv::Mat& rot = face.GetCVRotation();
		cv::Mat trx = face.GetCVTranslation();
		std::vector<cv::Point2f> projheadpoints;
		cv::projectPoints(headpoints, rot, trx, GetCVCamMatrix(), GetCVDistCoeffs(), projheadpoints);

		// debug: add all points
		//
		//for (auto p : projheadpoints) {
		//	points.push_back(p);
		//}
		//return;

		// select the correct points

		// HEAD_1 -> HEAD_5
		for (int i = 0, j = 0; i < 5; i++, j+=3) {
			int h0 = HP_HEAD_1 + i;
			int h1 = HP_HEAD_EXTRA_1 + j;
			int h2 = HP_HEAD_EXTRA_2 + j;

			if (projheadpoints[h0].x < projheadpoints[h1].x)
				points.push_back(projheadpoints[h0]);
			else if (projheadpoints[h1].x < projheadpoints[h2].x)
				points.push_back(projheadpoints[h1]);
			else
				points.push_back(projheadpoints[h2]);
		}
		// HEAD_6
		points.push_back(projheadpoints[HP_HEAD_6]);
		// HEAD_7 -> HEAD_11
		for (int i = 0, j = 12; i < 5; i++, j -= 3) {
			int h0 = HP_HEAD_7 + i;
			int h1 = HP_HEAD_EXTRA_3 + j;
			int h2 = HP_HEAD_EXTRA_2 + j;

			if (projheadpoints[h0].x > projheadpoints[h1].x)
				points.push_back(projheadpoints[h0]);
			else if (projheadpoints[h1].x > projheadpoints[h2].x)
				points.push_back(projheadpoints[h1]);
			else
				points.push_back(projheadpoints[h2]);
		}
	}


	// MakeHullPoints
	// - these are extra points added to the morph to smooth out the appearance,
	//   and keep the rest of the video frame from morphing with it
	//
	void FaceDetector::MakeHullPoints(const std::vector<cv::Point2f>& points,
		const std::vector<cv::Point2f>& warpedpoints, std::vector<cv::Point2f>& hullpoints) {
		// consider outside contours only
		const int num_contours = 2;
		const FaceContourID contours[num_contours] = { FACE_CONTOUR_CHIN, FACE_CONTOUR_HEAD };

		// find the center of the original points
		int numPoints = 0;
		cv::Point2f center(0.0f, 0.0f);
		for (int i = 0; i < num_contours; i++) {
			const FaceContour& fc = GetFaceContour(contours[i]);
			for (int j = 0; j < fc.indices.size(); j++, numPoints++) {
				center += points[fc.indices[j]];
			}
		}
		center /= (float)numPoints;

		// go through the warped points, see if they expand the hull
		// - we do this by checking the dot product of the delta to the
		//   warped point with the vector to the original point from
		//   the center
		for (int i = 0; i < num_contours; i++) {
			const FaceContour& fc = GetFaceContour(contours[i]);
			int is = 0;
			size_t ie = fc.indices.size();
			int ip = 1;
			if (fc.id == FACE_CONTOUR_HEAD) {
				// don't include jaw points twice, step backwards
				is = (int)fc.indices.size() - 2;
				ie = 0;
				ip = -1;
			}

			for (int j = is; j != ie; j += ip) {
				// get points
				const cv::Point2f&	p = points[fc.indices[j]];
				const cv::Point2f&	wp = warpedpoints[fc.indices[j]];

				// get vectors
				cv::Point2f d = wp - p;
				cv::Point2f v = p - center;
				// if dot product is positive
				if (d.dot(v) > 0) {
					// warped point expands hull
					hullpoints.push_back(wp);
				}
				else {
					// warped point shrinks hull, use original
					hullpoints.push_back(p);
				}
			}
		}

		// scale up hull points from center
		for (int i = 0; i < hullpoints.size(); i++) {
			hullpoints[i] = ((hullpoints[i] - center) * HULL_POINTS_SCALE) + center;
		}

		// subdivide
		for (int i = 0; i < NUM_BORDER_POINT_DIVS; i++) {
			Subdivide(hullpoints);
		}
	}

	// MakeAreaIndices : make index buffers for different areas of the face
	//
	void FaceDetector::MakeAreaIndices(TriangulationResult& result,
		const std::vector<cv::Vec3i>& triangleList) {

		// Allocate temp storage for triangle indices
		std::vector<uint32_t> triangles[TriangulationResult::NUM_INDEX_BUFFERS];
		triangles[TriangulationResult::IDXBUFF_BACKGROUND].reserve(triangleList.size() * 3);
		triangles[TriangulationResult::IDXBUFF_FACE].reserve(triangleList.size() * 3);
		triangles[TriangulationResult::IDXBUFF_HULL].reserve(triangleList.size() * 3);
		if (result.buildLines) {
			triangles[TriangulationResult::IDXBUFF_LINES].reserve(triangleList.size() * 6);
		}

		// Sort out our triangles 
		for (int i = 0; i < triangleList.size(); i++) {
			const cv::Vec3i& tri = triangleList[i];
			int i0 = tri[0];
			int i1 = tri[1];
			int i2 = tri[2];
			if (i0 == 0 && i1 == 0 && i2 == 0) {
				// SKIP INVALID
				continue;
			}

			// lookup bitmasks
			const LandmarkBitmask& b0 = m_vtxBitmaskLookup[i0];
			const LandmarkBitmask& b1 = m_vtxBitmaskLookup[i1];
			const LandmarkBitmask& b2 = m_vtxBitmaskLookup[i2];
			
			//LandmarkBitmask bgmask = TriangulationResult::GetBitmasks()[TriangulationResult::IDXBUFF_BACKGROUND];
			LandmarkBitmask hullmask = TriangulationResult::GetBitmasks()[TriangulationResult::IDXBUFF_HULL];
			LandmarkBitmask facemask = TriangulationResult::GetBitmasks()[TriangulationResult::IDXBUFF_FACE] |
				TriangulationResult::GetBitmasks()[TriangulationResult::IDXBUFF_LINES]; // include extra points here
			LandmarkBitmask leyemask = GetFaceArea(FACE_AREA_EYE_LEFT).bitmask;
			LandmarkBitmask reyemask = GetFaceArea(FACE_AREA_EYE_RIGHT).bitmask;
			LandmarkBitmask mouthmask = GetFaceArea(FACE_AREA_MOUTH_LIPS_TOP).bitmask |
				GetFaceArea(FACE_AREA_MOUTH_LIPS_BOTTOM).bitmask;

			// one freakin' triangle! We think this is part of the mouth unless
			// we consider this one triangle made entirely of mouth points, but STILL
			// is part of the face.
			LandmarkBitmask facemask2;
			facemask2.set(MOUTH_OUTER_3);
			facemask2.set(MOUTH_OUTER_4);
			facemask2.set(MOUTH_OUTER_5);

			// remove eyes and mouth, except for the special triangle
			if ((b0 & facemask2).any() &&
				(b1 & facemask2).any() &&
				(b2 & facemask2).any()) {} // do nothing
			else if (((b0 & leyemask).any() && (b1 & leyemask).any() && (b2 & leyemask).any())||
				((b0 & reyemask).any() && (b1 & reyemask).any() && (b2 & reyemask).any()) ||
				((b0 & mouthmask).any() && (b1 & mouthmask).any() && (b2 & mouthmask).any()))
			{
				// Skip eyes and mouth
				continue;
			}

			// lines
			if (result.buildLines) {
				triangles[TriangulationResult::IDXBUFF_LINES].push_back(i0);
				triangles[TriangulationResult::IDXBUFF_LINES].push_back(i1);
				triangles[TriangulationResult::IDXBUFF_LINES].push_back(i1);
				triangles[TriangulationResult::IDXBUFF_LINES].push_back(i2);
				triangles[TriangulationResult::IDXBUFF_LINES].push_back(i2);
				triangles[TriangulationResult::IDXBUFF_LINES].push_back(i0);
			}

			if ((b0 & facemask).any() &&
				(b1 & facemask).any() &&
				(b2 & facemask).any()) {
				triangles[TriangulationResult::IDXBUFF_FACE].push_back(i0);
				triangles[TriangulationResult::IDXBUFF_FACE].push_back(i1);
				triangles[TriangulationResult::IDXBUFF_FACE].push_back(i2);
			}
			else if ((b0 & hullmask).any() &&
				(b1 & hullmask).any() &&
				(b2 & hullmask).any()) {
				triangles[TriangulationResult::IDXBUFF_HULL].push_back(i0);
				triangles[TriangulationResult::IDXBUFF_HULL].push_back(i1);
				triangles[TriangulationResult::IDXBUFF_HULL].push_back(i2);
			}
			else /*if ((b0 & bgmask).any() ||
				(b1 & bgmask).any() ||
				(b2 & bgmask).any())*/ { 
				triangles[TriangulationResult::IDXBUFF_BACKGROUND].push_back(i0);
				triangles[TriangulationResult::IDXBUFF_BACKGROUND].push_back(i1);
				triangles[TriangulationResult::IDXBUFF_BACKGROUND].push_back(i2);
			}
		}

		// Build index buffers
		for (int i = 0; i < TriangulationResult::NUM_INDEX_BUFFERS; i++) {
			if (i == TriangulationResult::IDXBUFF_LINES && !result.buildLines)
				continue;
			obs_enter_graphics();
			result.indexBuffers[i] = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG,
				(void*)triangles[i].data(), triangles[i].size(), 0);
			obs_leave_graphics();
		}
	}

	// Subdivide : insert points half-way between all the points
	//
	void FaceDetector::Subdivide(std::vector<cv::Point2f>& points) {
		points.reserve(points.size() * 2);
		for (unsigned int i = 0; i < points.size(); i++) {
			int i2 = (i + 1) % points.size();
			points.insert(points.begin()+i2, cv::Point2f(
				(points[i].x + points[i2].x) / 2.0f,
				(points[i].y + points[i2].y) / 2.0f));
			i++;
		}
	}

	// Curve Fitting - Catmull-Rom spline 
	// https://gist.github.com/pr0digy/1383576
	// - converted to C++
	// - modified for my uses
	void FaceDetector::CatmullRomSmooth(std::vector<cv::Point2f>& points, 
		const std::vector<int>& indices, int steps) {

		if (indices.size() < 3)
			return;

		points.reserve(points.size() + ((indices.size() - 1) * (steps - 1)));

		float dt = 1.0f / (float)steps;

		float x, y;
		size_t i0, i1, i2, i3;
		size_t count = indices.size() - 1;
		size_t count_m1 = count - 1;
		for (size_t i = 0; i < count; i++) {
			if (i == 0) {
				// 0 0 1 2 (i == 0)
				i0 = indices[i];
				i1 = indices[i];
				i2 = indices[i + 1];
				i3 = indices[i + 2];
			}
			else if (i == count_m1) {
				// 6 7 8 8 (i == 7)
				i0 = indices[i - 1];
				i1 = indices[i];
				i2 = indices[i + 1];
				i3 = indices[i + 1];
			}
			else {
				// 2 3 4 5 (i == 3)
				i0 = indices[i - 1];
				i1 = indices[i];
				i2 = indices[i + 1];
				i3 = indices[i + 2];
			}
			const cv::Point2f& p0 = points[i0];
			const cv::Point2f& p1 = points[i1];
			const cv::Point2f& p2 = points[i2];
			const cv::Point2f& p3 = points[i3];

			// TODO: we have more than a couple of SSE-enabled math libraries on tap
			//       we should be using one here
			//       ie) dlib/openCV/libOBS
			//
			// Note: skip points at t=0 and t=1, they are already in our set
			//
			for (float t = dt; t < 1.0f; t += dt) {
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

				points.push_back(cv::Point2f(x, y));
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

		// get cropping info from config and detect image dimensions
		int ww = (int)((float)m_detect.w * 
			Config::singleton().get_double(
				CONFIG_DOUBLE_FACE_DETECT_CROP_WIDTH));
		int hh = (int)((float)m_detect.h * 
			Config::singleton().get_double(
				CONFIG_DOUBLE_FACE_DETECT_CROP_HEIGHT));
		int xx = (int)((float)(m_detect.w / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_FACE_DETECT_CROP_X)) + 
			(m_detect.w / 2);
		int yy = (int)((float)(m_detect.h / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_FACE_DETECT_CROP_Y)) + 
			(m_detect.h / 2);

		// cropping offset
		int offsetX = xx - (ww / 2);
		int offsetY = yy - (hh / 2);
		char* cropdata = m_detect.data +
			(m_detect.getStride() * offsetY) +
			(m_detect.getNumElems() * offsetX);

		// need to scale back
		float scale = (float)m_capture.width / m_detect.w;

        // detect faces
		// NOTE : WE ASSUME A LUMA IMAGE HERE
        dlib_image_wrapper<unsigned char> fdimg(cropdata, 
			ww, hh, m_detect.getStride());
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
    }
    
        
    void FaceDetector::StartObjectTracking() {

		// get crop info from config and track image dimensions
		int ww = (int)((float)m_track.w *
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_WIDTH));
		int hh = (int)((float)m_track.h * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_HEIGHT));
		int xx = (int)((float)(m_track.w / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_X)) +
			(m_track.w / 2);
		int yy = (int)((float)(m_track.h / 2) *
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_Y)) + 
			(m_track.h / 2);

		// cropping offset
		int offsetX = xx - (ww / 2);
		int offsetY = yy - (hh / 2);
		char* cropdata = m_track.data +
			(m_track.getStride() * offsetY) + 
			(m_track.getNumElems() * offsetX);

		// need to scale back
		float scale = (float)m_capture.width / m_track.w;

        // wrap up our image
        dlib_image_wrapper<unsigned char> trimg(cropdata, ww, hh, 
			m_track.getStride());
        
        // start tracking
        for (int i = 0; i < m_faces.length; ++i) {
            m_faces[i].StartTracking(trimg, scale, offsetX, offsetY);
        }
	}
    
    
    void FaceDetector::UpdateObjectTracking() {

		// get cropping info from config and track image dimensions
		int ww = (int)((float)m_track.w * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_WIDTH));
		int hh = (int)((float)m_track.h *
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_HEIGHT));
		int xx = (int)((float)(m_track.w / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_X)) +
			(m_track.w / 2);
		int yy = (int)((float)(m_track.h / 2) * 
			Config::singleton().get_double(CONFIG_DOUBLE_TRACKING_CROP_Y)) +
			(m_track.h / 2);

		// cropping offset
		int offsetX = xx - (ww / 2);
		int offsetY = yy - (hh / 2);
		char* cropdata = m_track.data +
			(m_track.getStride() * offsetY) +
			(m_track.getNumElems() * offsetX);

		// wrap up our image
        dlib_image_wrapper<unsigned char> trimg(cropdata, ww, hh,
			m_track.getStride());
        
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
	}
    
    
    void FaceDetector::DetectLandmarks()
    {

		if (Config::singleton().get_bool(CONFIG_BOOL_LANDMARKS_ENABLE)) {
			// detect landmarks
			for (int f = 0; f < m_faces.length; f++) {
				StageCaptureTexture();

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

				UnstageCaptureTexture();

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
		std::vector<cv::Point3f> model_points = GetLandmarkPoints(model_indices);

		int numIterations = Config::singleton().get_int(
			CONFIG_INT_SOLVEPNP_ITERATIONS);

		for (int i = 0; i < m_faces.length; i++) {
			point* p = m_faces[i].m_points;

			// 2D image points. 
			std::vector<cv::Point2f> image_points;
			for (int j = 0; j < model_indices.size(); j++) {
				int idx = model_indices[j];
				image_points.push_back(cv::Point2f((float)p[idx].x(), (float)p[idx].y()));
			}

			if (m_faces[i].IncPoseResetCounter() > POSE_RESET_INTERVAL) {
				m_faces[i].ResetPose();
			}

			// Solve for pose
			if (pnpMethod == PNP_RANSAC) {
				cv::solvePnPRansac(model_points, image_points,
					GetCVCamMatrix(), GetCVDistCoeffs(),
					m_faces[i].m_cvRotation, m_faces[i].m_cvTranslation,
					m_faces[i].m_poseInitialized,
					numIterations);
			}
			else {
				cv::solvePnP(model_points, image_points,
					GetCVCamMatrix(), GetCVDistCoeffs(),
					m_faces[i].m_cvRotation, m_faces[i].m_cvTranslation,
					m_faces[i].m_poseInitialized,
					pnpMethod);
			}
			m_faces[i].m_poseInitialized = true;

			// check for solvePnp result flip
			// - make sure it doesn't use these results for next iteration
			bool flipped = (m_faces[i].m_cvTranslation.at<double>(2, 0) < 0.0);
			if (flipped) {
				// this will ensure it gets reset before next calculation
				m_faces[i].SetPoseResetCounter(POSE_RESET_INTERVAL+1);
			}
		}
	}

	/*
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
	}*/

	void FaceDetector::StageCaptureTexture() {
		// stage texture and enter graphics context
		obs_enter_graphics();

		// need to stage the surface so we can read from it
		// (re)alloc the stage surface if necessary
		if (m_captureStage == nullptr ||
			(int)gs_stagesurface_get_width(m_captureStage) != m_capture.width ||
			(int)gs_stagesurface_get_height(m_captureStage) != m_capture.height) {
			if (m_captureStage)
				gs_stagesurface_destroy(m_captureStage);
			m_captureStage = gs_stagesurface_create(m_capture.width, m_capture.height,
				gs_texture_get_color_format(m_capture.texture));
		}
		gs_stage_texture(m_captureStage, m_capture.texture);

		// mapping the stage surface 
		uint8_t *data; uint32_t linesize;
		if (gs_stagesurface_map(m_captureStage, &data, &linesize)) {

			// Wrap the staged texture data
			m_stageWork.w = m_capture.width;
			m_stageWork.h = m_capture.height;
			m_stageWork.stride = linesize;
			m_stageWork.type = OBSRenderer::OBSToSMLL(
				gs_texture_get_color_format(m_capture.texture));
			m_stageWork.data = (char*)data;
		}
		else {
			blog(LOG_DEBUG, "unable to stage texture!!! bad news!");
			m_stageWork = ImageWrapper();
		}
	}

	void FaceDetector::UnstageCaptureTexture() {
		// unstage the surface and leave graphics context
		gs_stagesurface_unmap(m_captureStage);
		obs_leave_graphics();
	}
	
} // smll namespace





