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

// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

	This example program shows how to use dlib's implementation of the paper:
		One Millisecond Face Alignment with an Ensemble of Regression Trees by
		Vahid Kazemi and Josephine Sullivan, CVPR 2014

	In particular, we will train a face landmarking model based on a small dataset
	and then evaluate it.  If you want to visualize the output of the trained
	model on some images then you can run the face_landmark_detection_ex.cpp
	example program with sp.dat as the input model.

	It should also be noted that this kind of model, while often used for face
	landmarking, is quite general and can be used for a variety of shape
	prediction tasks.  But here we demonstrate it only on a simple face
	landmarking task.
*/


#include <dlib/image_processing.h>
#include <dlib/data_io.h>
#include <iostream>

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------

std::vector<std::vector<double> > get_interocular_distances(
	const std::vector<std::vector<full_object_detection> >& objects
);
/*!
	ensures
		- returns an object D such that:
			- D[i][j] == the distance, in pixels, between the eyes for the face represented
			  by objects[i][j].
!*/

// ----------------------------------------------------------------------------------------

template <
	typename image_array_type,
	typename T
>
void add_image_left_right_flips_68points(
	image_array_type& images,
	std::vector<std::vector<T> >& objects
)
{
	// make sure requires clause is not broken
	DLIB_ASSERT(images.size() == objects.size(),
		"\t void add_image_left_right_flips()"
		<< "\n\t Invalid inputs were given to this function."
		<< "\n\t images.size():  " << images.size()
		<< "\n\t objects.size(): " << objects.size()
	);

	typename image_array_type::value_type temp;
	std::vector<T> rects;

	const unsigned long num = images.size();
	for (unsigned long j = 0; j < num; ++j)
	{
		const point_transform_affine tran = flip_image_left_right(images[j], temp);

		rects.clear();
		for (unsigned long i = 0; i < objects[j].size(); ++i)
		{
			rects.push_back(impl::tform_object(tran, objects[j][i]));

			DLIB_CASSERT(rects.back().num_parts() == 68);
			// Custom Swaps
			// Chin
			swap(rects.back().part(0), rects.back().part(16));
			swap(rects.back().part(1), rects.back().part(15));
			swap(rects.back().part(2), rects.back().part(14));
			swap(rects.back().part(3), rects.back().part(13));
			swap(rects.back().part(4), rects.back().part(12));
			swap(rects.back().part(5), rects.back().part(11));
			swap(rects.back().part(6), rects.back().part(10));
			swap(rects.back().part(7), rects.back().part(9));

			// Nose
			swap(rects.back().part(31), rects.back().part(35));
			swap(rects.back().part(32), rects.back().part(34));

			// Eyebrows
			swap(rects.back().part(17), rects.back().part(26));
			swap(rects.back().part(18), rects.back().part(25));
			swap(rects.back().part(19), rects.back().part(24));
			swap(rects.back().part(20), rects.back().part(23));
			swap(rects.back().part(21), rects.back().part(22));

			// Eyes
			swap(rects.back().part(36), rects.back().part(45));
			swap(rects.back().part(37), rects.back().part(44));
			swap(rects.back().part(38), rects.back().part(43));
			swap(rects.back().part(39), rects.back().part(42));
			swap(rects.back().part(40), rects.back().part(47));
			swap(rects.back().part(41), rects.back().part(46));

			// Mouth
			swap(rects.back().part(48), rects.back().part(54));
			swap(rects.back().part(49), rects.back().part(53));
			swap(rects.back().part(50), rects.back().part(52));
			swap(rects.back().part(61), rects.back().part(63));
			swap(rects.back().part(60), rects.back().part(64));
			swap(rects.back().part(67), rects.back().part(65));
			swap(rects.back().part(59), rects.back().part(55));
			swap(rects.back().part(58), rects.back().part(56));
		}

		images.push_back(temp);
		objects.push_back(rects);
	}
}


