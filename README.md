# obs-facemask-plugin

A libOBS filter plugin that detects faces and draws masks with the detected data.

## Compiling

* Download cmake:

  [cmake](https://cmake.org/download/)

* Get Visual Studio 2015 (vc14). When you install it, make sure you include the C++ stuff. libOBS is fixed at this version, so other versions of Visual Studio *will not work*.

  [microsoft](https://www.visualstudio.com/vs/older-downloads/)

* Download dlib:

  [dlib](https://github.com/davisking/dlib)

* Download our fork of OBS Studio:

  [obs-studio](https://github.com/stream-labs/obs-studio)

* Download freetype:

  [freetype](https://www.freetype.org/download.html)
  
* Build obs-studio
	* Follow the build instructions here:
    
    [build instructions](https://github.com/obsproject/obs-studio/wiki/Install-Instructions#windows-build-directions)
    
* Configure freetype.

	You will need to turn off a bunch of options in freetype, because we don't need them. 
    * Edit **freetype/include/freetype/config/ftmodule.h**, and comment out all the lines, except for the truetype driver and smooth rendering modules:
    
    ```C
    FT_USE_MODULE( FT_Module_Class, autofit_module_class )
    FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, t1_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, cff_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, t1cid_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, pfr_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, t42_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, winfnt_driver_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, pcf_driver_class )
    //FT_USE_MODULE( FT_Module_Class, psaux_module_class )
    //FT_USE_MODULE( FT_Module_Class, psnames_module_class )
    //FT_USE_MODULE( FT_Module_Class, pshinter_module_class )
    //FT_USE_MODULE( FT_Renderer_Class, ft_raster1_renderer_class )
    //FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
    FT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )
    FT_USE_MODULE( FT_Renderer_Class, ft_smooth_lcd_renderer_class )
    FT_USE_MODULE( FT_Renderer_Class, ft_smooth_lcdv_renderer_class )
    //FT_USE_MODULE( FT_Driver_ClassRec, bdf_driver_class )
    ```

* Run cmake in the facemasks folder. When you hit `CONFIGURE`, you will get errors on fields you need to fill in:

    **PATH_DLIB** Path to the Dlib folder

    **PATH_OBS_STUDIO** Path to the obs-studio folder.
    
    **PATH_FREETYPE** Path to the freetype folder.

    You will also want to turn on AVX:

    **USE_AVX_INSTRUCTIONS**

    It will default to SSE2, but setting to SSE4 or AVX is much faster. 

    You'll probably want to set these too:

    **DLIB_NO_GUI_SUPPORT** - don't need it

    **DLIB_USE_CUDA** - turn it off.

    **BUILD_SLOBS** - Distributes to slobs instead of OBS Studio

    **DLIB_GIF_SUPPORT** - don't need it
    **DLIB_JPEG_SUPPORT** - don't need it

    If you have the [Intel Math Kernel Library](https://software.intel.com/en-us/mkl) installed on your system, you might have **DLIB_USE_BLAS** or **DLIB_USE_LAPACK** turned on. Keep in mind that dlib links dynamically with these libs, so the MKL and TBB dlls will need to be found by slobs when it runs (for instance, by copying them into the slobs-client folder). I don't reccommend using these libs for this reason.

* Once you have successfully configured and generated your Visual Studio project with cmake, you can compile the plugin, which will give you a distribution folder structure that mimics the structure in slobs. For example, if you built your files in the build64 folder:

	`build64/distribute/slobs/RelWithDebInfo/`
    
	You can copy the files in manually, or set up symbolic links so you can easily hit F5 and debug from Visual Studio.

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
 
 * **Face Morphing** The 68 landmark points are used to subdivide the video quad into a mesh. Another 11 points are calculated to form the head points, and then catmull rom smoothing is performed to smooth out the contours. Then the mesh is distorted to create face morphs.

The FaceDetect object manages these operations and a current state, so that it performs the face detection, then uses object tracking to follow the face, then does landmark/3d pose estimation, and then the mesh subdivision for face morphing.


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

* [Delaunay Triangulation and Voronoi Diagram using OpenCV](https://www.learnopencv.com/delaunay-triangulation-and-voronoi-diagram-using-opencv-c-python/)

* [Curve Fitting - Catmull-Rom spline](https://gist.github.com/pr0digy/1383576)

* [Silhouette Edge Detection Algorithms for use with 3D Models](http://negativesum.net/Members/hoss/resume/SEDAlgorithms.pdf)

* [FaceWarehouse: a 3D Facial Expression Database for Visual Computing](http://gaps-zju.org/facewarehouse/)

* [3D Face Reconstruction with Geometry Details from a Single Image](https://arxiv.org/pdf/1702.05619.pdf)

* [Fine-Grained Head Pose Estimation Without Keypoints](https://arxiv.org/pdf/1710.00925.pdf)

* [Spatial indexing with Quadtrees and Hilbert Curves](http://blog.notdot.net/2009/11/Damn-Cool-Algorithms-Spatial-indexing-with-Quadtrees-and-Hilbert-Curves)

* [Geometric Algorithms](https://www.cs.princeton.edu/~rs/AlgsDS07/16Geometric.pdf)
