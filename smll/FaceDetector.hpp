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

	FaceDetector(const char* predictorFilename);
	~FaceDetector();

	void DetectFaces(const OBSTexture& capture, 
					 const OBSTexture& detect, 
					 const OBSTexture& track);

	uint32_t MakeTriangulation(gs_vertbuffer_t** vbuff, 
		gs_indexbuffer_t** ibuff, gs_indexbuffer_t** linebuff=nullptr);


	const Face& GetFace(int i) const {
		if (i < 0 || i >= m_faces.length)
			throw std::invalid_argument("face point index out of range");
		return m_faces[i];
	}
	const Faces& GetFaces() const {
		return m_faces;
	}
	int CaptureWidth() const {
		return m_capture.width;
	}
	int CaptureHeight() const {
		return m_capture.height;
	}
    
private:

	// Faces
	Faces			m_faces;

	typedef enum SourceFrameType
	{
		SFT_UNDEFINED,
		SFT_CAPTURE,
		SFT_DETECT,
		SFT_TRACK,

	} SourceFrameType;

	// Image Buffers
	OBSTexture		m_capture;
	OBSTexture		m_detect;
	OBSTexture		m_track;

	gs_stagesurf_t* m_captureStage;
	gs_stagesurf_t*	m_detectStage;
	gs_stagesurf_t*	m_trackingStage;

	int				m_stageSize;
	SourceFrameType m_stageType;
	ImageWrapper	m_stageWrapper;
	ImageWrapper	m_stageWork;
	struct memcpy_environment* m_memcpyEnv;

	// Face detection timeouts
	int				m_timeout;
    int             m_trackingTimeout;
    int             m_detectionTimeout;

	// Tracking time-slicer
	int				m_trackingFaceIndex;

	// dlib stuff
	dlib::frontal_face_detector		m_detector;
	dlib::shape_predictor			m_predictor;

	void	InvalidatePoses();

    void    DoFaceDetection();
    void    StartObjectTracking();
    void    UpdateObjectTracking();
    void    DetectLandmarks();
	void    DoPoseEstimation();

	void 	StageAndCopyTexture(SourceFrameType sft);
	void 	StageTexture(SourceFrameType sft);
	void 	UnstageTexture(SourceFrameType sft);

	void	Subdivide(std::vector<cv::Point2f>& points);
	void	CatmullRomSmooth(std::vector<cv::Point2f>& points, int steps);
	void	ScaleMorph(std::vector<cv::Point2f>& points,
		std::vector<int> indices, cv::Point2f& center, cv::Point2f& scale);
};



} // smll namespace
#endif // __SMLL_FACE_DETECTOR_HPP__

