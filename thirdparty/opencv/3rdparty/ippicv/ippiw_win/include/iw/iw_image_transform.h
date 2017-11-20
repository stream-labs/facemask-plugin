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

#if !defined( __IPP_IW_IMAGE_TRANSFORM__ )
#define __IPP_IW_IMAGE_TRANSFORM__

#include "iw/iw_image.h"

#ifdef __cplusplus
extern "C" {
#endif

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiMirror
///////////////////////////////////////////////////////////////////////////// */

// Mirrors image around specified axis.
// For ippAxs45 and ippAxs135 destination image must have flipped size: dstWidth = srcHeight, dstHeight = srcWidth
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: no
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer. In tiling mode
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiMirror(
    const IwiImage *pSrcImage,  // [in]     Pointer to the source image
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    IppiAxis        axis,       // [in]     Mirror axis
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Calculates source ROI by destination one
// Returns:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiMirror_GetSrcRoi(
    IppiAxis        axis,       // [in]  Mirror axis
    IppiSizeL       imageSize,  // [in]  Size of image in pixels
    IppiRectL       dstRoi,     // [in]  Destination image ROI
    IppiRectL      *pSrcRoi     // [out] Pointer to source image ROI
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiRotate
///////////////////////////////////////////////////////////////////////////// */

// Performs rotation of the image around (0,0).
// This is a more simple version of iwiWarpAffine function designed specifically for rotation.
// Features support:
//      Internal threading:     no
//      Manual tiling:          no
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: no
// Returns:
//      ippStsInterpolationErr              interpolation value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
//      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
IW_DECL(IppStatus) iwiRotate(
    const IwiImage         *pSrcImage,      // [in]     Pointer to the source image
    IwiImage               *pDstImage,      // [in,out] Pointer to the destination image
    double                  angle,          // [in]     Angle of clockwise rotation in degrees
    IppiInterpolationType   interpolation,  // [in]     Interpolation method: ippNearest, ippLinear, ippCubic
    IppiBorderType          border,         // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    const Ipp64f           *pBorderVal,     // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi                 *pRoi            // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Calculates destination image size for iwiRotate function.
// This is a more simple version of iwiWarpAffine function designed specifically for rotation.
// Returns:
//      Destination size or 0x0 in case of error
IW_DECL(IppiSizeL) iwiRotate_GetDstSize(
    IppiSizeL       srcSize,    // [in]  Size of the source image in pixels
    double          angle       // [in]  Angle of clockwise rotation in degrees
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiResize
///////////////////////////////////////////////////////////////////////////// */

// Internal structure for data sharing between resize functions
typedef struct _IwiResizeSpec IwiResizeSpec;

// Optional parameters structure
typedef struct _IwiResizeParams
{
    Ipp32f cubicBVal;       // The first parameter for cubic filters
    Ipp32f cubicCVal;       // The second parameter for cubic filters
    Ipp32u lanczosLobes;    // Parameter for Lanczos filters. Possible values are 2 or 3
    Ipp32u antialiasing;    // Use resize with anti-aliasing. Use this to reduce the image size with minimization of moire artifacts
} IwiResizeParams;

// Sets optional parameters to default values
IW_DECL(void) iwiResize_GetDefaultParams(
    IwiResizeParams *pParams    // [out] Pointer to optional parameters structure
);

// Simplified version of resize function without spec structure and initialization
// Features support:
//      Internal threading:     yes (check IW_ENABLE_THREADING_LAYER definition)
//      Manual tiling:          no
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: no
// Returns:
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
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
IW_DECL(IppStatus) iwiResize(
    const IwiImage         *pSrcImage,      // [in]     Pointer to the source image
    IwiImage               *pDstImage,      // [in,out] Pointer to the destination image
    IppiInterpolationType   interpolation,  // [in]     Interpolation method: ippNearest, ippLinear, ippCubic, ippLanczos, ippSuper
    IwiResizeParams        *pParams,        // [in]     Pointer to optional parameters structure. If NULL - default parameters will be used
    IppiBorderType          border,         // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f           *pBorderVal,     // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi                 *pRoi            // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Free internal data structure
IW_DECL(void) iwiResize_Free(
    IwiResizeSpec *pSpec        // [in]  Pointer to internal spec structure
);

// Allocates and initializes internal data structure
// Returns:
//      ippStsInterpolationErr              interpolation value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
IW_DECL(IppStatus) iwiResize_InitAlloc(
    IwiResizeSpec         **pSpec,          // [out] Pointer to pointer to internal spec structure. Structure will be allocated here.
    IppiSizeL               srcSize,        // [in]  Size of the source image in pixels
    IppiSizeL               dstSize,        // [in]  Size of the destination image in pixels
    IppDataType             dataType,       // [in]  Image pixel type
    int                     channels,       // [in]  Number of image channels
    IppiInterpolationType   interpolation,  // [in]  Interpolation method: ippNearest, ippLinear, ippCubic, ippLanczos, ippSuper
    IwiResizeParams        *pParams,        // [in]  Pointer to optional parameters structure. If NULL - default parameters will be used
    IppiBorderType          border,         // [in]  Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f           *pBorderVal      // [in]  Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
);

// Performs resize operation on given image ROI
// Features support:
//      Internal threading:     yes (check IW_ENABLE_THREADING_LAYER definition)
//      Manual tiling:          yes (if iwiResize_InitAlloc was initialized for a full image, not a tile)
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsBorderErr                     border value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
IW_DECL(IppStatus) iwiResize_Process(
    const IwiResizeSpec    *pSpec,      // [in]     Pointer to internal spec structure
    const IwiImage         *pSrcImage,  // [in]     Pointer to the source image
    IwiImage               *pDstImage,  // [in,out] Pointer to the destination image
    IwiRoi                 *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Calculates source ROI by destination one
// Returns:
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiResize_GetSrcRoi(
    const IwiResizeSpec *pSpec,         // [in]  Pointer to internal spec structure
    IppiRectL            dstRoi,        // [in]  Destination image ROI
    IppiRectL           *pSrcRoi        // [out] Pointer to source image ROI
);

// Get border size for resize
// Returns:
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiResize_GetBorderSize(
    const IwiResizeSpec *pSpec,         // [in]  Pointer to internal spec structure
    IppiBorderSize      *pBorderSize    // [out] Pointer to border size structure
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiWarpAffine
///////////////////////////////////////////////////////////////////////////// */

// Internal structure for data sharing between warp affine functions
typedef struct _IwiWarpAffineSpec IwiWarpAffineSpec;

// Optional parameters structure
typedef struct _IwiWarpAffineParams
{
    Ipp32f cubicBVal;       // The first parameter for cubic filters
    Ipp32f cubicCVal;       // The second parameter for cubic filters
    Ipp32u smoothEdge;      // Edges smooth post-processing. Only for ippBorderTransp, ippBorderInMem
} IwiWarpAffineParams;

// Sets optional parameters to default values
IW_DECL(void) iwiWarpAffine_GetDefaultParams(
    IwiWarpAffineParams *pParams    // [out] Pointer to optional parameters structure
);

// Simplified version of warp affine function without spec structure and initialization
// Features support:
//      Internal threading:     no
//      Manual tiling:          no
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: no
// Returns:
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
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
//      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
IW_DECL(IppStatus) iwiWarpAffine(
    const IwiImage         *pSrcImage,      // [in]     Pointer to the source image
    IwiImage               *pDstImage,      // [in,out] Pointer to the destination image
    const double            coeffs[2][3],   // [in]     Coefficients for the affine transform
    IppiWarpDirection       direction,      // [in]     Transformation direction
    IppiInterpolationType   interpolation,  // [in]     Interpolation method: ippNearest, ippLinear, ippCubic
    IwiWarpAffineParams    *pParams,        // [in]     Pointer to optional parameters structure. If NULL - default parameters will be used
    IppiBorderType          border,         // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    const Ipp64f           *pBorderVal,     // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi                 *pRoi            // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Releases internal data structure
IW_DECL(void) iwiWarpAffine_Free(
    IwiWarpAffineSpec *pSpec        // [in]  Pointer to internal spec structure
);

// Allocates and initializes internal data structure
// Returns:
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
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
//      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
IW_DECL(IppStatus) iwiWarpAffine_InitAlloc(
    IwiWarpAffineSpec     **pSpec,          // [out] Pointer to pointer to internal spec structure. Structure will be allocated here.
    IppiSizeL               srcSize,        // [in]  Size of the source image in pixels
    IppiSizeL               dstSize,        // [in]  Size of the destination image in pixels
    IppDataType             dataType,       // [in]  Image pixel type
    int                     channels,       // [in]  Number of image channels
    const double            coeffs[2][3],   // [in]  Coefficients for the affine transform
    IppiWarpDirection       direction,      // [in]  Transformation direction
    IppiInterpolationType   interpolation,  // [in]  Interpolation method: ippNearest, ippLinear, ippCubic
    IwiWarpAffineParams    *pParams,        // [in]  Pointer to optional parameters structure. If NULL - default parameters will be used
    IppiBorderType          border,         // [in]  Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderTransp, ippBorderInMem
    const Ipp64f           *pBorderVal      // [in]  Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
);

// Performs warp affine transform of the given image ROI
// Features support:
//      Internal threading:     no
//      Manual tiling:          no
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: no
// Returns:
//      ippStsInterpolationErr              interpolation value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsBorderErr                     border value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
//      ippStsNoOperation                   [warning] width or height of an image is zero
//      ippStsWrongIntersectQuad            [warning] transformed source image has no intersection with the destination image
IW_DECL(IppStatus) iwiWarpAffine_Process(
    const IwiWarpAffineSpec    *pSpec,      // [in]     Pointer to internal spec structure
    const IwiImage             *pSrcImage,  // [in]     Pointer to the source image
    IwiImage                   *pDstImage,  // [in,out] Pointer to the destination image
    IwiRoi                     *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);


#ifdef __cplusplus
}
#endif

#endif
