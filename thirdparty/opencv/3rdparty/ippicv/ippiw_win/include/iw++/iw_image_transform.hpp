/* 
// Copyright 2016-2017 Intel Corporation All Rights Reserved.
// 
// The source code, information and material ("Material") contained herein is
// owned by Intel Corporation or its suppliers or licensors, and title
// to such Material remains with Intel Corporation or its suppliers or
// licensors. The Material contains proprietary information of Intel
// or its suppliers and licensors. The Material is protected by worldwide
// copyright laws and treaty provisions. No part of the Material may be used,
// copied, reproduced, modified, published, uploaded, posted, transmitted,
// distributed or disclosed in any way without Intel's prior express written
// permission. No license under any patent, copyright or other intellectual
// property rights in the Material is granted to or conferred upon you,
// either expressly, by implication, inducement, estoppel or otherwise.
// Any license under such intellectual property rights must be express and
// approved by Intel in writing.
// 
// Unless otherwise agreed by Intel in writing,
// you may not remove or alter this notice or any other notice embedded in
// Materials by Intel or Intel's suppliers or licensors in any way.
// 
*/

#if !defined( __IPP_IWPP_IMAGE_TRANSFORM__ )
#define __IPP_IWPP_IMAGE_TRANSFORM__

#include "iw/iw_image_transform.h"
#include "iw++/iw_image.hpp"

