#include "test-detection-tracking.hpp"


std::vector<int> generateTraceFromTimestamps(std::vector<uint64_t> timestamps, int originalFrameRate) {
	std::vector<int> trace;
	std::vector<int> frameIndices;

	for (int i = 0; i < timestamps.size(); i++) {
		int frameIndex = round(timestamps[i] / (CLOCKS_PER_SEC/originalFrameRate));
		frameIndices.push_back(frameIndex);
	}

	return frameIndices;
}

std::vector<uint64_t> readTimestampsFromFile(std::string timestampFile) {
	std::vector<uint64_t> timestamps;

	std::ifstream trace(timestampFile.c_str());
	std::string line;
	while (std::getline(trace, line))
	{
		uint64_t timestamp;
		std::istringstream iss(line);

		if (!(iss >> timestamp)) {
			break;
		}

		timestamps.push_back(timestamp);
	}
	return timestamps;
}

bool evaluateDetection(dlib::rectangle rect1, dlib::rectangle rect2, float thresholdWidth, float thresholdHeight, float &iou) {
	float intersectionWidth = std::min(rect1.right(), rect2.right()) - std::max(rect1.left(), rect2.left());
	float unionWidth = std::max(rect1.right(), rect2.right()) - std::min(rect1.left(), rect2.left());

	float intersectionHeight = std::min(rect1.bottom(), rect2.bottom()) - std::max(rect1.top(), rect2.top());
	float unionHeight = std::max(rect1.bottom(), rect2.bottom()) - std::min(rect1.top(), rect2.top());

	if (intersectionWidth / unionWidth >= thresholdWidth && intersectionHeight / unionHeight >= thresholdHeight) {
		return true;
	}

	return false;
}

bool compareToMultipleRectangles(std::vector<dlib::rectangle> rectangles, dlib::rectangle rectangle, float thresholdWidth, float thresholdHeight, float &iou) {
	
	for (int i = 0; i < rectangles.size(); i++) {
		if (evaluateDetection(rectangle, rectangles[i], thresholdWidth, thresholdHeight, iou)) {
			return true;
		}
	}
	return false;
}

std::vector<std::vector<dlib::rectangle>> SimulateDetectionAndTracking(dlib::array<dlib::array2d<unsigned char> > &images, std::string format) {
	std::vector<std::vector<dlib::rectangle>> simulationResult;

	int resizeWidth = smll::Config::singleton().get_int(smll::CONFIG_INT_FACE_DETECT_WIDTH);
	int resizeHeight = 0;

	smll::FaceDetector smllFaceDetector;
	smll::DetectionResults detect_results;

	for (int i = 0; i < images.size(); i++) {
		cv::Mat frame = toMat(images[i]);
		resizeHeight = (int)((float)resizeWidth * (float)frame.rows / (float)frame.cols);

		cv::Mat grayImage;
		if(strcmp("bgra", format.c_str()) == 0){
			cv::cvtColor(frame, grayImage, cv::COLOR_BGRA2GRAY);
		}

		smllFaceDetector.DetectFaces(grayImage, resizeWidth, resizeHeight, detect_results);

		std::vector<dlib::rectangle> detected_rectangles;
		for (int i = 0; i < detect_results.length; i++) {
			detected_rectangles.push_back(detect_results[i].bounds);
		}

		simulationResult.push_back(detected_rectangles);
	}

	return simulationResult;
}

std::vector<std::vector<dlib::rectangle>> loadImagesAndGroundTruth(std::string imageFolder, std::vector<int> trace, dlib::array<dlib::array2d<unsigned char> > &images, std::vector<std::vector<dlib::rectangle> > &ignore) {
	std::vector<std::vector<dlib::rectangle>> allFaceLocations, faceLocations;
	dlib::array<dlib::array2d<unsigned char> > allImages;
	if (trace.size() == 0) {
		ignore = dlib::load_image_dataset(images, faceLocations, imageFolder);
	}
	else {
		ignore = dlib::load_image_dataset(allImages, allFaceLocations, imageFolder);
	}
	for (int i = 0; i < trace.size(); i++) {
		int ind = std::min(int(trace.at(i)), int(allImages.size()));
		images.push_back(allImages[ind]);
		faceLocations.push_back(allFaceLocations.at(ind));
	}

	return faceLocations;
}

float GetAccurateFacesPercentage(std::vector<std::vector<dlib::rectangle>> simulationResult, std::vector<std::vector<dlib::rectangle>> groundTruth) {

	float iou;

	int totalFaces = 0;
	int foundFaces = 0;

	float thresholdWidth = 0.5;
	float thresholdHeight = 0.5;

	for (int i = 0; i < groundTruth.size(); i++) {
		std::vector<dlib::rectangle> frameGroundTruth = groundTruth.at(i);
		std::vector<dlib::rectangle> frameSimulationResult = simulationResult.at(i);
		totalFaces += frameGroundTruth.size();
		for (int j = 0; j < frameGroundTruth.size(); j++) {
			bool found = compareToMultipleRectangles(frameSimulationResult, frameGroundTruth.at(j), thresholdWidth, thresholdHeight, iou);
			if (found) {
				foundFaces++;
			}
		}
	}

	return float(foundFaces) / totalFaces;
}

int main(int argc, char *argv[]) {
	std::vector<std::vector<dlib::rectangle> > ignore, groundTruth;
	dlib::array<dlib::array2d<unsigned char> > images;
	std::vector<uint64_t> timestamps;
	std::vector<int> trace;

	std::string timestampFile = argv[1];
	std::string imageFolder = argv[2];
	int originalFrameRate = std::stoi(argv[3]);
	std::string format = argv[4]; 

	timestamps = readTimestampsFromFile(timestampFile);
	trace = generateTraceFromTimestamps(timestamps, originalFrameRate);
	groundTruth = loadImagesAndGroundTruth(imageFolder, trace, images, ignore);
	std::vector<std::vector<dlib::rectangle>> simulationResult = SimulateDetectionAndTracking(images, format);

	float p = GetAccurateFacesPercentage(simulationResult, groundTruth);
}
