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

#if !defined( __IPP_IWPP_IMAGE_FILTER__ )
#define __IPP_IWPP_IMAGE_FILTER__

#include "iw/iw_image_filter.h"
#include "iw++/iw_image.hpp"

namespace ipp
{

/* /////////////////////////////////////////////////////////////////////////////
//                   iwiFilterSobel
///////////////////////////////////////////////////////////////////////////// */

// Applies Sobel filter of specific type to the source image
// C API descriptions has more details.
// Throws:
//      ippStsMaskSizeErr                   mask value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterSobel(
    const IwiImage     *pSrcImage,                      // [in]     Pointer to the source image
    IwiImage           *pDstImage,                      // [in,out] Pointer to the destination image
    IwiDerivativeType   opType,                         // [in]     Type of derivative from IwiDerivativeType
    IppiMaskSize        kernelSize = ippMskSize3x3,     // [in]     Size of filter kernel: ippMskSize3x3, ippMskSize5x5
    IwiBorderType       border     = ippBorderRepl,     // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    IwiRoi             *pRoi       = NULL               // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiFilterSobel(pSrcImage, pDstImage, opType, kernelSize, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/* /////////////////////////////////////////////////////////////////////////////
//                   iwiFilterScharr
///////////////////////////////////////////////////////////////////////////// */

// Applies Scharr filter of specific type to the source image
// C API descriptions has more details.
// Throws:
//      ippStsMaskSizeErr                   mask value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterScharr(
    const IwiImage     *pSrcImage,                  // [in]     Pointer to the source image
    IwiImage           *pDstImage,                  // [in,out] Pointer to the destination image
    IwiDerivativeType   opType,                     // [in]     Type of derivative from IwiDerivativeType
    IppiMaskSize        kernelSize = ippMskSize3x3, // [in]     Size of filter kernel: ippMskSize3x3, ippMskSize5x5
    IwiBorderType       border     = ippBorderRepl, // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    IwiRoi             *pRoi       = NULL           // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiFilterScharr(pSrcImage, pDstImage, opType, kernelSize, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterLaplacian
///////////////////////////////////////////////////////////////////////////// */

// Applies Laplacian filter to the source image
// C API descriptions has more details.
// Throws:
//      ippStsMaskSizeErr                   mask value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterLaplacian(
    const IwiImage     *pSrcImage,                  // [in]     Pointer to the source image
    IwiImage           *pDstImage,                  // [in,out] Pointer to the destination image
    IppiMaskSize        kernelSize = ippMskSize3x3, // [in]     Size of filter kernel: ippMskSize3x3, ippMskSize5x5
    IwiBorderType       border     = ippBorderRepl, // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    IwiRoi             *pRoi       = NULL           // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiFilterLaplacian(pSrcImage, pDstImage, kernelSize, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}


/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterGaussian
///////////////////////////////////////////////////////////////////////////// */

// Applies Gaussian filter to the source image
// C API descriptions has more details.
// Throws:
//      ippStsMaskSizeErr                   mask value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsBorderErr                     border value is illegal
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterGaussian(
    const IwiImage     *pSrcImage,                  // [in]     Pointer to the source image
    IwiImage           *pDstImage,                  // [in,out] Pointer to the destination image
    int                 kernelSize,                 // [in]     Size of the Gaussian kernel (odd, greater or equal to 3)
    double              sigma,                      // [in]     Standard deviation of the Gaussian kernel
    IwiBorderType       border     = ippBorderRepl, // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    IwiRoi             *pRoi       = NULL           // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiFilterGaussian(pSrcImage, pDstImage, kernelSize, sigma, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/* /////////////////////////////////////////////////////////////////////////////
//                   iwiFilterCanny
///////////////////////////////////////////////////////////////////////////// */

// Applies Canny edge detector to the source image
// C API descriptions has more details.
// Throws:
//      ippStsMaskSizeErr                   mask value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsBorderErr                     border value is illegal
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterCanny(
    const IwiImage         *pSrcImage,                      // [in]     Pointer to the source image
    IwiImage               *pDstImage,                      // [in,out] Pointer to the destination image
    IppiDifferentialKernel  kernel      = ippFilterSobel,   // [in]     Type of differential kernel: ippFilterSobel, ippFilterScharr
    IppiMaskSize            kernelSize  = ippMskSize3x3,    // [in]     Size of filter kernel: ippFilterSobel: ippMskSize3x3, ippMskSize5x5; ippFilterScharr: ippMskSize3x3
    IppNormType             norm        = ippNormL2,        // [in]     Normalization mode: ippNormL1, ippNormL2
    Ipp32f                  treshLow    = 50,               // [in]     Lower threshold for edges detection
    Ipp32f                  treshHigh   = 150,              // [in]     Upper threshold for edges detection
    IwiBorderType           border      = ippBorderRepl     // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
)
{
    IppStatus ippStatus = ::iwiFilterCanny(pSrcImage, pDstImage, kernel, kernelSize, norm, treshLow, treshHigh, border, border);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Applies Canny edge detector to the image derivatives
// C API descriptions has more details.
// Throws:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterCannyDeriv(
    const IwiImage         *pSrcImageDx,                    // [in]     Pointer to X derivative of the source image
    const IwiImage         *pSrcImageDy,                    // [in]     Pointer to Y derivative of the source image
    IwiImage               *pDstImage,                      // [in,out] Pointer to the destination image
    IppNormType             norm        = ippNormL2,        // [in]     Normalization mode: ippNormL1, ippNormL2
    Ipp32f                  treshLow    = 50,               // [in]     Lower threshold for edges detection
    Ipp32f                  treshHigh   = 150               // [in]     Upper threshold for edges detection
)
{
    IppStatus ippStatus = ::iwiFilterCannyDeriv(pSrcImageDx, pSrcImageDy, pDstImage, norm, treshLow, treshHigh);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterMorphology
///////////////////////////////////////////////////////////////////////////// */

// Performs morphology filter operation on given image ROI
// C API descriptions has more details.
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
IW_DECL_CPP(IppStatus) iwiFilterMorphology(
    const IwiImage     *pSrcImage,                  // [in]     Pointer to the source image
    IwiImage           *pDstImage,                  // [in,out] Pointer to the destination image
    IwiMorphologyType   morphType,                  // [in]     Morphology filter type
    const IwiImage     *pMaskImage,                 // [in]     Pointer to morphology mask image
    IwiPoint           *pAnchor    = NULL,          // [in]     Anchor point inside kernel. NULL to use default Intel IPP anchor. Only NULL is supported currently
    IwiBorderType       border     = ippBorderRepl, // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem, ippBorderCascadeInMem
    IwiRoi             *pRoi       = NULL           // [in,out] Pointer to IwiRoi class for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiFilterMorphology(pSrcImage, pDstImage, morphType, pMaskImage, pAnchor, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

// Calculates border size for morphology operation
// Throws:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterMorphology_GetBorderSize(
    IwiMorphologyType   morphType,      // [in]  Morphology filter type
    IwiSize             maskSize,       // [in]  Size of morphology mask
    IppiBorderSize     &borderSize      // [out] Border size structure
)
{
    IppStatus ippStatus = ::iwiFilterMorphology_GetBorderSize(morphType, maskSize, &borderSize);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterBilateral
///////////////////////////////////////////////////////////////////////////// */

// Performs bilateral filtering of an image
// C API descriptions has more details.
// Throws:
//      ippStsSizeErr                       size fields values are illegal
//      ippStsBadArgErr                     valSquareSigma or posSquareSigma is less or equal 0
//      ippStsMaskSizeErr                   radius is less or equal 0
//      ippStsBorderErr                     border value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNotSupportedModeErr           filter or distMethod is not supported
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
// Returns:
//      ippStsNoErr                         no errors
IW_DECL_CPP(IppStatus) iwiFilterBilateral(
    const IwiImage         *pSrcImage,                                  // [in]     Pointer to the source image
    IwiImage               *pDstImage,                                  // [in,out] Pointer to the destination image
    int                     radius,                                     // [in]     Radius of circular neighborhood what defines pixels for calculation
    Ipp32f                  valSquareSigma,                             // [in]     Square of Sigma for factor function for pixel intensity
    Ipp32f                  posSquareSigma,                             // [in]     Square of Sigma for factor function for pixel position
    IppiFilterBilateralType filter          = ippiFilterBilateralGauss, // [in]     Type of bilateral filter: ippiFilterBilateralGauss
    IppiDistanceMethodType  distMethod      = ippDistNormL1,            // [in]     Method for definition of distance between pixel intensity: ippDistNormL1
    IwiBorderType           border          = ippBorderRepl,            // [in]     Extrapolation algorithm and value for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    IwiRoi                 *pRoi            = NULL                      // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
)
{
    IppStatus ippStatus = ::iwiFilterBilateral(pSrcImage, pDstImage, radius, valSquareSigma, posSquareSigma, filter, distMethod, border, border, pRoi);
    OWN_ERROR_CHECK(ippStatus)
    return ippStatus;
}

};

#endif