namespace ipp
{

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiMirror
///////////////////////////////////////////////////////////////////////////// */

// Mirrors image around specified axis.
// For ippAxs45 and ippAxs135 destination image must have flipped size: dstWidth = srcHeight, dstHeight = srcWidth.
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer. In tiling mode
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiMirror(
    const IwiImage *pSrcImage,          // [in]     Pointer to the source image
    IwiImage       *pDstImage,          // [in,out] Pointer to the destination image
    IppiAxis        axis,               // [in]     Mirror axis
    IwiRoi         *pRoi        = NULL  // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiMirror(pSrcImage, pDstImage, axis, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Calculates source ROI by destination one
// Throws:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiMirror_GetSrcRoi(
    IppiAxis        axis,       // [in]  Mirror axis
    IwiSize         imageSize,  // [in]  Size of image in pixels
    IwiRect         dstRoi,     // [in]  Destination image ROI
    IwiRect        &srcRoi      // [out] Source image ROI
)
{
    IppStatus ippStatus = ::iwiMirror_GetSrcRoi(axis, imageSize, dstRoi, &srcRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiRotate
///////////////////////////////////////////////////////////////////////////// */

// Performs rotation of the image around (0,0).
// This is a simplified version of iwiWarpAffine function designed specifically for rotation.
// C API descriptions has more details.
// Throws:
//      ippStsInterpolationErr              interpolation value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
//      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
IW_DECL_CPP(IppStatus) iwiRotate(
    const IwiImage         *pSrcImage,                      // [in]     Pointer to the source image
    IwiImage               *pDstImage,                      // [in,out] Pointer to the destination image
    double                  angle,                          // [in]     Angle of clockwise rotation in degrees
    IppiInterpolationType   interpolation,                  // [in]     Interpolation method: ippNearest, ippLinear, ippCubic
    IwiBorderType           border      = ippBorderTransp,  // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    IwiRoi                 *pRoi        = NULL              // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiRotate(pSrcImage, pDstImage, angle, interpolation, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Calculates destination image size for iwiRotate function.
// This is a more simple version of iwiWarpAffine function designed specifically for rotation.
// Throws:
//      ippStsErr                           size calculation error
// Returns:
//      Destination size
IW_DECL_CPP(IwiSize) iwiRotate_GetDstSize(
    IwiSize         srcSize,    // [in]  Size of the source image in pixels
    double          angle       // [in]  Angle of clockwise rotation in degrees
)
{
    IwiSize size = ::iwiRotate_GetDstSize(srcSize, angle);
    if(!size.width || !size.height)
        OWN_ERROR_THROW_ONLY(ippStsErr);
    return size;
}

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiResize
///////////////////////////////////////////////////////////////////////////// */

// Optional parameters class
class IwiResizeParams: public ::IwiResizeParams
{
public:
    // Default constructor
    IwiResizeParams()
    {
        iwiResize_GetDefaultParams(this);
    }

    // Constructor from C-structure
    IwiResizeParams(::IwiResizeParams params)
    {
        *((::IwiResizeParams*)this) = params;
    }

    // Constructor with all parameters
    IwiResizeParams(
        Ipp32u _antialiasing,    // [in] Use resize with anti-aliasing. Use this to reduce the image size with minimization of moire artifacts
        Ipp32f _cubicBVal,       // [in] The first parameter for cubic filters
        Ipp32f _cubicCVal,       // [in] The second parameter for cubic filters
        Ipp32u _lanczosLobes     // [in] Parameter for Lanczos filters. Possible values are 2 or 3
    )
    {
        iwiResize_GetDefaultParams(this);
        antialiasing = _antialiasing;
        cubicBVal    = _cubicBVal;
        cubicCVal    = _cubicCVal;
        lanczosLobes = _lanczosLobes;
    }

    // Constructor for ippCubic
    IwiResizeParams(
        Ipp32u _antialiasing,    // [in] Use resize with anti-aliasing. Use this to reduce the image size with minimization of moire artifacts
        Ipp32f _cubicBVal,       // [in] The first parameter for cubic filters
        Ipp32f _cubicCVal        // [in] The second parameter for cubic filters
    )
    {
        iwiResize_GetDefaultParams(this);
        antialiasing = _antialiasing;
        cubicBVal    = _cubicBVal;
        cubicCVal    = _cubicCVal;
    }

    // Constructor for ippLanczos
    IwiResizeParams(
        Ipp32u _antialiasing,    // [in] Use resize with anti-aliasing. Use this to reduce the image size with minimization of moire artifacts
        Ipp32u _lanczosLobes     // [in] Parameter for Lanczos filters. Possible values are 2 or 3
    )
    {
        iwiResize_GetDefaultParams(this);
        antialiasing = _antialiasing;
        lanczosLobes = _lanczosLobes;
    }
};

// Simplified version of resize function without spec structure and initialization
// C API descriptions has more details.
// Throws:
//      ippStsInterpolationErr              interpolation value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsBorderErr                     border value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
IW_DECL_CPP(IppStatus) iwiResize(
    const IwiImage         *pSrcImage,                          // [in]     Pointer to the source image
    IwiImage               *pDstImage,                          // [in,out] Pointer to the destination image
    IppiInterpolationType   interpolation,                      // [in]     Interpolation method: ippNearest, ippLinear, ippCubic, ippLanczos, ippSuper
    IwiResizeParams         params      = IwiResizeParams(),    // [in]     Optional parameters
    IwiBorderType           border      = ippBorderRepl,        // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    IwiRoi                 *pRoi        = NULL                  // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiResize(pSrcImage, pDstImage, interpolation, &params, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Resize operation class
// C API descriptions has more details.
class IwiResize
{
public:
    // Default constructor
    IwiResize()
    {
        m_bInitialized = false;
    }

    // Constructor with initialization
    // Throws:
    //      ippStsInterpolationErr              interpolation value is illegal
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    //      ippStsBorderErr                     border value is illegal
    //      ippStsNotSupportedModeErr           selected function mode is not supported
    //      ippStsNoMemErr                      failed to allocate memory
    //      ippStsSizeErr                       size fields values are illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    IwiResize(
        IwiSize                 srcSize,                            // [in] Size of the source image in pixels
        IwiSize                 dstSize,                            // [in] Size of the destination image in pixels
        IppDataType             dataType,                           // [in] Image pixel type
        int                     channels,                           // [in] Number of image channels
        IppiInterpolationType   interpolation,                      // [in] Interpolation method: ippNearest, ippLinear, ippCubic, ippLanczos, ippSuper
        IwiResizeParams         params      = IwiResizeParams(),    // [in] Optional parameters
        IwiBorderType           border      = ippBorderRepl         // [in] Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    )
    {
        m_bInitialized = false;

        IppStatus ippStatus = InitAlloc(srcSize, dstSize, dataType, channels, interpolation, params, border);
        OWN_ERROR_CHECK_THROW_ONLY(ippStatus);
    }

    // Default destructor
    ~IwiResize()
    {
        if(m_bInitialized)
        {
            iwiResize_Free(m_pSpec);
            m_bInitialized = false;
        }
    }

    // Allocates and initializes internal data structure
    // Throws:
    //      ippStsInterpolationErr              interpolation value is illegal
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    //      ippStsBorderErr                     border value is illegal
    //      ippStsNotSupportedModeErr           selected function mode is not supported
    //      ippStsNoMemErr                      failed to allocate memory
    //      ippStsSizeErr                       size fields values are illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    //      ippStsNoOperation                   [warning] width or height of an image is zero
    IppStatus InitAlloc(
        IwiSize                 srcSize,                            // [in] Size of the source image in pixels
        IwiSize                 dstSize,                            // [in] Size of the destination image in pixels
        IppDataType             dataType,                           // [in] Image pixel type
        int                     channels,                           // [in] Number of image channels
        IppiInterpolationType   interpolation,                      // [in] Interpolation method: ippNearest, ippLinear, ippCubic, ippLanczos, ippSuper
        IwiResizeParams         params      = IwiResizeParams(),    // [in] Optional parameters
        IwiBorderType           border      = ippBorderRepl         // [in] Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    )
    {
        if(m_bInitialized)
        {
            iwiResize_Free(m_pSpec);
            m_bInitialized = false;
        }

        IppStatus ippStatus = iwiResize_InitAlloc(&m_pSpec, srcSize, dstSize, dataType, channels, interpolation, &params, border, border);
        OWN_ERROR_CHECK(ippStatus);

        m_bInitialized = true;
        return ippStatus;
    }

    // Performs resize operation on given image ROI
    // Throws:
    //      ippStsBorderErr                     border value is illegal
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    //      ippStsNotEvenStepErr                step value is not divisible by size of elements
    //      ippStsNotSupportedModeErr           selected function mode is not supported
    //      ippStsNoMemErr                      failed to allocate memory
    //      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    //      ippStsNoOperation                   [warning] width or height of an image is zero
    IppStatus operator()(
        const IwiImage *pSrcImage,      // [in]     Pointer to the source image
        IwiImage       *pDstImage,      // [in,out] Pointer to the destination image
        IwiRoi         *pRoi = NULL     // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
    )
    {
        if(m_bInitialized)
        {
            IppStatus ippStatus = iwiResize_Process(m_pSpec, pSrcImage, pDstImage, pRoi);
            OWN_ERROR_CHECK(ippStatus)
            return ippStatus;
        }
        else
            OWN_ERROR_THROW(ippStsBadArgErr);
    }

    // Calculates source ROI by destination one
    // Throws:
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus GetSrcRoi(
        IwiRect  dstRoi,             // [in]  Destination image ROI
        IwiRect &srcRoi              // [out] Source image ROI
    )
    {
        if(m_bInitialized)
        {
            IppStatus ippStatus = iwiResize_GetSrcRoi(m_pSpec, dstRoi, &srcRoi);
            OWN_ERROR_CHECK(ippStatus)
            return ippStatus;
        }
        else
            OWN_ERROR_THROW(ippStsBadArgErr)
    }

    // Get border size for resize
    // Throws:
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    IppStatus GetBorderSize(
        IppiBorderSize &borderSize  // [out] Border size structure
    )
    {
        if(m_bInitialized)
        {
            IppStatus ippStatus = iwiResize_GetBorderSize(m_pSpec, &borderSize);
            OWN_ERROR_CHECK(ippStatus)
            return ippStatus;
        }
        else
            OWN_ERROR_THROW(ippStsBadArgErr)
    }

private:
    IwiResizeSpec *m_pSpec;         // Pointer to internal spec structure
    bool           m_bInitialized;  // Initialization flag
};

/* /////////////////////////////////////////////////////////////////////////////
//                   IwiWarpAffine
///////////////////////////////////////////////////////////////////////////// */

// Optional parameters class
class IwiWarpAffineParams: public ::IwiWarpAffineParams
{
public:
    // Default constructor
    IwiWarpAffineParams()
    {
        iwiWarpAffine_GetDefaultParams(this);
    }

    // Constructor from C-structure
    IwiWarpAffineParams(::IwiWarpAffineParams params)
    {
        *((::IwiWarpAffineParams*)this) = params;
    }

    // Constructor with all parameters
    IwiWarpAffineParams(
        Ipp32f _cubicBVal,   // [in] The first parameter for cubic filters
        Ipp32f _cubicCVal,   // [in] The second parameter for cubic filters
        Ipp32u _smoothEdge   // [in] Edges smooth post-processing. Only for ippBorderTransp, ippBorderInMem
    )
    {
        iwiWarpAffine_GetDefaultParams(this);
        cubicBVal    = _cubicBVal;
        cubicCVal    = _cubicCVal;
        smoothEdge   = _smoothEdge;
    }
};

// Simplified version of warp affine function without spec structure and initialization
// C API descriptions has more details.
// Throws:
//      ippStsWarpDirectionErr              direction value is illegal
//      ippStsCoeffErr                      affine transformation is singular
//      ippStsInterpolationErr              interpolation value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsBorderErr                     border value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
//      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
IW_DECL_CPP(IppStatus) iwiWarpAffine(
    const IwiImage         *pSrcImage,                              // [in]     Pointer to the source image
    IwiImage               *pDstImage,                              // [in,out] Pointer to the destination image
    const double            coeffs[2][3],                           // [in]     Coefficients for the affine transform
    IppiWarpDirection       direction,                              // [in]     Transformation direction
    IppiInterpolationType   interpolation,                          // [in]     Interpolation method: ippNearest, ippLinear, ippCubic
    IwiWarpAffineParams     params      = IwiWarpAffineParams(),    // [in]     Optional parameters
    IwiBorderType           border      = ippBorderTransp,          // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    IwiRoi                 *pRoi        = NULL                      // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiWarpAffine(pSrcImage, pDstImage, coeffs, direction, interpolation, &params, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// WarpAffine operation class
// C API descriptions has more details.
class IwiWarpAffine
{
public:
    // Default constructor
    IwiWarpAffine()
    {
        m_bInitialized = false;
    }

    // Constructor with initialization
    // Throws:
    //      ippStsWarpDirectionErr              direction value is illegal
    //      ippStsCoeffErr                      affine transformation is singular
    //      ippStsInterpolationErr              interpolation value is illegal
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    //      ippStsBorderErr                     border value is illegal
    //      ippStsNotSupportedModeErr           selected function mode is not supported
    //      ippStsNoMemErr                      failed to allocate memory
    //      ippStsSizeErr                       size fields values are illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    IwiWarpAffine(
        IwiSize                 srcSize,                                // [in] Size of the source image in pixels
        IwiSize                 dstSize,                                // [in] Size of the destination image in pixels
        IppDataType             dataType,                               // [in] Image pixel type
        int                     channels,                               // [in] Number of image channels
        const double            coeffs[2][3],                           // [in] Coefficients for the affine transform
        IppiWarpDirection       direction,                              // [in] Transformation direction
        IppiInterpolationType   interpolation,                          // [in] Interpolation method: ippNearest, ippLinear, ippCubic
        IwiWarpAffineParams     params      = IwiWarpAffineParams(),    // [in] Optional parameters
        IwiBorderType           border      = ippBorderTransp           // [in] Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    )
    {
        m_bInitialized = false;

        IppStatus ippStatus = InitAlloc(srcSize, dstSize, dataType, channels, coeffs, direction, interpolation, params, border);
        OWN_ERROR_CHECK_THROW_ONLY(ippStatus);
    }

    // Default destructor
    ~IwiWarpAffine()
    {
        if(m_bInitialized)
        {
            iwiWarpAffine_Free(m_pSpec);
            m_bInitialized = false;
        }
    }

    // Allocates and initializes internal data structure
    // Throws:
    //      ippStsWarpDirectionErr              direction value is illegal
    //      ippStsCoeffErr                      affine transformation is singular
    //      ippStsInterpolationErr              interpolation value is illegal
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    //      ippStsBorderErr                     border value is illegal
    //      ippStsNotSupportedModeErr           selected function mode is not supported
    //      ippStsNoMemErr                      failed to allocate memory
    //      ippStsSizeErr                       size fields values are illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    //      ippStsNoOperation                   [warning] width or height of an image is zero
    //      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
    IppStatus InitAlloc(
        IwiSize                 srcSize,                                // [in] Size of the source image in pixels
        IwiSize                 dstSize,                                // [in] Size of the destination image in pixels
        IppDataType             dataType,                               // [in] Image pixel type
        int                     channels,                               // [in] Number of image channels
        const double            coeffs[2][3],                           // [in] Coefficients for the affine transform
        IppiWarpDirection       direction,                              // [in] Transformation direction
        IppiInterpolationType   interpolation,                          // [in] Interpolation method: ippNearest, ippLinear, ippCubic
        IwiWarpAffineParams     params      = IwiWarpAffineParams(),    // [in] Optional parameters
        IwiBorderType           border      = ippBorderTransp           // [in] Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    )
    {
        if(m_bInitialized)
        {
            iwiWarpAffine_Free(m_pSpec);
            m_bInitialized = false;
        }

        IppStatus ippStatus = iwiWarpAffine_InitAlloc(&m_pSpec, srcSize, dstSize, dataType, channels, coeffs, direction, interpolation, &params, border, border);
        OWN_ERROR_CHECK(ippStatus);

        m_bInitialized = true;
        return ippStatus;
    }

    // Performs warp affine operation on given image ROI
    // Throws:
    //      ippStsInterpolationErr              interpolation value is illegal
    //      ippStsDataTypeErr                   data type is illegal
    //      ippStsNumChannelsErr                channels value is illegal
    //      ippStsBorderErr                     border value is illegal
    //      ippStsNotEvenStepErr                step value is not divisible by size of elements
    //      ippStsNotSupportedModeErr           selected function mode is not supported
    //      ippStsNoMemErr                      failed to allocate memory
    //      ippStsSizeErr                       size fields values are illegal
    //      ippStsNullPtrErr                    unexpected NULL pointer
    // Returns:
    //      ippStsNoErr                         no errors
    //      ippStsNoOperation                   [warning] width or height of an image is zero
    //      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
    IppStatus operator()(
        const IwiImage *pSrcImage,      // [in]     Pointer to the source image
        IwiImage       *pDstImage,      // [in,out] Pointer to the destination image
        IwiRoi         *pRoi = NULL     // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
    )
    {
        if(m_bInitialized)
        {
            IppStatus ippStatus = iwiWarpAffine_Process(m_pSpec, pSrcImage, pDstImage, pRoi);
            OWN_ERROR_CHECK(ippStatus);
            return ippStatus;
        }
        else
            OWN_ERROR_THROW(ippStsBadArgErr);
    }

private:
    IwiWarpAffineSpec  *m_pSpec;        // Pointer to internal spec structure
    bool                m_bInitialized; // Initialization flag
};

};

#endif
