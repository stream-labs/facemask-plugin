Because OpenCV is so fun to compile, we have included a win64 build of openCV 3.2 and 3.3.

Which version the facemask plugin links against is configurable through the cmake variable OpenCV_VERSION (either 330 or 320).

NOTE: A possible memory leak was found in both 3.2 and 3.3. 

modules/core/src/ocl.cpp , lines 2518 and 2577. Added: 

		else if (handle)
		{
			clReleaseContext(handle);
			handle = NULL;
		}

In case clCreateContext() fails, but still returns a handle that needs to be released.