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
#ifndef __SMLL_FACE_DETECTOR_HPP__
#define __SMLL_FACE_DETECTOR_HPP__

#include "Face.hpp"
#include "Config.hpp"
#include "OBSTexture.hpp"
#include "ImageWrapper.hpp"
#include "TriangulationResult.hpp"
#include "MorphData.hpp"

#include <stdexcept>


#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <libobs/graphics/graphics.h>
#include <libobs/util/threaded-memcpy.h>
#pragma warning( pop )

namespace smll {


class FaceDetector
{
public:

	FaceDetector();
	~FaceDetector();

	void DetectFaces(const ImageWrapper& detect, DetectionResults& results);
	void DetectLandmarks(const OBSTexture& capture, DetectionResults& results);
	void DoPoseEstimation(DetectionResults& results);

	void MakeTriangulation(MorphData& morphData, DetectionResults& results, 
		TriangulationResult& result);

	int CaptureWidth() const {
		return m_capture.width;
	}
	int CaptureHeight() const {
		return m_capture.height;
	}

private:

	// Faces
	Faces			m_faces;

	// Image Buffers
	OBSTexture		m_capture;
	ImageWrapper	m_detect;

	// For staging the capture texture
	gs_stagesurf_t* m_captureStage;
	int				m_stageSize;
	ImageWrapper	m_stageWork;

	// Face detection timeouts
	int				m_timeout;
    int             m_trackingTimeout;
    int             m_detectionTimeout;

	// Tracking time-slicer
	int				m_trackingFaceIndex;

	// dlib HOG face detector
	dlib::frontal_face_detector		m_detector;

	// dlib landmark predictors (5 and 68 point)
	dlib::shape_predictor			m_predictor5;
	dlib::shape_predictor			m_predictor68;

	// openCV camera (saved for convenience)
	int				m_camera_w, m_camera_h;
	cv::Mat			m_camera_matrix;
	cv::Mat			m_dist_coeffs;
	void			SetCVCamera();
	const cv::Mat&	GetCVCamMatrix();
	const cv::Mat&	GetCVDistCoeffs();

	// lookup table for morph triangulation
	std::vector<LandmarkBitmask>	m_vtxBitmaskLookup;
	void							MakeVtxBitmaskLookup();

	// Main methods
    void    DoFaceDetection();
    void    StartObjectTracking();
    void    UpdateObjectTracking();

	// Staging the capture texture
	void 	StageCaptureTexture();
	void 	UnstageCaptureTexture();


	// Morph Triangulation Helpers
	void	Subdivide(std::vector<cv::Point2f>& points);
	void	CatmullRomSmooth(std::vector<cv::Point2f>& points, 
		const std::vector<int>& indices, int steps);
	void	ScaleMorph(std::vector<cv::Point2f>& points,
		std::vector<int> indices, cv::Point2f& center, cv::Point2f& scale);
	void	MakeHullPoints(const std::vector<cv::Point2f>& points,
		const std::vector<cv::Point2f>& warpedpoints, 
		std::vector<cv::Point2f>& hullpoints);
	void	MakeAreaIndices(TriangulationResult& result,
		const std::vector<cv::Vec3i>& triangles);
	void	AddHeadPoints(std::vector<cv::Point2f>& points, const Face& face);
	void    AddSelectivePoints(cv::Subdiv2D& subdiv, const std::vector<cv::Point2f>& points,
		const std::vector<cv::Point2f>& warpedpoints, std::map<int, int>& vtxMap);
	void	AddContourSelective(cv::Subdiv2D& subdiv, const FaceContour& fc,
		const std::vector<cv::Point2f>& points,
		const std::vector<cv::Point2f>& warpedpoints, std::map<int, int>& vtxMap, bool checkLeft=true);
	void	AddContour(cv::Subdiv2D& subdiv, const FaceContour& fc,	const std::vector<cv::Point2f>& points,
		std::map<int, int>& vtxMap);
};



} // smll namespace
#endif // __SMLL_FACE_DETECTOR_HPP__

