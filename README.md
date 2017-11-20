# obs-facemask-plugin

A libOBS filter plugin that detects faces and draws masks with the detected data.

This plugin relies heavily on Dlib and uses the solvePnP function from openCV.

## Compiling

* Download cmake, if you don't have it already

  [cmake](https://cmake.org/download/)

* Get Visual Studio 2015 (vc14). When you install it, make sure you include the C++ stuff. libOBS is fixed at this version, so make sure.

  [microsoft](https://www.visualstudio.com/vs/older-downloads/)

* Download Dlib

  [dlib](https://github.com/davisking/dlib)

* Download OBS Studio source

  [twitchalerts/obs-studio-fork](https://github.com/twitchalerts/obs-studio_fork)

  If you don't have access, you could also use the original, but you'll be better off with the twitchalerts fork.

    [jp9000/obs-studio](https://github.com/jp9000/obs-studio)

  *Download the plugin code

  [obs-facemask-plugin](https://github.com/twitchalerts/obs-facemask-plugin)

*   Create a build folder in the obs-facemask-plugin folder. For example:

  `obs-facemask-plugin/build64` (for x64)

  *Run `cmake-gui` in the obs-facemask-plugin folder. Set `Where is the source code:` to the obs-facemask-plugin folder, and `Where to build the binaries:` to one of the build foders you just made.

  *Hit the `CONFIGURE` button. Choose `Visual Studio 14 2015 Win64` for 64-bit builds.

  *Fill in these variables as it complains that they are not set, and keep hitting `CONFIGURE` again until it configures without error.

  You will need to set these in order for it to work:

    **PATH_DLIB** Path to the Dlib folder

    **PATH_OBS_STUDIO** Path to the obs-studio[-fork] folder.

    You will also want to consider enabling one of the following optimizations:

    **USE_AVX_INSTRUCTIONS**

    **USE_SSE4_INSTRUCTIONS**

    **USE_SSE2_INSTRUCTIONS**

    It will default to SSE2, but setting to SSE4 or AVX is much faster. 

    You'll probably want to set these too:

    **DLIB_NO_GUI_SUPPORT** - don't need it

    **DLIB_USE_CUDA** - Seems to use more CPU. Turn it off.

    **BUILD_SLOBS** - Distributes to slobs instead of OBS Studio

    **DLIB_GIF_SUPPORT** - don't need it
    **DLIB_JPEG_SUPPORT** - don't need it

    If you have the [Intel Math Kernel Library](https://software.intel.com/en-us/mkl) installed on your system, you might have **DLIB_USE_BLAS** or **DLIB_USE_LAPACK** turned on. Keep in mind that dlib links dynamically with these libs, so the MKL and TBB dlls will need to be found by slobs when it runs (for instance, by copying them into the slobs-client folder). 

* Hit `GENERATE`. 

* At this point, if you haven't already, you should set up your `build64/distribute/` folder correctly. If you are building for slobs, you'll want to make a link to your slobs-client folder, so that when you build the post-build step will copy the plugin stuff into slobs automatically. So, for example (as administrator):

  `mklink /D c:\streamlabs\obs-facemask-plugin\build64\distribute\slobs c:\streamlabs\slobs-client`

* Hit `OPEN PROJECT`

* Select the **RelWithDebInfo** configuration.

* Select the **ALL_BUILD** project in the Solution Explorer, then click on the little wrench icon (*Properties*) to bring up the *Property Pages*. Select the **Debugging** page, and set the following properties to run with the debugger (replace <slobs-client> with your slobs-client folder):

  `Command = <slobs-client>\node-modules\electron\dist\electron.exe`

  `Command Arguments = .`

  `Working Directory = <slobs-client>`

* Compile the plugin. (F7)

* Run it. (F5)

You should now have the face masks filter running in slobs. 

## How It Works

The plugin is an OBS filter plugin. It can be broken down into 2 main parts; the plugin, and the face detection. The face detection runs in its own thread, separate from OBS.

The **plugin** portion performs the following duties:

 * Render the current frame to a texture
 * Make 3 copies of the current frame, and add them to a circular buffer to be consumed by the face detection thread.
 * Receive new face detection data from another circular buffer fed by the face detection thread. 
 * Using smoothing algorithms, the current state of faces is updated.
 * According to the user's parameters, the current faces are rendered.

The **face detection** portion runs in its own thread. It consumes frame data from the circular buffer, does the face detection computation, then feeds the resulting face data to a circular buffer that is consumed by the rendering.

The process of face detection consists of four main operations:

 * **Face Detection** The faces are detected using the histogram of oriented gradients method (HOG) in Dlib. HOG's are trained feature descriptors used for detecting objects.

 * **Tracking** Once we have detected the faces we then use a cheaper method of object tracking to follow the face. This Dlib object tracking method takes an arbitrary rectangle in an image and follows it.

 * **Facial Landmarks** Given a rectangle that locates a face, we can then use Dlib's landmark detection algorithm which uses a trained regression tree solver to find 68 2D facial landmark points, corresponding to the Multi-PIE definition (see links below).

 * **3D Pose Estimation** A subset of key points are taken from the 2D facial landmark points, and using 3D points for an arbitrary rest pose, we use openCV's solvePnP method to obtain a 3D transformation. This transform can be used to render 3D objects in the scene that track the head movement.

The FaceDetect object manages these operations and a current state, so that it performs the face detection, then uses object tracking to follow the face, then does landmark/3d pose estimation. 

The face detection/object tracking are executed according to a frequency setting, so they are not necessarily executed on every frame. The face detection is re-checked according to another frequency setting, to ensure the object tracking is still correct. 

The face detection executes using the detect texture generated in the render thread. The tracking uses the tracking texture, and the landmark/3d pose estimation is done using the high resolution capture texture. If the tracking and detection textures are the same size, they are shared, and only 1 texture is used. Currently, the tracking and detection textures are scaled-down greyscale textures, due to the higher expense of these operations, where as the capture texture is full color and resolution, because the landmarks and pose estimation are inexpensive by comparison.


## Advanced Settings

In addition to the obvious settings in the plugin, the face detection code has a swath of config parameters to control the behaviour. 

Sticking with the default values is likely the best option, but they are exposed for development purposes, and to experiment with the various trade offs with accuracy/speed/smoothness/cpu and gpu consumption.



## Useful Links

 * [Dlib reference](http://dlib.net/)

 * [300 Faces In-the-Wild Challenge](https://ibug.doc.ic.ac.uk/resources/300-W/)

 * [Dlib data files (training data)](http://dlib.net/files/data)

 * [Multi-Pie](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2873597/)

 * [Multi-Pie 68 Point Image](https://ibug.doc.ic.ac.uk/media/uploads/images/annotpics/figure_68_markup.jpg)

 * [Make your own object detector](http://blog.dlib.net/2014/02/dlib-186-released-make-your-own-object.html)

 * [Real-time face pose estimation](http://blog.dlib.net/2014/08/real-time-face-pose-estimation.html)

 * [Speeding up Dlib's Facial Landmark Detector](http://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/)

 * [Detection and Tracking of Facial Features in Real Time...](http://ivizlab.sfu.ca/arya/Papers/IEEE/Proceedings/F%20G%20-%2000/Detecting%20and%20Tracking%20of%20Facial%20Features.pdf)

 * [3D Tracking of Facial Features for Augmented Reality Applications](https://repository.tudelft.nl/islandora/object/uuid:4a107a5a-a5a1-4d06-af4f-5c8da20c709b/datastream/OBJ)

 * [Real-time 3D Rotation Smoothing for Video Stabilization](https://pdfs.semanticscholar.org/8230/9be72bd51cd6912cadf62390e50c77f3b58b.pdf)

 * [Unscented Kalman Filtering for Single Camera Based Motion](http://www.mdpi.com/1424-8220/11/8/7437/pdf)

 * [Motion Tracking with Fixed-lag Smoothing](http://www.ee.ucr.edu/~mourikis/papers/DongSi2011-ICRA.pdf)

 * [An Introduction to the Kalman Filter](http://www.cs.unc.edu/~tracker/media/pdf/SIGGRAPH2001_CoursePack_08.pdf)

 * [Automatic filtering techniques for three-dimensional 
     kinematics data using 3D motion capture system](https://cours.etsmtl.ca/gts504/documents/notes/Aissaoui_IEEE_ISIE2006.pdf)

 * [Real-Time Filters (Dr. Dobb's Article)](http://www.drdobbs.com/real-time-filters/184401931)

 * [A Collection of Useful C++ Classes for Digital Signal Processing](https://github.com/vinniefalco/DSPFilters)

 * [C++ open source library for curve fitting](https://softwarerecs.stackexchange.com/questions/35554/c-open-source-library-for-curve-fitting)

* [A curvature optimal sharp corner
  smoothing algorithm...](http://research.engr.oregonstate.edu/mpcl/resources/Paper_PersonalCopy.pdf)

* [DX11 Texturing and Lighting](https://www.3dgep.com/texturing-lighting-directx-11/)

  ​

