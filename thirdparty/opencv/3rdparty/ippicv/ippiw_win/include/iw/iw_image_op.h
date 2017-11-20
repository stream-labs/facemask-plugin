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

#if !defined( __IPP_IW_IMAGE_OP__ )
#define __IPP_IW_IMAGE_OP__

#include "iw/iw_image.h"

#ifdef __cplusplus
extern "C" {
#endif

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiCopy family
///////////////////////////////////////////////////////////////////////////// */

// Copies image data to destination image with the same format
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopy(
    const IwiImage *pSrcImage,  // [in]     Pointer to the source image
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Copies image data to destination image with different number of channels.
// If number of channels is the same, then calls iwiCopy function.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopyMixed(
    const IwiImage *pSrcImage,      // [in]     Pointer to the source image
    IwiImage       *pDstImage,      // [in,out] Pointer to the destination image
    int             channelsShift,  // [in]     Number of channels to shift to source or destination. E.g.: C4+channelsShift->C3 or C3->C4+channelsShift
    IwiRoi         *pRoi            // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Copies image data to destination image with masking.
// If mask is NULL, then calls iwiCopy function.
// For masked operation, the function writes pixel values in the destination buffer only if the spatially corresponding
// mask array value is non-zero.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopyMask(
    const IwiImage *pSrcImage,  // [in]     Pointer to the source image
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    const IwiImage *pMaskImage, // [in]     Pointer to mask image. Mask must be 8-bit, 1 channel image
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Copies selected channel from one multi-channel image to another.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopyChannel(
    const IwiImage *pSrcImage,  // [in]     Pointer to the source image
    int             srcChannel, // [in]     Source channel to copy from
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    int             dstChannel, // [in]     Destination channel to copy to
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Splits multi-channel image into array of single channel images.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopySplit(
    const IwiImage  *pSrcImage,   // [in]     Pointer to the source image
    IwiImage* const  pDstImage[], // [in,out] Pointer to the destination image
    IwiRoi          *pRoi         // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Merges array of single channel images into one multi-channel image.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopyMerge(
    const IwiImage* const pSrcImage[],  // [in]     Array of pointers to source images
    IwiImage             *pDstImage,    // [in,out] Pointer to the destination image
    IwiRoi               *pRoi          // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Copies image data to destination image and constructs border of specified size
// Destination image must have enough memory for a border according to inMemSize member
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsNotSupportedModeErr           selected function mode is not supported
//      ippStsSizeErr                       1) size fields values are illegal
//                                          2) border.borderTop or border.borderLeft are greater than corresponding inMemSize values of destination image
//                                          3) dst_width  + dst_inMemSize.borderRight  < min_width  + border.borderRight
//                                          4) dst_height + dst_inMemSize.borderBottom < min_height + border.borderBottom
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiCopyMakeBorder(
    const IwiImage *pSrcImage,      // [in]     Pointer to the source image
    IwiImage       *pDstImage,      // [in,out] Pointer to the destination image which points to actual data start
    IppiBorderSize  borderSize,     // [in]     Size of border to reconstruct
    IppiBorderType  border,         // [in]     Extrapolation algorithm for out of image pixels: ippBorderConst, ippBorderRepl, ippBorderMirror, ippBorderWrap
    const Ipp64f   *pBorderVal,     // [in]     Pointer to array of border values for ippBorderConst. One element for each channel. Can be NULL for any other border
    IwiRoi         *pRoi            // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiSet family
///////////////////////////////////////////////////////////////////////////// */

// Sets each channel of the image to the specified values.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiSet(
    const double   *pValue,     // [in]     Pointer to array of values. One element for each channel
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Sets all channels of the image to the specified value.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiSetUniform(
    double          value,      // [in]     Value to set to
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Sets each channel of the image to the specified values with masking.
// If mask is NULL, then calls iwiSet function.
// For masked operation, the function writes pixel values in the destination buffer only if the spatially corresponding
// mask array value is non-zero.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiSetMask(
    const double   *pValue,     // [in]     Pointer to array of values. One element for each channel
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    const IwiImage *pMaskImage, // [in]     Pointer to mask image. Mask must be 8-bit, 1 channel image
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Sets all channels of the image to the specified value with masking.
// If mask is NULL, then calls iwiSetUniform function.
// For masked operation, the function writes pixel values in the destination buffer only if the spatially corresponding
// mask array value is non-zero.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiSetUniformMask(
    double          value,      // [in]     Value to set to
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    const IwiImage *pMaskImage, // [in]     Pointer to mask image
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Sets selected channel of the multi-channel image to the specified value.
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiSetChannel(
    double          value,      // [in]     Value to set to
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    int             channelNum, // [in]     Number of channel to be set
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiSwapChannels
///////////////////////////////////////////////////////////////////////////// */

// Swaps image channels according to the destination order parameter.
// One source channel can be mapped to several destination channels.
// If number of destination channels is bigger than number of source channels then:
// 1) if(dstOrder[channel] == srcChannels) dst[channel] = constValue
// 2) if(dstOrder[channel] > srcChannels) dst[channel] is unchanged
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsChannelOrderErr               destination order is out of the range
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNumChannelsErr                channels value is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiSwapChannels(
    const IwiImage *pSrcImage,  // [in]     Pointer to the source image
    IwiImage       *pDstImage,  // [in,out] Pointer to the destination image
    const int      *pDstOrder,  // [in]     Pointer to the destination image channels order: dst[channel] = src[dstOrder[channel]]
    double          value,      // [in]     Value to fill the destination channel if number of destination channels is bigger than number of source channels
    IwiRoi         *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

/**/////////////////////////////////////////////////////////////////////////////
//                   iwiScale
///////////////////////////////////////////////////////////////////////////// */

// Converts image from one data type to another with specified scaling and shifting
// DST = saturate(SRC*mulVal + addVal)
// Features support:
//      Internal threading:     no
//      Manual tiling:          yes
//      IwiRoi simple tiling:   yes
//      IwiRoi pipeline tiling: yes
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsSizeErr                       size fields values are illegal
//      ippStsInplaceModeNotSupportedErr    doesn't support output into the source buffer. If data types are different
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiScale(
    const IwiImage     *pSrcImage,  // [in]     Pointer to the source image
    IwiImage           *pDstImage,  // [in,out] Pointer to the destination image
    Ipp64f              mulVal,     // [in]     Multiplier
    Ipp64f              addVal,     // [in]     Addend
    IppHintAlgorithm    mode,       // [in]     Accuracy mode
    IwiRoi             *pRoi        // [in,out] Pointer to the IwiRoi structure for tiling. If NULL - the whole image will be processed in accordance to size
);

// Returns multiplication and addend values for iwiScale function to perform accurate data range scaling between two data types
// Data range for float values is considered to be from 0 to 1
// Returns:
//      ippStsDataTypeErr                   data type is illegal
//      ippStsNullPtrErr                    unexpected NULL pointer
//      ippStsNoErr                         no errors
IW_DECL(IppStatus) iwiScale_GetScaleVals(
    IppDataType srcType,    // [in]     Source data type
    IppDataType dstType,    // [in]     Destination data type
    Ipp64f     *pMulVal,    // [out]    Pointer to multiplier
    Ipp64f     *pAddVal     // [out]    Pointer to addend
);

#ifdef __cplusplus
}
#endif

#endif