int main(int argc, char** argv)
{
	try
	{
		// Path to DLIB's facial landmarks directory
		const std::string faces_directory = "C:/Users/srira/streamLabs/facemask-plugin/apps/FaceLandmarksTrain/data/landmarks_dataset_v_0_1";

		// Initialize images and faces data holders
		dlib::array<array2d<unsigned char> > images_train, images_test;
		std::vector<std::vector<full_object_detection> > faces_train, faces_test;


		cout << "Loading Train data..." << endl;
		load_image_dataset(images_train, faces_train, faces_directory + "/labels_dataset_v_0_1.xml");
		/*cout << "Loading Test data..." << endl;
		load_image_dataset(images_test, faces_test, faces_directory + "/labels_dataset_v_0_1_test.xml");*/

		// Now make the object responsible for training the model.  
		cout << "Intialize Trainer..." << endl;
		shape_predictor_trainer trainer;
		trainer.set_cascade_depth(15);
		//trainer.set_nu(0.05);
		//trainer.set_oversampling_amount(100);
		//trainer.set_num_test_splits(100);
		trainer.set_num_threads(4);

		// Tell the trainer to print status messages to the console so we can
		// see how long the training will take.
		trainer.be_verbose();

		// Now finally generate the shape model
		cout << "Training Shape predictor..." << endl;
		shape_predictor sp = trainer.train(images_train, faces_train);


		// Now that we have a model we can test it.  This function measures the
		// average distance between a face landmark output by the
		// shape_predictor and where it should be according to the truth data.
		// Note that there is an optional 4th argument that lets us rescale the
		// distances.  Here we are causing the output to scale each face's
		// distances by the interocular distance, as is customary when
		// evaluating face landmarking systems.
		cout << "Mean Training Error: " <<
			test_shape_predictor(sp, images_train, faces_train, get_interocular_distances(faces_train)) << endl;

		// Delete train data
		images_train.clear();
		faces_train.clear();

		/*std::cout << "Loading Test data..." << std::endl;
		dlib::load_image_dataset(images_test, faces_test, faces_directory + "/labels_dataset_v_0_1_test.xml");

		cout << "Mean Testing Error: " <<
			test_shape_predictor(sp, images_test, faces_test, get_interocular_distances(faces_test)) << endl;*/

			// The real test is to see how well it does on data it wasn't trained
			// on.  We trained it on a very small dataset so the accuracy is not
			// extremely high, but it's still doing quite good.  Moreover, if you
			// train it on one of the large face landmarking datasets you will
			// obtain state-of-the-art results, as shown in the Kazemi paper.
			/*cout << "mean testing error:  " <<
				test_shape_predictor(sp, images_test, faces_test, get_interocular_distances(faces_test)) << endl;
	*/
	// Finally, we save the model to disk so we can use it later.
		serialize("sp_v0.1_custom.dat") << sp; // Mean Training Error: 0.00928328
	}
	catch (exception& e)
	{
		cout << "\nexception thrown!" << endl;
		cout << e.what() << endl;
	}
}

// ----------------------------------------------------------------------------------------

double interocular_distance(
	const full_object_detection& det
)
{
	dlib::vector<double, 2> l, r;
	double cnt = 0;
	// Find the center of the left eye by averaging the points around 
	// the eye.
	for (unsigned long i = 36; i <= 41; ++i)
	{
		l += det.part(i);
		++cnt;
	}
	l /= cnt;

	// Find the center of the right eye by averaging the points around 
	// the eye.
	cnt = 0;
	for (unsigned long i = 42; i <= 47; ++i)
	{
		r += det.part(i);
		++cnt;
	}
	r /= cnt;

	// Now return the distance between the centers of the eyes
	return length(l - r);
}

std::vector<std::vector<double> > get_interocular_distances(
	const std::vector<std::vector<full_object_detection> >& objects
)
{
	std::vector<std::vector<double> > temp(objects.size());
	for (unsigned long i = 0; i < objects.size(); ++i)
	{
		for (unsigned long j = 0; j < objects[i].size(); ++j)
		{
			temp[i].push_back(interocular_distance(objects[i][j]));
		}
	}
	return temp;
}
