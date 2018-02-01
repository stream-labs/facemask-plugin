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
#include "landmarks.hpp"

std::vector<cv::Point3d>	g_landmark_points;


namespace smll {

	std::vector<cv::Point3d>&	GetLandmarkPoints() {
		if (g_landmark_points.size() == 0) {
			/* This code was generated using the landmarks.mb file (in the tools folder), and the
			   following MEL code:

			   string $landmarks[] = `ls -transforms "landmark*"`;
			   string $obj;
			   float $scale_x = 1;
			   float $scale_y = -1;
			   float $scale_z = -1;
			   print("\n\n\n\n\n\n");
			   for($obj in $landmarks) {
				   float $pos_x = `getAttr ($obj + ".translateX")`;
				   float $pos_y = `getAttr ($obj + ".translateY")`;
				   float $pos_z = `getAttr ($obj + ".translateZ")`;
				   $pos_x = $pos_x * $scale_x;
				   $pos_y = $pos_y * $scale_y;
				   $pos_z = $pos_z * $scale_z;
				   print("g_landmark_points.push_back(cv::Point3d(");
				   print($pos_x);
				   print(",");
				   print($pos_y);
				   print(",");
				   print($pos_z);
				   print("));  //" + $obj + "\n");
				}
			*/
			g_landmark_points.push_back(cv::Point3d(-3.406272, -1.874923, 6.205712));  //landmark1
			g_landmark_points.push_back(cv::Point3d(-3.23767, -0.79834, 6.201868));  //landmark2
			g_landmark_points.push_back(cv::Point3d(-3.154594, 0.146624, 6.035492));  //landmark3
			g_landmark_points.push_back(cv::Point3d(-3.028482, 1.172708, 5.494314));  //landmark4
			g_landmark_points.push_back(cv::Point3d(-2.807666, 1.887181, 4.930567));  //landmark5
			g_landmark_points.push_back(cv::Point3d(-2.444763, 2.551855, 4.279766));  //landmark6
			g_landmark_points.push_back(cv::Point3d(-1.913723, 3.223182, 3.2233));  //landmark7
			g_landmark_points.push_back(cv::Point3d(-0.952636, 3.763827, 2.210871));  //landmark8
			g_landmark_points.push_back(cv::Point3d(0, 3.841394, 1.835872));  //landmark9
			g_landmark_points.push_back(cv::Point3d(0.988395, 3.766559, 2.196558));  //landmark10
			g_landmark_points.push_back(cv::Point3d(1.945419, 3.27159, 3.281386));  //landmark11
			g_landmark_points.push_back(cv::Point3d(2.452621, 2.56053, 4.327683));  //landmark12
			g_landmark_points.push_back(cv::Point3d(2.824853, 1.718492, 5.409257));  //landmark13
			g_landmark_points.push_back(cv::Point3d(3.029557, 0.889899, 5.949514));  //landmark14
			g_landmark_points.push_back(cv::Point3d(3.293411, 0, 6.305057));  //landmark15
			g_landmark_points.push_back(cv::Point3d(3.359597, -0.823819, 6.656381));  //landmark16
			g_landmark_points.push_back(cv::Point3d(3.483503, -1.875, 6.785234));  //landmark17
			g_landmark_points.push_back(cv::Point3d(-2.742824, -2.431646, 2.349515));  //landmark18
			g_landmark_points.push_back(cv::Point3d(-2.295879, -2.766762, 1.83261));  //landmark19
			g_landmark_points.push_back(cv::Point3d(-1.713283, -3.033448, 1.573589));  //landmark20
			g_landmark_points.push_back(cv::Point3d(-1.046859, -2.977812, 1.349003));  //landmark21
			g_landmark_points.push_back(cv::Point3d(-0.414415, -2.684587, 1.237042));  //landmark22
			g_landmark_points.push_back(cv::Point3d(0.414417, -2.683012, 1.193537));  //landmark23
			g_landmark_points.push_back(cv::Point3d(0.979298, -2.992587, 1.351679));  //landmark24
			g_landmark_points.push_back(cv::Point3d(1.665074, -3.058339, 1.403463));  //landmark25
			g_landmark_points.push_back(cv::Point3d(2.240227, -2.81817, 1.764281));  //landmark26
			g_landmark_points.push_back(cv::Point3d(2.643977, -2.489206, 2.20066));  //landmark27
			g_landmark_points.push_back(cv::Point3d(0, -1.830124, 1.254827));  //landmark28
			g_landmark_points.push_back(cv::Point3d(0, -1.146743, 0.734821));  //landmark29
			g_landmark_points.push_back(cv::Point3d(0, -0.591507, 0.331088));  //landmark30
			g_landmark_points.push_back(cv::Point3d(0, 0, 0));  //landmark31
			g_landmark_points.push_back(cv::Point3d(-0.508, 0.479, 1.063));  //landmark32
			g_landmark_points.push_back(cv::Point3d(-0.305, 0.616909, 0.95));  //landmark33
			g_landmark_points.push_back(cv::Point3d(0, 0.720353, 0.864432));  //landmark34
			g_landmark_points.push_back(cv::Point3d(0.304534, 0.616909, 0.95));  //landmark35
			g_landmark_points.push_back(cv::Point3d(0.50783, 0.47857, 1.063397));  //landmark36
			g_landmark_points.push_back(cv::Point3d(-2.378729, -1.8, 2.455895));  //landmark37
			g_landmark_points.push_back(cv::Point3d(-2.055245, -2.1, 2.058117));  //landmark38
			g_landmark_points.push_back(cv::Point3d(-1.4, -2.1, 1.772257));  //landmark39
			g_landmark_points.push_back(cv::Point3d(-0.932, -1.648, 1.959017));  //landmark40
			g_landmark_points.push_back(cv::Point3d(-1.4, -1.51, 1.962884));  //landmark41
			g_landmark_points.push_back(cv::Point3d(-2.03959, -1.51, 2.071866));  //landmark42
			g_landmark_points.push_back(cv::Point3d(0.932132, -1.647759, 1.985483));  //landmark43
			g_landmark_points.push_back(cv::Point3d(1.4, -2.1, 1.822647));  //landmark44
			g_landmark_points.push_back(cv::Point3d(2.055, -2.1, 1.956067));  //landmark45
			g_landmark_points.push_back(cv::Point3d(2.379, -1.8, 2.442238));  //landmark46
			g_landmark_points.push_back(cv::Point3d(2.04, -1.51, 2.1085));  //landmark47
			g_landmark_points.push_back(cv::Point3d(1.4, -1.51, 1.952295));  //landmark48
			g_landmark_points.push_back(cv::Point3d(-1.364262, 1.778956, 1.849385));  //landmark49
			g_landmark_points.push_back(cv::Point3d(-0.852508, 1.569695, 1.431767));  //landmark50
			g_landmark_points.push_back(cv::Point3d(-0.353622, 1.462288, 1.154293));  //landmark51
			g_landmark_points.push_back(cv::Point3d(0, 1.516837, 1.022306));  //landmark52
			g_landmark_points.push_back(cv::Point3d(0.380074, 1.457382, 1.120382));  //landmark53
			g_landmark_points.push_back(cv::Point3d(0.88371, 1.555559, 1.408546));  //landmark54
			g_landmark_points.push_back(cv::Point3d(1.413398, 1.759402, 1.817869));  //landmark55
			g_landmark_points.push_back(cv::Point3d(0.956854, 2.13336, 1.61844));  //landmark56
			g_landmark_points.push_back(cv::Point3d(0.472234, 2.318487, 1.297367));  //landmark57
			g_landmark_points.push_back(cv::Point3d(0, 2.432608, 1.286434));  //landmark58
			g_landmark_points.push_back(cv::Point3d(-0.470388, 2.379584, 1.408444));  //landmark59
			g_landmark_points.push_back(cv::Point3d(-0.941289, 2.165958, 1.643228));  //landmark60
			g_landmark_points.push_back(cv::Point3d(-0.889933, 1.873988, 1.611149));  //landmark61
			g_landmark_points.push_back(cv::Point3d(-0.414658, 1.778936, 1.410545));  //landmark62
			g_landmark_points.push_back(cv::Point3d(0.019014, 1.830229, 1.150779));  //landmark63
			g_landmark_points.push_back(cv::Point3d(0.454843, 1.831591, 1.401979));  //landmark64
			g_landmark_points.push_back(cv::Point3d(0.866954, 1.863487, 1.647584));  //landmark65
			g_landmark_points.push_back(cv::Point3d(0.448663, 1.952122, 1.38448));  //landmark66
			g_landmark_points.push_back(cv::Point3d(0.008386, 1.984612, 1.157622));  //landmark67
			g_landmark_points.push_back(cv::Point3d(-0.40801, 1.955179, 1.418191));  //landmark68
		}
		return g_landmark_points;
	}

	std::vector<cv::Point3d> GetLandmarkPoints(const std::vector<int>& indices)	{
		GetLandmarkPoints();
		std::vector<cv::Point3d> points;
		for (int i = 0; i < indices.size(); i++) {
			points.push_back(g_landmark_points[indices[i]]);
		}
		return points;
	}

	cv::Point3d					GetLandmarkPoint(int which) {
		GetLandmarkPoints();
		return g_landmark_points[which];
	}



}