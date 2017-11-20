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

#if !defined( __IPP_IW_IMAGE_FILTER__ )
#define __IPP_IW_IMAGE_FILTER__

#include "iw/iw_image.h"

#ifdef __cplusplus
extern "C" {
#endif

// Derivative operator type enumerator
typedef enum _IwiDerivativeType
{
    iwiDerivHorFirst = 0,  // Horizontal first derivative
    iwiDerivHorSecond,     // Horizontal second derivative
    iwiDerivVerFirst,      // Vertical first derivative
    iwiDerivVerSecond,     // Vertical second derivative
    iwiDerivNVerFirst      // Negative vertical first derivative
} IwiDerivativeType;

// Morphology operator type enumerator
typedef enum _IwiMorphologyType
{
    iwiMorphErode  = 0, // Erode morphology operation
    iwiMorphDilate,     // Dilate morphology operation
    iwiMorphOpen,       // Open morphology operation
    iwiMorphClose,      // Close morphology operation
    iwiMorphTophat,     // Tophat morphology operation
    iwiMorphBlackhat,   // Blackhat morphology operation
    iwiMorphGradient    // Gradient morphology operation
} IwiMorphologyType;

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterSobel
///////////////////////////////////////////////////////////////////////////// */
//
//                               -1  0  1
//              SobelVert (3x3)  -2  0  2
//                               -1  0  1
//
//
//                                1  2  1
//              SobelHoriz (3x3)  0  0  0
//                               -1 -2 -1
//
//
//                                       1 -2  1
//              SobelVertSecond (3x3)    2 -4  2
//                                       1 -2  1
//
//
//                                       1  2  1
//              SobelHorizSecond (3x3)  -2 -4 -2
//                                       1  2  1
//
//                               -1  -2   0   2   1
//                               -4  -8   0   8   4
//              SobelVert (5x5)  -6 -12   0  12   6
//                               -4  -8   0   8   4
//                               -1  -2   0   2   1
//
//                                1   4   6   4   1
//                                2   8  12   8   2
//              SobelHoriz (5x5)  0   0   0   0   0
//                               -2  -8 -12  -8  -4
//                               -1  -4  -6  -4  -1
//
//                                       1   0  -2   0   1
//                                       4   0  -8   0   4
//              SobelVertSecond (5x5)    6   0 -12   0   6
//                                       4   0  -8   0   4
//                                       1   0  -2   0   1
//
//                                       1   4   6   4   1
//                                       0   0   0   0   0
//              SobelHorizSecond (5x5)  -2  -8 -12  -8  -2
//                                       0   0   0   0   0
//                                       1   4   6   4   1

// Applies Sobel filter of specific type to the source image
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
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
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterSobel(
    const IwiImage     *pSrcImage,  // [in]     Pointer to the source image
    IwiImage           *pDstImage,  // [in,out] Pointer to the destination image
    IwiDerivativeType   opType,     // [in]     Type of derivative from IwiDerivativeType
    IppiMaskSize        kernelSize, // [in]     Size of filter kernel: ippMskSize3x3, ippMskSize5x5
    IppiBorderType      border,     // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f       *pBorderVal, // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi             *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterScharr
///////////////////////////////////////////////////////////////////////////// */
//
//                                3  0  -3
//              ScharrVert       10  0 -10
//                                3  0  -3
//
//
//                                3  10  3
//              ScharrHoriz       0   0  0
//                               -3 -10 -3

// Applies Scharr filter of specific type to the source image
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
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
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterScharr(
    const IwiImage     *pSrcImage,  // [in]     Pointer to the source image
    IwiImage           *pDstImage,  // [in,out] Pointer to the destination image
    IwiDerivativeType   opType,     // [in]     Type of derivative from IwiDerivativeType
    IppiMaskSize        kernelSize, // [in]     Size of filter kernel: ippMskSize3x3, ippMskSize5x5
    IppiBorderType      border,     // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f       *pBorderVal, // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi             *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterLaplacian
///////////////////////////////////////////////////////////////////////////// */
//
//                                  2  0  2
//              Laplacian (3x3)     0 -8  0
//                                  2  0  2
//
//                                2   4   4   4   2
//                                4   0  -8   0   4
//              Laplacian (5x5)   4  -8 -24  -8   4
//                                4   0  -8   0   4
//                                2   4   4   4   2

// Applies Laplacian filter to the source image
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
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
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterLaplacian(
    const IwiImage     *pSrcImage,  // [in]     Pointer to the source image
    IwiImage           *pDstImage,  // [in,out] Pointer to the destination image
    IppiMaskSize        kernelSize, // [in]     Size of filter kernel: ippMskSize3x3, ippMskSize5x5
    IppiBorderType      border,     // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f       *pBorderVal, // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi             *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/* /////////////////////////////////////////////////////////////////////////////
//                   iwiFilterGaussian
///////////////////////////////////////////////////////////////////////////// */

// Applies Gaussian filter to the source image
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
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
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterGaussian(
    const IwiImage     *pSrcImage,  // [in]     Pointer to the source image
    IwiImage           *pDstImage,  // [in,out] Pointer to the destination image
    int                 kernelSize, // [in]     Size of the Gaussian kernel (odd, greater or equal to 3)
    double              sigma,      // [in]     Standard deviation of the Gaussian kernel
    IppiBorderType      border,     // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f       *pBorderVal, // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi             *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterCanny
///////////////////////////////////////////////////////////////////////////// */

// Applies Canny edge detector to the source image
// Features support:
//      Internal threading:     no
//      Manual tiling:          no
//      IwiRoi simple tiling:   no
//      IwiRoi pipeline tiling: no
// Returns:
//      ippStsMaskSizeErr                   mask value is illegal
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsBorderErr                     border value is illegal
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterCanny(
    const IwiImage         *pSrcImage,  // [in]     Pointer to the source image
    IwiImage               *pDstImage,  // [in,out] Pointer to the destination image
    IppiDifferentialKernel  kernel,     // [in]     Type of differential kernel: ippFilterSobel, ippFilterScharr
    IppiMaskSize            kernelSize, // [in]     Size of filter kernel: ippFilterSobel: ippMskSize3x3, ippMskSize5x5; ippFilterScharr: ippMskSize3x3
    IppNormType             norm,       // [in]     Normalization mode: ippNormL1, ippNormL2
    Ipp32f                  treshLow,   // [in]     Lower threshold for edges detection
    Ipp32f                  treshHigh,  // [in]     Upper threshold for edges detection
    IppiBorderType          border,     // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f           *pBorderVal  // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
);

// Applies Canny edge detector to the image derivatives
// Features support:
//      Internal threading:     no
//      Manual tiling:          no
//      IwiRoi simple tiling:   no
//      IwiRoi pipeline tiling: no
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsNotEvenStepErr                step value is not divisible by size of elements
//      ippStsNoMemErr                      failed to allocate memory
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterCannyDeriv(
    const IwiImage         *pSrcImageDx,    // [in]     Pointer to X derivative of the source image
    const IwiImage         *pSrcImageDy,    // [in]     Pointer to Y derivative of the source image
    IwiImage               *pDstImage,      // [in,out] Pointer to the destination image
    IppNormType             norm,           // [in]     Normalization mode: ippNormL1, ippNormL2
    Ipp32f                  treshLow,       // [in]     Lower threshold for edges detection
    Ipp32f                  treshHigh       // [in]     Upper threshold for edges detection
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterMorphology
///////////////////////////////////////////////////////////////////////////// */

// Performs morphology filter operation
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
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
IW_DECL(IppStatus) iwiFilterMorphology(
    const IwiImage      *pSrcImage,     // [in]     Pointer to the source image
    IwiImage            *pDstImage,     // [in,out] Pointer to the destination image
    IwiMorphologyType    morphType,     // [in]     Morphology filter type
    const IwiImage      *pMaskImage,    // [in]     Pointer to morphology mask image. Mask must be continuous, 8-bit, 1 channel image
    IppiPointL          *pAnchor,       // [in]     Anchor point inside kernel. Set NULL to use default Intel IPP anchor. Only NULL is supported currently
    IppiBorderType       border,        // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem, ippBorderCascadeInMem
    const Ipp64f        *pBorderVal,    // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi              *pRoi           // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Calculates border size for morphology operation
// Returns:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterMorphology_GetBorderSize(
    IwiMorphologyType   morphType,      // [in]  Morphology filter type
    IppiSizeL           maskSize,       // [in]  Size of morphology mask
    IppiBorderSize     *pBorderSize     // [out] Pointer to border size structure
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiFilterBilateral
///////////////////////////////////////////////////////////////////////////// */

// Performs bilateral filtering of an image
// Features support:
//      Internal threading:     yes (check IW_ENABLE_THREADING_LAYER definition)
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
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
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiFilterBilateral(
    const IwiImage         *pSrcImage,      // [in]     Pointer to the source image
    IwiImage               *pDstImage,      // [in,out] Pointer to the destination image
    int                     radius,         // [in]     Radius of circular neighborhood what defines pixels for calculation
    Ipp32f                  valSquareSigma, // [in]     Square of Sigma for factor function for pixel intensity
    Ipp32f                  posSquareSigma, // [in]     Square of Sigma for factor function for pixel position
    IppiFilterBilateralType filter,         // [in]     Type of bilateral filter: ippiFilterBilateralGauss
    IppiDistanceMethodType  distMethod,     // [in]     Method for definition of distance between pixel intensity: ippDistNormL1
    IppiBorderType          border,         // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderInMem
    const Ipp64f           *pBorderVal,     // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi                 *pRoi            // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

#ifdef __cplusplus
}
#endif

#endif
