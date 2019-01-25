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

#include "FaceDetector.hpp"


#define HULL_POINTS_SCALE		(1.25f)
// border points = 4 corners + subdivide
#define NUM_BORDER_POINTS		(4 * 2 * 2 * 2) 
#define NUM_BORDER_POINT_DIVS	(3)
// hull points = head + jaw + subdivide
#define NUM_HULL_POINTS			(28 * 2 * 2 * 2)
#define NUM_HULL_POINT_DIVS		(3)

static const char* const kFileShapePredictor68 = "sp_v1.2_custom.dat";


using namespace dlib;
using namespace std;


namespace smll {

	FaceDetector::FaceDetector()
		: m_timeout(0)
        , m_trackingTimeout(0)
        , m_detectionTimeout(0)
		, m_trackingFaceIndex(0)
		, m_camera_w(0)
		, m_camera_h(0)
		, isPrevInit(false)
		, cropInfo(0,0,0,0) {
		// Load face detection and pose estimation models.
		m_detector = get_frontal_face_detector();

		count = 0;

		char *filename = obs_module_file(kFileShapePredictor68);
#ifdef _WIN32
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wide_filename(converter.from_bytes(filename));

		/* DLIB will not accept a wifstream or widestring to construct
		 * an ifstream or wifstream itself. Here we use a non-standard
		 * constructor provided by Microsoft and then the direct
		 * serialization function with an ifstream. */
		std::ifstream predictor68_file(wide_filename.c_str(), std::ios::binary);

		if (!predictor68_file) {
			throw std::runtime_error("Failed to open predictor68 file");
		}

		deserialize(m_predictor68, predictor68_file);
#else
		deserialize(filename) >> m_predictor68;
#endif
		bfree(filename);
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
			float focal_length = (float)m_camera_w;
			cv::Point2f center = cv::Point2f(m_camera_w / 2.0f, m_camera_h / 2.0f);
			m_camera_matrix =
				(cv::Mat_<float>(3, 3) <<
					focal_length, 0,			center.x, 
					0, 			  focal_length, center.y,
					0,			  0,			1);
			// We assume no lens distortion
			m_dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<float>::type);
		}
	}
	void FaceDetector::computeDifference(DetectionResults& results) {
		CropInfo cropInfo = GetCropInfo();
		float scale = (float) grayImage.rows / resizeHeight ;
		if (!isPrevInit ) {
			isPrevInit = true;
			results.motionRect.set_bottom(currentImage.rows);
			results.motionRect.set_right(currentImage.cols);
			results.motionRect.set_left(0);
			results.motionRect.set_top(0);
			prevImage = currentImage.clone();
			return;
		}

		cv::Mat diffImage;
		cv::absdiff(prevImage, currentImage, diffImage);
		int blur_factor = Config::singleton().get_double(
			CONFIG_DOUBLE_BLUR_FACTOR);
		cv::GaussianBlur(diffImage, diffImage, cv::Size(blur_factor, blur_factor), 0, 0);

		int threshold = Config::singleton().get_double(
			CONFIG_DOUBLE_MOVEMENT_THRESHOLD);
		int minY = diffImage.rows;
		int minX = diffImage.cols;
		int maxY = 0;
		int maxX = 0;
	
		for (int j = 0; j < diffImage.rows; ++j){
			for (int i = 0; i < diffImage.cols; ++i) 
			{
				int pix = (int)diffImage.at<uchar>(j, i);
				if (pix > threshold)
				{
					minX = std::min(i, minX);
					minY = std::min(j, minY);
					maxX = std::max(i, maxX);
					maxY = std::max(j, maxY);
				}
			}
		}
		results.motionRect.set_left(minX*scale);
		results.motionRect.set_right(maxX*scale);
		results.motionRect.set_top(minY*scale);
		results.motionRect.set_bottom(maxY*scale);

	}

	void FaceDetector::addFaceRectangles(DetectionResults& results) {

		float paddingPercentage = Config::singleton().get_double(CONFIG_MOTION_RECTANGLE_PADDING);

		for (int i = 0; i < m_faces.length; i++) {
			// scale rectangle up to video frame size
			results.motionRect.set_left(std::min((int)results.motionRect.left(), (int)((m_faces[i].m_bounds.left()))));
			results.motionRect.set_right(std::max((int)results.motionRect.right(), (int)((m_faces[i].m_bounds.right()))));
			results.motionRect.set_top(std::min((int)results.motionRect.top(), (int)((m_faces[i].m_bounds.top()))));
			results.motionRect.set_bottom(std::max((int)results.motionRect.bottom(), (int)((m_faces[i].m_bounds.bottom()))));
		}

		int delta_w = (int)grayImage.cols*paddingPercentage;
		int delta_h = (int)grayImage.rows*paddingPercentage;
		results.motionRect.set_left(std::max((int)results.motionRect.left() - delta_w, 0));
		results.motionRect.set_right(std::min((int)results.motionRect.right()+ delta_w, grayImage.cols - 1));
		results.motionRect.set_top(std::max((int)results.motionRect.top() - delta_h, 0));
		results.motionRect.set_bottom(std::min((int)results.motionRect.bottom() + delta_h, grayImage.rows - 1));
	}


	void FaceDetector::computeCurrentImage(DetectionResults& results) {
		computeDifference(results);
		addFaceRectangles(results);

		float minMotionRectangle = Config::singleton().get_double(CONFIG_MIN_MOTION_RECTANGLE);
		int MRectMinW = minMotionRectangle *grayImage.cols;
		int MRectMinH = minMotionRectangle *grayImage.rows;
		if (results.motionRect.width() < MRectMinW || results.motionRect.height() < MRectMinH) {
			results.motionRect.set_bottom(grayImage.rows);
			results.motionRect.set_right(grayImage.cols);
			results.motionRect.set_left(0);
			results.motionRect.set_top(0);
		}
		SetCropInfo(results);
		// Do image cropping and cv::Mat initialization in single shot
		CropInfo cropInfo = GetCropInfo();
		
		cv::Mat cropped = grayImage(cv::Rect(cropInfo.offsetX, cropInfo.offsetY, cropInfo.width, cropInfo.height));
		currentImage = cropped.clone();
	}

	void FaceDetector::DetectFaces(cv::Mat &inputImage, int width, int height, DetectionResults& results) {
		// Wait for CONFIG_INT_FACE_DETECT_FREQUENCY after all faces are lost before trying to detect them again
		if (m_timeout > 0) {
			m_timeout--;
			results.processedResults.FrameSkipped();
			return;
		}

		// better check if the camera res has changed on us
		if ((resizeWidth != width) ||
			(resizeHeight != height)) {
			// forget whatever we thought were faces
			m_faces.length = 0;
			isPrevInit = false;
		}

		resizeWidth = width;
		resizeHeight = height;

		grayImage = inputImage;

		// Resize and cut out region of interest
		cv::resize(grayImage, currentImage, cv::Size(resizeWidth, resizeHeight), 0, 0, cv::INTER_LINEAR);
		currentOrigImage = currentImage.clone();

		bool trackingFailed = false;
		bool wasFaceDetected = (m_faces.length > 0);
		// if number of frames before the last detection is bigger than the threshold or if there are no faces to track
		if (m_detectionTimeout == 0 || !wasFaceDetected) {
			computeCurrentImage(results);
			DoFaceDetection();
			if (m_faces.length > 0) {
				m_detectionTimeout =
					Config::singleton().get_int(CONFIG_INT_FACE_DETECT_RECHECK_FREQUENCY);
				StartObjectTracking();
				results.processedResults.DetectionMade();
			}
			else {
				results.processedResults.DetectionFailed();
			}
		}
		else if (m_trackingTimeout == 0) {
			m_detectionTimeout--;

			UpdateObjectTracking();

			// Is Tracking is still good?
			if (m_faces.length > 0) {
				// next face for tracking time-slicing
				m_trackingFaceIndex = (m_trackingFaceIndex + 1) % m_faces.length;

				// tracking frequency
				m_trackingTimeout =
					Config::singleton().get_int(CONFIG_INT_TRACKING_FREQUNCY);
			}
			else {
				m_trackingFaceIndex = 0;
				// force detection on the next frame, do not wait for 5 frames
				m_timeout = 0;
				trackingFailed = true;
				results.processedResults.TrackingFailed();
			}

			results.processedResults.TrackingMade();
			// copy faces to results
			for (int i = 0; i < m_faces.length; i++) {
				results[i] = m_faces[i];
			}
			results.length = m_faces.length;
		}
		else
		{
			m_detectionTimeout--;
			m_trackingTimeout--;
			results.processedResults.FrameSkipped();
		}

		// copy faces to results
		for (int i = 0; i < m_faces.length; i++) {
			results[i] = m_faces[i];
		}
		results.length = m_faces.length;

		if (trackingFailed || m_faces.length == 0) {

		}
		// If faces are not found
		if (m_faces.length == 0 && !trackingFailed && !wasFaceDetected) {
            // Wait for 5 frames and do face detection
            m_timeout = Config::singleton().get_int(CONFIG_INT_FACE_DETECT_FREQUENCY);
		}
	}

	void FaceDetector::MakeTriangulation(MorphData& morphData, 
		DetectionResults& results,
		TriangulationResult& result) {

		// clear buffers
		result.DestroyBuffers();

		// need valid morph data
		if (!morphData.IsValid())
			return;

		// make sure we have our bitmask lookup table
		MakeVtxBitmaskLookup();

		// only 1 face supported (for now)
		if (results.length == 0)
			return;
		DetectionResult& face = results[0];

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
		dlib::point* facePoints = face.landmarks68;
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
		// TODO: we should probably leave the creation of this
		//       graphics stuff to the render method
		//
		obs_enter_graphics();
		gs_render_start(true);
		size_t nv = points.size();
		LandmarkBitmask hpbm;
		hpbm.set(HULL_POINT);
		for (int i = 0; i < nv; i++) {
			// position from warped points
			// uv from original points
			const cv::Point2f& p = warpedpoints[i];
			const cv::Point2f& uv = points[i];

			// add point and uv
			gs_texcoord(uv.x / width, uv.y / height, 0);
			gs_vertex2f(p.x, p.y);

			if ((m_vtxBitmaskLookup[i] & hpbm).any())
				gs_color(0x0);
			else
				gs_color(0xFFFFFFFF);
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

		cv::Rect rect(0, 0, CaptureWidth() + 1, CaptureHeight() + 1);

		// add points 
		for (int i = 0; i < fc.indices.size(); i++) {
			const cv::Point2f& p = points[fc.indices[i]];
			if (rect.contains(p)) {
				int vid = subdiv.insert(p);
				vtxMap[vid] = fc.indices[i];
			}
		}
		int smoothidx = fc.smooth_points_index;
		for (int i = 0; i < fc.num_smooth_points; i++, smoothidx++) {
			const cv::Point2f& p = points[smoothidx];
			if (rect.contains(p)) {
				int vid = subdiv.insert(p);
				vtxMap[vid] = smoothidx;
			}
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

		cv::Rect rect(0, 0, CaptureWidth() + 1, CaptureHeight() + 1);

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
			const cv::Point2f& p = points[fc.indices[i]];
			if (d > 10.0f && rect.contains(p)) {
				int vid = subdiv.insert(p);
				vtxMap[vid] = fc.indices[i];
			}
			else
				break;
		}
		int smoothidx = fc.smooth_points_index;
		for (int i = 0; i < fc.num_smooth_points; i++, smoothidx++) {
			const cv::Point2f& p1 = warpedpoints[smoothidx];
			float d = m * ((p1.x - lo.x) * (hi.y - lo.y) - (p1.y - lo.y) * (hi.x - lo.x));
			const cv::Point2f& p = points[smoothidx];
			if (d > 10.0f && rect.contains(p)) {
				int vid = subdiv.insert(p);
				vtxMap[vid] = smoothidx;
			}
			else
				break;
		}
	}

	void FaceDetector::AddHeadPoints(std::vector<cv::Point2f>& points, const DetectionResult& face) {

		points.reserve(points.size() + HP_NUM_HEAD_POINTS);

		// get the head points
		std::vector<cv::Point3f> headpoints = GetAllHeadPoints();

		// project all the head points
		const cv::Mat& rot = face.GetCVRotation();
		cv::Mat trx = face.GetCVTranslation();

		std::vector<cv::Point2f> projheadpoints;
		cv::projectPoints(headpoints, rot, trx, GetCVCamMatrix(), GetCVDistCoeffs(), projheadpoints);

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
			else { 
				triangles[TriangulationResult::IDXBUFF_BACKGROUND].push_back(i0);
				triangles[TriangulationResult::IDXBUFF_BACKGROUND].push_back(i1);
				triangles[TriangulationResult::IDXBUFF_BACKGROUND].push_back(i2);
			}
		}

		// Build index buffers
		// TODO: again, best to leave graphics calls to render() method
		//
		for (int i = 0; i < TriangulationResult::NUM_INDEX_BUFFERS; i++) {
			if (i == TriangulationResult::IDXBUFF_LINES && !result.buildLines)
				continue;
			obs_enter_graphics();
			uint32_t* indices = (uint32_t*)bmalloc(sizeof(uint32_t) * triangles[i].size());
			memcpy(indices, triangles[i].data(), sizeof(uint32_t) * triangles[i].size());
			result.indexBuffers[i] = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG,
				(void*)indices, triangles[i].size(), 0);
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

	FaceDetector::CropInfo FaceDetector::GetCropInfo() {
		return cropInfo;
	}

	void FaceDetector::SetCropInfo(DetectionResults& results) {
		int ww = results.motionRect.right() - results.motionRect.left();
		int hh = results.motionRect.bottom() - results.motionRect.top();
		int xx = results.motionRect.left() + ww / 2;
		int yy = results.motionRect.top()  + hh / 2;
		if (ww <= 0 || hh <= 0 || results.motionRect.left() < 0 || results.motionRect.right() < 0 || results.motionRect.top() < 0 || results.motionRect.bottom() < 0) {
			ww = grayImage.cols;
			hh = grayImage.rows;
			xx = ww/2;
			yy = hh/2;
		}
		cropInfo = CropInfo(xx, yy, ww, hh);
	}

   void FaceDetector::DoFaceDetection() {

		// get cropping info from config and detect image dimensions
		CropInfo cropInfo = GetCropInfo();
		// need to scale back
		float scale = (float)grayImage.rows / resizeHeight;
		cv::Mat detectionImg;
		if (currentImage.cols*currentImage.rows <= resizeWidth*resizeHeight || scale == 0) {
			scale = 1;
			detectionImg = currentImage;
		} else {
			cv::resize(currentImage, detectionImg, cv::Size(currentImage.cols / scale, currentImage.rows / scale), 0, 0, cv::INTER_LINEAR);
		}
		
        // detect faces
		std::vector<rectangle> faces;
		dlib::cv_image<unsigned char> img(detectionImg);
		faces = m_detector(img);

		// only consider the face detection results if:
        //
        // - tracking is disabled (so we have to) 
        // - we currently have no faces
        // - face detection just found some faces
        //
        // otherwise, we are tracking faces, and the tracking is still trusted, so don't trust
        // the FD results
        //
		if (faces.size() > 0) {
			prevImage = currentOrigImage.clone();
		}

		m_faces.length = (int)faces.size() > MAX_FACES ? MAX_FACES : (int)faces.size();

        // copy rects into our faces, start tracking
        for (int i = 0; i < m_faces.length; i++) {
            // scale rectangle up to video frame size
			m_faces[i].m_bounds.set_left((long)((float)(faces[i].left()*scale +
				cropInfo.offsetX)));
			m_faces[i].m_bounds.set_right((long)((float)(faces[i].right()*scale +
				cropInfo.offsetX)));
			m_faces[i].m_bounds.set_top((long)((float)(faces[i].top()*scale +
				cropInfo.offsetY)));
			m_faces[i].m_bounds.set_bottom((long)((float)(faces[i].bottom()*scale +
				cropInfo.offsetY)));
        }
    }
    
        
    void FaceDetector::StartObjectTracking() {
		// need to scale back
		float scale = (float)grayImage.rows / resizeHeight;

        // start tracking
		dlib::cv_image<unsigned char> img(currentOrigImage);
		for (int i = 0; i < m_faces.length; ++i) {
			m_faces[i].StartTracking(img, scale, 0, 0);
		}
	}
    
    
    void FaceDetector::UpdateObjectTracking() {
		// update object tracking
		dlib::cv_image<unsigned char> img(currentOrigImage);
		for (int i = 0; i < m_faces.length; i++) {
			if (i == m_trackingFaceIndex) {
				double confidence = m_faces[i].UpdateTracking(img);
				if (confidence < Config::singleton().get_double(
					CONFIG_DOUBLE_TRACKING_THRESHOLD)) {
					m_faces.length = 0;
					break;
				}
			}
		}
	}
    
	void FaceDetector::DetectLandmarks(DetectionResults& results)
    {
		// detect landmarks
		for (int f = 0; f < m_faces.length; f++) {
			// Detect features on full-size frame
			full_object_detection d68;

			dlib::cv_image<unsigned char> img(grayImage);
			d68 = m_predictor68(img, m_faces[f].m_bounds);

			// Sanity check
			if (d68.num_parts() != NUM_FACIAL_LANDMARKS)
				throw std::invalid_argument(
					"shape predictor got wrong number of landmarks");

			for (int j = 0; j < NUM_FACIAL_LANDMARKS; j++) {
				results[f].landmarks68[j] = point(d68.part(j).x(), d68.part(j).y());
			}
		}

		results.length = m_faces.length;
	}

	float FaceDetector::ReprojectionError(const std::vector<cv::Point3f>& model_points,
		const std::vector<cv::Point2f>& image_points,
		const cv::Mat& rotation, const cv::Mat& translation) {

		// reproject to check error
		std::vector<cv::Point2f> projectedPoints;
		cv::projectPoints(model_points, rotation, translation,
			GetCVCamMatrix(), GetCVDistCoeffs(), projectedPoints);
		// Compute error
		std::vector<cv::Point2f> absDiff;
		cv::absdiff(image_points, projectedPoints, absDiff);
		float error = cv::sum(cv::mean(absDiff))[0];
		return error;
	}


	void FaceDetector::DoPoseEstimation(DetectionResults& results)
	{
		// Build a set of model points to use for solving 3D pose
		std::vector<int> model_indices;
		model_indices.push_back(LEFT_OUTER_EYE_CORNER);
		model_indices.push_back(RIGHT_OUTER_EYE_CORNER);
		model_indices.push_back(NOSE_1);
		model_indices.push_back(NOSE_2);
		model_indices.push_back(NOSE_3);
		model_indices.push_back(NOSE_4);
		model_indices.push_back(NOSE_7);
		std::vector<cv::Point3f> model_points = GetLandmarkPoints(model_indices);

		if (m_poses.length != results.length) {
			m_poses.length = results.length;
			for (int i = 0; i < m_poses.length; i++) {
				m_poses[i].ResetPose();
			}
		}

		float threshold = 4.0f * model_points.size();
		threshold *= ((float)CaptureWidth() / 1920.0f);

		bool resultsBad = false;
		for (int i = 0; i < results.length; i++) {
			std::vector<cv::Point2f> image_points;
			// copy 2D image points. 
			point* p = results[i].landmarks68;
			for (int j = 0; j < model_indices.size(); j++) {
				int idx = model_indices[j];
				image_points.push_back(cv::Point2f((float)p[idx].x(), (float)p[idx].y()));
			}

			// Solve for pose
			cv::Mat translation = m_poses[i].GetCVTranslation();
			cv::Mat rotation = m_poses[i].GetCVRotation();
			cv::solvePnP(model_points, image_points,
				GetCVCamMatrix(), GetCVDistCoeffs(),
				rotation, translation,
				m_poses[i].PoseValid(),
				cv::SOLVEPNP_EPNP);


			// TODO: Check if we still get wrong results.
			if (translation.at<double>(2, 0) > 1000.0 ||
				translation.at<double>(2, 0) < -1000.0) {
				resultsBad = true;
				m_poses.length = 0;
				break;
			}

			// TODO: If possible, remove these sanity checks
			// NOTE: If no pose is generated, use previous pose.
			if (m_poses[i].PoseValid() &&
				ReprojectionError(model_points, image_points, rotation, translation) > threshold) {
				// reset pose
				translation = m_poses[i].GetCVTranslation();
				rotation = m_poses[i].GetCVRotation();
			}

			// Save it
			m_poses[i].SetPose(rotation, translation);
			results[i].SetPose(m_poses[i]);
		}

		if (resultsBad) {
			// discard results
			results.length = 0;
		}
	}

	void FaceDetector::ResetFaces() {
		m_faces.length = 0;
		m_detectionTimeout = 0;
	}
	
} // smll namespace





